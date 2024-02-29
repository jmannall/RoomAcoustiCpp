
// C++ headers
#if defined(_WINDOWS)
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#endif

// DSP headers
#include "DSP/IIRFilter.h"

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"

namespace UIE
{
	using namespace Common;
	namespace DSP
	{
		////////////////////////////////////////

		Real IIRFilter::GetOutput(const Real& input)
		{
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#elif(_ANDROID)

			unsigned m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (1 << 24));
#endif
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
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
#elif(_ANDROID)

			m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (0 << 24));
#endif
			return output;
		}

		////////////////////////////////////////

		void HighShelf::UpdateParameters(const Real& fc, const Real& g)
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

		void LowPass::UpdateParameters(const Real& fc)
		{
			Real K = PI_2 * fc * T;

			a[0] = 1.0 / (K + 2.0); // a[0] isn't used in GetOutput
			a[1] = (K - 2.0) * a[0];

			b[0] = K * a[0];
			b[1] = K * a[0];
		}

		////////////////////////////////////////

		void ZPKFilter::UpdateParameters(const ZPKParameters& zpk)
		{
			b[0] = zpk.k;
			b[1] = -zpk.k * (zpk.z[0] + zpk.z[1]);
			b[2] = zpk.k * zpk.z[0] * zpk.z[1];

			a[1] = -(zpk.p[0] + zpk.p[1]);
			a[2] = zpk.p[0] * zpk.p[1];
		}

		////////////////////////////////////////

		void PassFilter::UpdateLowPass(const Real& fc)
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

		void PassFilter::UpdateHighPass(const Real& fc)
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