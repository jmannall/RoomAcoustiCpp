/*
* @class GraphicEQ
*
* @brief Declaration of Graphic EQ class
*
* @remarks Based after Efficient Multi-Band Digital Audio Graphic Equalizer with Accurate Frequency Response Control. Oliver R, Jot J. 2015
*
*/

// Common headers
#include "Common/RACProfiler.h"
#include "Common/Complex.h"

// DSP headers
#include "DSP/GraphicEQ_private.h"

// Run all filters as a single operation without using a temporary variable. This improves performance
// slightly (~12%) since it eliminates read/writing the temporary value to memory.
#define BATCH_PROCESS_FILTERS			 ( 1 )


namespace RAC
{
	namespace DSP
	{
		//////////////////// GraphicEQ ////////////////////

		template<typename T>
		GraphicEQ<T>::GraphicEQ(const Coefficients<>& gain, const Coefficients<>& fc, const Real Q, const int sampleRate) :
			numFilters(ToInt(gain.Length() + 2)), filterResponseMatrix(numFilters, numFilters), previousInput(gain)
		{
			RAC_DEBUG_ASSERT(gain.Length() == fc.Length(), "Gain and frequency parameters must have the same length");

			Coefficients<> f = CreateFrequencyVector(fc);
			InitMatrix(f, Q, sampleRate);
			auto targetGains = CalculateGains(gain);

			// Increasing the low shelf frequency by SQRT_2 creates a smoother response at low frequencies
			lowShelf = std::make_unique<PeakLowShelf<T>>(f[0] * SQRT_2, targetGains.first(0), Q, sampleRate);
			for (int i = 1; i < numFilters - 1; i++)
				peakingFilters.emplace_back(std::make_unique<PeakingFilter<T>>(f[i], targetGains.first(i), Q, sampleRate));
			// (same as fc[numFilters - 1] * 2 / SQRT_2 ) Decreasing the high shelf frequency by SQRT_2 creates a smoother response at high frequencies
			highShelf = std::make_unique<PeakHighShelf<T>>(std::min(f[numFilters - 2] * SQRT_2, (Real)20000.0), targetGains.first(numFilters - 1), Q, sampleRate);

			// TODO: Add a debug warning if this is the case
			if (targetGains.second > REAL_CONST(100.0))
				return; // Note: initialised never gets set to true, silence.

			targetGain.store(targetGains.second, std::memory_order_release);
			currentGain = targetGains.second;

			gainsEqual.store(true, std::memory_order_release);

			bool isInitialised = lowShelf->IsValid() && highShelf->IsValid();
			for (const auto& peakingFilter : peakingFilters)
			{
				if (!peakingFilter->IsValid())
					isInitialised = false;
			}
			initialised.store(isInitialised, std::memory_order_release);
		}

		////////////////////////////////////////

		template<typename T>
		bool GraphicEQ<T>::SetTargetGains(const Coefficients<>& gains)
		{
			RAC_DEBUG_ASSERT(gains.Length() == ToInt(peakingFilters.size()), "Incorrect number of gains provided: " + ToString(gains.Length()));
			RAC_DEBUG_ASSERT(gains.IsGreaterEqThan(REAL_CONST(0.0)), "Invalid target gains: " + ToString(gains));
			RAC_DEBUG_ASSERT(gains.IsLessEqThan(REAL_CONST(1.0)), "Invalid target gains: " + ToString(gains));

			if (gains.IsApprox(previousInput)) // No change in gains
			{
				if (gainsEqual.load(std::memory_order_acquire) && gains.IsLessEqThan(0.0)) // Gains currently zero
					return true;
				return false;
			}

			previousInput = gains;

			const auto targetGains = CalculateGains(gains);

			// TODO: This is now covered by the assert above, remove?
			if (targetGains.second > REAL_CONST(100.0))
			{
				RAC_DEBUG_LOG("GraphicEQ target gain too high: " + ToString(targetGains.second), DebugType::Warning);
				return false; // Note: initialised never gets set to true, silence.
			}

			targetGain.store(targetGains.second, std::memory_order_release);
			gainsEqual.store(false, std::memory_order_release);
			lowShelf->SetTargetGain(targetGains.first(0));
			for (int i = 1; i < numFilters - 1; i++)
				peakingFilters[i - 1]->SetTargetGain(targetGains.first(i));
			highShelf->SetTargetGain(targetGains.first(numFilters - 1));
			return false;
		}

		////////////////////////////////////////

		template<typename T>
		std::pair<Rowvec<>, Real> GraphicEQ<T>::CalculateGains(const Coefficients<>& gains) const
		{
			RAC_DEBUG_ASSERT(gains.Length() + 2 == numFilters, "Incorrect number of gains provided: " + ToString(gains.Length()));

			// TODO: Should this be coefficients?
			Rowvec<> inputGains = Rowvec<>::Constant(numFilters, REAL_CONST(1.0));

			if (gains.IsLessEqThan(0.0))
				return std::make_pair(inputGains, REAL_CONST(0.0));

			if (gains.Length() == 1)
			{
				inputGains(0) = gains[0];
				inputGains(1) = gains[0];
				inputGains(2) = gains[0];
			}
			else
			{
				inputGains(0) = (gains[0] + gains[1]) / REAL_CONST(2.0); // Low shelf gain
				for (int i = 1; i < numFilters - 1; i++)
					inputGains(i) = gains[i - 1]; // Peaking filter gains
				inputGains(numFilters - 1) = (gains[numFilters - 4] + gains[numFilters - 3]) / REAL_CONST(2.0); // High shelf gain
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

		template<typename T>
		Coefficients<> GraphicEQ<T>::CreateFrequencyVector(const Coefficients<>& fc) const
		{
			Coefficients<> f(numFilters);
			f[0] = std::max(fc[0] / (Real)2.0, (Real)20.0);
			for (int i = 1; i < numFilters - 1; i++)
				f[i] = fc[i - 1];
			f[numFilters - 1] = std::min(fc[numFilters - 3] * (Real)2.0, (Real)20000.0);
			return f;
		}

		////////////////////////////////////////

		template<typename T>
		void GraphicEQ<T>::InitMatrix(const Coefficients<>& fc, const Real Q, const int fs)
		{
			RAC_DEBUG_ASSERT(fc.Length() == numFilters, "Incorrect number of frequencies provided: " + ToString(fc.Length()));

			Real pdb = 6.0;
			Real p = pow(REAL_CONST(10.0), pdb / REAL_CONST(20.0));

#if MATRIX_LIBRARY == EIGEN_FLAG
			const PeakLowShelf tempLowShelf(fc[0] * SQRT_2, p, Q, fs); // Times SQRT_2. See constructor
			filterResponseMatrix.Row(0) = tempLowShelf.GetFrequencyResponse(fc);

			for (int j = 1; j < numFilters - 1; j++)
			{
				const PeakingFilter tempPeakingFilter(fc[j], p, Q, fs);
				filterResponseMatrix.Row(j) = tempPeakingFilter.GetFrequencyResponse(fc);
			}
			const PeakHighShelf tempHighShelf(std::min(fc[numFilters - 2] * SQRT_2, REAL_CONST(20000.0)), p, Q, fs); // Times SQRT_2. See constructor
			filterResponseMatrix.Row(numFilters - 1) = tempHighShelf.GetFrequencyResponse(fc);
#else
			Coefficients<> out(numFilters);

			const PeakLowShelf tempLowShelf(fc[0] * SQRT_2, p, Q, static_cast<int>(fs)); // Times SQRT_2. See constructor
			out = tempLowShelf.GetFrequencyResponse(fc);

			for (int i = 0; i < numFilters; i++)
				filterResponseMatrix(0, i) = out[i];

			for (int j = 1; j < numFilters - 1; j++)
			{
				const PeakingFilter tempPeakingFilter(fc[j], p, Q, static_cast<int>(fs));
				out = tempPeakingFilter.GetFrequencyResponse(fc);

				for (int i = 0; i < out.Length(); i++)
					filterResponseMatrix(j, i) = out[i];
			}

			const PeakHighShelf tempHighShelf(std::min(fc[numFilters - 2] * SQRT_2, RAC_CONST(20000.0)), p, Q, static_cast<int>(fs)); // Times SQRT_2. See constructor
			out = tempHighShelf.GetFrequencyResponse(fc);

			for (int i = 0; i < out.Length(); i++)
				filterResponseMatrix(numFilters - 1, i) = out[i];
#endif
			filterResponseMatrix = filterResponseMatrix.InverseMatrix();
			filterResponseMatrix *= pdb;
		}

		////////////////////////////////////////

		template<typename T>
		T GraphicEQ<T>::GetOutput(const T input, const Real lerpFactor)
		{
			RAC_DEBUG_ASSERT(IsValid(), "Invalid filter");
			if (!gainsEqual.load(std::memory_order_acquire))
				InterpolateGain(lerpFactor);

			if (numFilters == 3) // Only one peaking filter
				return input * currentGain; // Single band EQ is just a gain

			T out;
			lowShelf->GetOutput(input, out, lerpFactor);
			for (const auto& filter : peakingFilters)
				filter->GetOutput(out, out, lerpFactor);
			highShelf->GetOutput(out, out, lerpFactor);
			out *= currentGain;
			return out;
		}

		template<typename T>
		void GraphicEQ<T>::GetOutputBatch(const Buffer<T>& inBuffer, Buffer<T>& outBuffer, const Real lerpFactor)
		{
			RAC_DEBUG_ASSERT(IsValid(), "Invalid filter");
			RAC_DEBUG_ASSERT(outBuffer.Length() >= inBuffer.Length(), "Input and output buffer lengths do not match");

			const int bufferLength = ToInt(inBuffer.Length());
			const T* input = inBuffer.data();
			T* output = outBuffer.data();

			if (numFilters == 3)
			{
				// Only one peaking filter -> just a gain, so copy the input directly to the output
				for (int index = 0; index < bufferLength; ++index)
					output[index] = input[index];
			}
			else
			{
#if BATCH_PROCESS_FILTERS
				// build a local copy of the filter array
				IIRFilter2<T>** filters = static_cast<IIRFilter2<T>**>(alloca(sizeof(IIRFilter2<T>*) * numFilters));
				filters[0] = &*lowShelf;
				int currentFilter = 1;
				for (const auto& filter : peakingFilters)
					filters[currentFilter++] = &*filter;
				filters[currentFilter++] = &*highShelf;
				RAC_DEBUG_ASSERT(currentFilter == numFilters, "Processed the wrong number of filters");

				// process all filters on each sample
				for (int index = 0; index < bufferLength; ++index)
					IIRFilter2<T>::GetOutputFromMultipleFilters(filters, numFilters, input[index], output[index], lerpFactor);

#else
				for (int index = 0; index < bufferLength; ++index)
				{
					T& out = output[index];
					lowShelf->GetOutput(input[index], out, lerpFactor);
					for (const auto& filter : peakingFilters)
						filter->GetOutput(out, out, lerpFactor);
					highShelf->GetOutput(out, out, lerpFactor);
				}
#endif
			}

			// process the gain
			ScaleGain(outBuffer, lerpFactor);
		}
		
		////////////////////////////////////////
		///
		template<typename T>
		void GraphicEQ<T>::ScaleGain(Buffer<T>& outBuffer, const Real lerpFactor)
		{
			const int length = ToInt(outBuffer.Length());
			T* output = outBuffer.data();

			if (!gainsEqual.load(std::memory_order_acquire))
			{
				for (int index = 0; index < length; ++index)
				{
					if (!gainsEqual.load(std::memory_order_acquire))
						InterpolateGain(lerpFactor);
					output[index] *= currentGain;
				}
			}
			else
			{
				for (int index = 0; index < length; ++index)
					output[index] *= currentGain;
			}
		}

		////////////////////////////////////////

#if USE_AVX

#if DATA_TYPE_DOUBLE
		template<>
		void GraphicEQ<double>::ScaleGain(Buffer<>& outBuffer, const Real lerpFactor)
		{
			const int length = ToInt(outBuffer.Length());
			double* output = outBuffer.data();

			if (!gainsEqual.load(std::memory_order_acquire) || (length % 4) != 0)
			{
				for (int index = 0; index < length; ++index)
				{
					if (!gainsEqual.load(std::memory_order_acquire))
						InterpolateGain(lerpFactor);
					output[index] *= currentGain;
				}
			}
			else
			{
				// make sure we are aligned (in practice this is true; we could always check it and fall
				// back on a slower case)
				RAC_DEBUG_ASSERT(IsAligned32(output), "Output buffer not aligned");

				__m256d scaleFactor = _mm256_broadcast_sd(&currentGain);
				double* current = output, *end = output + length;
				while (current < end)
				{
					__m256d currentValue = _mm256_load_pd(current);
					__m256d newValue = _mm256_mul_pd(currentValue, scaleFactor);
					_mm256_store_pd(current, newValue);
					current += 4;
				}
			}
		}

#else

		template<>
		void GraphicEQ<float>::ScaleGain(Buffer<>& outBuffer, const Real lerpFactor)
		{
			const int length = ToInt(outBuffer.Length());
			float* output = outBuffer.data();

			if (!gainsEqual.load(std::memory_order_acquire) || (length % 4) != 0)
			{
				for (int index = 0; index < length; ++index)
				{
					if (!gainsEqual.load(std::memory_order_acquire))
						InterpolateGain(lerpFactor);
					output[index] *= currentGain;
				}
			}
			else if ( (length % 8) != 0 )
			{
				// make sure we are aligned (in practice this is true; we could always check it and fall
				// back on a slower case)
				RAC_DEBUG_ASSERT(IsAligned32(output), "Output buffer not aligned");

				__m256 scaleFactor = _mm256_broadcast_ss(&currentGain);
				float* current = output, * end = output + length;
				while (current < end)
				{
					__m256 currentValue = _mm256_load_ps(current);
					__m256 newValue = _mm256_mul_ps(currentValue, scaleFactor);
					_mm256_store_ps(current, newValue);
					current += 8;
				}
			}
			else 
			{
				RAC_DEBUG_ASSERT(IsAligned16(output), "Output buffer not aligned");

				__m128 scaleFactor = _mm_broadcast_ss(&currentGain);
				float* current = output, * end = output + length;
				while (current < end)
				{
					__m128 currentValue = _mm_load_ps(current);
					__m128 newValue = _mm_mul_ps(currentValue, scaleFactor);
					_mm_store_ps(current, newValue);
					current += 4;
				}
			}
		}

#endif

#endif

		////////////////////////////////////////

		template<typename T>
		void GraphicEQ<T>::ProcessAudio(const Buffer<T>& inBuffer, Buffer<T>& outBuffer, const Real lerpFactor)
		{
			RAC_DEBUG_ASSERT(IsValid(), "Invalid filter");

			if (currentGain == 0.0 && gainsEqual.load(std::memory_order_acquire))
			{
				outBuffer.Reset();
				return;
			}

			GetOutputBatch(inBuffer, outBuffer, lerpFactor);
		}

		////////////////////////////////////////

		template<typename T>
		void GraphicEQ<T>::InterpolateGain(const Real lerpFactor)
		{
			gainsEqual.store(true, std::memory_order_release); // Prevents issues in case targetGain updated during this function call
			const Real gain = targetGain.load(std::memory_order_acquire);
			currentGain = Lerp(currentGain, gain, lerpFactor);
			if (Equals(currentGain, gain))
				currentGain = gain;
			else
				gainsEqual.store(false, std::memory_order_release);
		}

		//////////////////// Instantiate ////////////////////

		// we don't implement/use every function, so disable the warning (we can't re-enable it since the warning is generated after the file is parsed)
		#ifdef _MSC_VER
		#pragma warning (disable : 4661)
		#endif

		template class GraphicEQ<Real>;
		template class GraphicEQ<Complex>;

	}
}
