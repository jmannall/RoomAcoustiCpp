/*
*
*  \Global mutexes for the RoomAcoustiCpp Spatialiser
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
		// Mutexes
		extern std::mutex tuneInMutex;
	}
}

#endif