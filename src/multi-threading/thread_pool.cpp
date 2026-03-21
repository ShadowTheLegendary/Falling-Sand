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
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    cv.wait(lock, [this] { return stop || !tasks.empty(); });

                    if (stop && tasks.empty())
                        return;

                    task = std::move(tasks.front());
                    tasks.pop();

                    active_workers++;
                }

                task();

                active_workers--;
                cv.notify_all();
            }
        });
    }
}


void ThreadPool::enqueue(std::function<void()> task) {
    {
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }
    cv.notify_one();
}


void ThreadPool::wait_until_idle() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv.wait(lock, [this] {
        return tasks.empty() && active_workers.load() == 0;
    });
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