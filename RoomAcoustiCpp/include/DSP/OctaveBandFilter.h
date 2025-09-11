/*
* @class OctaveBand
*
* @brief Declaration of OctaveBand filter class
*
* @remarks Based after Linear-Phase Octave Graphic Equalizer. Bruschi V, Valimaki V, Liski J, Cecchi L. 2022
*
*/

#ifndef DSP_OctaveBandFilter_h
#define DSP_OctaveBandFilter_h

// C++ headers
#include <cassert>
#include <memory>
#include <atomic>

// DSP headers
#include "DSP/Interpolate.h"
#include "DSP/FIRFilter.h"
#include "DSP/DelayLine.h"

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"
#include "Common/ReleasePool.h"

namespace RAC
{
	using namespace Common;
	namespace DSP
	{
		/**
		* @brief Class that implements a linear-phase octave band filter
		* 
		* @remarks The current implementation begins at the band centre frequencies at 16kHz and halves the frequency for each band below it
		*/
		class OctaveBand
		{
			/**
			* @brief Class that implements the internal low-pass filters
			* 
			* @remarks The impulse response is zero every other sample, so the filter only expects the non-zero samples
			*/
			class Filter
			{
			public:

				/**
				* @brief Constructor that initialises the Filter with a given impulse response, midSample and step size
				* 
				* @param h The non-zero samples of the impulse response to initialise the Filter with
				* @param midSample The mid sample value of the impulse response (The only non-zero odd sample)
				* @param step The step size for stretched versions of the base filter (h)
				*/
				Filter(const Buffer<>& h, Real midSample, int step) : currentIR(h), step(step), midSample(midSample), midSampleStep(step * (2 * h.Length() - 1)),
					halfOutputLine(2 * step * (2 * h.Length() - 1) + 1), outputLine(4 * step * (2 * h.Length() - 1) + 2),
					count(4 * step * (2 * h.Length() - 1) + 1), irLength(h.Length()) {}

				/**
				* @brief Returns the output of the Filter given an input
				*
				* @param input The input to the Filter
				* @return The output of the Filter
				*/
				Real GetOutput(const Real input);

				/**
				* @brief Set the internal input line to zeros
				*/
				inline void ClearBuffers() { outputLine.Reset(); }

			private:

				const Buffer<> currentIR;			// Current impulse response buffer
				Buffer<> outputLine;

				const Real midSample;		// Value of the mid sample in the impulse response
				const int step;				// Step size for stretched versions of the base filter
				const int midSampleStep;	// Sample index of the mid sample in the impulse response
				const int halfOutputLine;	// Half the length of the output line
				const int irLength;

				int count;			// Current index in the output line
			};

			typedef Coefficients<> Parameters;
			typedef Coefficients<std::array<Real, 9>> CutOffFrequencies;
			CutOffFrequencies cutOffFrequencies{ CutOffFrequencies({ 46.875, 93.75, 187.5, 375.0, 750.0, 1.5e3, 3e3, 6e3, 12e3 }) };	// Possible frequencyBands

		public:
			/**
			* @brief Constructor that initialises the OctaveBand filter with given gains, sample rate and number of frequency bands
			* 
			* @param fs The sample rate for calculating the filter coefficients
			* @param numFrequencyBands The number of frequency bands for the filter
			*/
			OctaveBand(const Coefficients<>& frequencies, int fs);

			/**
			* @brief Returns the index of the octave band output that corresponds to a given frequency index
			* 
			* @param frequencyIndex The index of the frequency in the frequencies parameter given at construction
			* @return The index of the octave band output that corresponds to the given frequency index
			*/
			int GetBandIndex(int frequencyIndex) const { return octaveBandIndices[frequencyIndex] - numTopBandsToSum; }
			
			/**
			* @brief Returns the output of the OctaveBand filter given an input
			* 
			* @param input The input to the OctaveBand filter
			* @param lerpFactor The lerp factor for interpolation
			* @return A vector containing the outputs of each frequency band
			*/
			const std::vector<Real>& GetOutput(Real input, Real lerpFactor);


			/**
			* @brief Resets the filter buffers
			*/
			inline void ClearBuffers()
			{
				for (auto& filter : filters)
					filter->ClearBuffers();
				for (auto& delayLine : delayLines)
					delayLine.Reset();
			}

			inline int NumBands() const { return numOutputBands; }

			inline int GetLatency() const { return (std::pow((Real)2.0, numFrequencyBands) - 1) * Dwin; }

		private:
			inline Vec<int> CreateFrequencyIndices(Coefficients<> frequencies)
			{
				Vec<int> indices = Vec<int>(frequencies.Length());
				for (int i = 0; i < frequencies.Length(); i++)
					indices[i] = GetFrequencyIndex(frequencies[i]);
				return indices;
			}

			void InitFilter(int fs);

			int GetFrequencyIndex(Real f) const;

			void CombineTopBands(const std::vector<Real>& bands);

			/**
			* @param fs The sample rate for calculating the filter coefficients
			* @return Returns the impulse response of the low-pass filter
			*/
			Buffer<> CalculateH(int fs)
			{
				std::array<Real, Lwin> window = GenerateHanningWindow();
				Buffer<> h((Dwin + 1) / 2);
				int count = 0;
				Real omega = PI_2 * fc / static_cast<Real>(fs);
				for (int i = 0; i < (Dwin + 1) / 2; i++)
					h[i] = window[2 * i] * (sin(omega * (2 * i - Dwin)) / (PI_1 * (2 * i - Dwin)));
				return h;
			}

			static const int filterOrder{ 18 };			// Order of the low-pass filter (must be even)
			static const int Lwin{ filterOrder + 1 };	// Length of the window function (must be odd)
			static const int Dwin{ filterOrder / 2 };	// Half the filter order (must be even)

			/**
			* @return Returns a Hanning window function of length Lwin
			*/
			static constexpr inline std::array<Real, Lwin> GenerateHanningWindow()
			{
				std::array<Real, Lwin> w = {};
				for (int i = 0; i < Lwin; i++)
					w[i] = 0.5 * (1 - cos(PI_2 * (i + 1) / (Lwin + 1)));
				return w;
			}

			std::vector<Real> bands;
			std::vector<Real> outputs;
			const Vec<int> octaveBandIndices;

			int numFrequencyBands;		// Number of frequency bands for the filter
			int numTopBandsToSum;		// Number of highest octave bands to sum for the highest filter output
			int numOutputBands;			// Number of output bands (numFrequencyBands - numTopBandsToSum)
			const Real fc{ 12e3 };			// First cut-off frequency of the filter

			std::vector<DelayLine> delayLines;				// Filter delay lines, ordered with complementary filter delays first and then followed by the correction delays (highest band to the lowest band)
			std::vector<std::unique_ptr<Filter>> filters;	// Low-pass filters for each frequency band

			std::atomic<bool> initialised{ false };		// True if the filter has been initialised, false otherwise
		};
	}
}
#endif