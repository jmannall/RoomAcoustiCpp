/*
*  @brief Globals defined for the RoomAcoustiCpp Spatialiser
*
*/

#ifndef RoomAcoustiCpp_Globals_h
#define RoomAcoustiCpp_Globals_h

// C++ headers
#include <mutex>

// Common headers
#include "Common/ThreadPool.h"

namespace RAC
{
	namespace Spatialiser
	{
		extern std::mutex tuneInMutex;							// Global 3DTI Mutex
		extern std::unique_ptr<ThreadPool> audioThreadPool;		// Global Audio Thread Pool
	}
}

#endif