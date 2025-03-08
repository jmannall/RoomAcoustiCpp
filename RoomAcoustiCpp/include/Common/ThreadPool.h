
#ifndef RoomAcoustiCpp_ThreadPool_h
#define RoomAcoustiCpp_ThreadPool_h

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "Unity/UnityInterface.h"

// Thread pool implementation
class ThreadPool
{
public:
	explicit ThreadPool(size_t numThreads) : stop(false)
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back([this, i] {
                int i = RegisterAudioThread();
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return !tasks.empty() || stop; });
                        if (stop && tasks.empty())
                            break;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
                UnregisterAudioThread(i);
                });
        }
    }

    inline void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

#endif