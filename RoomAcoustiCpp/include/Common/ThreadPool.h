/**
* @class ThreadPool
*
* @brief Declaration of ThreadPool class
*/

#ifndef RoomAcoustiCpp_ThreadPool_h
#define RoomAcoustiCpp_ThreadPool_h

// C++ headers
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

// Unity headers
#include "Unity/UnityInterface.h"

namespace RAC
{
    namespace Common
    {
        /**
        * @brief Class that implements a thread pool for processing void functions
        */
        class ThreadPool
        {
        public:
            /**
            * @brief Constructor that initializes the thread pool with a given number of threads
            *
            * @param numThreads The number of threads to create in the pool
            */
            explicit ThreadPool(size_t numThreads) : stop(false)
            {
                for (size_t i = 0; i < numThreads; ++i)
                {
                    workers.emplace_back([this, i] {
                        int i = RegisterThread();
                        while (true)
                        {
                            std::function<void()> task;
                            {
                                std::unique_lock<std::mutex> lock(queueMutex);
                                condition.wait(lock, [this] { return !tasks.empty() || stop; });
                                if (stop.load(std::memory_order_relaxed) && tasks.empty())
                                    break;
                                task = std::move(tasks.front());
                                tasks.pop();
                            }
                            task();
                        }
                        UnregisterThread(i);
                        });
                }
            }

            /**
            * @brief Destructor that stops all threads in the thread pool
            */
            ~ThreadPool()
            {
                stop.store(true, std::memory_order_relaxed);
                condition.notify_all();
                for (std::thread& worker : workers) {
                    worker.join();
                }
            }

            /**
            * @brief Adds a task to be executed by the thread pool
            *
            * @param task The task to be executed
            */
            inline void Enqueue(std::function<void()> task) {
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    tasks.push(std::move(task));
                }
                condition.notify_one();
            }

        private:
            std::vector<std::thread> workers;           // Worker threads in the pool
            std::queue<std::function<void()>> tasks;    // Task queue for storing tasks to be executed
            std::mutex queueMutex;                      // Mutex for synchronizing access to the task queue 
            std::condition_variable condition;          // Condition variable for notifying worker threads when tasks are available
            std::atomic<bool> stop;                     // Flag to indicate whether the thread pool is stopping
        };
    }
}

#endif