/*
* @class AirAbsorption
*
* @brief Declaration of AirAbsorption class
*
* @remark Based after Implementation and perceptual evaluation of a simulation method for coupled rooms in higher order ambisonics. Grimm G et al. 2014
* @note Error in paper eq (1). should be y_k = a_1 * x_k - (1 - a_1) * y_k
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

		void AirAbsorption::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor)
		{
			FlushDenormals();
			for (int i = 0; i < inBuffer.Length(); i++)
				outBuffer[i] = GetOutput(inBuffer[i], lerpFactor);
			NoFlushDenormals();
		}

		////////////////////////////////////////

		void AirAbsorption::InterpolateParameters(const Real lerpFactor)
		{
			parametersEqual.store(true); // Prevents issues in case targetFc/Gain updated during this function call
			const Real distance = targetDistance.load();
			currentDistance = Lerp(currentDistance, distance, lerpFactor);
			if (Equals(currentDistance, distance))
				currentDistance = distance;
			else
				parametersEqual.store(false);
			UpdateCoefficients(currentDistance);
		}
	}
}
