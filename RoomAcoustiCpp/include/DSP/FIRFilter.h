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
		* @brief Class that implements a Finite Impulse Response filter
		*/
		class FIRFilter
		{
		public:
			/**
			* @brief Constructor that initialises the FIRFilter with a given impulse response
			*
			* @param impulseResponse The impulse response to initialise the FIRFilter with
			*/
			FIRFilter(const Buffer& ir) : count(0) { SetImpulseResponse(ir); };
			
			/**
			* @brief Default deconstructor
			*/
			~FIRFilter() {};

			/**
			* @brief Returns the output of the FIRFilter given an input
			*
			* @param input The input to the FIRFilter
			* @return The output of the FIRFilter
			*/
			Real GetOutput(const Real input);

			/**
			* @brief Resizes the impulse response and input line of the FIRFilter
			* 
			* @details If the new length is not a multiple of 8, it is rounded up to the nearest multiple of 8 to allow for vectorisation in the GetOutput function.
			* 
			* @param length The new length of the impulse response
			*/
			inline void Resize(int length)
			{
				int taps = impulseResponse.Length();
				if (length % 8 != 0)
					length += (8 - length % 8);

				if (length == taps)
					return;
				if (length > taps)
					IncreaseSize(length);
				else
					DecreaseSize(length);
			}

			/**
			* @brief Updates the impulse response of the FIRFilter using linear interpolation
			*/
			inline void UpdateImpulseResponse(const Buffer& targetIr, const Real lerpFactor)
			{
				Lerp(impulseResponse, targetIr, lerpFactor);
			}

			inline const Buffer& GetImpulseResponse() const { return impulseResponse; }

			/**
			* @brief Sets the impulse response of the FIRFilter
			*
			* @param ir The new impulse response to set
			*/
			void SetImpulseResponse(const Buffer& ir);

			/**
			* @brief Reset internal inputLine to zeros
			*/
			inline void Reset() { inputLine.ResetBuffer(); }

		private:

			/**
			* @brief Increases the size of the impulse response and input line of the FIRFilter
			* @details New samples are initialised to 0.
			*
			* @param length The new length of the impulse response
			*/
			void IncreaseSize(const int length);

			/**
			* @brief Decreases the size of the impulse response and input line of the FIRFilter
			* @details The input line is shifted to ensure the most recent samples are retained.
			*
			* @param length The new length of the impulse response
			*/
			void DecreaseSize(const int length);

			Buffer impulseResponse;		// Impulse response buffer
			Buffer inputLine;			// Input line buffer

			int count;			// Index for the next sample entry to the input line buffer
		};
	}
}
#endif