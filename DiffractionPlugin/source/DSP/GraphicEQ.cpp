
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

namespace UIE
{
	namespace DSP
	{
		GraphicEQ::GraphicEQ(const Coefficients& fc, const Real& Q, const int& sampleRate) : numFilters(fc.Length()), lowShelf(fc[0], Q, sampleRate), highShelf(fc[numFilters - 1], Q, sampleRate),
			targetGain(fc.Length(), 1.0), currentGain(fc.Length(), 1.0), dbGain(fc.Length()), inputGain(fc.Length()), mat(fc.Length(), fc.Length()), valid(false)
		{
			InitFilters(fc, Q, sampleRate);
			InitMatrix(fc);
		}

		GraphicEQ::GraphicEQ(const Coefficients& gain, const Coefficients& fc, const Real& Q, const int& sampleRate) : numFilters(fc.Length()), lowShelf(fc[0], Q, sampleRate), highShelf(fc[numFilters - 1], Q, sampleRate),
			targetGain(fc.Length()), currentGain(fc.Length()), dbGain(fc.Length()), inputGain(fc.Length()), mat(fc.Length(), fc.Length()), valid(false)
		{
			InitFilters(fc, Q, sampleRate);
			InitMatrix(fc);
		}

		void GraphicEQ::InitFilters(const Coefficients& fc, const Real& Q, const int& sampleRate)
		{
			Real shelfFc = fc[0] * sqrt(fc[numFilters - 1] / fc[0]);
			lowShelf = PeakLowShelf(shelfFc, Q, sampleRate);
			highShelf = PeakHighShelf(shelfFc, Q, sampleRate);
			for (int i = 1; i < numFilters - 1; i++)
				peakingFilters.push_back(PeakingFilter(fc[i], Q, sampleRate));
		}

		void GraphicEQ::InitMatrix(const Coefficients& fc)
		{
			auto idxL = std::lower_bound(thirdOctBands.begin(), thirdOctBands.end(), fc[0]);
			auto idxH = std::upper_bound(thirdOctBands.begin(), thirdOctBands.end(), fc[numFilters - 1]);

			for (int i = 0; i < 4; i++)
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
				counter[i]++;

			Real pdb = 6.0;
			Real p = pow(10.0, pdb / 20.0);

			lowShelf.UpdateGain(p);
			std::vector<Real> out = lowShelf.GetFrequencyResponse(f);
			lowShelf.UpdateGain(1.0);

			for (int i = 0; i < out.size(); i++)
				mat.IncreaseEntry(out[i] / counter[fidx[i]], 0, fidx[i]);

			highShelf.UpdateGain(p);
			out = highShelf.GetFrequencyResponse(f);
			highShelf.UpdateGain(1.0);

			for (int i = 0; i < out.size(); i++)
				mat.IncreaseEntry(out[i] / counter[fidx[i]], numFilters - 1, fidx[i]);

			int j = 1;
			for (PeakingFilter& filter : peakingFilters)
			{
				filter.UpdateGain(p);
				out = filter.GetFrequencyResponse(f);
				filter.UpdateGain(1.0);

				for (int i = 0; i < out.size(); i++)
					mat.IncreaseEntry(out[i] / counter[fidx[i]], j, fidx[i]);
				j++;
			}
				
			mat.Inverse();
			mat *= pdb;
		}

		void GraphicEQ::InitParameters(const Coefficients& g)
		{
			currentGain = g;
			targetGain = g;
			UpdateParameters();
		}

		void GraphicEQ::UpdateParameters()
		{
			if (currentGain == 0)
				valid = false;
			else
				valid = true;
			
			if (valid)
			{
				for (int i = 0; i < numFilters; i++)
					dbGain.AddEntry(std::max(currentGain[i], EPS), i); // Prevent log10(0)

				dbGain.Log10();
				Mulitply(inputGain, dbGain, mat);
				inputGain.Pow10();


				lowShelf.UpdateGain(inputGain.GetEntry(0));
				highShelf.UpdateGain(inputGain.GetEntry(numFilters - 1));

				int i = 0;
				for (PeakingFilter& filter : peakingFilters)
				{
					filter.UpdateGain(inputGain.GetEntry(i + 1));
					i++;
				}
			}
		}

		Real GraphicEQ::GetOutput(const Real& input)
		{
			if (valid)
			{
				out = lowShelf.GetOutput(input);
				for (PeakingFilter& filter : peakingFilters)
					out = filter.GetOutput(out);
				out = highShelf.GetOutput(out);
				return out;
			}
			else
				return 0.0;
		}

		void GraphicEQ::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor)
		{
			if (currentGain == targetGain)
			{
				for (int i = 0; i < numFrames; i++)
					outBuffer[i] = GetOutput(inBuffer[i]);
			}
			else
			{
				for (int i = 0; i < numFrames; i++)
				{
					outBuffer[i] = GetOutput(inBuffer[i]);
					BeginLerp();
					Lerp(currentGain, targetGain, lerpFactor);
					EndLerp();
					BeginFIR();
					UpdateParameters();
					EndFIR();
				}
			}
		}
	}
}