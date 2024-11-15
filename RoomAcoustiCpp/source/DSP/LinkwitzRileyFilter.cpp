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
			PassFilter lpFilter[3] = { PassFilter(fc[0], true, sampleRate), PassFilter(fc[1], true, sampleRate), PassFilter(fc[2], true, sampleRate) };
			PassFilter hpFilter[3] = { PassFilter(fc[0], false, sampleRate), PassFilter(fc[1], false, sampleRate), PassFilter(fc[2], false, sampleRate) };

			filters.reserve(20);
			filters.push_back(lpFilter[1]);
			filters.push_back(lpFilter[1]);
			filters.push_back(lpFilter[2]);
			filters.push_back(lpFilter[2]);
			filters.push_back(hpFilter[2]);
			filters.push_back(hpFilter[2]);
			filters.push_back(lpFilter[0]);
			filters.push_back(lpFilter[0]);
			filters.push_back(hpFilter[0]);
			filters.push_back(hpFilter[0]);
			filters.push_back(hpFilter[1]);
			filters.push_back(hpFilter[1]);
			filters.push_back(lpFilter[0]);
			filters.push_back(lpFilter[0]);
			filters.push_back(hpFilter[0]);
			filters.push_back(hpFilter[0]);
			filters.push_back(lpFilter[2]);
			filters.push_back(lpFilter[2]);
			filters.push_back(hpFilter[2]);
			filters.push_back(hpFilter[2]);
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

		Real LinkwitzRiley::GetOutput(const Real input)
		{
			Real mid[2];
			mid[0] = filters[1].GetOutput(filters[0].GetOutput(input));
			mid[1] = filters[11].GetOutput(filters[10].GetOutput(input));

			mid[0] = filters[3].GetOutput(filters[2].GetOutput(mid[0])) + filters[5].GetOutput(filters[4].GetOutput(mid[0]));
			mid[1] = filters[13].GetOutput(filters[12].GetOutput(mid[1])) + filters[15].GetOutput(filters[14].GetOutput(mid[1]));

			Real out[4];
			out[0] = gains[0] * filters[7].GetOutput(filters[6].GetOutput(mid[0]));
			out[1] = gains[1] * filters[9].GetOutput(filters[8].GetOutput(mid[0]));
			out[2] = gains[2] * filters[17].GetOutput(filters[16].GetOutput(mid[1]));
			out[3] = gains[3] * filters[19].GetOutput(filters[18].GetOutput(mid[1]));

			return out[0] + out[1] + out[2] + out[3];
		}
	}
}

