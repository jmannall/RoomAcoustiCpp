/*
* @type Real, TimerPair
* 
* @brief Defines Real and TimerPair data type
*
*/

#ifndef Common_Types_h
#define Common_Types_h

// C++ headers
#include <cmath>
#include <string>

namespace RAC
{
	namespace Common
	{

		//////////////////// Data Types ////////////////////

#define DATA_TYPE_DOUBLE true

#if DATA_TYPE_DOUBLE
		typedef double Real; /**< Real type */
#else
		typedef float Real; /**< Real type */
#endif

		struct TimerPair
		{
			size_t id;
			time_t time;
			TimerPair(const size_t _id, const time_t _time) : id(_id), time(_time) {};
		};
	}
}

#endif