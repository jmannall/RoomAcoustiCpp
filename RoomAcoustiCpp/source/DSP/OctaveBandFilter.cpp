/*
* @class OctaveBand
*
* @brief Declaration of OctaveBand filter class
*
* @remarks Based after Linear-Phase Octave Graphic Equalizer. Bruschi V, Valimaki V, Liski J, Cecchi L. 2022
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

		////////////////////////////////////////

		OctaveBand::OctaveBand(const Coefficients<>& frequencies, int fs) :
			octaveBandIndices(CreateFrequencyIndices(frequencies))
		{
			 std::vector<Real> uniqueIndices;
			 int maxIndex = 0;
			 int minIndex = cutOffFrequencies.Length();
			 for (int i = 0; i < octaveBandIndices.Rows(); i++)
			 {
				 int index = octaveBandIndices[i];
				 if (index > maxIndex)
					 maxIndex = index;
				 if (index < minIndex)
					 minIndex = index;
				 if (std::find(uniqueIndices.begin(), uniqueIndices.end(), index) == uniqueIndices.end())
					 uniqueIndices.push_back(index);
			 }

			 numFrequencyBands = maxIndex + 1;
			 numTopBandsToSum = minIndex;
			 numOutputBands = numFrequencyBands - numTopBandsToSum;

			 // Do not initialise filter if less than 1 frequency band
			 if (uniqueIndices.size() == 1)
				 return;
			
			InitFilter(fs);
			initialised.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		void OctaveBand::InitFilter(int fs)
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
		}

		////////////////////////////////////////

		std::vector<Real> OctaveBand::GetOutput(Real input, Real lerpFactor)
		{
			if (!initialised.load(std::memory_order_acquire)) // Do nothing if not initialised
				return std::vector<Real>(1, input);

			std::vector<Real> bands(numFrequencyBands, input);
			for (int i = 0; i < numFrequencyBands - 1; i++) // output 0 is 16kHz, output 1 is 8kHz etc
			{
				Real output = filters[i]->GetOutput(input);
				bands[i] = delayLines[i].GetOutput(input) - output;
				input = output;
			}
			bands[numFrequencyBands - 1] = input;

			for (int i = 0; i < numFrequencyBands - 2; i++)
				bands[i] = delayLines[i + numFrequencyBands - 1].GetOutput(bands[i]);

			if (numOutputBands == numFrequencyBands)
				return bands;
			return CombineTopBands(bands);
		}

		////////////////////////////////////////

		std::vector<Real> OctaveBand::CombineTopBands(const std::vector<Real>& bands)
		{
			std::vector<Real> outputs(numOutputBands, 0.0);
			for (int i = 0; i < numOutputBands; i++)
				outputs[i] = bands[i + numTopBandsToSum];
			for (int i = 0; i < numTopBandsToSum; i++)
				outputs[0] += bands[i];
			return outputs;		// Return split bands
		}

		////////////////////////////////////////

		int OctaveBand::GetFrequencyIndex(Real f) const
		{
			int numEdges = cutOffFrequencies.Length();
			if (f <= cutOffFrequencies[0])
				return numEdges;

			// Iterate from bottom to the top
			for (int i = 1; i < numEdges; i++)
			{
				if (f > cutOffFrequencies[i - 1] && f <= cutOffFrequencies[i])
					return numEdges - i;
			}

			// f > *cutOffFrequencies.end()
			return 0;
		}
	}
}