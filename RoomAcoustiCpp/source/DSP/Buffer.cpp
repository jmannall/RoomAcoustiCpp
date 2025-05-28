/*
* @class Buffer, BufferF
*
* @brief Declaration of Buffer and BufferF classes
*
*/

// DSP headers
#include "DSP/Buffer.h"

namespace RAC
{
	namespace DSP
	{
		//////////////////// Buffer ////////////////////

		////////////////////////////////////////

		void Buffer::ResizeBuffer(const int numSamples)
		{
			assert(numSamples >= 0);

			int size = Length();
			if (size == numSamples)
				return;
			if (size < numSamples)
			{
				mBuffer.reserve(numSamples);
				mBuffer.insert(mBuffer.end(), numSamples - size, 0.0);
			}
			else if (numSamples > 0)
				mBuffer.resize(numSamples);
			else
				mBuffer.reserve(1);
		}

		////////////////////////////////////////

		bool Buffer::Valid()
		{
			for (int i = 0; i < Length(); i++)
				if (std::isnan(mBuffer[i]))
					return false;
			return true;
		}
	}
}