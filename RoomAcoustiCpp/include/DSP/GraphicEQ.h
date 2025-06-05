/*
* @class GraphicEQ
*
* @brief Declaration of Graphic EQ class
* 
* @remarks Based after Efficient Multi-Band Digital Audio Graphic Equalizer with Accurate Frequency Response Control. Oliver R, Jot J. 2015
* This appears to have typos in the formulae. Instead see Audio EQ Cookbook:
* https://webaudio.github.io/Audio-EQ-Cookbook/Audio-EQ-Cookbook.txt
* https://www.w3.org/TR/audio-eq-cookbook/#intro
* 
*/

#ifndef DSP_GraphicEQ_h
#define DSP_GraphicEQ_h

// C++ headers
#include <vector> 

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
		* @brief Class that implements a graphic equaliser
		*/
		class GraphicEQ
		{
		public:
			/**
			* @brief Constructor that initialises the GraphicEQ with a default gain zero and the given frequency bands, Q factor and sample rate
			*
			* @param fc The filter band center frequencies
			* @param Q The Q factor for the filters
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			GraphicEQ(const Coefficients& fc, const Real Q, const int sampleRate) : GraphicEQ(Coefficients(fc.Length(), 0.0), fc, Q, sampleRate) {}

			/**
			* @brief Constructor that initialises the GraphicEQ with given gains, frequency bands, Q factor and sample rate
			*
			* @param gain The target gain for each center frequency
			* @param fc The filter band center frequencies
			* @param Q The Q factor for the filters
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			GraphicEQ(const Coefficients& gain, const Coefficients& fc, const Real Q, const int sampleRate);

			/**
			* @brief Default deconstructor
			*/
			~GraphicEQ() {};

			/**
			* @brief Sets new target gains for each center frequency
			* 
			* @param gains The target response for the GraphicEQ
			* @return True if the target and current gains are currently zero, false otherwise
			*/
			bool SetTargetGains(const Coefficients& gains);

			/**
			* @brief Returns the output of the GraphicEQ given an input
			*
			* @param input The input to the GraphicEQ
			* @return The output of the GraphicEQ
			*/
			Real GetOutput(const Real input, const Real lerpFactor);

			/**
			* @brief Processes an input buffer and updates the output buffer
			*
			* @param inBuffer The input buffer
			* @param outBuffer The output buffer
			* @param numFrames The number of frames in the buffer
			* @param lerpFactor The linear interpolation factor
			*/
			void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor);

			/**
			* @brief Resets the filter buffers
			*/
			inline void ClearBuffers()
			{
				lowShelf->ClearBuffers();
				for (auto& filter : peakingFilters)
					filter->ClearBuffers();
				highShelf->ClearBuffers();
			}

		private:
			/**
			* @brief Initialises a matrix representing the filter responses used to calculate the gain parameters for the filters
			* @ details: TO DO: Matrix inverse can behave badly if fc is not sensible eg: 250, 500 1e3, 20e3 20e3.
			* 
			* @param fc The filter band center frequencies
			* @param Q The Q factor for the filters
			* @param fs The sample rate for calculating the filter coefficients
			*/
			void InitMatrix(const Coefficients& fc, const Real Q, const Real fs);

			/**
			* @brief Calculates the filter gains based on the target filter response
			* 
			* @param gains The target response for the GraphicEQ
			* @returns A pair containing the calculated gain for each filter and a DC gain
			*/
			std::pair<Rowvec, Real> CalculateGains(const Coefficients& gains) const;

			/**
			* @brief Creates a frequency vector based on the target response center frequencies
			* 
			* @param fc The filter band center frequencies
			* @returns A frequency vector for calculating the filter responses
			*/
			Coefficients CreateFrequencyVector(const Coefficients& fc) const;

			/**
			* @brief Linearly interpolates the current gain with the target gain
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			void InterpolateGain(const Real lerpFactor);

			const int numFilters;			// Number of filters
			Coefficients previousInput;		// Previous target response to check if they have changed

			std::unique_ptr<PeakLowShelf> lowShelf;							// Low-shelf filter
			std::vector<std::unique_ptr<PeakingFilter>> peakingFilters;		// Peaking filters
			std::unique_ptr<PeakHighShelf> highShelf;						// High-shelf filter

			Matrix filterResponseMatrix;	// Matrix used to calculate the filter gains

			std::atomic<Real> targetGain;	// Target DC gain
			Real currentGain;				// Current DC gain (should only be accessed from the audio thread)

			std::atomic<bool> initialised{ false };		// True if the GraphicEQ has been initialised
			std::atomic<bool> gainsEqual{ false };		// True if the currentGain and targetGain are known to be equal

		};
	}
}
#endif