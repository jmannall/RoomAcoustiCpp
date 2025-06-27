/*
* @class LinkwitzRiley
*
* @brief Declaration of LinkwitzRiley filter class
*
*/

// C++ headers
#include <array>

// DSP headers
#include "DSP/LinkwitzRileyFilter.h"

namespace RAC
{
	namespace DSP
	{
		//////////////////// LinkwitzRiley ////////////////////

		ReleasePool LinkwitzRiley::releasePool;

		////////////////////////////////////////

		void LinkwitzRiley::InitFilters(const int sampleRate, const std::array<Real, 3>& fc)
		{
			// First Split Low
			lowPassFilters[0].emplace(fc[1], sampleRate);
			lowPassFilters[1].emplace(fc[1], sampleRate);
			// Split High (for phase?)
			lowPassFilters[2].emplace(fc[2], sampleRate);
			lowPassFilters[3].emplace(fc[2], sampleRate);
			// Split Low (for phase?)
			highPassFilters[0].emplace(fc[2], sampleRate);
			highPassFilters[1].emplace(fc[2], sampleRate);
			// Second Split Low
			lowPassFilters[4].emplace(fc[0], sampleRate);
			lowPassFilters[5].emplace(fc[0], sampleRate);
			// Second Split High
			highPassFilters[2].emplace(fc[0], sampleRate);
			highPassFilters[3].emplace(fc[0], sampleRate);
			// First Split High
			highPassFilters[4].emplace(fc[1], sampleRate);
			highPassFilters[5].emplace(fc[1], sampleRate);
			// Split Low (for phase?)
			lowPassFilters[6].emplace(fc[0], sampleRate);
			lowPassFilters[7].emplace(fc[0], sampleRate);
			// Split High (for phase?)
			highPassFilters[6].emplace(fc[0], sampleRate);
			highPassFilters[7].emplace(fc[0], sampleRate);
			// Second Split Low
			lowPassFilters[8].emplace(fc[2], sampleRate);
			lowPassFilters[9].emplace(fc[2], sampleRate);
			// Second Split High
			highPassFilters[8].emplace(fc[2], sampleRate);
			highPassFilters[9].emplace(fc[2], sampleRate);
		}

		////////////////////////////////////////

		Real LinkwitzRiley::GetOutput(const Real input, const Real lerpFactor)
		{
			if (!initialised.load())
				return 0.0;

			if (!gainsEqual.load())
				InterpolateGains(lerpFactor);

			std::array<Real, 2> midResult = {
				lowPassFilters[1]->GetOutput(lowPassFilters[0]->GetOutput(input, lerpFactor), lerpFactor),
				highPassFilters[5]->GetOutput(highPassFilters[4]->GetOutput(input, lerpFactor), lerpFactor) };

			midResult[0] = lowPassFilters[3]->GetOutput(lowPassFilters[2]->GetOutput(midResult[0], lerpFactor), lerpFactor) + highPassFilters[1]->GetOutput(highPassFilters[0]->GetOutput(midResult[0], lerpFactor), lerpFactor);
			midResult[1] = lowPassFilters[7]->GetOutput(lowPassFilters[6]->GetOutput(midResult[1], lerpFactor), lerpFactor) + highPassFilters[7]->GetOutput(highPassFilters[6]->GetOutput(midResult[1], lerpFactor), lerpFactor);

			std::array<Real, 4> out = {
				currentGains[0] * lowPassFilters[5]->GetOutput(lowPassFilters[4]->GetOutput(midResult[0], lerpFactor), lerpFactor),
				currentGains[1] * highPassFilters[3]->GetOutput(highPassFilters[2]->GetOutput(midResult[0], lerpFactor), lerpFactor),
				currentGains[2] * lowPassFilters[9]->GetOutput(lowPassFilters[8]->GetOutput(midResult[1], lerpFactor), lerpFactor),
				currentGains[3] * highPassFilters[9]->GetOutput(highPassFilters[8]->GetOutput(midResult[1], lerpFactor), lerpFactor) };

			return out[0] + out[1] + out[2] + out[3];
		}

		////////////////////////////////////////

		void LinkwitzRiley::InterpolateGains(const Real lerpFactor)
		{
			gainsEqual.store(true); // Prevents issues in case targetGains updated during this function call
			const std::shared_ptr<const Parameters> gain = targetGains.load();

			Lerp(currentGains, *gain, lerpFactor);
			if (Equals(currentGains, *gain))
			{
				currentGains = *gain;
				return;
			}
			gainsEqual.store(false);
		}

	}
}

