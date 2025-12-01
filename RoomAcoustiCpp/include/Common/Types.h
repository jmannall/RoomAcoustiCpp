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

#ifdef __AVX__
#	define USE_AVX		1
#else
#	define USE_AVX		0
#endif

#if USE_AVX
#include <intrin.h>
#include <smmintrin.h>
#endif

namespace RAC
{
	namespace Common
	{

		/**
		* Contols Matrix library
		*/
#define EIGEN_FLAG 0
#define CUSTOM_FLAG 1
		
#if defined(USE_EIGEN) && defined(USE_CUSTOM)
#error You cannot use both Eigen and Custom at the same time.
#elif defined(USE_EIGEN)
#define MATRIX_LIBRARY EIGEN_FLAG
#elif defined(USE_CUSTOM)
#define MATRIX_LIBRARY CUSTOM_FLAG
#else
#error Please specify a matrix library via USE_EIGEN or USE_CUSTOM
#endif

#define EIGEN_PLAINOBJECTBASE_PLUGIN "Eigen/PlainObjectBaseAddons.h"
#define EIGEN_ARRAY_PLUGIN "Eigen/ArrayAddons.h"
#define EIGEN_ARRAYBASE_PLUGIN "Eigen/ArrayBaseAddons.h"
#define EIGEN_MATRIX_PLUGIN "Eigen/MatrixAddons.h"
#define EIGEN_MATRIXBASE_PLUGIN "Eigen/MatrixBaseAddons.h"
#define EIGEN_DENSEBASE_PLUGIN "Eigen/DenseBaseAddons.h"
#define EIGEN_QUATERNIONBASE_PLUGIN "Eigen/QuaternionBaseAddons.h"

		/**
		* Contols Real typedef
		*/
#define DATA_TYPE_DOUBLE true

#if DATA_TYPE_DOUBLE
		typedef double Real; // Define Real as double

		// Defines a floating point "Real" constant (e.g., a double)
#define REAL_CONST(n)		n
#else
		typedef float Real; // Define Real as float

		// Defines a floating point "Real" constant (e.g., a float)
#define REAL_CONST(n)		n##f
#endif

		/** @brief Shorthands for quiet NaN. */
		constexpr Real qNaN = std::numeric_limits<Real>::quiet_NaN();

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