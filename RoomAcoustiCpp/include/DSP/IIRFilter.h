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

namespace RAC
{
	namespace DSP
	{
		/**
		* @brief Class that implements an Infinite Impulse Response filter
		* 
		* @details Uses the Direct-Form-II implementation
		*/
		class IIRFilter
		{
		public:
			/**
			* @brief Constructor that initialises an IIRFilter with a given filter order and sample rate
			*
			* @param filterOrder The order of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			IIRFilter(const int filterOrder, const int sampleRate) : order(filterOrder),
				T(1.0 / static_cast<Real>(sampleRate)), b(filterOrder + 1),
				a(filterOrder + 1), x(filterOrder + 1), y(filterOrder + 1) {};
			
			/**
			* @brief Default deconstructor
			*/
			~IIRFilter() {};

			/**
			* @brief Returns the output of the IIRFilter given an input
			*
			* @param input The input to the IIRFilter
			* @return The output of the IIRFilter
			*/
			Real GetOutput(const Real input);

			/**
			* @brief Resets the input and output buffers to zeros.
			*/
			inline void ClearBuffers() { x.ResetBuffer(); y.ResetBuffer(); }

			/**
			* @brief Returns the filter response at given frequencies
			*
			* @param frequencies The frequencies at which to calculate the response
			* @return The frequency response of the filter
			*/
			std::vector<Real> GetFrequencyResponse(const std::vector<Real>& frequencies) const;

		protected:

			int order;		// Order of the filter
			Real T;			// Sample rate time period

			Coefficients b;		// Numerator coefficients
			Coefficients a;		// Denominator coefficients
			Buffer x;			// Input buffer
			Buffer y;			// Output buffer
		};

		/**
		* @brief Class that implements a second order Infinite Impulse Response filter
		*
		* @details Uses the Direct-Form-II implementation
		*/
		class IIRFilter1
		{
		public:
			/**
			* @brief Constructor that initialises a second order IIRFilter with a given sample rate
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			IIRFilter1(const int sampleRate) : order(1),
				T(1.0 / static_cast<Real>(sampleRate)), y0(0.0), y1(0.0),
				a0(0.0), a1(0.0), b0(0.0), b1(0.0) {};

			/**
			* @brief Default deconstructor
			*/
			~IIRFilter1() {};

			/**
			* @brief Returns the output of the IIRFilter given an input
			*
			* @param input The input to the IIRFilter
			* @return The output of the IIRFilter
			*/
			Real GetOutput(const Real input);

			/**
			* @brief Resets the output buffers to zeros.
			*/
			inline void ClearBuffers() { y0 = 0.0; y1 = 0.0; }

			/**
			* @brief Returns the filter response at given frequencies
			*
			* @param frequencies The frequencies at which to calculate the response
			* @return The frequency response of the filter
			*/
			std::vector<Real> GetFrequencyResponse(const std::vector<Real>& frequencies) const;

		protected:

			int order;		// Order of the filter
			Real T;			// Sample rate time period

			Real a0;		// First Denominator coefficient
			Real a1;		// Second Denominator coefficient
			Real b0;		// First Numerator coefficient
			Real b1;		// Second Numerator coefficient
			Real y0;		// Current output
			Real y1;		// Previous output
		};

		/**
		* @brief Class that implements a second order Infinite Impulse Response filter
		*
		* @details Uses the Direct-Form-II implementation
		*/
		class IIRFilter2
		{
		public:
			/**
			* @brief Constructor that initialises a second order IIRFilter with a given sample rate
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			IIRFilter2(const int sampleRate) : order(2),
				T(1.0 / static_cast<Real>(sampleRate)), y0(0.0), y1(0.0), y2(0.0),
				a0(0.0), a1(0.0), a2(0.0), b0(0.0), b1(0.0), b2(0.0) {};

			/**
			* @brief Default deconstructor
			*/
			~IIRFilter2() {};

			/**
			* @brief Returns the output of the IIRFilter given an input
			*
			* @param input The input to the IIRFilter
			* @return The output of the IIRFilter
			*/
			Real GetOutput(const Real input);

			/**
			* @brief Resets the output buffers to zeros.
			*/
			inline void ClearBuffers() { y0 = 0.0; y1 = 0.0; y2 = 0.0; }

			/**
			* @brief Returns the filter response at given frequencies
			*
			* @param frequencies The frequencies at which to calculate the response
			* @return The frequency response of the filter
			*/
			std::vector<Real> GetFrequencyResponse(const std::vector<Real>& frequencies) const;

		protected:

			int order;		// Order of the filter	
			Real T;			// Sample rate time period

			Real a0;		// First Denominator coefficient
			Real a1;		// Second Denominator coefficient
			Real a2;		// Third Denominator coefficient
			Real b0;		// First Numerator coefficient
			Real b1;		// Second Numerator coefficient
			Real b2;		// Third Numerator coefficient
			Real y0;		// Current output
			Real y1;		// Previous output
			Real y2;		// Previous previous output
		};

		/**
		* @brief Class that implements a 1st order high shelf IIR filter
		*/
		class HighShelf : public IIRFilter1
		{
		public:
			/**
			* @brief Constructor that initialises a default 1st order high shelf filter
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			HighShelf(const int sampleRate) : IIRFilter1(sampleRate) { UpdateParameters(1000.0, 1.0); };

			/**
			* @brief Constructor that initialises an 1st order high shelf filter with a given cut off frequency and shelf gain
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			HighShelf(const Real fc, const Real gain, const int sampleRate) : IIRFilter1(sampleRate) { UpdateParameters(fc, gain); };

			/**
			* @brief Default deconstructor
			*/
			~HighShelf() {};

			/**
			* @brief Updates the parameters of the high shelf filter
			* 
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			*/
			void UpdateParameters(const Real fc, const Real gain);
		};

		/**
		* @brief Class that implements a 1st order low pass IIR filter
		*/
		class LowPass : public IIRFilter1
		{
		public:
			/**
			* @brief Constructor that initialises a deafult 1st order low pass filter
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LowPass(const int sampleRate) : IIRFilter1(sampleRate) { UpdateParameters(1000.0); };
			
			/**
			* @brief Constructor that initialises an 1st order low pass filter with a given cut off frequency
			*
			* @param fc The cut off frequency of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LowPass(const Real fc, const int sampleRate) : IIRFilter1(sampleRate) { UpdateParameters(fc); };

			/**
			* @brief Default deconstructor
			*/
			~LowPass() {};

			/**
			* @brief Updates the parameters of the low pass filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateParameters(const Real fc);
		};

		/**
		* @brief Class that implements a 2nd order high shelf IIR filter (used by GraphicEQ)
		*/
		class PeakHighShelf : public IIRFilter2
		{
		public:
			/**
			* @brief Constructor that initialises an 2nd order high shelf filter with a given cut off frequency
			*
			* @param fc The cut off frequency of the filter
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakHighShelf(const Real fc, const Real Q, const int sampleRate) : IIRFilter2(sampleRate) { SetParameters(fc, Q); }

			/**
			* @brief Constructor that initialises an 2nd order high shelf filter with a given cut off frequency and shelf gain
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakHighShelf(const Real fc, const Real gain, const Real Q, const int sampleRate) : IIRFilter2(sampleRate) { SetParameters(fc, Q); UpdateGain(gain); }

			/**
			* @brief Default deconstructor
			*/
			~PeakHighShelf() {};

			/**
			* @brief Updates the gain of the high shelf filter
			*
			* @param gain The shelf gain of the filter (linear)
			*/
			void UpdateGain(const Real gain);

		private:
			/**
			* @brief Sets the cut off frequency of the high shelf filter
			*
			* @param fs The cut off frequency of the filter
			* @param Q The quality factor of the filter
			*/
			void SetParameters(const Real fc, const Real Q);

			Real cosOmega;		// Cos of the cut off frequency
			Real alpha;			// Alpha value for the filter
		};

		/**
		* @brief Class that implements a 2nd order low shelf IIR filter (used by GraphicEQ)
		*/
		class PeakLowShelf : public IIRFilter2
		{
		public:
			/**
			* @brief Constructor that initialises an 2nd order low shelf filter with a given cut off frequency
			*
			* @param fc The cut off frequency of the filter
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakLowShelf(const Real fc, const Real Q, const int sampleRate) : IIRFilter2(sampleRate) { SetParameters(fc, Q); }

			/**
			* @brief Constructor that initialises an 2nd order low shelf filter with a given cut off frequency and shelf gain
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakLowShelf(const Real fc, const Real gain, const Real Q, const int sampleRate) : IIRFilter2(sampleRate) { SetParameters(fc, Q); UpdateGain(gain); }

			/**
			* @brief Default deconstructor
			*/
			~PeakLowShelf() {};

			/**
			* @brief Updates the gain of the low shelf filter
			*
			* @param gain The shelf gain of the filter (linear)
			*/
			void UpdateGain(const Real gain);

		private:
			/**
			* @brief Sets the cut off frequency of the low shelf filter
			*
			* @param fs The cut off frequency of the filter
			* @param Q The quality factor of the filter
			*/
			void SetParameters(const Real fc, const Real Q);

			Real cosOmega;		// Cos of the cut off frequency
			Real alpha;			// Alpha value for the filter
		};

		/**
		* @brief Class that implements a 2nd order peaking IIR filter (used by GraphicEQ)
		*/
		class PeakingFilter : public IIRFilter2
		{
		public:
			/**
			* @brief Constructor that initialises an 2nd order peaking filter with a given cut off frequency
			*
			* @param fc The center frequency of the filter
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakingFilter(const Real fc, const Real Q, const int sampleRate) : IIRFilter2(sampleRate) { SetParameters(fc, Q); }

			/**
			* @brief Constructor that initialises an 2nd order peaking filter with a given cut off frequency and gain
			*
			* @param fc The center frequency of the filter
			* @param gain The gain of the filter (linear)
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakingFilter(const Real fc, const Real gain, const Real Q, const int sampleRate) : IIRFilter2(sampleRate) { SetParameters(fc, Q); UpdateGain(gain); }

			/**
			* @brief Default deconstructor
			*/
			~PeakingFilter() {};

			/**
			* @brief Updates the gain of the peaking filter
			*
			* @param gain The gain of the filter (linear)
			*/
			void UpdateGain(const Real gain);

		private:
			/**
			* @brief Sets the center frequency of the peaking filter
			*
			* @param fs The center frequency of the filter
			* @param Q The quality factor of the filter
			*/
			void SetParameters(const Real fc, const Real Q);

			Real cosOmega;		// Cos of the cut off frequency
			Real alpha;			// Alpha value for the filter
		};

		/**
		* @brief Class that implements a 2nd order IIR filter from poles, zeros and gain (used by NN models)
		*/
		class ZPKFilter : public IIRFilter2
		{
		public:
			/**
			* @brief Constructor that initialises a default second order IIRFilter with a given sample rate
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			ZPKFilter(const int& sampleRate) : IIRFilter2(sampleRate) { a0 = 1.0; UpdateParameters(Coefficients(std::vector<Real>({ 0.25, -0.99, 0.99, -0.25, 0.0 }))); };
			
			/**
			* @brief Constructor that initialises a second order IIRFilter with a given sample rate
			*
			* @param zpk The poles, zeros and gain of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			ZPKFilter(const Coefficients& zpk, const int& sampleRate) : IIRFilter2(sampleRate) { a0 = 1.0; UpdateParameters(zpk); };

			/**
			* @brief Updates the parameters of the IIR filter
			* 
			* @param zpk The new poles, zeros and gain of the filter
			*/
			void UpdateParameters(const Coefficients& zpk);
		};

		/**
		* @brief Class that implements a low or high pass IIR filter (used by LinkwitzRiley filter)
		*/
		class PassFilter : public IIRFilter2
		{
		public:
			/**
			* @brief Constructor that intialises a default pass filter with a given sample rate
			* 
			* @param isLowPass True if the filter is a low pass filter, false if it is a high pass filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PassFilter(const bool isLowPass, const int sampleRate) : IIRFilter2(sampleRate) { a0 = 1.0; SetUpdatePointer(isLowPass); };
			
			/**
			* @brief Constructor that intialises a default pass filter with a given cut off frequency and sample rate
			*
			* @param fc The cut off frequency of the filter
			* @param isLowPass True if the filter is a low pass filter, false if it is a high pass filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PassFilter(const Real fc, const bool isLowPass, const int sampleRate) : IIRFilter2(sampleRate)
			{ 
				a0 = 1.0;
				SetUpdatePointer(isLowPass);
				UpdateParameters(fc);
			};

			/**
			* @brief Updates the parameters of the pass filter
			* 
			* @param fc The new cut off frequency of the filter
			*/
			void UpdateParameters(const Real fc) { (this->*UpdatePass)(fc); };

		private:
			/**
			* @brief Sets the function pointer to the correct update function
			*
			* @param isLowPass True if the filter is a low pass filter, false if it is a high pass filter
			*/
			inline void SetUpdatePointer(bool isLowPass)
			{
				if (isLowPass)
					UpdatePass = &PassFilter::UpdateLowPass;
				else
					UpdatePass = &PassFilter::UpdateHighPass;
			};

			/**
			* @brief Updates the parameters of the low pass filter
			* 
			* @param fc The cut off frequency of the filter
			*/
			void UpdateLowPass(const Real fc);

			/**
			* @brief Updates the parameters of the high pass filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateHighPass(const Real fc);

			void (PassFilter::* UpdatePass)(const Real fc);		// Function pointer to the update function
		};
	}
}

#endif