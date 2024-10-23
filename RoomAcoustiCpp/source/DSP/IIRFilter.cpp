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
		////////////////////////////////////////

		Real IIRFilter::GetOutput(const Real input)
		{
			FlushDenormals();

			Real v = input;
			Real output = 0.0;
			for (size_t i = 1; i <= order; ++i)
			{
				v -= y[i - 1] * a[i];
				output += y[i - 1] * b[i];
			}

			for (size_t i = order; i >= 1; --i)
				y[i] = y[i - 1];

			y[0] = v;
			output += v * b[0];

			NoFlushDenormals();

			return output;
		}

		std::vector<Real> IIRFilter::GetFrequencyResponse(const std::vector<Real>& frequencies) const
		{
			Real omega;
			Complex e;
			std::vector<Real> magnitudes(frequencies.size(), 0.0);
			for (int i = 0; i < frequencies.size(); i++)
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

		////////////////////////////////////////

		Real IIRFilter2::GetOutput(const Real input)
		{
			FlushDenormals();

			Real v = input;
			Real output = 0.0;

			v -= y0 * a1;
			output += y0 * b1;

			v -= y1 * a2;
			output += y1 * b2;

			y1 = y0;
			y2 = y1;
			y0 = v;

			output += v * b0;

			NoFlushDenormals();

			return output;
		}

		std::vector<Real> IIRFilter2::GetFrequencyResponse(const std::vector<Real>& frequencies) const
		{
			Real omega;
			Complex e;
			std::vector<Real> magnitudes(frequencies.size(), 0.0);
			for (int i = 0; i < frequencies.size(); i++)
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

		////////////////////////////////////////

		Real IIRFilter1::GetOutput(const Real input)
		{
			FlushDenormals();

			Real v = input;
			Real output = 0.0;

			v -= y0 * a1;
			output += y0 * b1;

			y1 = y0;
			y0 = v;

			output += v * b0;

			NoFlushDenormals();

			return output;
		}

		std::vector<Real> IIRFilter1::GetFrequencyResponse(const std::vector<Real>& frequencies) const
		{
			Real omega;
			Complex e;
			std::vector<Real> magnitudes(frequencies.size(), 0.0);
			for (int i = 0; i < frequencies.size(); i++)
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

		////////////////////////////////////////

		void HighShelf::UpdateParameters(const Real fc, const Real g)
		{
			Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			Real sqrtG = sqrt(g);

			Real store = omega / sqrtG;
			a0 = 1.0 / (1.0 + store); // a[0] isn't used in GetOutput
			a1 = (1 - store) * a0;

			store = omega * sqrtG;
			b0 = (1 + store) * a0;
			b1 = (1 - store) * a0;
		}

		////////////////////////////////////////

		void LowPass::UpdateParameters(const Real fc)
		{
			Real K = PI_2 * fc * T;

			a0 = 1.0 / (K + 2.0); // a[0] isn't used in GetOutput
			a1 = (K - 2.0) * a0;

			b0 = K * a0;
			b1 = K * a0;
		}

		////////////////////////////////////////

		void PeakHighShelf::SetParameters(const Real fc, const Real Q)
		{
			Real omega = PI_2 * fc * T;
			cosOmega = cos(omega);
			alpha = sin(omega) / Q; // sin(omega) / (2 * Q) (factor of two cancelled out)
		}

		////////////////////////////////////////

		void PeakHighShelf::UpdateGain(const Real g)
		{
			Real A = sqrt(g);
			Real v1 = A + 1.0;
			Real v2 = A - 1.0;
			Real v3 = v1 * cosOmega;
			Real v4 = v2 * cosOmega;
			Real v5 = sqrt(A) * alpha; // 2 * sqrt(A) * alpha

			a0 = 1.0 / (v1 - v4 + v5); // a[0] isn't used in GetOutput
			a1 = (2.0 * (v2 - v3)) * a0;
			a2 = (v1 - v4 - v5) * a0;

			b0 = A * (v1 + v4 + v5) * a0;
			b1 = -2.0 * A * (v2 + v3) * a0;
			b2 = A * (v1 + v4 - v5) * a0;
		}

		////////////////////////////////////////

		void PeakLowShelf::SetParameters(const Real fc, const Real Q)
		{
			Real omega = PI_2 * fc * T;
			cosOmega = cos(omega);
			alpha = sin(omega) / Q; // sin(omega) / (2 * Q) (factor of two cancelled out)
		}

		////////////////////////////////////////

		void PeakLowShelf::UpdateGain(const Real g)
		{
			Real A = sqrt(g);
			Real v1 = A + 1.0;
			Real v2 = A - 1.0;
			Real v3 = v1 * cosOmega;
			Real v4 = v2 * cosOmega;
			Real v5 = sqrt(A) * alpha; // 2 * sqrt(A) * alpha

			a0 = 1.0 / (v1 + v4 + v5); // a[0] isn't used in GetOutput
			a1 = (-2.0 * (v2 + v3)) * a0;
			a2 = (v1 + v4 - v5) * a0;

			b0 = A * (v1 - v4 + v5) * a0;
			b1 = 2.0 * A * (v2 - v3) * a0;
			b2 = A * (v1 - v4 - v5) * a0;
		}

		////////////////////////////////////////

		void PeakingFilter::SetParameters(const Real fc, const Real Q)
		{
			Real omega = PI_2 * fc * T;
			cosOmega = -2.0 * cos(omega);
			alpha = sin(omega) / (2.0 * Q);
		}

		////////////////////////////////////////

		void PeakingFilter::UpdateGain(const Real g)
		{
			Real A = sqrt(g);
			Real v1 = alpha * A;
			Real v2 = alpha / A;

			a0 = 1.0 / (1 + v2); // a[0] isn't used in GetOutput
			a1 = cosOmega * a0;
			a2 = (1 - v2) * a0;

			b0 = (1 + v1) * a0;
			b1 = a1;
			b2 = (1 - v1) * a0;
		}

		////////////////////////////////////////

		void ZPKFilter::UpdateParameters(const Coefficients& zpk) // z[0 - 1], p[2 - 3], k[4]
		{
			b0 = zpk[4];
			b1 = -zpk[4] * (zpk[0] + zpk[1]);
			b2 = zpk[4] * zpk[0] * zpk[1];

			a1 = -(zpk[2] + zpk[3]);
			a2 = zpk[2] * zpk[3];
		}

		////////////////////////////////////////

		void PassFilter::UpdateLowPass(const Real fc)
		{
			Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			Real omega_sq = omega * omega;

			a0 = 1.0 / (1.0 + SQRT_2 * omega + omega_sq); // a[0] isn't used in GetOutput
			a1 = (2.0 - 2.0 * omega_sq) * a0;
			a2 = (1.0 - SQRT_2 * omega + omega_sq) * a0;

			b0 = a0;
			b1 = 2.0 * a0;
			b2 = a0;
		}

		////////////////////////////////////////

		void PassFilter::UpdateHighPass(const Real fc)
		{
			Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			Real omega_sq = omega * omega;

			a0 = 1.0 / (1.0 + SQRT_2 * omega + omega_sq); // a[0] isn't used in GetOutput
			a1 = (2.0 - 2.0 * omega_sq) * a0;
			a2 = (1.0 - SQRT_2 * omega + omega_sq) * a0;

			b0 = omega_sq * a0;
			b1 = -2.0 * omega_sq * a0;
			b2 = omega_sq * a0;
		}

		////////////////////////////////////////

		
	}
}