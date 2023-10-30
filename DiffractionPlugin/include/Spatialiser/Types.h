#pragma once

#ifndef SPATIALISER_TYPES_H_
#define SPATIALISER_TYPES_H_

#include <string>
#include "vec3.h"
#include "vec.h"

using namespace std;

namespace Spatialiser
{
	class VirtualSourceData;

	enum class HRTFMode
	{
		quality,
		performance,
		none
	};

	enum class ReverbWall
	{
		posZ,
		negZ,
		posX,
		negX,
		posY,
		negY,
		none
	};

	struct Config
	{
		// DSP parameters
		int sampleRate = 0;
		int bufferSize = 0;
		int hrtfResamplingStep = 45;
		int maxRefOrder = 2;
		HRTFMode hrtfMode = HRTFMode::performance;

		// Binaural resource file paths
		//string resourcePath = "D:\\Joshua Mannall\\GitHub\\3dti_AudioToolkit\\resources";
		//string hrtfPath = "\\HRTF\\3DTI\\3DTI_HRTF_IRC1008_128s_48000Hz.3dti-hrtf";
		//string ildPath = "\\ILD\\NearFieldCompensation_ILD_48000.3dti-ild";
	};

	const int numAbsorptionFreq = 5;
	const float ABSORPTION_FREQUENCIES[] = { 250, 500, 1000, 2000, 4000 };
}
#endif SPATIALISER_TYPES_H_