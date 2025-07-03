/*
*  @brief Globals defined for the RoomAcoustiCpp Spatialiser
*
*/

#ifndef RoomAcoustiCpp_Globals_h
#define RoomAcoustiCpp_Globals_h

// C++ headers
#include <mutex>
#include <shared_mutex>

// Common headers
#include "Common/ThreadPool.h"

namespace RAC
{
	namespace Spatialiser
	{
		static_assert(std::atomic<bool>::is_always_lock_free, "Bool type must be lock-free for atomic operations");
		static_assert(!std::atomic<std::shared_ptr<Real>>::is_always_lock_free, "Shared ptr type is now lock free");
		static_assert(std::atomic<Real>::is_always_lock_free, "Real type must be lock-free for atomic operations");
		const constexpr size_t MAX_IMAGESOURCES = 1024;		// Maximum number of image sources
		const constexpr size_t MAX_SOURCES = 128;			// Maximum number of sources

		extern std::shared_mutex tuneInMutex;					// Global 3DTI Mutex
		extern std::unique_ptr<ThreadPool> audioThreadPool;		// Global Audio Thread Pool
	}
}

#endif