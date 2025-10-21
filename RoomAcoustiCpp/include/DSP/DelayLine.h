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
			DelayLine(int length) : buffer(length), initialised(length > 0)
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
			T GetOutput(T input)
			{
				if (!initialised)
					return input;
				if (idx >= buffer.Length())
					idx = 0;
				T out = buffer[idx];
				buffer[idx] = input;
				++idx;
				return out;
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
			bool initialised{ false };	// True if the delay line has been initialized
		};
	}
}

#endif