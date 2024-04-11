/*
* @class ParametricEQ, BandFilter, BandSection
*
* @brief Declaration of ParametricEQ, BandFilter, BandSection classes
*
*/

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"
#include "Common/Coefficients.h"

// DSP headers
#include "DSP/ParametricEQ.h"
#include "DSP/Interpolate.h"

#include "Unity/UnityInterface.h"

namespace RAC
{
	namespace DSP
	{
		BandSection::BandSection(const int idx, const int order, const bool isLowBand, const int sampleRate)
			: IIRFilter(2, sampleRate), m(idx), M(order)
		{
			a[0] = 1.0;
			SetUpdatePointer(isLowBand);
		}

		BandSection::BandSection(const Real fb, const Real g, const int idx, const int order, const bool isLowBand, const int sampleRate)
			: IIRFilter(2, sampleRate), m(idx), M(order)
		{
			a[0] = 1.0;
			SetUpdatePointer(isLowBand);
			UpdateParameters(fb, g);
		};

		void BandSection::UpdateLowBand(const Real fb, const Real g)
		{
			Real K = tan(PI_1 * fb * T);
			Real K_2 = 2.0 * K;
			Real K_sq = K * K;
			Real K_sq_2 = 2.0 * K_sq;
			Real V = pow(g, 1.0 / M) - 1.0;
			Real VK = V * K;
			Real VK_2 = 2.0 * VK;
			Real VK_sq = VK * VK;

			Real alpha = (0.5 - (2.0 * m - 1.0) / (2.0 * M)) * PI_1;
			Real cm = cos(alpha);
			Real K2cm = K_2 * cm;
			a[0] = 1.0 / (1.0 + K2cm + K_sq); // a[0] not used in GetOutput
			a[1] = (K_sq_2 - 2.0) * a[0];
			a[2] = (1.0 - K2cm + K_sq) * a[0];

			b[0] = 1.0 + (VK_2 * (K + cm) + VK_sq) * a[0];
			b[1] = a[1] + (VK_2 * K_2 + VK_sq * 2.0) * a[0];
			b[2] = a[2] + (VK_2 * (K - cm) + VK_sq) * a[0];
		}

		void BandSection::UpdateHighBand(const Real fb, const Real g)
		{
			Real K = tan(PI_1 * fb * T);
			Real K_2 = 2.0 * K;
			Real K_sq = K * K;
			Real K_sq_2 = 2.0 * K_sq;
			Real M_2 = 2.0 * M;
			Real V = pow(g, 1.0 / static_cast<Real>(M)) - 1.0;
			Real VK = V * K;
			Real VK_2 = 2.0 * VK;
			Real VK_sq = VK * VK;

			Real alpha = (0.5 - (2.0 * static_cast<Real>(m) - 1.0) / M_2) * PI_1;
			Real cm = cos(alpha);
			Real K2cm = K_2 * cm;
			a[0] = 1.0 / (1.0 + K2cm + K_sq); // a[0] not used in GetOutput
			a[1] = (2.0 - K_sq_2) * a[0];
			a[2] = (1.0 - K2cm + K_sq) * a[0];

			b[0] = 1.0 + (VK_2 * (K + cm) + VK_sq) * a[0];
			b[1] = a[1] - (VK_2 * K_2 + VK_sq * 2.0) * a[0];
			b[2] = a[2] + (VK_2 * (K - cm) + VK_sq) * a[0];
		}

		BandFilter::BandFilter(const size_t order, const bool useLowBands, const Real fb, const Real g, const int sampleRate) : out(0.0)
		{
			InitSections(order, useLowBands, sampleRate);
			UpdateParameters(fb, g);
		}

		void BandFilter::InitSections(const size_t& order, const bool& useLowBands, const int fs)
		{
			size_t numSections = order / 2;
			assert(order == numSections * 2); // order must be even

			sections.reserve(numSections);
			for (int i = 0; i < numSections; i++)
				sections.push_back(BandSection(i + 1, order, useLowBands, fs));
		}

		void BandFilter::UpdateParameters(const Real fb, const Real g)
		{
			for (BandSection& section : sections)
				section.UpdateParameters(fb, g);
		}

		Real BandFilter::GetOutput(const Real input)
		{
			out = input;
			for (BandSection& section : sections)
				out = section.GetOutput(out);
			return out;
		}

		ParametricEQ::ParametricEQ(const size_t& order, const Coefficients& fc, const int sampleRate)
			: numFilters(fc.Length() - 1), fb(numFilters), mGain(0.0), out(0.0), currentGain(fc.Length()), targetGain(fc.Length()), singleGain(false)
		{
			InitBands(order, fc, sampleRate);
		}

		ParametricEQ::ParametricEQ(const Coefficients& gain, const size_t& order, const Coefficients& fc, const int sampleRate)
			: numFilters(fc.Length() - 1), fb(numFilters), out(0.0), currentGain(gain), targetGain(gain), singleGain(false)
		{
			InitBands(order, fc, sampleRate);
			UpdateParameters();
		}

		void ParametricEQ::SetTargetGain(Coefficients& gain)
		{
			for (int i = 0; i < numFilters + 1; i++)
				gain[i] = std::max(EPS, gain[i]); // Prevent division by zero
			for (int i = 0; i < numFilters; i++)
				targetGain[i] = gain[i] / gain[i + 1];
			targetGain[numFilters] = gain[numFilters];
			bool noFilter = true;
			int i = 0;
			while (noFilter && i < numFilters)
				noFilter = currentGain[i++] == 1.0;
			if (!singleGain && noFilter)
			{
				singleGain = true;
				ClearBuffers();
			}
		}

		void ParametricEQ::UpdateParameters()
		{
			for (int i = 0; i < numFilters; i++)
				filters[i].UpdateParameters(fb[i], currentGain[i]);
			mGain = currentGain[numFilters];
		}

		void ParametricEQ::UpdateParameters(const Real lerpFactor)
		{
			if (currentGain != targetGain)
			{
				Lerp(currentGain, targetGain, lerpFactor);
				UpdateParameters();
			}
		}

		Real ParametricEQ::GetOutput(const Real input)
		{
			out = input;
			for (BandFilter& filter : filters)
				out = filter.GetOutput(out);
			out *= mGain;
			return out;
		}

		void ParametricEQ::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor)
		{
			if (currentGain == targetGain)
			{
				if (singleGain)
				{
					for (int i = 0; i < numFrames; i++)
						outBuffer[i] = mGain * inBuffer[i];
				}
				else
				{
					for (int i = 0; i < numFrames; i++)
						outBuffer[i] = GetOutput(inBuffer[i]);
				}
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

		void ParametricEQ::InitBands(const size_t& order, const Coefficients& fc, int fs)
		{
			filters = std::vector<BandFilter>(numFilters, BandFilter(order, true, fs));
			for (int i = 0; i < numFilters; i++)
				fb[i] = fc[i] * sqrt(fc[i + 1] / fc[i]);
		}
	}
}
