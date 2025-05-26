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
			if (!initialised.load())
				return 0.0;

			if (!irsEqual.load())
				InterpolateIR(lerpFactor);

			if (clearInputLine.load())
			{
				inputLine.ResetBuffer();
				clearInputLine.store(false);
			}

			Real output = 0.0;
			int index = count;

			// Using a double buffer size to avoid checks in process loop
			inputLine[index] = input;
			inputLine[index + maxFilterLength] = input;

			assert(currentIR.Length() >= irLength);
			assert(irLength % 8 == 0);

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

		bool FIRFilter::SetTargetIR(const Buffer& ir)
		{
			int length = ir.Length();

			// Pad to multiple of 8
			if (length % 8 != 0)
				length += (8 - length % 8);

			if (length > maxFilterLength)
				return false;
			assert(length <= maxFilterLength);

			std::shared_ptr<Buffer> irCopy = std::make_shared<Buffer>(ir);
			irCopy->ResizeBuffer(length);

			releasePool.Add(irCopy);

			targetIR.store(irCopy);
			irsEqual.store(false);

			return true;
		}

		////////////////////////////////////////

		void FIRFilter::InterpolateIR(const Real lerpFactor)
		{
			irsEqual.store(true); // Prevents issues in case targetIR updated during this function call
			const std::shared_ptr<const Buffer> ir = targetIR.load();

			irLength = ir->Length();

			Lerp(currentIR, *ir, lerpFactor);
			if (irLength < oldIrLength)
				Lerp(currentIR, irLength, oldIrLength, lerpFactor); // Interpolate end of old ir towards zero

			if (Equals(currentIR, *ir, irLength))
			{
				std::copy(ir->begin(), ir->end(), currentIR.begin());
				oldIrLength = irLength;
				return;
			}
			irsEqual.store(false);
		}
	}
}