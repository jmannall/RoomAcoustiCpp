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
			* @brief Constructor that initialises the GraphicEQ with given frequency bands, Q factor and sample rate
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
			* @brief Default deconstructor
			*/
			~GraphicEQ() {};

			/**
			* @brief Initialises the filter parameters with the given gains
			*
			* @param targetBandGains The target gains for each frequency band
			*/
			void InitParameters(const Coefficients& targetBandGains);

			/**
			* @brief Interpolates between the current and target gains and updates the filter parameters
			*
			* @param lerpFactor The linear interpolation factor
			*/
			inline void UpdateParameters(const Real lerpFactor)
			{
				if (equal)
					return;
				if (Equals(currentFilterGains, targetFilterGains))
				{
					currentFilterGains = targetFilterGains;
					equal = true;
					UpdateParameters();
				}
				else
				{
					Lerp(currentFilterGains, targetFilterGains, lerpFactor);
					UpdateParameters();
				}
			}

			/**
			* @brief Sets the current gains of the filter bank
			*
			* @param targetBandGains The target gains for each frequency band
			*/
			void SetGain(const Coefficients& targetBandGains);

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
				lowShelf.ClearBuffers();
				for (auto& filter : peakingFilters)
					filter->ClearBuffers();
				highShelf.ClearBuffers();
			}

			/**
			* @return True if the filter is invalid, false otherwise
			*/
			inline bool Invalid() { return !valid; }

			/**
			* @return The overall gain of the filter
			*/
			inline Real GetDCGain() { return currentFilterGains[0]; }

		private:

			/**
			* @brief Updates the filter parameters using the current gain
			*/
			void UpdateParameters();

			/**
			* @brief Initialises the filter bands with the given center frequencies and Q factor
			*
			* @param fc The filter band center frequencies
			* @param Q The Q factor
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			void InitFilters(const Coefficients& fc, const Real Q, const int sampleRate);

			/**
			* @brief Initialises the matrix used to calculate the input gains
			*
			* @param fc The filter band center frequencies
			*/
			void InitMatrix(const Coefficients& fc);

			int numFilters;									// Number of filters
			PeakLowShelf lowShelf;							// Low-shelf filter
			PeakHighShelf highShelf;						// High-shelf filter
			std::vector<std::unique_ptr<PeakingFilter>> peakingFilters;		// Peaking filters

			Coefficients lastInput;				// Previous target filter gains
			Coefficients targetFilterGains;		// Target filter gains
			Coefficients currentFilterGains;	// Current filter gains

			Matrix filterResponseMatrix;		// Matrix used to calculate the targetFilterGains
			Rowvec dbGains;						// Stores dB gains during SetGain()
			Rowvec inputGains;					// Stores gain values during SetGain()

			bool equal;		// True if currentFilterGains == targetFilterGains
			bool valid;		// True if currentFilterGains[0] != 0
		};
	}
}
#endif