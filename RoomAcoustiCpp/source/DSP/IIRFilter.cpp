/*
* @class IIRFilter
*
* @brief Declaration of base IIRFilter class and derived HighShelf, LowPass, ZPKFilter, BandFilter and PassFilter classes
*
*/

// C++ headers
#ifdef _WIN32
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
			assert(IsValid());

			if (!parametersEqual.load(std::memory_order_acquire))
				InterpolateParameters(lerpFactor);

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

		Coefficients<> IIRFilter::GetFrequencyResponse(const Coefficients<>& frequencies) const
		{
			std::vector<Real> magnitudes(frequencies.Length(), 0.0);
			for (int i = 0; i < frequencies.Length(); i++)
			{
				Real omega = PI_2 * frequencies[i] * T;
				Complex num = b[0];
				Complex den = 1.0;

				for (int j = 1; j <= order; j++)
				{
					Complex e = std::exp(-j * imUnit * omega);
					num += b[j] * e;
					den += a[j] * e;
				}

				magnitudes[i] = 20 * std::log10(std::abs(num / den));
			}
			return magnitudes;
		}

		//////////////////// IIRFIlter2 ////////////////////

		////////////////////////////////////////

		template<typename In>
		Coefficients<> IIRFilter2<In>::GetFrequencyResponse(const Coefficients<>& frequencies) const
		{
			Coefficients<> magnitudes = Coefficients<>::Zero(frequencies.Length());
			for (int i = 0; i < frequencies.Length(); i++)
			{
				Real omega = PI_2 * frequencies[i] * T;
				Complex num = b0;
				Complex den = 1.0;

				Complex e = std::exp(-1 * imUnit * omega);
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

		template class IIRFilter2<Real>;
		template class IIRFilter2<Complex>;

		//////////////////// IIRFIlter1 ////////////////////

		////////////////////////////////////////

		Real IIRFilter1::GetOutput(const Real input, const Real lerpFactor)
		{
			assert(IsValid());

			if (!parametersEqual.load(std::memory_order_acquire))
				InterpolateParameters(lerpFactor);

			Real v = input;
			Real output = 0.0;

			v -= y0 * a1;
			output += y0 * b1;

			y0 = v;

			output += v * b0;

			return output;
		}

		////////////////////////////////////////

		Coefficients<> IIRFilter1::GetFrequencyResponse(const Coefficients<>& frequencies) const
		{
			Real omega;
			Complex e;
			Coefficients<> magnitudes = Coefficients<>::Zero(frequencies.Length());
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
			parametersEqual.store(true, std::memory_order_release); // Prevents issues in case targetFc/Gain updated during this function call
			const Real fc = targetFc.load(std::memory_order_acquire);
			const Real gain = targetGain.load(std::memory_order_acquire);
			currentFc = Lerp(currentFc, fc, lerpFactor);
			currentGain = Lerp(currentGain, gain, lerpFactor);
			if (Equals(currentFc, fc) && Equals(currentGain, gain))
			{
				currentFc = fc;
				currentGain = gain;
			}
			else
				parametersEqual.store(false, std::memory_order_release);
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
			parametersEqual.store(true, std::memory_order_release); // Prevents issues in case targetFc updated during this function call
			const Real fc = targetFc.load(std::memory_order_acquire);
			currentFc = Lerp(currentFc, fc, lerpFactor);
			if (Equals(currentFc, fc))
				currentFc = fc;
			else
				parametersEqual.store(false, std::memory_order_release);
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

		template<typename In>
		void IIRFilter2Param1<In>::InterpolateParameters(const Real lerpFactor)
		{
			this->parametersEqual.store(true, std::memory_order_release); // Prevents issues in case target updated during this function call
			const Real parameter = target.load(std::memory_order_acquire);
			current = Lerp(current, parameter, lerpFactor);
			if (Equals(current, parameter))
				current = parameter;
			else
				this->parametersEqual.store(false, std::memory_order_release);
			UpdateCoefficients(current);
		}

		////////////////////////////////////////

		template class IIRFilter2Param1<Real>;
		template class IIRFilter2Param1<Complex>;

		//////////////////// PeakHighSelf ////////////////////

		////////////////////////////////////////

		template<typename In>
		void PeakHighShelf<In>::UpdateCoefficients(const Real gain)
		{
			assert(gain > 0.0);

			const Real A = sqrt(gain);
			const Real v1 = A + 1.0;
			const Real v2 = A - 1.0;
			const Real v3 = v1 * cosOmega;
			const Real v4 = v2 * cosOmega;
			const Real v5 = sqrt(A) * alpha; // 2 * sqrt(A) * alpha

			this->a0 = 1.0 / (v1 - v4 + v5); // a0 isn't used in GetOutput
			this->a1 = (2.0 * (v2 - v3)) * this->a0;
			this->a2 = (v1 - v4 - v5) * this->a0;

			this->b0 = A * (v1 + v4 + v5) * this->a0;
			this->b1 = -2.0 * A * (v2 + v3) * this->a0;
			this->b2 = A * (v1 + v4 - v5) * this->a0;
		}

		////////////////////////////////////////

		template class PeakHighShelf<Real>;
		template class PeakHighShelf<Complex>;

		//////////////////// PeakLowShelf ////////////////////

		////////////////////////////////////////

		template<typename In>
		void PeakLowShelf<In>::UpdateCoefficients(const Real gain)
		{
			assert(gain > 0.0);

			const Real A = sqrt(gain);
			const Real v1 = A + 1.0;
			const Real v2 = A - 1.0;
			const Real v3 = v1 * cosOmega;
			const Real v4 = v2 * cosOmega;
			const Real v5 = sqrt(A) * alpha; // 2 * sqrt(A) * alpha

			this->a0 = 1.0 / (v1 + v4 + v5); // a0 isn't used in GetOutput
			this->a1 = (-2.0 * (v2 + v3)) * this->a0;
			this->a2 = (v1 + v4 - v5) * this->a0;

			this->b0 = A * (v1 - v4 + v5) * this->a0;
			this->b1 = 2.0 * A * (v2 - v3) * this->a0;
			this->b2 = A * (v1 - v4 - v5) * this->a0;
		}

		////////////////////////////////////////

		template class PeakLowShelf<Real>;
		template class PeakLowShelf<Complex>;

		//////////////////// PeakingFilter ////////////////////

		////////////////////////////////////////

		template<typename In>
		void PeakingFilter<In>::UpdateCoefficients(const Real gain)
		{
			assert(gain > 0.0);

			const Real A = sqrt(gain);
			const Real v1 = alpha * A;
			const Real v2 = alpha / A;

			this->a0 = 1.0 / (1.0 + v2); // a0 isn't used in GetOutput
			this->a1 = cosOmega * this->a0;
			this->a2 = (1.0 - v2) * this->a0;

			this->b0 = (1.0 + v1) * this->a0;
			this->b1 = this->a1;
			this->b2 = (1.0 - v1) * this->a0;
		}

		////////////////////////////////////////

		template class PeakingFilter<Real>;
		template class PeakingFilter<Complex>;

		//////////////////// ZPKFilter ////////////////////

		ReleasePool ZPKFilter::releasePool;

		////////////////////////////////////////

		void ZPKFilter::SetTargetParameters(const Parameters& zpk)
		{
			const std::shared_ptr<Parameters> zpkCopy = std::make_shared<Parameters>(zpk);

			releasePool.Add(zpkCopy);

#ifdef __ANDROID__
			std::atomic_store(&targetZPK, zpkCopy);
			std::atomic_store(&parametersEqual, false);
#else
			targetZPK.store(zpkCopy, std::memory_order_release);
			parametersEqual.store(false, std::memory_order_release);
#endif
		}

		////////////////////////////////////////

		void ZPKFilter::SetTargetGain(const Real k)
		{
#ifdef __ANDROID__
			Parameters zpk = *std::atomic_load(&targetZPK);
#else
			Parameters zpk = *targetZPK.load(std::memory_order_acquire);
#endif
			zpk[4] = k;
			SetTargetParameters(zpk);
		}

		////////////////////////////////////////

		void ZPKFilter::UpdateCoefficients(const Parameters& zpk) // z[0 - 1], p[2 - 3], k[4]
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
			parametersEqual.store(true, std::memory_order_release); // Prevents issues in case targetZPK updated during this function call
#ifdef __ANDROID__
			std::shared_ptr<const Parameters> zpk = std::atomic_load(&targetZPK);
#else
			std::shared_ptr<const Parameters> zpk = targetZPK.load(std::memory_order_acquire);
#endif
			Lerp(currentZPK, *zpk, lerpFactor);
			if (Equals(currentZPK, *zpk))
				currentZPK = *zpk;
			else
				parametersEqual.store(false, std::memory_order_release);
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

		//////////////////// HighSelfMatched ////////////////////

		////////////////////////////////////////

		void HighShelfMatched::InterpolateParameters(const Real lerpFactor)
		{
			parametersEqual.store(true, std::memory_order_release); // Prevents issues in case targetFc/Gain updated during this function call
			const Real fc = targetFc.load(std::memory_order_acquire);
			const Real gain = targetGain.load(std::memory_order_acquire);
			currentFc = Lerp(currentFc, fc, lerpFactor);
			currentGain = Lerp(currentGain, gain, lerpFactor);
			if (Equals(currentFc, fc) && Equals(currentGain, gain))
			{
				currentFc = fc;
				currentGain = gain;
			}
			else
				parametersEqual.store(false, std::memory_order_release);
			UpdateCoefficients(currentFc, currentGain);
		}

		////////////////////////////////////////

		void HighShelfMatched::UpdateCoefficients(const Real fc, const Real gain)
		{
			constexpr static Real fm = 0.9;
			constexpr static Real fmSq = 1.0 / (fm * fm);
			Real newFc = 2 * fc * T;
			newFc *= newFc;
			Real Phim = 1 - cos(PI_1 * fm);
			Phim = 1 / Phim;

			Real alpha = 2.0 / PI_SQ * (fmSq + 1 / (gain * newFc)) - Phim;
			Real beta = 2.0 / PI_SQ * (fmSq + gain / newFc) - Phim;

			a1 = -alpha / (1 + alpha + sqrt(1 + 2 * alpha));
			Real bAll = -beta / (1 + beta + sqrt(1 + 2 * beta));
			b0 = (1 + alpha) / (1 + bAll);
			b1 = bAll * b0;

			Real DCg = (b0 + b1) / (1 + a1);

			b0 /= DCg;
			b1 /= DCg;
		}

#if USE_AVX
		template<>
		void IIRFilter2<double>::GetOutputBatch(const double* input, double* output, int inputOutputLength, const Real lerpFactor)
		{
			assert(IsValid());

			// ~64b / 8 = 8 samples/page. In practice, this doesn't help much
			//for ( int prefetchIndex = 0; prefetchIndex < (inputOutputLength>>3); ++prefetchIndex )
			//	_mm_prefetch( reinterpret_cast<const char*>(input + (prefetchIndex << 3)), _MM_HINT_T0);

			int index = 0;
			if (!parametersEqual.load(std::memory_order_acquire))
			{
				while (index < inputOutputLength)
				{
					InterpolateParameters(lerpFactor);
					if (parametersEqual.load(std::memory_order_acquire))
						break;
					GetOutputInternal(input[index], output[index]);
					++index;
				}
				if (index >= inputOutputLength)
					return;
			}

			// In v = input - y0 * a1 + y1 * a2 --> input - y[0:1] . a[0:1] -> sub(input, y.a)
			__m128d y = _mm_load_pd(&this->y0);
			__m128d a = _mm_load_pd(&this->a1);
			__m128d b = _mm_load_pd(&this->b1);
			__m128d b0 = _mm_set_sd(this->b0);

			const double* inputIteratorEnd = input + inputOutputLength;
			double* outputIterator = output;
			for (const double* inputIterator = input + index; inputIterator < inputIteratorEnd; ++inputIterator)
			{
				__m128d aDotY = _mm_dp_pd(a, y, 0x31);
				// v[low] has the result v[high] is not used
				__m128d v = _mm_sub_sd(_mm_set_sd(*inputIterator), aDotY);

				// In output	 = y0 * b1 + y1 * b2 + v * b0 --> y[0:1] . b[0:1] + v * b0 --> b0 * v + (y[0:1] . b[0:1]) --> fmadd( b0, v, y.b )
				__m128d bDotY = _mm_dp_pd(b, y, 0x31);
				// output[low] has the result; output[high] is not used
				_mm_store_sd(outputIterator++, _mm_fmadd_pd(v, b0, bDotY));

				// shift the filter y1 = y0, y0 = v
				y = _mm_move_sd(_mm_movedup_pd(y), v);
			}

			// save it
			_mm_store_pd(&this->y0, y);
		}
#endif

		template<typename In>
		void IIRFilter2<In>::GetOutputBatch(const In* input, In* output, int inputOutputLength, const Real lerpFactor)
		{
			assert(IsValid());

			if (!parametersEqual.load(std::memory_order_acquire))
			{
				for (int index = 0; index < inputOutputLength; ++index)
				{
					if (!parametersEqual.load(std::memory_order_acquire))
						InterpolateParameters(lerpFactor);

					GetOutputInternal(input[index], output[index]);
				}
			}
			else
			{
				for (int index = 0; index < inputOutputLength; ++index)
					GetOutputInternal(input[index], output[index]);
			}
		}

	}
}
