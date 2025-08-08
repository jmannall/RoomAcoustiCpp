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
		class DelayLine
		{
		public:
			/**
			* @brief Constructor that initialises the delay line with a given length.
			* 
			* @param length The length of the delay line.
			*/
			DelayLine(int length) : buffer(length) {}

			/**
			* @brief Adds a sample to the delay line and returns the delayed sample.
			*
			* @param input The sample to add to the delay line.
			* @return The delayed sample.
			*/
			Real GetOutput(Real input)
			{
				if (idx >= buffer.Length())
					idx = 0;
				Real out = buffer[idx];
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
			Buffer<> buffer;	// Delay line buffer
			int idx{ 0 };		// Current write index
		};
	}
}

#endif