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

		Real OctaveBand::Filter::GetOutput(const Real input)
		{
			const int index = count;
			const int indexMinusHalf = index - halfOutputLine + 1;

			for (int i = 0; i < irLength; i++)
			{
				Real value = currentIR[i] * input;
				int offset = 2 * step * i;
				outputLine[indexMinusHalf + offset] += value;
				outputLine[index - offset] += value;
			}
			outputLine[index - midSampleStep] += midSample * input;

			Real output = outputLine[index] + outputLine[index - halfOutputLine];
			outputLine[index] = 0.0;
			outputLine[index - halfOutputLine] = 0.0;

			if (--count < halfOutputLine)
				count = 2 * halfOutputLine - 1;
			return output;
		}

		//////////////////// OctaveBand ////////////////////

		////////////////////////////////////////

		OctaveBand::OctaveBand(const Coefficients<>& frequencies, int fs) :
			octaveBandIndices(CreateFrequencyIndices(frequencies))
		{
			 std::vector<int> uniqueIndices;
			 int maxIndex = 0;
			 int minIndex = ToInt(cutOffFrequencies.Length());
			 for (int i = 0; i < octaveBandIndices.Length(); i++)
			 {
				 int index = octaveBandIndices(i);
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

			 bands = Buffer<>::Zero(numFrequencyBands);
			 outputs = Buffer<>::Zero(numOutputBands);

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
			delay += static_cast<int>(std::pow(2.0, numFrequencyBands - 2)) - 1;
			int offset = 1;
			for (int i = 0; i < numFrequencyBands - 2; i++)
			{
				delay -= offset;
				delayLines.emplace_back(delay * Dwin);
				offset *= 2;
			}
		}

		////////////////////////////////////////

		const Buffer<>& OctaveBand::GetOutput(Real input, Real lerpFactor)
		{
			assert(IsValid());
			
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

			CombineTopBands(bands);
			return outputs;
		}

		////////////////////////////////////////

		void OctaveBand::CombineTopBands(const Buffer<>& bands)
		{
			for (int i = 0; i < numOutputBands; i++)
				outputs[i] = bands[i + numTopBandsToSum];
			for (int i = 0; i < numTopBandsToSum; i++)
				outputs[0] += bands[i];
		}

		////////////////////////////////////////

		int OctaveBand::GetFrequencyIndex(Real f) const
		{
			int numEdges = ToInt(cutOffFrequencies.Length());
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