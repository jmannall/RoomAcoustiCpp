/*
* @brief Defines constants (speed of sound, PI, SQRT_2, LOG_10) and some simple mathematical functions
*
*/

#ifndef Common_Definitions_h
#define Common_Definitions_h

// C++ headers
#include <vector>

// Common headers
#include "Common/Types.h"

namespace RAC
{
	namespace Common
	{

#if DATA_TYPE_DOUBLE	// Double

		const constexpr Real T_CELCIUS = 20.0;							// Temperature in degrees celcius
		const constexpr Real SPEED_OF_SOUND = 331.5 + 0.6 * T_CELCIUS;	// Speed of sound in air
		const constexpr Real INV_SPEED_OF_SOUND = 1.0 / SPEED_OF_SOUND;	// Inverse speed of sound in air

		const constexpr Real ROUND_FACTOR = 1e3;			// Factor for rounding operations
		const constexpr Real EPS = 0.000001;				// Tolerance for some floating point comparisons
		const constexpr Real MIN_VALUE = 10.0 * DBL_MIN;	// Minimum value for trimming BTM FIR filter

		//////////////////// Mathematical Constants ////////////////////

		const constexpr Real PI_1 = 3.141592653589793238462643383279502884197169399375105820974944;
		const constexpr Real PI_2 = 2.0 * PI_1;
		const constexpr Real PI_4 = 4.0 * PI_1;
		const constexpr Real PI_8 = 8.0 * PI_1;

		const constexpr Real SQRT_2 = 1.414213562373095048801688724209698078569671875376948073176680;
		const constexpr Real SQRT_3 = 1.732050807568877293527446341505872366942805253810380628055806;
		const constexpr Real INV_SQRT_2 = 1.0 / SQRT_2;
		const constexpr Real INV_SQRT_3 = 1.0 / SQRT_3;

		const constexpr Real LOG_10 = 2.302585092994045684017991454684364207601101488628772976033328;
		const constexpr Real LOG2_10 = 3.321928094887362347870319429489390175864831393024580612054756;
		const constexpr Real INV_LOG2_10 = 1.0 / LOG2_10;

		//////////////////// Mathematical Functions ////////////////////

		/**
		* @brief Converts degrees to radians
		*/
		inline Real Deg2Rad(Real x) { return x * PI_1 / 180.0; }

		/**
		* @brief Converts radians to degrees
		*/
		inline Real Rad2Deg(Real x) { return x * 180.0 / PI_1; }

		/**
		* @brief Calculates 10 raised to the power of x
		*/
		inline Real Pow10(Real x) { return exp(LOG_10 * x); }

		/**
		* @brief Calculates the base 10 logarithm of x
		*/
		inline Real Log10(Real x) { return std::log2(x) * INV_LOG2_10; }

		/**
		* Calculates the cotangent of x
		*/
		inline Real cot(const Real x) { return cos(x) / sin(x); }

#else	// Float

		const constexpr Real T_CELCIUS = 20.0f;								// Temperature in degrees celcius
		const constexpr Real SPEED_OF_SOUND = 331.5f + 0.6f * T_CELCIUS;	// Speed of sound in air
		const constexpr Real INV_SPEED_OF_SOUND = 1.0f / SPEED_OF_SOUND;	// Inverse speed of sound in air

		const constexpr Real ROUND_FACTOR = 1e3f;			// Factor for rounding operations
		const constexpr Real EPS = 0.000001f;				// Tolerance for some floating point comparisons
		const constexpr Real MIN_VALUE = 10.0f * FLT_MIN;	// Minimum value for trimming BTM FIR filter

		//////////////////// Mathematical Constants ////////////////////

		const constexpr Real PI_1 = 3.141592653589793238462643383279502884197169399375105820974944f;
		const constexpr Real PI_2 = 2.0f * PI_1;
		const constexpr Real PI_4 = 4.0f * PI_1;
		const constexpr Real PI_8 = 8.0f * PI_1;

		const constexpr Real SQRT_2 = 1.414213562373095048801688724209698078569671875376948073176680f;
		const constexpr Real SQRT_3 = 1.732050807568877293527446341505872366942805253810380628055806f;
		const constexpr Real INV_SQRT_2 = 1.0f / SQRT_2;
		const constexpr Real INV_SQRT_3 = 1.0f / SQRT_3;

		const constexpr Real LOG_10 = 2.302585092994045684017991454684364207601101488628772976033328f;
		const constexpr Real LOG2_10 = 3.321928094887362347870319429489390175864831393024580612054756f;
		const constexpr Real INV_LOG2_10 = 1.0f / LOG2_10;

		//////////////////// Mathematical Functions ////////////////////

		/**
		* @brief Converts degrees to radians
		*/
		inline Real Deg2Rad(Real x) { return x * PI_1 / 180.0f; }

		/**
		* @brief Converts radians to degrees
		*/
		inline Real Rad2Deg(Real x) { return x * 180.0f / PI_1; }

		/**
		* @brief Calculates 10 raised to the power of x
		*/
		inline Real Pow10(Real x) { return expf(LOG_10 * x); }

		/**
		* @brief Calculates the base 10 logarithm of x
		*/
		inline Real Log10(Real x) { return std::log2f(x) * INV_LOG2_10; }

		/**
		* Calculates the cotangent of x
		*/
		inline Real cot(const Real x) { return cosf(x) / sinf(x); }
#endif

		//////////////////// Mathematical Constants ////////////////////

		const constexpr Real PI_EPS = PI_1 + EPS;	// PI with a small increase
		const constexpr Real PI_SQ = PI_1 * PI_1;

		//////////////////// Utility Functions ////////////////////

		/**
		* @brief Returns the sign of x
		*/
		inline Real Sign(const Real x) { return (x > 0) - (x < 0); }

		/**
		* @brief Rounds x to decimal places based on ROUND_FACTOR
		*/
		inline Real Round(Real x) { return round(x * ROUND_FACTOR) / ROUND_FACTOR; }

		/**
		* @brief Rounds x to a given number of decimal places
		* 
		* @params x The number to round
		* @params dp The number of decimal places
		*/
		inline Real Round(Real x, size_t dp)
		{
			Real factor = Pow10(static_cast<Real>(dp));
			return round(x * factor) / factor;
		}
	}
}

#endif