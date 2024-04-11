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
		////////////////////////////////////////

		void Buffer::ResizeBuffer(const size_t numSamples)
		{
			size_t size = Length();
			if (size == numSamples)
				return;
			if (size < numSamples)
			{
				mBuffer.reserve(numSamples);
				mBuffer.insert(mBuffer.end(), numSamples - size, 0.0);
			}
			else
				mBuffer.resize(numSamples);
		}

		////////////////////////////////////////

		bool Buffer::Valid()
		{
			for (int i = 0; i < Length(); i++)
				if (std::isnan(mBuffer[i]))
					return false;
			return true;
		}

		////////////////////////////////////////

		void BufferF::ResizeBuffer(const size_t numSamples)
		{
			size_t size = mBuffer.size();
			if (size == numSamples)
				return;
			if (size < numSamples)
			{
				mBuffer.reserve(numSamples);
				mBuffer.insert(mBuffer.end(), numSamples - size, 0.0);
			}
			else
				mBuffer.resize(numSamples);
		}
	}
}