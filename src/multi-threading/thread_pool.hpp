#include <SFML/System/Vector2.hpp>

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>


class ThreadPool {
public:
    ThreadPool(size_t thread_count);

    void enqueue(std::function<void()> task);

    void wait_until_idle();

    ~ThreadPool();

private:
    std::atomic<size_t> active_workers{0};

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable cv;
    bool stop;
};