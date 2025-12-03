/*
* @class FIRFilter
*
* @brief Declaration of FIRFilter class
*
*/

// DSP headers
#include "DSP/FIRFilter.h"

namespace RAC
{
	namespace DSP
	{
		//////////////////// FIRFilter ////////////////////

		ReleasePool FIRFilter::releasePool;

		////////////////////////////////////////

		Real FIRFilter::GetOutput(const Real input, const Real lerpFactor)
		{
			RAC_DEBUG_ASSERT(IsValid(), "Invalid filter");

			if (!irsEqual.load(std::memory_order_acquire))
				InterpolateIR(lerpFactor);

			Real output = 0.0;
			int index = count;

			// Using a double buffer size to avoid checks in process loop
			inputLine[index] = input;
			inputLine[index + maxFilterLength] = input;

			RAC_DEBUG_ASSERT(currentIR.Length() >= ToInt(irLength), "IR length exceeds max length of the filter");
			RAC_DEBUG_ASSERT(irLength % 8 == 0, "IR length is not a multiple of eight");

			// Assume length is always a multiple of 8
			for (int i = 0; i < irLength; i += 8)
			{
				output += currentIR[i] * inputLine[index++];
				output += currentIR[i + 1] * inputLine[index++];
				output += currentIR[i + 2] * inputLine[index++];
				output += currentIR[i + 3] * inputLine[index++];
				output += currentIR[i + 4] * inputLine[index++];
				output += currentIR[i + 5] * inputLine[index++];
				output += currentIR[i + 6] * inputLine[index++];
				output += currentIR[i + 7] * inputLine[index++];
			}

			if (--count < 0)
				count = maxFilterLength - 1;

			return output;
		}

		////////////////////////////////////////

		bool FIRFilter::SetTargetIR(const Buffer<>& ir)
		{
			int length = ToInt( ir.Length() );

			// Pad to multiple of 8
			if (length % 8 != 0)
				length += (8 - length % 8);

			if (length > maxFilterLength)
				return false;
			RAC_DEBUG_ASSERT(length <= maxFilterLength, "Length exceeds max length of the filter");

			const std::shared_ptr<Buffer<>> irCopy = std::make_shared<Buffer<>>(ir);
			irCopy->Resize(length);

			releasePool.Add(irCopy);

#ifdef __ANDROID__
			std::atomic_store(&targetIR, irCopy);
			std::atomic_store(&irsEqual, false);
#else
			targetIR.store(irCopy, std::memory_order_release);
			irsEqual.store(false, std::memory_order_release);
#endif
			return true;
		}

		////////////////////////////////////////

		void FIRFilter::InterpolateIR(const Real lerpFactor)
		{
			irsEqual.store(true, std::memory_order_release); // Prevents issues in case targetIR updated during this function call
#ifdef __ANDROID__
			const std::shared_ptr<const Buffer<>> ir = std::atomic_load(&targetIR);
#else
			const std::shared_ptr<const Buffer<>> ir = targetIR.load(std::memory_order_acquire);
#endif
			irLength = ir->Length();

			Lerp(currentIR, *ir, ToInt( oldIrLength ), lerpFactor);

			if (Equals(currentIR, *ir, ToInt(irLength)))
			{
				std::copy(ir->begin(), ir->end(), currentIR.begin());
				oldIrLength = irLength;
				return;
			}
			irsEqual.store(false, std::memory_order_release);
		}
	}
}