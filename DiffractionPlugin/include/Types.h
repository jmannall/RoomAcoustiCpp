#pragma once

#ifndef TYPES_H_
#define TYPES_H_

#include <cmath>
#include <stdlib.h>
#include "vec3.h"
#include "quaternion.h"

float inline Sign(float x)
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

float inline Round(float x, size_t dp)
{
	float factor = powf(10, (float)dp);
	return round(x * factor) / factor;
}

#pragma region structs
struct DSPConfig
{
	// factored into lerping dspParams over multiple audio callbacks
	// 1 means DSP parameters are lerped over only 1 audio callback
	// 5 means lerped over 5 separate audio callbacks
	// must be greater than 0
	unsigned short dspSmoothingFactor = 2;

	// sampling rate of audio engine
	// must be set manually by user
	unsigned samplingRate = 0;
};
#pragma endregion

enum class Model
{
	attenuate,
	off,
	lowPass,
	udfa,
	udfai,
	nnBest,
	nnSmall,
	utd,
	btm
};

inline float cot(float x)
{
	return cosf(x) / sinf(x);
}

#endif TYPES_H_