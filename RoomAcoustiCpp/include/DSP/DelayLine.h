/*
* @class DelayLine
*
* @brief Declaration of DelayLine class
*
*/

#ifndef DSP_DelayLine_h
#define DSP_DelayLine_h

#include "Common/Types.h"
#include "DSP/Buffer.h"

namespace RAC
{
	using namespace Common;
	namespace DSP
	{
		/**
		* @brief Class that implements a fixed-length delay line
		*/
		template<typename T = Real>
		class DelayLine
		{
		public:
			/**
			* @brief Constructor that initialises the delay line with a given length.
			* 
			* @param length The length of the delay line.
			*/
			inline DelayLine(int length) : buffer(length), length(length), initialised(length > 0)
			{
#if MATRIX_LIBRARY == EIGEN_FLAG
				buffer.Reset();
#endif
			}

			/**
			* @brief Adds a sample to the delay line and returns the delayed sample.
			*
			* @param input The sample to add to the delay line.
			* @return The delayed sample.
			*/
			inline T GetOutput(T input)
			{
				assert(initialised);
				T out = buffer[idx];
				buffer[idx] = input;
				if (++idx >= length) [[unlikely]]
					idx = 0;
				return out;
			}

			/**
			* @brief Adds a sample to the delay line and returns the delayed sample
			*
			* @param output The location to store the result
			* @param input The sample to add to the delay line.
			*/
			inline void GetOutput(const T& input, T& output)
			{
				assert(initialised);
				output = buffer[idx];
				buffer[idx] = input;
				if (++idx >= length) [[unlikely]]
					idx = 0;
			}

			/**
			* @brief Zeroes the delay line
			*/
			inline void Reset()
			{
				buffer.Reset();
				idx = 0;
			}

		private:
			Buffer<T> buffer;	// Delay line buffer
			int idx{ 0 };		// Current write index
			int length;			// Local copy of the length
			bool initialised{ false };	// True if the delay line has been initialized
		};

		///////////////////////////////////////////////////////////////////////////////

#if USE_AVX

		template<>
		inline void DelayLine<std::complex<std::double_t>>::GetOutput(const std::complex<std::double_t>& input, std::complex<std::double_t>& output)
		{
			assert(initialised);
			assert(IsAligned16(&input));
			assert(IsAligned16(&output));

			double* entryPointer = reinterpret_cast<double*>(buffer.data() + idx);
			double* outputPointer = reinterpret_cast<double*>(&output);
			const double* inputPointer = reinterpret_cast<const double*>(&input);

			// start fetching the entry pointer
			_mm_prefetch(reinterpret_cast<const char*>(entryPointer), _MM_HINT_T0);

			// while we're doing that, increment our index
			if (++idx >= length) [[unlikely]]
				idx = 0;

			// now we start prefetching the new value to load
			_mm_prefetch(reinterpret_cast<const char*>(inputPointer), _MM_HINT_T0);

			// grab the existing entry
			_mm_store_pd(outputPointer, _mm_load_pd(entryPointer));

			// and now store the new value
			_mm_store_pd(entryPointer, _mm_load_pd(inputPointer));
		}

		template<>
		inline void DelayLine<std::complex<float>>::GetOutput(const std::complex<float>& input, std::complex<float> & output)
		{
			assert(initialised);

			__m64 *entryPointer = reinterpret_cast<__m64*>(buffer.data() + idx);
			__m64 *outputPointer = reinterpret_cast<__m64*>(&output);
			const __m64* inputPointer = reinterpret_cast<const __m64*>(&input);

			// start fetching the entry pointer
			_mm_prefetch(reinterpret_cast<const char*>(entryPointer), _MM_HINT_T0);

			// while we're doing that, increment our index
			if (++idx >= length) [[unlikely]]
				idx = 0;

			// now we start prefetching the new value to load
			_mm_prefetch(reinterpret_cast<const char*>(inputPointer), _MM_HINT_T0);

			__m128 zero = _mm_setzero_ps();

			// grab the existing entry
			_mm_storel_pi(outputPointer, _mm_loadl_pi(zero, entryPointer));

			// and now store the new value
			_mm_storel_pi(entryPointer, _mm_loadl_pi(zero, inputPointer));
		}

#endif

	}
}

#endif