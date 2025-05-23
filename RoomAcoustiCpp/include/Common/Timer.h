/*
* @class Timer
*
* @brief Create a thread with a callback function
*
*/

#ifndef RoomAcoustiCpp_Timer_h
#define RoomAcoustiCpp_Timer_h

// C++ headers
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>

/**
* @brief Class that implements a thread with a callback function
*/
class Timer
{
public:
    /**
	* @brief Default constructor
    */
    Timer() : running(false) {}

    /**
	* @brief Default destructor that stops the timer
    */
    virtual ~Timer() { StopTimer(); }

    /**
	* @brief Starts the timer with a given interval
    * 
	* @param intervalMs The interval for the callback function in milliseconds
    */
    void StartTimer(int intervalMs)
    {
        if (running.load())
            StopTimer();
        running.store(true);
        timerThread = std::thread([this, intervalMs]() {
            while (running.load())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
                TimerCallback();
            }
        });
    }

    /**
	* @brief Stops the timer
    */
    void StopTimer()
    {
        running.store(false);
        if (timerThread.joinable())
            timerThread.join();
    }

protected:
    /**
	* @brief Callback function that is called every interval
    */
    virtual void TimerCallback() = 0;

private:
	std::thread timerThread;        // Thread running the timer
	std::atomic<bool> running;      // Flag to check if the timer is running
};

#endif // RoomAcoustiCpp_Timer_h
