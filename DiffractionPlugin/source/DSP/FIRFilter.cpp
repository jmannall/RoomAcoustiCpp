/*
* @class FIRFilter
*
* @brief Declaration of FIRFilter class
*
*/

// DSP headers
#include "DSP/FIRFilter.h"

// Common headers
#include "Common/Types.h"

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

			if (irLen % 8 != 0)
			{
				for (int i = 0; i < irLen; i++)
				{
					output += mIr[i] * inputLine[index++];
					if (index >= irLen) { index = 0; }
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
				while (i < irLen)
				{
					if (index < (irLen - 8))
					{
						result_a += mIr[i++] * inputLine[index++];
						result_b += mIr[i++] * inputLine[index++];
						result_c += mIr[i++] * inputLine[index++];
						result_d += mIr[i++] * inputLine[index++];
						result_e += mIr[i++] * inputLine[index++];
						result_f += mIr[i++] * inputLine[index++];
						result_g += mIr[i++] * inputLine[index++];
						result_h += mIr[i++] * inputLine[index++];
					}
					else
					{
						for (int k = 0; k < 8; k++)
						{
							output += mIr[i++] * inputLine[index++];
							if (index >= irLen) { index = 0; }
						}
					}
				}
				output += result_a + result_b + result_c + result_d + result_e + result_f + result_g + result_h;
			}
			if (--count < 0) { count = static_cast<int>(irLen) - 1; }
			return output;
		}
	}
}