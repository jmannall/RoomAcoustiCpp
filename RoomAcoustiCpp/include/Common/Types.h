/*
* @type Real
* @struct TimePair
* 
* @brief Defines Real type and TimerPair struct
*
*/

#ifndef Common_Types_h
#define Common_Types_h

// C++ headers
#include <cmath>
#include <string>
#include <ctime>

namespace RAC
{
	namespace Common
	{

		/**
		* Contols Real typedef
		*/
#define DATA_TYPE_DOUBLE true 

#if DATA_TYPE_DOUBLE
		typedef double Real; // Define Real as double
#else
		typedef float Real; // Define Real as float
#endif

		/**
		* @brief Struct that stores an id and time
		*/
		struct TimerPair
		{
			/**
			* @brief Constructor that initialises the TimerPair with the given id and time
			* 
			* @param _id The id
			* @param _time The time
			*/
			TimerPair(const size_t _id, const time_t _time) : id(_id), time(_time) {};

			/**
			* @brief Constructor that initialises the TimerPair with the given id and the current time
			*
			* @param _id The id
			*/
			TimerPair(const size_t _id) : id(_id), time(::time(nullptr)) {};

			size_t id;		// Stored ID
			time_t time;	// Stored time
		};
	}
}

#endif