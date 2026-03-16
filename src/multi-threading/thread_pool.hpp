#include <SFML/System/Vector2.hpp>

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

using F = std::function<void()>;

class ThreadPool {
public:
    ThreadPool(size_t thread_count);

    void enqueue(F task);

    ~ThreadPool();

private:
    std::vector<std::thread> workers;
    std::queue<F> tasks;

    std::mutex queue_mutex;
    std::condition_variable cv;
    bool stop;
};