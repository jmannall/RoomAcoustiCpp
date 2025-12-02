/*
* @class DCBlocker
*
* @brief Declaration of DCBlocker class
*
* @remark Based after DSP related DC Blocker
*
*/

#ifndef RoomAcoustiCpp_DCBlocker_h
#define RoomAcoustiCpp_DCBlocker_h

// Common headers
#include "Common/Types.h"

// DSP headers
#include "DSP/Buffer.h"
#include "DSP/IIRFilter.h"

namespace RAC
{
	namespace DSP
	{
		/**
		* @brief Class that implements an air absorption filter
		*/
		class DCBlocker
		{
		public:

			/**
			* @brief Constructor that initialises the AirAbsorption with a given distance and sample rate
			*
			* @param sampleRate The sample rate for calculating the filter pole
			*/
			DCBlocker(const int sampleRate) : R(std::exp(-PI_2 * REAL_CONST(20.0) / sampleRate)) {}

			/**
			* @brief Default deconstructor
			*/
			~DCBlocker() {}

			/**
			* @brief Removes the DC offset from a buffer
			*
			* @param buffer The buffer to apply the filter to
			*/
			inline void ProcessAudio(Buffer<>& buffer)
			{
				for (auto& sample : buffer)
				{
					x0 = sample;
					sample = x0 - x1 + R * y0;
					x1 = x0;
					y0 = sample;
				}
			}

		private:
			
			const Real R;		// Pole of the filter

			Real y0{ 0.0 };		// Previous output
			Real x0{ 0.0 };		// Current input
			Real x1{ 0.0 };		// Previous input
		};
	}
}

#endif
