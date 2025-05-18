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

		////////////////////////////////////////

		Real FIRFilter::GetOutput(const Real input)
		{
			Real output = 0.0;
			int index = count;
			const int irLength = impulseResponse.Length();

			inputLine[index] = input;
			assert(inputLine.Length() == 2 * irLength);
			inputLine[index + irLength] = input;

			assert(irLength % 8 == 0);
			// Assume length is always a multiple of 8
			for (int i = 0; i < irLength; i += 8)
			{
				output += impulseResponse[i] * inputLine[index++];
				output += impulseResponse[i + 1] * inputLine[index++];
				output += impulseResponse[i + 2] * inputLine[index++];
				output += impulseResponse[i + 3] * inputLine[index++];
				output += impulseResponse[i + 4] * inputLine[index++];
				output += impulseResponse[i + 5] * inputLine[index++];
				output += impulseResponse[i + 6] * inputLine[index++];
				output += impulseResponse[i + 7] * inputLine[index++];
			}

			if (--count < 0)
				count = irLength - 1; // Check all logic is correct, integrate new Wrap index function - double buffer size to avoid checks in process loop
			return output;
		}

		////////////////////////////////////////

		void FIRFilter::SetImpulseResponse(const Buffer& ir)
		{
			const int irLength = ir.Length();
			Resize(irLength);
			for (int i = 0; i < irLength; i++)
				impulseResponse[i] = ir[i];
			//count = impulseResponse.Length() - 1;
		}

		////////////////////////////////////////

		void FIRFilter::IncreaseSize(const int length)
		{
			const int oldLength = impulseResponse.Length();
			assert(length % 8 == 0);
			inputLine.ResizeBuffer(2 * length);

			for (int i = count + oldLength; i < 2 * oldLength; i++)
				inputLine[i] = 0.0;
			impulseResponse.ResizeBuffer(length);
		}

		////////////////////////////////////////

		void FIRFilter::DecreaseSize(const int length)
		{
			assert(length % 8 == 0);
			for (int i = 0, index = count; i < length; i++, index++)
				inputLine[i] = inputLine[index];
			inputLine.ResizeBuffer(2 * length);
			for (int i = 0, index = length; i < length; i++, index++)
				inputLine[index] = inputLine[i];
			impulseResponse.ResizeBuffer(length);
			count = 0;
		}
	}
}