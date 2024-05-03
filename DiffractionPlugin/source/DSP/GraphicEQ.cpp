/*
* @class GraphicEQ
*
* @brief Declaration of Graphic EQ class
*
*/

// C++ headers
#include <vector> 

// DSP headers
#include "DSP/GraphicEQ.h"
#include "DSP/Interpolate.h"

// Common headers
#include "Common/Coefficients.h"
#include "Common/Matrix.h"
#include "Common/Vec.h"

// Unity headers
#include "Unity/UnityInterface.h"

namespace RAC
{
	namespace DSP
	{
		GraphicEQ::GraphicEQ(const Coefficients& fc, const Real Q, const int sampleRate) : numFilters(fc.Length() + 2), lowShelf((fc[0] / 2.0) * sqrt(fc[0] / (fc[0] / 2.0)), Q, sampleRate), highShelf(fc[numFilters - 3] * sqrt((2.0 * fc[numFilters - 3]) / fc[numFilters - 3]), Q, sampleRate),
			dbGain(numFilters), inputGain(numFilters), targetGain(numFilters + 1), currentGain(numFilters + 1), lastInput(fc.Length()), mat(numFilters, numFilters), equal(false), valid(false)
		{
			InitFilters(fc, Q, sampleRate);
			InitMatrix(fc);
		}

		GraphicEQ::GraphicEQ(const Coefficients& gain, const Coefficients& fc, const Real Q, const int sampleRate) : numFilters(fc.Length() + 2), lowShelf((fc[0] / 2.0)* sqrt(fc[0] / (fc[0] / 2.0)), Q, sampleRate), highShelf(fc[numFilters - 3] * sqrt((2.0 * fc[numFilters - 3]) / fc[numFilters - 3]), Q, sampleRate),
			dbGain(numFilters), inputGain(numFilters), targetGain(numFilters + 1), currentGain(numFilters + 1), lastInput(fc.Length()), mat(numFilters, numFilters), equal(false), valid(false)
		{
			InitFilters(fc, Q, sampleRate);
			InitMatrix(fc);
		}

		void GraphicEQ::InitFilters(const Coefficients& fc, const Real Q, const int sampleRate)
		{
			for (int i = 0; i < numFilters - 2; i++)
				peakingFilters.push_back(PeakingFilter(fc[i], Q, sampleRate));
		}

		void GraphicEQ::InitMatrix(const Coefficients& fc)
		{
			/*auto idxL = std::lower_bound(thirdOctBands.begin(), thirdOctBands.end(), fc[0]);
			auto idxH = std::upper_bound(thirdOctBands.begin(), thirdOctBands.end(), fc[numFilters - 1]);

			for (int i = 0; i < 1; i++)
			{
				if (idxL != thirdOctBands.begin())
					idxL--;
				if (idxH != thirdOctBands.end())
					idxH++;
			}

			std::vector<Real> f = std::vector<Real>(idxL, idxH);



			Real fm;
			std::vector<int> counter = std::vector<int>(numFilters, 0);
			std::vector<Real> fidx = std::vector<Real>(f.size(), 0.0);
			for (int i = 0; i < numFilters - 1; i++)
			{
				fm = fc[i] * sqrt(fc[i + 1] / fc[i]);
				for (int j = 0; j < f.size(); j++)
				{
					if (f[j] > fm)
						fidx[j] = i + 1;
				}
			}

			for (auto& i : fidx)
				counter[i]++;*/


			std::vector<Real> f = std::vector<Real>(numFilters, 0.0);;

			f[0] = fc[0] / 2.0;
			for (int i = 1; i < numFilters - 1; i++)
				f[i] = fc[i - 1];
			f[numFilters - 1] = 2.0 * fc[numFilters - 3];

			Real pdb = 6.0;
			Real p = pow(10.0, pdb / 20.0);

			std::vector<Real> out = std::vector<Real>(f.size(), 0.0);
			int j = 0;

			lowShelf.UpdateGain(p);
			out = lowShelf.GetFrequencyResponse(f);
			lowShelf.UpdateGain(0.0);

			for (int i = 0; i < out.size(); i++)
				mat.IncreaseEntry(out[i], j, i);

			j++;

			for (PeakingFilter& filter : peakingFilters)
			{
				filter.UpdateGain(p);
				out = filter.GetFrequencyResponse(f);
				filter.UpdateGain(1.0);

				/*for (int i = 0; i < out.size(); i++)
					mat.IncreaseEntry(out[i] / counter[fidx[i]], j, fidx[i]);*/

				for (int i = 0; i < out.size(); i++)
					mat.IncreaseEntry(out[i], j, i);
				j++;
			}

			highShelf.UpdateGain(p);
			out = highShelf.GetFrequencyResponse(f);
			highShelf.UpdateGain(0.0);

			for (int i = 0; i < out.size(); i++)
				mat.IncreaseEntry(out[i], j, i);

			mat.Inverse();
			mat *= pdb;
		}

		void GraphicEQ::InitParameters(const Coefficients& gain)
		{
			SetGain(gain);
			currentGain = targetGain;
			equal = true;
			UpdateParameters();
		}

		void GraphicEQ::SetGain(const Coefficients& gain)
		{
			if (gain == lastInput)
				return;

			lastInput = gain;
			if (gain == 0)
			{
				inputGain.Reset();
				targetGain[0] = 0;
			}
			else
			{
				// when dB is used here. Factors of 20 are cancelled out.
				Real g = (gain[0] + gain[1]) / 2.0;
				dbGain.AddEntry(std::max(g, EPS), 0); // Prevent log10(0)

				for (int i = 1; i < numFilters - 1; i++)
					dbGain.AddEntry(std::max(gain[i - 1], EPS), i); // Prevent log10(0)

				g = (gain[numFilters - 4] + gain[numFilters - 3]) / 2.0;
				dbGain.AddEntry(std::max(g, EPS), numFilters - 1); // Prevent log10(0)

				dbGain.Log10();
				Real meandBGain = dbGain.Sum() / dbGain.Cols();
				targetGain[0] = Pow10(meandBGain); // 10 ^ mean(dbGain);
				dbGain -= meandBGain; // dbGain - mean(dbGain);

				Mulitply(inputGain, dbGain, mat);
				inputGain.Pow10();
			}
			for (int i = 0; i < numFilters; i++)
				targetGain[i + 1] = inputGain.GetEntry(i);
		}

		void GraphicEQ::UpdateParameters()
		{
			int i = 1;

			lowShelf.UpdateGain(currentGain[i]);
			i++;

			for (PeakingFilter& filter : peakingFilters)
			{
				filter.UpdateGain(currentGain[i]);
				i++;
			}

			highShelf.UpdateGain(currentGain[i]);

			if (currentGain[0] == 0)
				valid = false;
			else
				valid = true;
		}

		Real GraphicEQ::GetOutput(const Real input)
		{
			if (valid)
			{
				Real out = input;
				out = lowShelf.GetOutput(out);
				for (PeakingFilter& filter : peakingFilters)
					out = filter.GetOutput(out);
				out = highShelf.GetOutput(out);
				out *= currentGain[0];
				return out;
			}
			else
				return 0.0;
		}

		void GraphicEQ::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor)
		{
			if (equal)
			{
				for (int i = 0; i < numFrames; i++)
					outBuffer[i] = GetOutput(inBuffer[i]);
			}
			else if (Equals(currentGain, targetGain))
			{
				currentGain = targetGain;
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
					Lerp(currentGain, targetGain, lerpFactor);
					UpdateParameters();
				}
			}
		}
	}
}