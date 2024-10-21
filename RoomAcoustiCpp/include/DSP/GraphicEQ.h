/*
* @class GraphicEQ
*
* @brief Declaration of Graphic EQ class
* 
* @remarks Based after Efficient Multi-Band Digital Audio Graphic Equalizer with Accurate Frequency Response Control. Oliver R, Jot J. 2015
* 
*/

#ifndef DSP_GraphicEQ_h
#define DSP_GraphicEQ_h

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"
#include "Common/Matrix.h"
#include "Common/Vec.h"

// DSP headers
#include "DSP/IIRFilter.h"
#include "DSP/Interpolate.h"

namespace RAC
{
	namespace DSP
	{
		/**
		* Class that implements a graphic equaliser
		*/
		class GraphicEQ
		{
		public:
			/**
			* Constructor that initialises the GraphicEQ with given frequency bands, Q factor and sample rate
			*
			* @param fc The filter band center frequencies
			* @param Q The Q factor for the filters
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			GraphicEQ(const Coefficients& fc, const Real Q, const int sampleRate);

			/**
			* Constructor that initialises the GraphicEQ with given gains, frequency bands, Q factor and sample rate
			*
			* @param gain The gain for each filter band
			* @param fc The filter band center frequencies
			* @param Q The Q factor for the filters
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			GraphicEQ(const Coefficients& gain, const Coefficients& fc, const Real Q, const int sampleRate);

			/**
			* Default deconstructor
			*/
			~GraphicEQ() {};

			/**
			* Initialises the filter parameters with the given gains
			*
			* @param gain The gains for each filter band
			*/
			void InitParameters(const Coefficients& gain);

			/**
			* Interpolates between the current and target gains and updates the filter parameters
			*
			* @param lerpFactor The linear interpolation factor
			*/
			inline void UpdateParameters(const Real lerpFactor)
			{
				if (equal)
					return;
				if (Equals(currentGain, targetGain))
				{
					currentGain = targetGain;
					equal = true;
					UpdateParameters();
				}
				else
				{
					Lerp(currentGain, targetGain, lerpFactor);
					UpdateParameters();
				}
			}

			/**
			* Sets the current gain of the filter bank
			*
			* @param gain The new gains for each filter band
			*/
			void SetGain(const Coefficients& gain);

			/**
			* Returns the output of the GraphicEQ given an input
			*
			* @param input The input to the GraphicEQ
			* @return The output of the GraphicEQ
			*/
			Real GetOutput(const Real input);

			/**
			* Processes an input buffer and updates the output buffer
			*
			* @param inBuffer The input buffer
			* @param outBuffer The output buffer
			* @param numFrames The number of frames in the buffer
			* @param lerpFactor The linear interpolation factor
			*/
			void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor);

			/**
			* Resets the filter buffers
			*/
			inline void ClearBuffers()
			{
				lowShelf.ClearBuffers();
				for (PeakingFilter& filter : peakingFilters)
					filter.ClearBuffers();
				highShelf.ClearBuffers();
			}
		private:
			/**
			* Updates the filter parameters using the current gain
			*/
			void UpdateParameters();

			/**
			* Initialises the filter bands with the given center frequencies and Q factor
			*
			* @param fc The filter band center frequencies
			* @param Q The Q factor
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			void InitFilters(const Coefficients& fc, const Real Q, const int sampleRate);

			/**
			* Initialises the matrix used to calculate the input gains
			*
			* @param fc The filter band center frequencies
			*/
			void InitMatrix(const Coefficients& fc);

			/**
			* Number of filters
			*/
			size_t numFilters;

			/**
			* Filters
			*/
			PeakLowShelf lowShelf;
			PeakHighShelf highShelf;
			std::vector<PeakingFilter> peakingFilters;

			/**
			* Gain matrix
			*/
			matrix mat;

			/**
			* The stored input, target and current gains
			*/
			Coefficients lastInput;
			Coefficients targetGain;
			Coefficients currentGain;

			/**
			* Rowvecs used to store dB and gain values during SetGain()
			*/
			rowvec dbGain;
			rowvec inputGain;

			/**
			* Booleans to skip unnecessary update and audio calculations
			*/
			bool equal;
			bool valid;
		};
	}
}
#endif