#include <SFML/System/Vector2.hpp>

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "thread_pool.hpp"

ThreadPool::ThreadPool(size_t thread_count) : stop(false) {
    for (size_t i = 0; i < thread_count; ++i) {
        workers.emplace_back([this] {
            while (true) {
                F task;

                // fetch task
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    cv.wait(lock, [this]{ return stop || !tasks.empty(); });

                    if (stop && tasks.empty()) return; // exit thread
                    task = std::move(tasks.front());
                    tasks.pop();
                }

                // execute task
                task();
            }
        });
    }
}

void ThreadPool::enqueue(F task) {
    {
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }
    cv.notify_one();
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    cv.notify_all();

    for (std::thread &worker : workers) {
        worker.join();
    }
}