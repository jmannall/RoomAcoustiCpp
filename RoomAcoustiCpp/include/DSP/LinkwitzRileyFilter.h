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

// DSP headers
#include "DSP/IIRFilter.h"

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"

namespace RAC
{
	using namespace Common;
	namespace DSP
	{
		class LinkwitzRiley
		{
		public:
			/**
			* @brief Constructor that initialises a default Linkwitz Riley filterbank
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LinkwitzRiley(const int sampleRate) : fm(4), fc(std::vector<Real>({ 176.0, 775.0, 3408.0 })),
				gains(4, 1.0) { InitFilters(sampleRate); CalcMidFrequencies(); }

			/**
			* @brief Constructor that initialises a Linkwitz Riley filterbank with three cutoff frequencies
			*
			* @param fc0 The first cutoff frequency
			* @param fc1 The second cutoff frequency
			* @param fc2 The third cutoff frequency
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LinkwitzRiley(const Real fc0, const Real fc1, const Real fc2, const int sampleRate) : fm(4),
				fc({ fc0, fc1, fc2 }), gains(4, 1.0) { InitFilters(sampleRate); CalcMidFrequencies(); }

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
			Real GetOutput(const Real input);

			/**
			* @brief Updates the gain parameters of the LinkwitzRiley filter
			*
			* @param filterGains The new gain parameters
			*/
			inline void UpdateParameters(const Coefficients& filterGains)
			{
				assert(filterGains.Length() == gains.Length());
				gains = filterGains;
			};

			Coefficients fm;		// Filter band mid frequencies

		private:
			/**
			* @brief Intialises the PassFilter sections
			*
			* @param sampleRate The samplerate for calculating filter coefficients
			*/
			void InitFilters(const int sampleRate);

			/**
			* @brief Calculate the pass band center frequencies 
			*/
			void CalcMidFrequencies();

			Coefficients fc;						// Filter band cut off frequencies
			Coefficients gains;						// Filter band gains
			std::vector<PassFilter> filters;		// PassFilter sections
		};
	}
}
#endif