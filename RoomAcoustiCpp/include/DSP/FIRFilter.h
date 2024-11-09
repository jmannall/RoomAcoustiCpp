/*
* @class FIRFilter
*
* @brief Declaration of FIRFilter class
*
*/

#ifndef DSP_FIRFilter_h
#define DSP_FIRFilter_h

// DSP headers
#include "DSP/Buffer.h"
#include "DSP/Interpolate.h"

// Common headers
#include "Common/Types.h"

namespace RAC
{
	namespace DSP
	{
		/**
		* Class that implements a Finite Impulse Response filter
		*/
		class FIRFilter
		{
		public:
			/**
			* Constructor that initialises the FIRFilter with a given impulse response
			*
			* @param mIr The impulse response to initialise the FIRFilter with
			*/
			FIRFilter(const Buffer& ir) : mIr(), irLen(mIr.Length()), count(static_cast<int>(ir.Length()) - 1), inputLine() { SetImpulseResponse(ir); };
			
			/**
			* Default deconstructor
			*/
			~FIRFilter() {};

			/**
			* Returns the output of the FIRFilter given an input
			*
			* @param input The input to the FIRFilter
			* @return The output of the FIRFilter
			*/
			Real GetOutput(const Real input);

			/**
			* Resizes the impulse response and input line of the FIRFilter
			* 
			* @details If the new length is not a multiple of 8, it is rounded up to the nearest multiple of 8 to allow for vectorisation in the GetOutput function.
			* 
			* @param len The new length of the impulse response
			*/
			inline void Resize(int len)
			{
				if (len % 8 != 0)
					len += (8 - len % 8);
				if (len != irLen)
				{
					if (len > irLen)
						IncreaseSize(len);
					else
						DecreaseSize(len);
					irLen = len;
				}
			}

			inline void UpdateImpulseResponse(const Buffer& targetIr, const Real lerpFactor)
			{
				Lerp(mIr, targetIr, lerpFactor);
			}

			inline Buffer GetImpulseResponse() const
			{
				return mIr;
			}

			/**
			* Sets the impulse response of the FIRFilter
			*
			* @param ir The new impulse response to set
			*/
			inline void SetImpulseResponse(const Buffer& ir)
			{
				Resize(ir.Length());
				for (int i = 0; i < ir.Length(); i++)
					mIr[i] = ir[i];
			}

		private:

			/**
			* Increases the size of the impulse response and input line of the FIRFilter
			*
			* @details New samples are initialised to 0.
			*
			* @param len The new length of the impulse response
			*/
			inline void IncreaseSize(const int len)
			{
				inputLine.ResizeBuffer(len);
				mIr.ResizeBuffer(len);
			}

			/**
			* Decreases the size of the impulse response and input line of the FIRFilter
			*
			* @details The input line is shifted to ensure the most recent samples are retained.
			*
			* @param len The new length of the impulse response
			*/
			inline void DecreaseSize(const int len)
			{
				Buffer store = inputLine;
				int index = count;
				for (int i = 0; i < irLen; i++)
				{
					inputLine[i] = store[index++];
					if (index >= irLen) { index = 0; }
				}
				inputLine.ResizeBuffer(len);
				mIr.ResizeBuffer(len);
				count = 0;
			}

			/**
			* The impulse response and input line buffers
			*/
			Buffer mIr;
			Buffer inputLine;

			/**
			* The length of the impulse response and input line buffers
			*/
			int irLen;

			/**
			* The index for the next sample entry to the input line buffer
			*/
			int count;
		};
	}
}
#endif