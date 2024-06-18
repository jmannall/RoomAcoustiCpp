/*
* @class LinkwitzRileyFilter
*
* @brief Declaration of LinkwitzRileyFilter class
*
*/

#ifndef DSP_LinkwitzRileyFilter_h
#define DSP_LinkwitzRileyFilter_h

// C++ headers
#include "assert.h"

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
			 * Constructor that initialises a default Linkwitz Riley filterbank
			 *
			 * @param sampleRate The sample rate for calculating filter coefficients
			 */
			LinkwitzRiley(const int sampleRate) : fm(4), fc(std::vector<Real>({ 176.0, 775.0, 3408.0 })),
				g(4, 1.0) { InitFilters(sampleRate); CalcMidFrequencies(); }

			/**
			 * Constructor that initialises a Linkwitz Riley filterbank with three cutoff frequencies
			 *
			 * @param fc0 The first cutoff frequency
			 * @param fc1 The second cutoff frequency
			 * @param fc2 The third cutoff frequency
			 * @param sampleRate The sample rate for calculating filter coefficients
			 */
			LinkwitzRiley(const Real fc0, const Real fc1, const Real fc2, const int sampleRate) : fm(4),
				fc({ fc0, fc1, fc2 }), g(4, 1.0) { InitFilters(sampleRate); CalcMidFrequencies(); }

			/**
			 * Default deconstructor
			 */
			~LinkwitzRiley() {};

			/**
			 * Returns the output of the LinkwitzRiley filter given an input
			 *
			 * @param input The input to the FIRFilter
			 * @return The output of the FIRFilter
			 */
			Real GetOutput(const Real input);

			/**
			 * Updates the gain parameters of the LinkwitzRiley filter
			 *
			 * @param gain The new gain parameters
			 */
			inline void UpdateParameters(const Coefficients& gain)
			{
				assert(gain.Length() == g.Length());
				g = gain;
			};

			/**
			 * Filter band mid frequencies
			 */
			Coefficients fm;

		private:
			void InitFilters(const int fs);
			void CalcMidFrequencies();

			/**
			 * Filter band cut off frequencies and gains
			 */
			Coefficients fc;
			Coefficients g;

			/**
			 * Pass filters sections
			 */
			std::vector<PassFilter> filters;
		};
	}
}
#endif