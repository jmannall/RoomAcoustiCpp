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

		//template <typename T>
		//void Buffer<T>::ResizeBuffer(const size_t numSamples)
		//{
		//	assert(numSamples >= 0);

		//	size_t size = Length();
		//	if (size == numSamples)
		//		return;
		//	if (size < numSamples)
		//	{
		//		mBuffer.reserve(numSamples);
		//		mBuffer.insert(mBuffer.end(), numSamples - size, 0.0);
		//	}
		//	else
		//		mBuffer.resize(numSamples);
		//}

		//// Explicit instantiations
		//template void Buffer<Real>::ResizeBuffer(const size_t);
		//template void Buffer<Complex>::ResizeBuffer(const size_t);

		////////////////////////////////////////

		/*template <typename T>
		bool Buffer<T>::Valid()
		{
			for (int i = 0; i < Length(); i++)
				if (std::isnan(mBuffer[i]))
					return false;
			return true;
		}*/

		//// Explicit instantiations
		//template bool Buffer<Real>::Valid();
		//template bool Buffer<Complex>::Valid();
	}
}