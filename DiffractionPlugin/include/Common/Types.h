/*
*
*  \Common type definitions
*
*/

#ifndef Common_Types_h
#define Common_Types_h

#include <cmath>
#include <string>

namespace UIE
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

		//////////////////// Functions ////////////////////

// Double
#if DATA_TYPE_DOUBLE

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

		inline Real Lerp(Real start, Real end, Real factor)
		{
			return start * (1.0 - factor) + end * factor;
		}

		inline Real StrToReal(const std::string& str)
		{
			return std::stod(str);
		}

// Float
#else
		
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

		inline Real Lerp(Real start, Real end, Real factor)
		{
			return start * (1.0f - factor) + end * factor;
		}

		inline Real StrToReal(const std::string& str)
		{
			return std::stof(str);
		}

#endif

	}
}

#endif