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
			FIRFilter(const Buffer<>& ir, const int maxSize) : maxFilterLength((maxSize % 8 == 0) ? maxSize : (maxSize + (8 - maxSize % 8))),
				inputLine(2 * maxFilterLength), currentIR(maxFilterLength)
			{
				if (!SetTargetIR(ir))
					return;

				assert(ir.Length() <= maxSize);

#ifdef __ANDROID__
				irLength = std::atomic_load(&targetIR)->Length();
#else
				irLength = targetIR.load(std::memory_order_acquire)->Length();
#endif
				oldIrLength = irLength;

				std::copy(ir.begin(), ir.end(), currentIR.begin());
#if MATRIX_LIBRARY == EIGEN_FLAG
				inputLine.Reset();
#endif
				irsEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			};
			
			/**
			 * @brief Returns if this filter is valid.
			 *
			 * @return true if the valid is valid and GetOutput() can be called.
			 */
			bool IsValid() const { return initialised.load(std::memory_order_acquire); }

			/**
			* @brief Returns the output of the FIRFilter given an input
			*
			* @param input The input to the FIRFilter
			* @param lerpFactor The lerp factor for interpolation
			* @return The output of the FIRFilter
			*/
			virtual Real GetOutput(const Real input, const Real lerpFactor);

			/**
			* @brief Atomically sets the new target impulse response
			*
			* @param ir The new target impulse response
			* @return True if the target impulse response was set successfully, false otherwise
			*/
			bool SetTargetIR(const Buffer<>& ir);

			/**
			* @brief Set the internal input line to zeros
			*/
			inline void ClearBuffers() { inputLine.Reset(); }

		protected:
			const int maxFilterLength;	// Maximum filter length
			size_t irLength;		// Length of the current impulse response (should only be accessed from the audio thread)

			std::atomic<bool> initialised;		// True if the filter has been initialised, false otherwise
            
			Buffer<> currentIR;	// Current impulse response buffer (should only be accessed from the audio thread)
			Buffer<> inputLine;	// Input line buffer (should only be accessed from the audio thread)

			int count{ 0 };			// Index for the next sample entry to the input line buffer (should only be accessed from the audio thread)

		private:

			/*
			* @brief Linearly interpolates the current impulse response with the target impulse response
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			void InterpolateIR(const Real lerpFactor);

#ifdef __ANDROID__
			std::shared_ptr<Buffer<>> targetIR;		// Target impulse response
#else
			std::atomic<std::shared_ptr<const Buffer<>>> targetIR;	// Target impulse response
#endif

			

			size_t oldIrLength;		// Previous length of the impulse response (should only be accessed from the audio thread)

			std::atomic<bool> irsEqual;			// True if the current impulse response is known to be equal to the target impulse response

			static ReleasePool releasePool;		// Garbage collector for shared pointers after atomic replacement
		};
	}
}
#endif // DSP_FIRFilter_h