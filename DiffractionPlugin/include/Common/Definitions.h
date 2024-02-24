/*
*
*  \Common definitions
*
*/

#ifndef Common_Definitions_h
#define Common_Definitions_h

#include <Common/ScopedTimer.h>

#include "Common/Types.h"

//////////////////// Constants ////////////////////

namespace UIE
{
	namespace Common
	{

		// Double
#if DATA_TYPE_DOUBLE

		const constexpr Real T_CELCIUS = 20.0;
		//const constexpr Real SPEED_OF_SOUND = 331.5 + 0.6 * T_CELCIUS;
		const constexpr Real SPEED_OF_SOUND = 344;

		const constexpr Real INV_SPEED_OF_SOUND = 1.0 / SPEED_OF_SOUND;

		const constexpr Real EPS = 0.000001;
		const constexpr Real PI_1 = 3.141592653589793238462643383279502884197169399375105820974944;
		const constexpr Real PI_2 = 2.0 * PI_1;
		const constexpr Real PI_4 = 4.0 * PI_1;
		const constexpr Real PI_8 = 8.0 * PI_1;
		const constexpr Real SQRT_2 = 1.414213562373095048801688724209698078569671875376948073176680;
		const constexpr Real INV_SQRT_2 = 1.0 / SQRT_2;

		const constexpr Real MIN_VALUE = 10.0 * DBL_MIN;


		inline Real Deg2Rad(Real x)
		{
			return x * PI_1 / 180.0;
		}

		inline Real Rad2Deg(Real x)
		{
			return x * 180.0 / PI_1;
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

		const constexpr Real MIN_VALUE = 10.0 * FLT_MIN;


		inline Real Deg2Rad(Real x)
		{
			return x * PI_1 / 180.0f;
		}

		inline Real Rad2Deg(Real x)
		{
			return x * 180.0f / PI_1;
		}

#endif
		const constexpr size_t NUM_PRECISION = 3;
		const constexpr int REFLECTION_FILTER_ORDER = 4;

		const constexpr Real PI_EPS = PI_1 + EPS;
		const constexpr Real PI_SQ = PI_1 * PI_1;
	}
}

//////////////////// Android specific functions ////////////////////

#if(_ANDROID)
inline int getStatusWord()
{
	int result;
	asm volatile("mrs %[result], FPCR" : [result] "=r" (result));
	return result;
}

inline void setStatusWord(int a)
{
	asm volatile("msr FPCR, %[src]" : : [src] "r" (a));
}
#endif

//////////////////// Macros ////////////////////

#define DSP_SAFE_ARRAY_DELETE(ptr)	\
if((ptr))	\
{	\
	delete[] (ptr);	\
	(ptr) = nullptr;	\
}

//////////////////// Debug print macros ////////////////////

// Redefine these to true to enable debug print information to stdout
#define PRINT_AUDIO_PROFILE false
#define PRINT_AUDIO_PROFILE_SECTION false
#define PRINT_UPDATE_PROFILE false
#define PRINT_UPDATE_PROFILE_SECTION false

#if PRINT_AUDIO_PROFILE
#define AUDIO_PROFILE_TIME(line, tag)	\
	std::cout << tag << ": ";	\
	{							\
		ScopedTimer _t;			\
		(line);					\
	}							\
	std::cout << std::endl
#else
#define AUDIO_PROFILE_TIME(line, tag) line
#endif

#if PRINT_AUDIO_PROFILE_SECTION
#define AUDIO_PROFILE_SECTION(section, tag)	\
	std::cout << tag << ":\n";	\
	{							\
		ScopedTimer _s;			\
		section					\
	}							\
	std::cout << std::endl
#else
#define AUDIO_PROFILE_SECTION(section, tag) section
#endif

#if PRINT_UPDATE_PROFILE
#define UPDATE_PROFILE_TIME(line, tag)	\
	std::cout << tag << ": ";	\
	{							\
		ScopedTimer _t;			\
		(line);					\
	}							\
	std::cout << std::endl
#else
#define UPDATE_PROFILE_TIME(line, tag) line
#endif

#if PRINT_UPDATE_PROFILE_SECTION
#define UPDATE_PROFILE_SECTION(section, tag)	\
	std::cout << tag << ":\n";	\
	{							\
		ScopedTimer _s;			\
		section					\
	}							\
	std::cout << std::endl
#else
#define UPDATE_PROFILE_SECTION(section, tag) section
#endif

#endif