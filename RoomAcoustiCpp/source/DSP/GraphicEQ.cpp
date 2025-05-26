/*
* @class GraphicEQ
*
* @brief Declaration of Graphic EQ class
*
* @remarks Based after Efficient Multi-Band Digital Audio Graphic Equalizer with Accurate Frequency Response Control. Oliver R, Jot J. 2015
*
*/

// DSP headers
#include "DSP/GraphicEQ.h"

namespace RAC
{
	namespace DSP
	{
		//////////////////// GraphicEQ ////////////////////

		////////////////////////////////////////

		GraphicEQ::GraphicEQ(const Coefficients& fc, const Real Q, const int sampleRate) : numFilters(fc.Length() + 2), lowShelf(std::max((fc[0] / 2.0)* sqrt(fc[0] / (fc[0] / 2.0)), 20.0), Q, sampleRate), highShelf(std::min(fc[numFilters - 3] * sqrt((2.0 * fc[numFilters - 3]) / fc[numFilters - 3]), 20000.0), Q, sampleRate),
			dbGains(numFilters), inputGains(numFilters), targetFilterGains(numFilters + 1), currentFilterGains(numFilters + 1), lastInput(fc.Length()), filterResponseMatrix(numFilters, numFilters), equal(false), valid(false)
		{
			InitFilters(fc, Q, sampleRate);
			InitMatrix(fc);
		}

		////////////////////////////////////////

		GraphicEQ::GraphicEQ(const Coefficients& gain, const Coefficients& fc, const Real Q, const int sampleRate) : GraphicEQ(fc, Q, sampleRate)
		{ 
			InitParameters(gain);
		}

		////////////////////////////////////////

		void GraphicEQ::InitFilters(const Coefficients& fc, const Real Q, const int sampleRate)
		{
			for (int i = 0; i < numFilters - 2; i++)
				peakingFilters.push_back(std::make_unique<PeakingFilter>(fc[i], Q, sampleRate));
		}

		////////////////////////////////////////

		void GraphicEQ::InitMatrix(const Coefficients& fc)
		{
			std::vector<Real> f = std::vector<Real>(numFilters, 0.0);

			f[0] = std::max(fc[0] / 2.0, 20.0);
			for (int i = 1; i < numFilters - 1; i++)
				f[i] = fc[i - 1];
			f[numFilters - 1] = std::min(2.0 * fc[numFilters - 3], 20000.0);

			Real pdb = 6.0;
			Real p = pow(10.0, pdb / 20.0);

			std::vector<Real> out = std::vector<Real>(f.size(), 0.0);
			int j = 0;

			lowShelf.SetTargetGain(p);
			out = lowShelf.GetFrequencyResponse(f);
			lowShelf.SetTargetGain(0.0);

			for (int i = 0; i < out.size(); i++)
				filterResponseMatrix[j][i] += out[i];

			j++;

			for (auto& filter : peakingFilters)
			{
				filter->SetTargetGain(p);
				out = filter->GetFrequencyResponse(f);
				filter->SetTargetGain(1.0);

				for (int i = 0; i < out.size(); i++)
					filterResponseMatrix[j][i] += out[i];
				j++;
			}

			highShelf.SetTargetGain(p);
			out = highShelf.GetFrequencyResponse(f);
			highShelf.SetTargetGain(0.0);

			for (int i = 0; i < out.size(); i++)
				filterResponseMatrix[j][i] += out[i];

			filterResponseMatrix.Inverse();
			filterResponseMatrix *= pdb;
		}

		////////////////////////////////////////

		void GraphicEQ::InitParameters(const Coefficients& targetBandGains)
		{
			SetGain(targetBandGains);
			currentFilterGains = targetFilterGains;
			equal = true;
		}

		////////////////////////////////////////

		void GraphicEQ::SetGain(const Coefficients& targetBandGains)
		{
			if (targetBandGains == lastInput)
				return;

			lastInput = targetBandGains;
			if (targetBandGains == 0)
			{
				inputGains.Reset();
				targetFilterGains[0] = 0;
			}
			else
			{
				if (!valid)
				{
					valid = true;
					ClearBuffers();
				}

				// when dB is used here. Factors of 20 are cancelled out.
				Real g = (targetBandGains[0] + targetBandGains[1]) / 2.0;
				dbGains[0] = std::max(g, EPS); // Prevent log10(0)

				for (int i = 1; i < numFilters - 1; i++)
					dbGains[i] = std::max(targetBandGains[i - 1], EPS); // Prevent log10(0)

				g = (targetBandGains[numFilters - 4] + targetBandGains[numFilters - 3]) / 2.0;
				dbGains[numFilters - 1] = std::max(g, EPS); // Prevent log10(0)

				dbGains.Log10();
				Real meandBGain = dbGains.Sum() / dbGains.Cols();
				targetFilterGains[0] = Pow10(meandBGain); // 10 ^ mean(dbGains);
				dbGains -= meandBGain; // dbGains - mean(dbGains);

				Mulitply(inputGains, dbGains, filterResponseMatrix);
				inputGains.Pow10();
			}
			for (int i = 0; i < numFilters; i++)
				targetFilterGains[i + 1] = inputGains[i];

			if (targetFilterGains != currentFilterGains)
				equal = false;

			UpdateParameters();
		}

		////////////////////////////////////////

		void GraphicEQ::UpdateParameters()
		{
			if (currentFilterGains[0] == 0.0)
			{
				valid = false;
				return;
			}

			int i = 1;

			lowShelf.SetTargetGain(currentFilterGains[i]);
			i++;

			for (auto& filter : peakingFilters)
			{
				filter->SetTargetGain(currentFilterGains[i]);
				i++;
			}

			highShelf.SetTargetGain(currentFilterGains[i]);
		}

		////////////////////////////////////////

		Real GraphicEQ::GetOutput(const Real input, const Real lerpFactor)
		{
			if (!valid)
				return 0.0;

			Real out = lowShelf.GetOutput(input, lerpFactor);
			for (auto& filter : peakingFilters)
				out = filter->GetOutput(out, lerpFactor);
			out = highShelf.GetOutput(out, lerpFactor);
			out *= currentFilterGains[0];
			return out;
		}

		////////////////////////////////////////

		void GraphicEQ::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor)
		{
			if (!valid)
				return;

			FlushDenormals();
			for (int i = 0; i < numFrames; i++)
				outBuffer[i] = GetOutput(inBuffer[i], lerpFactor);
			NoFlushDenormals();
		}
	}
}