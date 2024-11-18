/*
*  @brief Global mutexes for the RoomAcoustiCpp Spatialiser
*
*/

#ifndef RoomAcoustiCpp_Mutexes_h
#define RoomAcoustiCpp_Mutexes_h

// C++ headers
#include <mutex>

namespace RAC
{
	namespace Spatialiser
	{
		extern std::mutex tuneInMutex;		// Global 3DTI Mutex
	}
}

#endif