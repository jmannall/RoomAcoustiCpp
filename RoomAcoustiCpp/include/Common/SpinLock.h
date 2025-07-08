/**
* @class SpinLock
* 
* @brief Declaration of SpinLock class
*
* @details From: https://timur.audio/using-locks-in-real-time-audio-processing-safely
*/

#ifndef RoomAcoustiCpp_SpinLock_h
#define RoomAcoustiCpp_SpinLock_h

// C++ headers
#include <array>
#include <thread>
#include <atomic>
#include <emmintrin.h>

/**
* @brief Class that implements a spin lock
* 
* @details Lock function from: https://timur.audio/using-locks-in-real-time-audio-processing-safely
*/
class SpinLock
{ 
public:
    /**
	* @brief Constructor that initializes the spin lock with the number of tasks to be processed
    */
    SpinLock(size_t startCounter) : counter(startCounter) {}

    /**
    * @brief Lock the current thread until number of tasks is less than 1.
    */
    inline void Lock() noexcept
    {
        // This seems to run better than the suggestion by timur below when testing in Unity
        // Most impact when the source was moving. It would also drastically reduce the Unity profiler performance and framerate
        // The busy waiting may have been getting in the way of the many other Unity threads?
		while (!TryUnlock())
            std::this_thread::yield();
        return;

        //// approx. 5x5 ns (= 25 ns), 10x40 ns (= 400 ns), and 3000x350 ns 
        //// (~ 1 ms), respectively, when measured on a 2.9 GHz Intel i9
        //constexpr std::array iterations = { 5, 10, 3000 };

        //for (int i = 0; i < iterations[0]; ++i) {
        //    if (TryUnlock())
        //        return;
        //}

        //for (int i = 0; i < iterations[1]; ++i) {
        //    if (TryUnlock())
        //        return;

        //    _mm_pause();
        //}

        //while (true) {
        //    for (int i = 0; i < iterations[2]; ++i) {
        //        if (TryUnlock())
        //            return;

        //        _mm_pause();
        //        _mm_pause();
        //        _mm_pause();
        //        _mm_pause();
        //        _mm_pause();
        //        _mm_pause();
        //        _mm_pause();
        //        _mm_pause();
        //        _mm_pause();
        //        _mm_pause();
        //    }

        //    // waiting longer than we should, let's give other threads 
        //    // a chance to recover
        //    std::this_thread::yield();
        //}
    }

    /**
	* @brief Try to unlock the current thread
    */
    inline bool TryUnlock() noexcept { return counter.load(std::memory_order_acquire) < 1; }

    /**
    * @brief Add one to the counter
    */
	inline void Add() noexcept { counter.fetch_add(1, std::memory_order_release); }

    /**
	* @brief Subtract one from the counter
    */
    inline void Subtract() noexcept { counter.fetch_sub(1, std::memory_order_release); }

private:
	std::atomic<int> counter; // Number of tasks remaining to be processed
};

#endif