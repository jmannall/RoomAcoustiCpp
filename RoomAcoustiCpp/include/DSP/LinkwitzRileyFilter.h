/*
* @class LinkwitzRiley
*
* @brief Declaration of LinkwitzRiley filter class
*
*/

#ifndef DSP_LinkwitzRileyFilter_h
#define DSP_LinkwitzRileyFilter_h

// C++ headers
#include <cassert>
#include <memory>

// DSP headers
#include "DSP/IIRFilter.h"
#include "DSP/Interpolate.h"

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"

namespace RAC
{
	using namespace Common;
	namespace DSP
	{
		// NOTE!!! default values of Peaking filters 1000Hz, 1.0 gain. Not good for lr filter
		class LinkwitzRiley
		{
			typedef Coefficients<std::array<Real, 4>> Parameters;
		public:
			/**
			* @brief Constructor that initialises a default Linkwitz Riley filterbank
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LinkwitzRiley(const int sampleRate) : LinkwitzRiley(Parameters(1.0), { 176.0, 775.0, 3408.0 }, sampleRate) {}

			/**
			* @brief Constructor that initialises a default Linkwitz Riley filterbank
			*
			* @param gains The filter band gains
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LinkwitzRiley(const Parameters& gains, const int sampleRate) : LinkwitzRiley(gains, { 176.0, 775.0, 3408.0 }, sampleRate) {}

			/**
			* @brief Constructor that initialises a Linkwitz Riley filterbank with three cutoff frequencies
			*
			* @param gains The filter band gains
			* @param fc The cutoff frequencies
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LinkwitzRiley(const Parameters& gains, const std::array<Real, 3> fc, const int sampleRate) :
				fm(CalculateMidFrequencies(fc)), currentGains(gains)
			{
				InitFilters(sampleRate, fc);
				SetTargetGains(gains);

				gainsEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			}

			/**
			* @brief Default deconstructor
			*/
			~LinkwitzRiley() {};

			/**
			* @brief Returns the output of the LinkwitzRiley filter given an input
			*
			* @param input The input to the LinkwitzRiley Filter
			* @return The output of the LinkwitzRiley Filter
			*/
			Real GetOutput(const Real input, const Real lerpFactor);

			/**
			* @brief Updates the target gains of the LinkwitzRiley filter
			*
			* @param gains The new target gain parameters
			*/
			inline void SetTargetGains(const Parameters& gains)
			{
				std::shared_ptr<Parameters> gainsCopy = std::make_shared<Parameters>(gains);

				releasePool.Add(gainsCopy);
				targetGains.store(gainsCopy, std::memory_order_release);
				gainsEqual.store(false, std::memory_order_release);
			};

			/**
			* @brief Resets the filter buffers
			*/
			inline void ClearBuffers()
			{
				for (auto& filter : lowPassFilters)
					filter->ClearBuffers();
				for (auto& filter : highPassFilters)
					filter->ClearBuffers();
			}

			static inline Parameters DefaultFM() { return CalculateMidFrequencies({ 176.0, 775.0, 3408.0 }); }

			const Parameters fm;		// Filter band mid frequencies

		private:
			/**
			* @brief Intialises the PassFilter sections
			*
			* @param sampleRate The samplerate for calculating filter coefficients
			* @param fc The cutoff frequencies for the filters
			*/
			void InitFilters(const int sampleRate, const std::array<Real, 3>& fc);

			/**
			* @brief Calculate the pass band center frequencies
			*
			* @param fc The cutoff frequencies of the filters
			*/
			static inline Parameters CalculateMidFrequencies(const std::array<Real, 3>& fc)
			{
				return Parameters({ std::sqrt(20.0 * fc[0]), std::sqrt(fc[0] * fc[1]), std::sqrt(fc[1] * fc[2]), std::sqrt(fc[2] * 20000.0) });
			}

			/*
			* @brief Linearly interpolates the current gains with the target gains
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			void InterpolateGains(const Real lerpFactor);

			std::atomic<std::shared_ptr<Parameters>> targetGains;		// Target filter band gains
			Parameters currentGains;									// Current filter band gains (should only be accessed from the audio thread)
			
			std::array<std::optional<LowPass>, 10> lowPassFilters;		// LowFilter sections
			std::array<std::optional<HighPass>, 10> highPassFilters;	// HighPass sections

			std::atomic<bool> initialised{ false };		// True if the filter has been initialised, false otherwise
			std::atomic<bool> gainsEqual{ false };		// True if the current gains are know to be equal to the target gains

			static ReleasePool releasePool;		// ReleasePool for managing memory of shared pointers
		};
	}
}
#endif