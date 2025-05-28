/*
* @class IIRFilter
*
* @brief Declaration of base IIRFilter class and derived HighShelf, LowPass, ZPKFilter, BandFilter and PassFilter classes
*
*/

// C++ headers
#if defined(_WINDOWS)
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#endif
#include <cmath>
#include <vector>

// DSP headers
#include "DSP/IIRFilter.h"

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"
#include "Common/Complex.h"
#include "DSP/Interpolate.h"

namespace RAC
{
	using namespace Common;
	namespace DSP
	{
		//////////////////// IIRFilter ////////////////////

		////////////////////////////////////////

		Real IIRFilter::GetOutput(const Real input, const Real lerpFactor)
		{
			if (!initialised.load())
				return 0.0;

			if (!parametersEqual.load())
				InterpolateParameters(lerpFactor);

			if (clearBuffers.load())
			{
				y.ResetBuffer();
				clearBuffers.store(false);
			}

			Real v = input;
			Real output = 0.0;
			for (int i = 1; i <= order; ++i)
			{
				v -= y[i - 1] * a[i];
				output += y[i - 1] * b[i];
			}

			for (int i = order; i >= 1; --i)
				y[i] = y[i - 1];

			y[0] = v;
			output += v * b[0];

			return output;
		}

		////////////////////////////////////////

		Coefficients IIRFilter::GetFrequencyResponse(const Coefficients& frequencies) const
		{
			Real omega;
			Complex e;
			std::vector<Real> magnitudes(frequencies.Length(), 0.0);
			for (int i = 0; i < frequencies.Length(); i++)
			{
				omega = PI_2 * frequencies[i] * T;
				Complex num = b[0];
				Complex den = 1.0;

				for (int j = 1; j <= order; j++)
				{
					e = std::exp(-j * imUnit * omega);
					num += b[j] * e;
					den += a[j] * e;
				}

				magnitudes[i] = 20 * std::log10(std::abs(num / den));
			}
			return magnitudes;
		}

		//////////////////// IIRFIlter2 ////////////////////

		////////////////////////////////////////

		Real IIRFilter2::GetOutput(const Real input, const Real lerpFactor)
		{
			if (!initialised.load())
				return 0.0;

			if (!parametersEqual.load())
				InterpolateParameters(lerpFactor);

			if (clearBuffers.load())
			{
				y0 = 0.0; y1 = 0.0;
				clearBuffers.store(false);
			}

			Real v = input;
			Real output = 0.0;

			v -= y0 * a1;
			output += y0 * b1;

			v -= y1 * a2;
			output += y1 * b2;

			y1 = y0;
			y0 = v;

			output += v * b0;

			return output;
		}

		////////////////////////////////////////

		Coefficients IIRFilter2::GetFrequencyResponse(const Coefficients& frequencies) const
		{
			Real omega;
			Complex e;
			Coefficients magnitudes(frequencies.Length(), 0.0);
			for (int i = 0; i < frequencies.Length(); i++)
			{
				omega = PI_2 * frequencies[i] * T;
				Complex num = b0;
				Complex den = 1.0;

				e = std::exp(-1 * imUnit * omega);
				num += b1 * e;
				den += a1 * e;

				e = std::exp(-2 * imUnit * omega);
				num += b2 * e;
				den += a2 * e;

				magnitudes[i] = 20 * std::log10(std::abs(num / den));
			}
			return magnitudes;
		}

		//////////////////// IIRFIlter1 ////////////////////

		////////////////////////////////////////

		Real IIRFilter1::GetOutput(const Real input, const Real lerpFactor)
		{
			if (!initialised.load())
				return 0.0;

			if (!parametersEqual.load())
				InterpolateParameters(lerpFactor);

			if (clearBuffers.load())
			{
				y0 = 0.0;
				clearBuffers.store(false);
			}

			Real v = input;
			Real output = 0.0;

			v -= y0 * a1;
			output += y0 * b1;

			y0 = v;

			output += v * b0;

			return output;
		}

		////////////////////////////////////////

		Coefficients IIRFilter1::GetFrequencyResponse(const Coefficients& frequencies) const
		{
			Real omega;
			Complex e;
			std::vector<Real> magnitudes(frequencies.Length(), 0.0);
			for (int i = 0; i < frequencies.Length(); i++)
			{
				omega = PI_2 * frequencies[i] * T;
				Complex num = b0;
				Complex den = 1.0;

				e = std::exp(-1 * imUnit * omega);
				num += b1 * e;
				den += a1 * e;

				magnitudes[i] = 20 * std::log10(std::abs(num / den));
			}
			return magnitudes;
		}

		//////////////////// HighSelf ////////////////////

		////////////////////////////////////////

		void HighShelf::InterpolateParameters(const Real lerpFactor)
		{
			parametersEqual.store(true); // Prevents issues in case targetFc/Gain updated during this function call
			const Real fc = targetFc.load();
			const Real gain = targetGain.load();
			currentFc = Lerp(currentFc, fc, lerpFactor);
			currentGain = Lerp(currentGain, gain, lerpFactor);
			if (Equals(currentFc, fc) && Equals(currentGain, gain))
			{
				currentFc = fc;
				currentGain = gain;
			}
			else
				parametersEqual.store(false);
			UpdateCoefficients(currentFc, currentGain);
		}

		////////////////////////////////////////

		void HighShelf::UpdateCoefficients(const Real fc, const Real gain)
		{
			const Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			const Real sqrtG = sqrt(gain);

			const Real v1 = omega / sqrtG;
			const Real v2 = omega * sqrtG;

			a0 = 1.0 / (1.0 + v1); // a0 isn't used in GetOutput
			a1 = (1 - v1) * a0;

			b0 = (1 + v2) * a0;
			b1 = (1 - v2) * a0;
		}

		//////////////////// LowPass1 ////////////////////

		////////////////////////////////////////

		void LowPass1::InterpolateParameters(const Real lerpFactor)
		{
			parametersEqual.store(true); // Prevents issues in case targetFc updated during this function call
			const Real fc = targetFc.load();
			currentFc = Lerp(currentFc, fc, lerpFactor);
			if (Equals(currentFc, fc))
				currentFc = fc;
			else
				parametersEqual.store(false);
			UpdateCoefficients(currentFc);
		}

		////////////////////////////////////////

		void LowPass1::UpdateCoefficients(const Real fc)
		{
			const Real K = PI_2 * fc * T;

			a0 = 1.0 / (K + 2.0); // a0 isn't used in GetOutput
			a1 = (K - 2.0) * a0;

			b0 = K * a0;
			b1 = K * a0;
		}

		//////////////////// IIRFilter2Param1 ////////////////////

		////////////////////////////////////////

		void IIRFilter2Param1::InterpolateParameters(const Real lerpFactor)
		{
			parametersEqual.store(true); // Prevents issues in case target updated during this function call
			const Real parameter = target.load();
			current = Lerp(current, parameter, lerpFactor);
			if (Equals(current, parameter))
				current = parameter;
			else
				parametersEqual.store(false);
			UpdateCoefficients(current);
		}

		//////////////////// PeakHighSelf ////////////////////

		////////////////////////////////////////

		void PeakHighShelf::UpdateCoefficients(const Real gain)
		{
			assert(gain > 0.0);

			const Real A = sqrt(gain);
			const Real v1 = A + 1.0;
			const Real v2 = A - 1.0;
			const Real v3 = v1 * cosOmega;
			const Real v4 = v2 * cosOmega;
			const Real v5 = sqrt(A) * alpha; // 2 * sqrt(A) * alpha

			a0 = 1.0 / (v1 - v4 + v5); // a0 isn't used in GetOutput
			a1 = (2.0 * (v2 - v3)) * a0;
			a2 = (v1 - v4 - v5) * a0;

			b0 = A * (v1 + v4 + v5) * a0;
			b1 = -2.0 * A * (v2 + v3) * a0;
			b2 = A * (v1 + v4 - v5) * a0;
		}

		//////////////////// PeakLowShelf ////////////////////

		////////////////////////////////////////

		void PeakLowShelf::UpdateCoefficients(const Real gain)
		{
			assert(gain > 0.0);

			const Real A = sqrt(gain);
			const Real v1 = A + 1.0;
			const Real v2 = A - 1.0;
			const Real v3 = v1 * cosOmega;
			const Real v4 = v2 * cosOmega;
			const Real v5 = sqrt(A) * alpha; // 2 * sqrt(A) * alpha

			a0 = 1.0 / (v1 + v4 + v5); // a0 isn't used in GetOutput
			a1 = (-2.0 * (v2 + v3)) * a0;
			a2 = (v1 + v4 - v5) * a0;

			b0 = A * (v1 - v4 + v5) * a0;
			b1 = 2.0 * A * (v2 - v3) * a0;
			b2 = A * (v1 - v4 - v5) * a0;
		}

		//////////////////// PeakingFilter ////////////////////

		////////////////////////////////////////

		void PeakingFilter::UpdateCoefficients(const Real gain)
		{
			assert(gain > 0.0);

			const Real A = sqrt(gain);
			const Real v1 = alpha * A;
			const Real v2 = alpha / A;

			a0 = 1.0 / (1.0 + v2); // a0 isn't used in GetOutput
			a1 = cosOmega * a0;
			a2 = (1.0 - v2) * a0;

			b0 = (1.0 + v1) * a0;
			b1 = a1;
			b2 = (1.0 - v1) * a0;
		}

		//////////////////// ZPKFilter ////////////////////

		ReleasePool ZPKFilter::releasePool;

		////////////////////////////////////////

		void ZPKFilter::SetTargetParameters(const Coefficients& zpk)
		{
			const std::shared_ptr<Coefficients> zpkCopy = std::make_shared<Coefficients>(zpk);

			releasePool.Add(zpkCopy);

			targetZPK.store(zpkCopy);
			parametersEqual.store(false);
		}

		////////////////////////////////////////

		void ZPKFilter::UpdateCoefficients(const Coefficients& zpk) // z[0 - 1], p[2 - 3], k[4]
		{
			b0 = zpk[4];
			b1 = -zpk[4] * (zpk[0] + zpk[1]);
			b2 = zpk[4] * zpk[0] * zpk[1];

			a1 = -(zpk[2] + zpk[3]);
			a2 = zpk[2] * zpk[3];
		}

		////////////////////////////////////////

		void ZPKFilter::InterpolateParameters(const Real lerpFactor)
		{
			parametersEqual.store(true); // Prevents issues in case targetZPK updated during this function call
			std::shared_ptr<const Coefficients> zpk = targetZPK.load();
			Lerp(currentZPK, *zpk, lerpFactor);
			if (Equals(currentZPK, *zpk))
				currentZPK = *zpk;
			else
				parametersEqual.store(false);
			UpdateCoefficients(currentZPK);
		}

		//////////////////// LowPass ////////////////////

		////////////////////////////////////////

		void LowPass::UpdateCoefficients(const Real fc)
		{
			const Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			const Real omega_sq = omega * omega;

			a0 = 1.0 / (1.0 + SQRT_2 * omega + omega_sq); // a[0] isn't used in GetOutput
			a1 = (2.0 - 2.0 * omega_sq) * a0;
			a2 = (1.0 - SQRT_2 * omega + omega_sq) * a0;

			b0 = a0;
			b1 = 2.0 * a0;
			b2 = a0;
		}

		//////////////////// HighPass ////////////////////

		////////////////////////////////////////

		void HighPass::UpdateCoefficients(const Real fc)
		{
			const Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			const Real omega_sq = omega * omega;

			a0 = 1.0 / (1.0 + SQRT_2 * omega + omega_sq); // a[0] isn't used in GetOutput
			a1 = (2.0 - 2.0 * omega_sq) * a0;
			a2 = (1.0 - SQRT_2 * omega + omega_sq) * a0;

			b0 = omega_sq * a0;
			b1 = -2.0 * omega_sq * a0;
			b2 = omega_sq * a0;
		}	
	}
}