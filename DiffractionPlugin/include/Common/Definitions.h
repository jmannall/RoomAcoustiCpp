#pragma once

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <ScopedTimer.h>

#pragma region import/export
// Enforce OS requirement
//#if !defined(_WIN32)
// #error "Only supports Windows!"
//#endif

 // API export/import
#ifdef UNITYGAPLUGIN_EXPORTS
 // For Windows
 #define GA_API __declspec(dllexport)
#else
 // For Windows
 #define GA_API __declspec(dllimport)
#endif

#ifdef _TEST
 #ifdef _MAKEDLL
  #define GA_TEST __declspec(dllexport)
 #else
  #define GA_TEST __declspec(dllimport)
 #endif
#else
 #define GA_TEST
#endif

#pragma endregion

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

#pragma region constants
const constexpr unsigned short CHANNEL_COUNT = 2;
const constexpr float T_CELCIUS= 20;
const constexpr float SPEED_OF_SOUND = 331.5 + 0.6 * T_CELCIUS;
const constexpr float INV_SPEED_OF_SOUND = 1.0f / SPEED_OF_SOUND;

const constexpr float EPS = 0.000001f;
const constexpr float PI_1 = 3.141593f;
const constexpr float PI_EPS = 3.141594f;
const constexpr float PI_SQ = 9.869604f;
const constexpr float PI_2 = 6.283185f;
const constexpr float PI_4 = 12.566371f;
const constexpr float SQRT_2 = 1.414214f;
const constexpr float INV_SQRT_2 = 1.0f / SQRT_2;
#pragma endregion

#define LERP_FLOAT(start, end, factor) (((start * (1.f - factor)) + (end * factor)))

#define DSP_SAFE_ARRAY_DELETE(ptr)	\
if((ptr))	\
{	\
	delete[] (ptr);	\
	(ptr) = nullptr;	\
}

#endif DEFINITIONS_H_