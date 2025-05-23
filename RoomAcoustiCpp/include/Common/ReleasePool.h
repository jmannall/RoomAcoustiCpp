/*
* @class ReleasePool
* 
* @brief Garbage collector for shared pointers
*
* @remark Code from CppCon 2015: Timur Doumler “C++ in the Audio Industry”
* 
*/

#ifndef RoomAcoustiCpp_ReleasePool_h
#define RoomAcoustiCpp_ReleasePool_h

// C++ headers
#include <vector>
#include <mutex>

// Common headers
#include "Common/Timer.h"

/**
* * @brief Class that implements a garbage collector for shared pointers called every second
*/
class ReleasePool : private Timer
{
public:
	/**
	* @brief Constructor that initialises the ReleasePool and starts a callback timer at one second intervals
	*/
    ReleasePool() { StartTimer(1000); }

	/**
	* @brief Default destructor that stops the timer
	*/
	~ReleasePool() { StopTimer(); }

	/**
	* @brief Adds a shared pointer to the pool
	* 
	* @param object The shared pointer to add to the pool
	*/
    template<typename T>
	void Add(const std::shared_ptr<T>& object)
    {
		std::lock_guard<std::mutex> lock(m);
		pool.emplace_back(object);
    }

private:
	/**
	* @brief Callback function that is called every second and erases any shared pointers that are no longer referenced
	*/
	void TimerCallback() override
	{
		std::lock_guard<std::mutex> lock(m);
		pool.erase(
			std::remove_if(pool.begin(), pool.end(),
				[] (auto& object) { return object.use_count() <= 1; }),
			pool.end());
	}

	std::vector<std::shared_ptr<void>> pool;	// Pool of shared pointers
	std::mutex m;								// Mutex that controls access to pool
};

#endif // RoomAcoustiCpp_ReleasePool_h