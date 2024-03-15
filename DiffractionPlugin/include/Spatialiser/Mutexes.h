/*
*
*  \Global mutexes for the spatialiser
*
*/

#ifndef Spatialiser_Mutexes_h
#define Spatialiser_Mutexes_h

// C++ headers
#include <mutex>

namespace UIE
{
	namespace Spatialiser
	{
		// Mutexes
		extern std::mutex tuneInMutex;
	}
}

#endif