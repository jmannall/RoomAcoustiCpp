/*
* @class AirAbsorption
*
* @brief Declaration of AirAbsorption class
*
* @remark Based after Implementation and perceptual evaluation of a simulation method for coupled rooms in higher order ambisonics. Grimm G et al. 2014
*
*/

// Spatialiser headers
#include "Spatialiser/AirAbsorption.h"

// DSP headers
#include "DSP/Interpolate.h"

namespace RAC
{
	using namespace DSP;
	namespace Spatialiser
	{
		////////////////////////////////////////

		Real AirAbsorption::GetOutput(const Real input)
		{
			FlushDenormals();
			Real v = input;
			Real output = 0.0;

			v += y * a;
			y = v;
			output += v * b;

			NoFlushDenormals();
			return output;
		}

		////////////////////////////////////////

		void AirAbsorption::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor)
		{
			if (currentD == targetD)
			{
				for (int i = 0; i < numFrames; i++)
					outBuffer[i] = GetOutput(inBuffer[i]);
			}
			else
			{
				for (int i = 0; i < numFrames; i++)
				{
					outBuffer[i] = GetOutput(inBuffer[i]);
					currentD = Lerp(currentD, targetD, lerpFactor);
					UpdateParameters();
				}
			}
		}
	}
}
