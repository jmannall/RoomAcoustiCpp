/*
* @class IIRFilter
*
* @brief Declaration of base IIRFilter class and derived HighShelf, LowPass, ZPKFilter, BandFilter and PassFilter classes
*
*/

#ifndef DSP_IIRFilter_h
#define DSP_IIRFilter_h

// DSP headers
#include "DSP/Buffer.h"

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"

namespace UIE
{
	namespace DSP
	{
		/**
		 * Class that implements an Infinite Impulse Response filter
		 * 
		 * @details Uses the Direct-Form-II implementation
		 */
		class IIRFilter
		{
		public:
			/**
			 * Constructor that initialises an IIRFilter with a given filter order and sample rate
			 *
			 * @param filterOrder The order of the filter
			 * @param sampleRate The sample rate for calculating filter coefficients
			 */
			IIRFilter(const size_t& filterOrder, const int& sampleRate) : order(filterOrder),
				T(1.0 / static_cast<Real>(sampleRate)), b(filterOrder + 1),
				a(filterOrder + 1), x(filterOrder + 1), y(filterOrder + 1) {};
			
			/**
			 * Default deconstructor
			 */
			~IIRFilter() {};

			/**
			 * Returns the output of the IIRFilter given an input
			 *
			 * @param input The input to the IIRFilter
			 * @return The output of the IIRFilter
			 */
			Real GetOutput(const Real& input);

			/**
			 * Resets the input and output buffers to zeros.
			 */
			inline void ClearBuffers() { x.ResetBuffer(); y.ResetBuffer(); }

		protected:

			/**
			 * Filter order
			 */
			size_t order;

			/**
			 * Sample rate time period
			 */
			Real T;

			/**
			 * Filter coefficients
			 */
			Coefficients b;
			Coefficients a;

			/**
			 * Input (x) and output (y) buffers
			 */
			Buffer x;
			Buffer y;
		};

		/**
		 * Class that implements a 1st order high shelf IIR filter
		 */
		class HighShelf : public IIRFilter
		{
		public:
			/**
			 * Constructor that initialises a default 1st order high shelf filter
			 *
			 * @param sampleRate The sample rate for calculating filter coefficients
			 */
			HighShelf(const int& sampleRate) : IIRFilter(1, sampleRate) { UpdateParameters(1000.0, 1.0); };

			/**
			 * Constructor that initialises an 1st order high shelf filter with a given cut off frequency and shelf gain
			 *
			 * @param fc The cut off frequency of the filter
			 * @param g The shelf gain of the filter (linear)
			 * @param sampleRate The sample rate for calculating filter coefficients
			 */
			HighShelf(const Real& fc, const Real& g, const int& sampleRate) : IIRFilter(1, sampleRate) { UpdateParameters(fc, g); };

			/**
			 * Default deconstructor
			 */
			~HighShelf() {};

			/**
			* Updates the parameters of the high shelf filter
			* 
			* @param fc The cut off frequency of the filter
			* @param g The shelf gain of the filter (linear)
			*/
			void UpdateParameters(const Real& fc, const Real& g);
		};

		/**
		 * Class that implements a 1st order low pass IIR filter
		 */
		class LowPass : public IIRFilter
		{
		public:
			/**
			 * Constructor that initialises a deafult 1st order low pass filter
			 *
			 * @param sampleRate The sample rate for calculating filter coefficients
			 */
			LowPass(const int& sampleRate) : IIRFilter(1, sampleRate) { UpdateParameters(1000.0); };
			
			/**
			 * Constructor that initialises an 1st order low pass filter with a given cut off frequency
			 *
			 * @param fc The cut off frequency of the filter
			 * @param sampleRate The sample rate for calculating filter coefficients
			 */
			LowPass(const Real& fc, const int& sampleRate) : IIRFilter(1, sampleRate) { UpdateParameters(fc); };

			/**
			 * Default deconstructor
			 */
			~LowPass() {};

			/**
			* Updates the parameters of the low pass filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateParameters(const Real& fc);
		};

		struct ZPKParameters
		{
			Coefficients z;
			Coefficients p;
			Real k;
			ZPKParameters() : z(std::vector<Real>({ 0.25, -0.99 })), p(std::vector<Real>({ 0.99, -0.25 })), k(0.0) {};
			ZPKParameters(Real _z, Real _p, Real _k) : z(2, _z), p(2, _p), k(_k) {};
		};

		class ZPKFilter : public IIRFilter
		{
		public:
			ZPKFilter(const int& sampleRate) : IIRFilter(2, sampleRate) { a[0] = 1.0; UpdateParameters(ZPKParameters()); };
			ZPKFilter(const ZPKParameters& zpk, const int& sampleRate) : IIRFilter(2, sampleRate) { a[0] = 1.0; UpdateParameters(zpk); };

			void UpdateParameters(const ZPKParameters& zpk);
		};

		class PassFilter : public IIRFilter
		{
		public:
			PassFilter(const bool& isLowPass, const int& sampleRate) : IIRFilter(2, sampleRate) { a[0] = 1.0; SetUpdatePointer(isLowPass); };
			PassFilter(const Real& fc, const bool& isLowPass, const int& sampleRate) : IIRFilter(2, sampleRate)
			{ 
				a[0] = 1.0;
				SetUpdatePointer(isLowPass);
				UpdateParameters(fc);
			};

			void UpdateParameters(const Real& fc) { (this->*UpdatePass)(fc); };

			inline void SetUpdatePointer(bool isLowPass)
			{
				if (isLowPass)
					UpdatePass = &PassFilter::UpdateLowPass;
				else
					UpdatePass = &PassFilter::UpdateHighPass;
			};

		private:
			void UpdateLowPass(const Real& fc);
			void UpdateHighPass(const Real& fc);

			// Function pointer
			void (PassFilter::* UpdatePass)(const Real& fc);
		};
	}
}

#endif