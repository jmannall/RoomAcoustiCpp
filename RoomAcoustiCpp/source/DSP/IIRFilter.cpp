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
			for (int i = 1; i <= order; i++)
			{
				v += y[i - 1] * (-a[i]);
				output += y[i - 1] * b[i];
			}

			for (int i = order; i >= 1; i--)
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

		void HighShelf::UpdateParameters(const Real fc, const Real g)
		{
			Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			Real sqrtG = sqrt(g);

			Real store = omega / sqrtG;
			a[0] = 1.0 / (1.0 + store); // a[0] isn't used in GetOutput
			a[1] = (1 - store) * a[0];

			store = omega * sqrtG;
			b[0] = (1 + store) * a[0];
			b[1] = (1 - store) * a[0];
		}

		////////////////////////////////////////

		void LowPass::UpdateParameters(const Real fc)
		{
			Real K = PI_2 * fc * T;

			a[0] = 1.0 / (K + 2.0); // a[0] isn't used in GetOutput
			a[1] = (K - 2.0) * a[0];

			b[0] = K * a[0];
			b[1] = K * a[0];
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

			a[0] = 1.0 / (v1 - v4 + v5); // a[0] isn't used in GetOutput
			a[1] = (2.0 * (v2 - v3)) * a[0];
			a[2] = (v1 - v4 - v5) * a[0];

			b[0] = A * (v1 + v4 + v5) * a[0];
			b[1] = -2.0 * A * (v2 + v3) * a[0];
			b[2] = A * (v1 + v4 - v5) * a[0];
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

			a[0] = 1.0 / (v1 + v4 + v5); // a[0] isn't used in GetOutput
			a[1] = (-2.0 * (v2 + v3)) * a[0];
			a[2] = (v1 + v4 - v5) * a[0];

			b[0] = A * (v1 - v4 + v5) * a[0];
			b[1] = 2.0 * A * (v2 - v3) * a[0];
			b[2] = A * (v1 - v4 - v5) * a[0];
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

			a[0] = 1.0 / (1 + v2); // a[0] isn't used in GetOutput
			a[1] = cosOmega * a[0];
			a[2] = (1 - v2) * a[0];

			b[0] = (1 + v1) * a[0];
			b[1] = a[1];
			b[2] = (1 - v1) * a[0];
		}

		////////////////////////////////////////

		void ZPKFilter::UpdateParameters(const Coefficients& zpk) // z[0 - 1], p[2 - 3], k[4]
		{
			b[0] = zpk[4];
			b[1] = -zpk[4] * (zpk[0] + zpk[1]);
			b[2] = zpk[4] * zpk[0] * zpk[1];

			a[1] = -(zpk[2] + zpk[3]);
			a[2] = zpk[2] * zpk[3];
		}

		////////////////////////////////////////

		void PassFilter::UpdateLowPass(const Real fc)
		{
			Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			Real omega_sq = omega * omega;

			a[0] = 1.0 / (1.0 + SQRT_2 * omega + omega_sq); // a[0] isn't used in GetOutput
			a[1] = (2.0 - 2.0 * omega_sq) * a[0];
			a[2] = (1.0 - SQRT_2 * omega + omega_sq) * a[0];

			b[0] = a[0];
			b[1] = 2.0 * a[0];
			b[2] = a[0];
		}

		////////////////////////////////////////

		void PassFilter::UpdateHighPass(const Real fc)
		{
			Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			Real omega_sq = omega * omega;

			a[0] = 1.0 / (1.0 + SQRT_2 * omega + omega_sq); // a[0] isn't used in GetOutput
			a[1] = (2.0 - 2.0 * omega_sq) * a[0];
			a[2] = (1.0 - SQRT_2 * omega + omega_sq) * a[0];

			b[0] = omega_sq * a[0];
			b[1] = -2.0 * omega_sq * a[0];
			b[2] = omega_sq * a[0];
		}

		////////////////////////////////////////

		
	}
}