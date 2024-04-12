/*
* @brief Defines constants and some simple mathematicla functions
*
*/

#ifndef Common_Definitions_h
#define Common_Definitions_h

// C++ headers
#include <vector>

// Common headers
#include "Common/Types.h"

//////////////////// Constants ////////////////////

namespace RAC
{
	namespace Common
	{

		// Double
#if DATA_TYPE_DOUBLE

		const constexpr Real T_CELCIUS = 20.0;
		const constexpr Real SPEED_OF_SOUND = 331.5 + 0.6 * T_CELCIUS;
		//const constexpr Real SPEED_OF_SOUND = 344;

		const constexpr Real INV_SPEED_OF_SOUND = 1.0 / SPEED_OF_SOUND;

		const constexpr Real EPS = 0.000001;	// Tolerance for some floating point comparisons
		const constexpr Real PI_1 = 3.141592653589793238462643383279502884197169399375105820974944;
		const constexpr Real PI_2 = 2.0 * PI_1;
		const constexpr Real PI_4 = 4.0 * PI_1;
		const constexpr Real PI_8 = 8.0 * PI_1;
		const constexpr Real SQRT_2 = 1.414213562373095048801688724209698078569671875376948073176680;
		const constexpr Real INV_SQRT_2 = 1.0 / SQRT_2;
		const constexpr Real LOG_10 = 2.302585092994045684017991454684364207601101488628772976033328;
		const constexpr Real LOG2_10 = 3.321928094887362347870319429489390175864831393024580612054756;
		const constexpr Real INV_LOG2_10 = 1.0 / LOG2_10;

		const constexpr Real MIN_VALUE = 10.0 * DBL_MIN;

		const std::vector<Real> thirdOctBands = {
				25.0, 31.5, 40.0, 50.0, 63.0, 80.0,
				100.0, 125.0, 160.0, 200.0, 250.0, 315.0,
				400.0, 500.0, 630.0, 800.0, 1.0e3, 1.25e3,
				1.6e3, 2.0e3, 2.5e3, 3.15e3, 4.0e3, 5.0e3,
				6.3e3, 8.0e3, 10.0e3, 12.5e3, 16.0e3, 20.0e3 };

		inline Real Deg2Rad(Real x)
		{
			return x * PI_1 / 180.0;
		}

		inline Real Rad2Deg(Real x)
		{
			return x * 180.0 / PI_1;
		}

		inline Real Pow10(Real x)
		{
			return exp(LOG_10 * x);
		}

		inline Real Log10(Real x)
		{
			return std::log2(x) * INV_LOG2_10;
		}

		inline Real Sign(const Real x)
		{
			if (x == 0)
				return 0.0;
			else
			{
				if (signbit(x))
					return -1.0;
				else
					return 1.0;
			}
		}

		inline Real cot(const Real x)
		{
			return cos(x) / sin(x);
		}

		inline Real Round(Real x, size_t dp)
		{
			Real factor = pow(10.0, (Real)dp);
			return round(x * factor) / factor;
		}

		inline Real StrToReal(const std::string& str)
		{
			return std::stod(str);
		}

		// Float
#else

		const constexpr Real T_CELCIUS = 20.0f;
		const constexpr Real SPEED_OF_SOUND = 331.5f + 0.6f * T_CELCIUS;
		const constexpr Real INV_SPEED_OF_SOUND = 1.0f / SPEED_OF_SOUND;

		const constexpr Real EPS = 0.000001f;
		const constexpr Real PI_1 = 3.141592653589793238462643383279502884197169399375105820974944f;
		const constexpr Real PI_2 = 2.0f * PI_1;
		const constexpr Real PI_4 = 4.0f * PI_1;
		const constexpr Real PI_8 = 8.0f * PI_1;
		const constexpr Real SQRT_2 = 1.414213562373095048801688724209698078569671875376948073176680f;
		const constexpr Real INV_SQRT_2 = 1.0f / SQRT_2;
		const constexpr Real LOG_10 = 2.302585092994045684017991454684364207601101488628772976033328f;
		const constexpr Real LOG2_10 = 3.321928094887362347870319429489390175864831393024580612054756f;

		const constexpr Real MIN_VALUE = 10.0 * FLT_MIN;


		inline Real Deg2Rad(Real x)
		{
			return x * PI_1 / 180.0f;
		}

		inline Real Rad2Deg(Real x)
		{
			return x * 180.0f / PI_1;
		}

		inline Real Pow10(Real x)
		{
			return expf(LOG_10 * x);
		}

		inline Real Log10(Real x)
		{
			return std::log2f(x) * LOG2_10;
		}

		inline Real Sign(const Real x)
		{
			if (x == 0)
				return 0.0f;
			else
			{
				if (signbit(x))
					return -1.0f;
				else
					return 1.0f;
			}
		}

		inline Real cot(const Real x)
		{
			return cosf(x) / sinf(x);
		}

		inline Real Round(Real x, size_t dp)
		{
			Real factor = powf(10.0f, (Real)dp);
			return round(x * factor) / factor;
		}

		inline Real StrToReal(const std::string& str)
		{
			return std::stof(str);
		}

#endif
		const constexpr size_t NUM_PRECISION = 3;
		const constexpr int REFLECTION_FILTER_ORDER = 4;

		const constexpr Real PI_EPS = PI_1 + EPS;
		const constexpr Real PI_SQ = PI_1 * PI_1;
	}
}

#endif