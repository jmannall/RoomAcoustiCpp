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
			filters.clear();
            filters.reserve(20);

			// First Split Low
			filters.emplace_back(std::make_unique<LowPass>(fc[1], sampleRate));
			filters.emplace_back(std::make_unique<LowPass>(fc[1], sampleRate));
			// Split High (for phase?)
			filters.emplace_back(std::make_unique<LowPass>(fc[2], sampleRate));
			filters.emplace_back(std::make_unique<LowPass>(fc[2], sampleRate));
			// Split Low (for phase?)
			filters.emplace_back(std::make_unique<HighPass>(fc[2], sampleRate));
			filters.emplace_back(std::make_unique<HighPass>(fc[2], sampleRate));
			// Second Split Low
			filters.emplace_back(std::make_unique<LowPass>(fc[0], sampleRate));
			filters.emplace_back(std::make_unique<LowPass>(fc[0], sampleRate));
			// Second Split High
			filters.emplace_back(std::make_unique<HighPass>(fc[0], sampleRate));
			filters.emplace_back(std::make_unique<HighPass>(fc[0], sampleRate));
			// First Split High
			filters.emplace_back(std::make_unique<HighPass>(fc[1], sampleRate));
			filters.emplace_back(std::make_unique<HighPass>(fc[1], sampleRate));
			// Split Low (for phase?)
			filters.emplace_back(std::make_unique<LowPass>(fc[0], sampleRate));
			filters.emplace_back(std::make_unique<LowPass>(fc[0], sampleRate));
			// Split High (for phase?)
			filters.emplace_back(std::make_unique<HighPass>(fc[0], sampleRate));
			filters.emplace_back(std::make_unique<HighPass>(fc[0], sampleRate));
			// Second Split Low
			filters.emplace_back(std::make_unique<LowPass>(fc[2], sampleRate));
			filters.emplace_back(std::make_unique<LowPass>(fc[2], sampleRate));
			// Second Split High
			filters.emplace_back(std::make_unique<HighPass>(fc[2], sampleRate));
			filters.emplace_back(std::make_unique<HighPass>(fc[2], sampleRate));
		}

		////////////////////////////////////////

		Real LinkwitzRiley::GetOutput(const Real input, const Real lerpFactor)
		{
			if (!initialised.load())
				return 0.0;

			if (!gainsEqual.load())
				InterpolateGains(lerpFactor);

			std::array<Real, 2> midResult = {
				filters[1]->GetOutput(filters[0]->GetOutput(input, lerpFactor), lerpFactor),
				filters[11]->GetOutput(filters[10]->GetOutput(input, lerpFactor), lerpFactor) };

			midResult[0] = filters[3]->GetOutput(filters[2]->GetOutput(midResult[0], lerpFactor), lerpFactor) + filters[5]->GetOutput(filters[4]->GetOutput(midResult[0], lerpFactor), lerpFactor);
			midResult[1] = filters[13]->GetOutput(filters[12]->GetOutput(midResult[1], lerpFactor), lerpFactor) + filters[15]->GetOutput(filters[14]->GetOutput(midResult[1], lerpFactor), lerpFactor);

			std::array<Real, 4> out = {
				currentGains[0] * filters[7]->GetOutput(filters[6]->GetOutput(midResult[0], lerpFactor), lerpFactor),
				currentGains[1] * filters[9]->GetOutput(filters[8]->GetOutput(midResult[0], lerpFactor), lerpFactor),
				currentGains[2] * filters[17]->GetOutput(filters[16]->GetOutput(midResult[1], lerpFactor), lerpFactor),
				currentGains[3] * filters[19]->GetOutput(filters[18]->GetOutput(midResult[1], lerpFactor), lerpFactor) };

			return out[0] + out[1] + out[2] + out[3];
		}

		////////////////////////////////////////

		void LinkwitzRiley::InterpolateGains(const Real lerpFactor)
		{
			gainsEqual.store(true); // Prevents issues in case targetGains updated during this function call
			const std::shared_ptr<const Coefficients> gain = targetGains.load();

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

