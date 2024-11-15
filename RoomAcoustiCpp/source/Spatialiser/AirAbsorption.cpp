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
		//////////////////// AirAbsorption ////////////////////

		////////////////////////////////////////

		Real AirAbsorption::GetOutput(const Real input)
		{
			Real v = input;
			Real output = 0.0;

			v += y * a;
			y = v;
			output += v * b;

			return output;
		}

		////////////////////////////////////////

		void AirAbsorption::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor)
		{
			FlushDenormals();
			if (equal)
			{
				for (int i = 0; i < numFrames; i++)
					outBuffer[i] = GetOutput(inBuffer[i]);
			}
			else if (Equals(currentDistance, targetDistance))
			{
				currentDistance = targetDistance;
				equal = true;
				UpdateParameters();
				for (int i = 0; i < numFrames; i++)
					outBuffer[i] = GetOutput(inBuffer[i]);
			}
			else
			{
				for (int i = 0; i < numFrames; i++)
				{
					outBuffer[i] = GetOutput(inBuffer[i]);
					currentDistance = Lerp(currentDistance, targetDistance, lerpFactor);
					UpdateParameters();
				}
			}
			NoFlushDenormals();
		}
	}
}
