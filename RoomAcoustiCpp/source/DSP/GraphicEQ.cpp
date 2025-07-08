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

		GraphicEQ::GraphicEQ(const Coefficients<>& gain, const Coefficients<>& fc, const Real Q, const int sampleRate) :
			numFilters(gain.Length() + 2), filterResponseMatrix(numFilters, numFilters), previousInput(gain)
		{ 
			assert(gain.Length() == fc.Length());

			Coefficients f = CreateFrequencyVector(fc);
			InitMatrix(f, Q, sampleRate);
			auto targetGains = CalculateGains(gain);

			// Increasing the low shelf frequency by SQRT_2 creates a smoother response at low frequencies
			lowShelf = std::make_unique<PeakLowShelf>(f[0] * SQRT_2, targetGains.first[0], Q, sampleRate);
			for (int i = 1; i < numFilters - 1; i++)
				peakingFilters.emplace_back(std::make_unique<PeakingFilter>(f[i], targetGains.first[i], Q, sampleRate));
			// (same as fc[numFilters - 1] * 2 / SQRT_2 ) Decreasing the high shelf frequency by SQRT_2 creates a smoother response at high frequencies
			highShelf = std::make_unique<PeakHighShelf>(std::min(f[numFilters - 2] * SQRT_2, 20000.0), targetGains.first[numFilters - 1], Q, sampleRate);

			targetGain.store(targetGains.second, std::memory_order_release);
			currentGain = targetGains.second;

			gainsEqual.store(true, std::memory_order_release);
			initialised.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		bool GraphicEQ::SetTargetGains(const Coefficients<>& gains)
		{
			assert(gains.Length() == peakingFilters.size());

			if (gains == previousInput) // No change in gains
			{
				if (gainsEqual.load(std::memory_order_acquire) && gains <= 0.0) // Gains currently zero
					return true;
				return false;
			}
			previousInput = gains;

			const auto targetGains = CalculateGains(gains);
			targetGain.store(targetGains.second, std::memory_order_release);
			gainsEqual.store(false, std::memory_order_release);
			lowShelf->SetTargetGain(targetGains.first[0]);
			for (int i = 1; i < numFilters - 1; i++)
				peakingFilters[i - 1]->SetTargetGain(targetGains.first[i]);
			highShelf->SetTargetGain(targetGains.first[numFilters - 1]);
			return false;
		}

		////////////////////////////////////////

		std::pair<Rowvec, Real> GraphicEQ::CalculateGains(const Coefficients<>& gains) const
		{
			assert(gains.Length() + 2 == numFilters);

			Rowvec inputGains(std::vector<Real>(numFilters, 1.0));
			if (gains <= 0.0)
				return std::make_pair(inputGains, 0.0);

			if (gains.Length() == 1)
			{
				inputGains[0] = gains[0];
				inputGains[1] = gains[0];
				inputGains[2] = gains[0];
			}
			else
			{
				inputGains[0] = (gains[0] + gains[1]) / 2.0; // Low shelf gain
				for (int i = 1; i < numFilters - 1; i++)
					inputGains[i] = gains[i - 1]; // Peaking filter gains
				inputGains[numFilters - 1] = (gains[numFilters - 4] + gains[numFilters - 3]) / 2.0; // High shelf gain
			}
			inputGains.Max(EPS); // Prevent log10(0)
			inputGains.Log10();

			// Remove average filter gain. Improves DC and nyquist response, and simplifies interpolation if filter shape is consistent
			Real meandBGain = inputGains.Mean();
			inputGains -= meandBGain;

			inputGains = inputGains * filterResponseMatrix;
			inputGains.Pow10();

			return std::make_pair(inputGains, Pow10(meandBGain));
		}

		////////////////////////////////////////

		Coefficients<> GraphicEQ::CreateFrequencyVector(const Coefficients<>& fc) const
		{
			Coefficients f(numFilters);
			f[0] = std::max(fc[0] / 2, 20.0);
			for (int i = 1; i < numFilters - 1; i++)
				f[i] = fc[i - 1];
			f[numFilters - 1] = std::min(fc[numFilters - 3] * 2.0, 20000.0);
			return f;
		}

		////////////////////////////////////////

		void GraphicEQ::InitMatrix(const Coefficients<>& fc, const Real Q, const Real fs)
		{
			assert(fc.Length() == numFilters);

			Real pdb = 6.0;
			Real p = pow(10.0, pdb / 20.0);

			Coefficients out(numFilters);

			const PeakLowShelf tempLowShelf(fc[0] * SQRT_2, p, Q, fs); // Times SQRT_2. See constructor
			out = tempLowShelf.GetFrequencyResponse(fc);

			for (int i = 0; i < numFilters; i++)
				filterResponseMatrix[0][i] += out[i];

			for (int j = 1; j < numFilters - 1; j++)
			{
				const PeakingFilter tempPeakingFilter(fc[j], p, Q, fs);
				out = tempPeakingFilter.GetFrequencyResponse(fc);

				for (int i = 0; i < out.Length(); i++)
					filterResponseMatrix[j][i] += out[i];
			}

			const PeakHighShelf tempHighShelf(std::min(fc[numFilters - 2] * SQRT_2, 20000.0), p, Q, fs); // Times SQRT_2. See constructor
			out = tempHighShelf.GetFrequencyResponse(fc);

			for (int i = 0; i < out.Length(); i++)
				filterResponseMatrix[numFilters - 1][i] += out[i];

			filterResponseMatrix.Inverse();
			filterResponseMatrix *= pdb;
		}

		////////////////////////////////////////

		Real GraphicEQ::GetOutput(const Real input, const Real lerpFactor)
		{
			if (!initialised.load(std::memory_order_acquire))
				return 0.0;

			if (!gainsEqual.load(std::memory_order_acquire))
				InterpolateGain(lerpFactor);

			Real out = lowShelf->GetOutput(input, lerpFactor);
			for (auto& filter : peakingFilters)
				out = filter->GetOutput(out, lerpFactor);
			out = highShelf->GetOutput(out, lerpFactor);
			out *= currentGain;
			return out;
		}

		////////////////////////////////////////

		void GraphicEQ::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor)
		{
			if (!initialised.load(std::memory_order_acquire))
			{
				outBuffer.Reset();
				return;
			}

			if (currentGain == 0.0 && gainsEqual.load(std::memory_order_acquire))
			{
				outBuffer.Reset();
				return;
			}

			FlushDenormals();
			for (int i = 0; i < inBuffer.Length(); i++)
				outBuffer[i] = GetOutput(inBuffer[i], lerpFactor);
			NoFlushDenormals();
		}

		////////////////////////////////////////

		void GraphicEQ::InterpolateGain(const Real lerpFactor)
		{
			gainsEqual.store(true, std::memory_order_release); // Prevents issues in case targetGain updated during this function call
			const Real gain = targetGain.load(std::memory_order_acquire);
			currentGain = Lerp(currentGain, gain, lerpFactor);
			if (Equals(currentGain, gain))
				currentGain = gain;
			else
				gainsEqual.store(false, std::memory_order_release);
		}
	}
}