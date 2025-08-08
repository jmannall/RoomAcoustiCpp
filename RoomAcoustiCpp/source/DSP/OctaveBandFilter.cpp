/*
* @class OctaveBand
*
* @brief Declaration of OctaveBand filter class
*
*/

// C++ headers
#include <array>

// DSP headers
#include "DSP/OctaveBandFilter.h"

namespace RAC
{
	namespace DSP
	{
		//////////////////// OctaveBand::Filter ////////////////////
		
		////////////////////////////////////////

		Real OctaveBand::Filter::GetOutput(const Real input, const Real lerpFactor)
		{
			if (!initialised.load(std::memory_order_acquire))
				return 0.0;

			if (clearInputLine.load(std::memory_order_acquire))
			{
				inputLine.Reset();
				clearInputLine.store(false, std::memory_order_release);
			}

			Real output = 0.0;
			int index = count;

			// Using a double buffer size to avoid checks in process loop
			inputLine[index] = input;
			inputLine[index + halfInputLine] = input;

			assert(currentIR.Length() >= irLength);
			assert(irLength % 8 == 0);

			// Assume length is always a multiple of 8
			for (int i = 0; i < irLength; i += 8) // Every other sample is be zero
			{
				output += currentIR[i] * inputLine[index];
				index += 2 * step;
				output += currentIR[i + 1] * inputLine[index];
				index += 2 * step;
				output += currentIR[i + 2] * inputLine[index];
				index += 2 * step;
				output += currentIR[i + 3] * inputLine[index];
				index += 2 * step;
				output += currentIR[i + 4] * inputLine[index];
				index += 2 * step;
				output += currentIR[i + 5] * inputLine[index];
				index += 2 * step;
				output += currentIR[i + 6] * inputLine[index];
				index += 2 * step;
				output += currentIR[i + 7] * inputLine[index];
				index += 2 * step;
			}
			output += midSample * inputLine[count + midSampleStep];

			if (--count < 0)
				count = halfInputLine - 1;
			return output;
		}

		//////////////////// OctaveBand ////////////////////

		ReleasePool OctaveBand::releasePool;

		////////////////////////////////////////

		OctaveBand::OctaveBand(const Parameters& gains, int fs, int numFrequencyBands) :
			numFrequencyBands(numFrequencyBands), currentGains(gains)
		{
			Buffer<> h = CalculateH(fs);
			Real midSample = 2 * fc / static_cast<Real>(fs);

			filters.reserve(numFrequencyBands - 1);
			delayLines.reserve(2 * numFrequencyBands - 3);
			int delay = 1;
			for (int i = 0; i < numFrequencyBands - 1; i++)
			{
				delayLines.emplace_back(delay * Dwin);
				filters.emplace_back(std::make_unique<Filter>(h, midSample, delay));
				delay *= 2;
			}
			delay /= 2;
			delay += std::pow(2.0, numFrequencyBands - 2) - 1;
			int offset = 1;
			for (int i = 0; i < numFrequencyBands - 2; i++)
			{
				delay -= offset;
				delayLines.emplace_back(delay * Dwin);
				offset *= 2;
			}

			SetTargetGains(gains);

			gainsEqual.store(true, std::memory_order_release);
			initialised.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		std::vector<Real> OctaveBand::GetOutput(Real input, Real lerpFactor)
		{
			std::vector<Real> outputs(numFrequencyBands, 0.0);

			if (!initialised.load(std::memory_order_acquire))
				return outputs;

			if (!gainsEqual.load(std::memory_order_acquire))
				InterpolateGains(lerpFactor);

			for (int i = 0; i < numFrequencyBands - 1; i++)
			{
				Real output = filters[i]->GetOutput(input);
				outputs[i] = delayLines[i].GetOutput(input) - output;
				input = output;
			}
			outputs[numFrequencyBands - 1] = input;

			for (int i = 0; i < numFrequencyBands - 2; i++)
				outputs[i] = delayLines[i + numFrequencyBands - 1].GetOutput(outputs[i]);

			Real output = 0.0;
			for (int i = 0; i < numFrequencyBands; i++)
			{
				outputs[i] *= currentGains[i];
				output += outputs[i];
			}
			return outputs;
			// return output;
		}
	}
}