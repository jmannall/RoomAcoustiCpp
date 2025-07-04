/*
*  @brief Globals defined for the RoomAcoustiCpp Spatialiser
*
*/

#ifndef RoomAcoustiCpp_Globals_h
#define RoomAcoustiCpp_Globals_h

// C++ headers
#include <mutex>
#include <shared_mutex>
#include <memory>

#include "DSP/AudioThreadPool.h"

namespace RAC
{
	namespace DSP
	{
		extern std::shared_mutex tuneInMutex;						// Global 3DTI Mutex
		extern std::unique_ptr<AudioThreadPool> audioThreadPool;	// Global Audio Thread Pool
	}
}

#endif