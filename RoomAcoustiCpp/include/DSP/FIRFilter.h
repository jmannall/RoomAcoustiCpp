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
#include "Common/ReleasePool.h"

namespace RAC
{
	namespace DSP
	{
		/**
		* @brief Class that implements a lock free Finite Impulse Response filter with variable length
		*/
		class FIRFilter
		{
		public:
			/**
			* @brief Constructor that initialises the FIRFilter with a given impulse response and maximum size
			*
			* @param impulseResponse The impulse response to initialise the FIRFilter with
			* @param maxSize The maximum size of the FIRFilter
			*/
			FIRFilter(const Buffer& ir, const int maxSize) : maxFilterLength((maxSize % 8 == 0) ? maxSize : (maxSize + (8 - maxSize % 8))),
				inputLine(2 * maxFilterLength), currentIR(maxFilterLength)
			{
				if (!SetTargetIR(ir))
					return;

				assert(ir.Length() <= maxSize);

				irLength = targetIR.load(std::memory_order_acquire)->Length();
				oldIrLength = irLength;

				std::copy(ir.begin(), ir.end(), currentIR.begin());
				irsEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			};
			
			/**
			* @brief Default deconstructor
			*/
			~FIRFilter() {};

			/**
			* @brief Returns the output of the FIRFilter given an input
			*
			* @param input The input to the FIRFilter
			* @param lerpFactor The lerp factor for interpolation
			* @return The output of the FIRFilter
			*/
			Real GetOutput(const Real input, const Real lerpFactor);

			/**
			* @brief Atomically sets the new target impulse response
			*
			* @param ir The new target impulse response
			* @return True if the target impulse response was set successfully, false otherwise
			*/
			bool SetTargetIR(const Buffer& ir);

			/**
			* @brief Set flag to clear input line to zeros next time GetOutput is called
			*/
			inline void Reset() { clearInputLine.store(true, std::memory_order_release); }

		private:

			/*
			* @brief Linearly interpolates the current impulse response with the target impulse response
			* 
			* @param lerpFactor The lerp factor for interpolation
			*/
			void InterpolateIR(const Real lerpFactor);
				
            const int maxFilterLength;	// Maximum filter length

			std::atomic<std::shared_ptr<const Buffer>> targetIR;	// Target impulse response

			Buffer currentIR;	// Current impulse response buffer (should only be accessed from the audio thread)
			Buffer inputLine;	// Input line buffer (should only be accessed from the audio thread)

			size_t irLength;		// Length of the current impulse response (should only be accessed from the audio thread)
			size_t oldIrLength;		// Previous length of the impulse response (should only be accessed from the audio thread)
			int count{ 0 };			// Index for the next sample entry to the input line buffer (should only be accessed from the audio thread)

			std::atomic<bool> clearInputLine;	// Flag to clear input line
			std::atomic<bool> irsEqual;			// True if the current impulse response is known to be equal to the target impulse response
			std::atomic<bool> initialised;		// True if the filter has been initialised, false otherwise

			static ReleasePool releasePool;		// Garbage collector for shared pointers after atomic replacement
		};
	}
}
#endif // DSP_FIRFilter_h