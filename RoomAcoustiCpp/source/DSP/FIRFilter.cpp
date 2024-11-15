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
		////////////////////////////////////////

		Real FIRFilter::GetOutput(const Real input)
		{
			inputLine[count] = input;
			Real output = 0.0;
			int index = count;

			if (filterTaps % 8 != 0)
			{
				for (int i = 0; i < filterTaps; i++)
				{
					output += impulseResponse[i] * inputLine[index++];
					if (index >= filterTaps) { index = 0; }
				}
			}
			else
			{
				// This is easier for the compiler to vectorise
				Real result_a = 0.0;
				Real result_b = 0.0;
				Real result_c = 0.0;
				Real result_d = 0.0;
				Real result_e = 0.0;
				Real result_f = 0.0;
				Real result_g = 0.0;
				Real result_h = 0.0;
				int i = 0;
				while (i < filterTaps)
				{
					if (index < (filterTaps - 8))
					{
						result_a += impulseResponse[i++] * inputLine[index++];
						result_b += impulseResponse[i++] * inputLine[index++];
						result_c += impulseResponse[i++] * inputLine[index++];
						result_d += impulseResponse[i++] * inputLine[index++];
						result_e += impulseResponse[i++] * inputLine[index++];
						result_f += impulseResponse[i++] * inputLine[index++];
						result_g += impulseResponse[i++] * inputLine[index++];
						result_h += impulseResponse[i++] * inputLine[index++];
					}
					else
					{
						for (int k = 0; k < 8; k++)
						{
							output += impulseResponse[i++] * inputLine[index++];
							if (index >= filterTaps) { index = 0; }
						}
					}
				}
				output += result_a + result_b + result_c + result_d + result_e + result_f + result_g + result_h;
			}
			if (--count < 0) { count = static_cast<int>(filterTaps) - 1; }
			return output;
		}

		void FIRFilter::SetImpulseResponse(const Buffer& ir)
		{
			Resize(ir.Length());
			for (int i = 0; i < ir.Length(); i++)
				impulseResponse[i] = ir[i];
		}

		void FIRFilter::IncreaseSize(const int length)
		{
			inputLine.ResizeBuffer(length);
			impulseResponse.ResizeBuffer(length);
		}

		void FIRFilter::DecreaseSize(const int length)
		{
			Buffer store = inputLine;
			int index = count;
			for (int i = 0; i < filterTaps; i++)
			{
				inputLine[i] = store[index++];
				if (index >= filterTaps) { index = 0; }
			}
			inputLine.ResizeBuffer(length);
			impulseResponse.ResizeBuffer(length);
			count = 0;
		}
	}
}