/*
* @class LinkwitzRiley
*
* @brief Declaration of LinkwitzRiley filter class
*
*/

// DSP headers
#include "DSP/LinkwitzRileyFilter.h"

namespace RAC
{
	namespace DSP
	{
		//////////////////// LinkwitzRiley ////////////////////

		////////////////////////////////////////

		void LinkwitzRiley::InitFilters(const int sampleRate)
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

		void LinkwitzRiley::CalcMidFrequencies()
		{
			fm[0] = sqrt(20.0 * fc[0]);
			fm[1] = sqrt(fc[0] * fc[1]);
			fm[2] = sqrt(fc[1] * fc[2]);
			fm[3] = sqrt(fc[2] * 20000.0);
		}

		////////////////////////////////////////

		Real LinkwitzRiley::GetOutput(const Real input, const Real lerpFactor)
		{
			Real mid[2];
			mid[0] = filters[1]->GetOutput(filters[0]->GetOutput(input, lerpFactor), lerpFactor);
			mid[1] = filters[11]->GetOutput(filters[10]->GetOutput(input, lerpFactor), lerpFactor);

			mid[0] = filters[3]->GetOutput(filters[2]->GetOutput(mid[0], lerpFactor), lerpFactor) + filters[5]->GetOutput(filters[4]->GetOutput(mid[0], lerpFactor), lerpFactor);
			mid[1] = filters[13]->GetOutput(filters[12]->GetOutput(mid[1], lerpFactor), lerpFactor) + filters[15]->GetOutput(filters[14]->GetOutput(mid[1], lerpFactor), lerpFactor);

			Real out[4];
			out[0] = gains[0] * filters[7]->GetOutput(filters[6]->GetOutput(mid[0], lerpFactor), lerpFactor);
			out[1] = gains[1] * filters[9]->GetOutput(filters[8]->GetOutput(mid[0], lerpFactor), lerpFactor);
			out[2] = gains[2] * filters[17]->GetOutput(filters[16]->GetOutput(mid[1], lerpFactor), lerpFactor);
			out[3] = gains[3] * filters[19]->GetOutput(filters[18]->GetOutput(mid[1], lerpFactor), lerpFactor);

			return out[0] + out[1] + out[2] + out[3];
		}
	}
}

