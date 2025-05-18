/*
* @class HeadphoneEQ
* 
* @brief Declaration of HeadphoneEQ class
*
*/

#ifndef RoomAcoustiCpp_HeadphoneEQ_h
#define RoomAcoustiCpp_HeadphoneEQ_h

#include "DSP/FIRFilter.h"

namespace RAC
{
	using namespace DSP;
	namespace Spatialiser
	{
		/**
		* @brief Applies a sterophonic FIR filter to the audio signal
		*/
		class HeadphoneEQ
		{
		public:
			/**
			* @brief Constructor that initialises the HeadphoneEQ with the given sample rate
			*
			* @param fs The sample rate
			*/
			HeadphoneEQ(int fs) : leftFilter(fs), rightFilter(fs) {};

			/**
			* @brief Set the FIR filter impulse responses for the left and right channels
			*
			* @param leftIR The impulse response for the left channel
			* @param rightIR The impulse response for the right channel
			*/
			inline void SetFilters(const Buffer& leftIR, const Buffer& rightIR)
			{
				leftFilter.SetImpulseResponse(leftIR);
				rightFilter.SetImpulseResponse(rightIR);
			}

			/**
			* @brief Process a single audio frame
			*
			* @params inputBuffer The input audio buffer
			* @params outputBuffer The output buffer to write to
			*/
			inline void ProcessAudio(const Buffer& inputBuffer, Buffer& outputBuffer)
			{
				for (int i = 0; i < inputBuffer.Length(); i += 2)
				{
					outputBuffer[i] = leftFilter.GetOutput(inputBuffer[i]);
					outputBuffer[i + 1] = rightFilter.GetOutput(inputBuffer[i + 1]);
				}
			}

			inline void Reset() { leftFilter.Reset(); rightFilter.Reset(); }

		private:
			FIRFilter leftFilter;		// FIR filter for the left channel
			FIRFilter rightFilter;		// FIR filter for the right channel
		};
	}
}

#endif