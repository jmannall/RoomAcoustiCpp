/*
* @class IIRFilter
*
* @brief Declaration of IIRFilter, IIRFilter1, IIRFilter2 and IIRFilter2Param1 base classes
* and derived HighShelf, LowPass1, PeakHighShelf, PeakLowShelf, PeakingFilter, ZPKFilter, LowPass and HighPass classes
*
*/

#ifndef DSP_IIRFilter_h
#define DSP_IIRFilter_h

// C++ headers
#include <atomic>
#include <memory>

// DSP headers
#include "DSP/Buffer.h"

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"
#include "Common/ReleasePool.h"

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
			IIRFilter(const int filterOrder, const int sampleRate) : order(filterOrder), T(1.0 / static_cast<Real>(sampleRate)),
				b(filterOrder + 1), a(filterOrder + 1), y(filterOrder + 1) {};
			
			/**
			* @brief Default virtual deconstructor
			*/
			virtual ~IIRFilter() {};

			/**
			* @brief Returns the output of the IIRFilter given an input
			*
			* @param input The input to the IIRFilter
			* @param lerpFactor The lerp factor for interpolation
			* @return The output of the IIRFilter
			*/
			Real GetOutput(const Real input, const Real lerpFactor);

			/**
			* @brief Set flag to clear buffers to zeros next time GetOutput is called
			*/
			inline void ClearBuffers() { clearBuffers.store(true, std::memory_order_release); }

			/**
			* @brief Returns the filter response at given frequencies.
			* Not thread-safe, should only be called when GetOutput is not being called.
			*
			* @param frequencies The frequencies at which to calculate the response
			* @return The frequency response of the filter
			*/
			Coefficients<> GetFrequencyResponse(const Coefficients<>& frequencies) const;

		protected:
			const int order;		// Order of the filter
			const Real T;			// Sample rate time period

			Coefficients<> b;		// Numerator coefficients
			Coefficients<> a;		// Denominator coefficients
			Buffer<> y;			// Output buffer

			std::atomic<bool> parametersEqual{ false };		// True if the current parameters are known to be equal to the target parameters
			std::atomic<bool> initialised{ false };			// True if the filter has been initialised, false otherwise

		private:
			/**
			* @brief Pure virtual function to lineraly interpolates filter parameters. Must be overloaded in the derived classes
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			virtual void InterpolateParameters(const Real lerpFactor) = 0;

			std::atomic<bool> clearBuffers{ false };		// Flag to clear the output buffers to zeros next time GetOutput is called

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
			IIRFilter1(const int sampleRate) : T(1.0 / static_cast<Real>(sampleRate)) {};

			/**
			* @brief Default deconstructor
			*/
			virtual ~IIRFilter1() {};

			/**
			* @brief Returns the output of the IIRFilter given an input
			*
			* @param input The input to the IIRFilter
			* @return The output of the IIRFilter
			*/
			Real GetOutput(const Real input, const Real lerpFactor);

			/**
			* @brief Set flag to clear buffers to zeros next time GetOutput is called
			*/
			inline void ClearBuffers() { clearBuffers.store(true, std::memory_order_release); }

			/**
			* @brief Returns the filter response at given frequencies.
			* Not thread-safe, should only be called when GetOutput is not being called.
			*
			* @param frequencies The frequencies at which to calculate the response
			* @return The frequency response of the filter
			*/
			Coefficients<> GetFrequencyResponse(const Coefficients<>& frequencies) const;

		protected:
			const Real T;				// Sample rate time period

			Real a0{ 0.0 }, a1{ 0.0 };		// Denominator coefficients (should only be accessed from the audio thread)
			Real b0{ 0.0 }, b1{ 0.0 };		// Numerator coefficients (should only be accessed from the audio thread)
			Real y0{ 0.0 };					// Outputs (should only be accessed from the audio thread)

			std::atomic<bool> parametersEqual{ false };		// True if the current parameters are known to be equal to the target parameters
			std::atomic<bool> initialised{ false };			// True if the filter has been initialised, false otherwise
		
		private:
			/**
			* @brief Pure virtual function to lineraly interpolates filter parameters. Must be overloaded in the derived classes
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			virtual void InterpolateParameters(const Real lerpFactor) = 0;

			std::atomic<bool> clearBuffers{ false };		// Flag to clear the output buffers to zeros next time GetOutput is called

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
			IIRFilter2(const int sampleRate) : T(1.0 / static_cast<Real>(sampleRate)) {};

			/**
			* @brief Default deconstructor
			*/
			virtual ~IIRFilter2() {};

			/**
			* @brief Returns the output of the IIRFilter given an input
			*
			* @param input The input to the IIRFilter
			* @return The output of the IIRFilter
			*/
			Real GetOutput(const Real input, const Real lerpFactor);

			/**
			* @brief Set flag to clear buffers to zeros next time GetOutput is called
			*/
			inline void ClearBuffers() { clearBuffers.store(true, std::memory_order_release); }

			/**
			* @brief Returns the filter response at given frequencies.
			* Not thread-safe, should only be called when GetOutput is not being called.
			*
			* @param frequencies The frequencies at which to calculate the response
			* @return The frequency response of the filter
			*/
			Coefficients<> GetFrequencyResponse(const Coefficients<>& frequencies) const;

		protected:
			const Real T;				// Sample rate time period

			Real a0{ 0.0 }, a1{ 0.0 }, a2{ 0.0 };		// Denominator coefficients (should only be accessed from the audio thread)
			Real b0{ 0.0 }, b1{ 0.0 }, b2{ 0.0 };		// Numerator coefficients (should only be accessed from the audio thread)
			Real y0{ 0.0 }, y1{ 0.0 };					// Outputs (should only be accessed from the audio thread)
		
			std::atomic<bool> parametersEqual{ false };		// True if the current parameters are known to be equal to the target parameters
			std::atomic<bool> initialised{ false };			// True if the filter has been initialised, false otherwise
		
		private:
			/**
			* @brief Pure virtual function to lineraly interpolates filter parameters. Must be overloaded in the derived classes
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			virtual void InterpolateParameters(const Real lerpFactor) = 0;

			std::atomic<bool> clearBuffers{ false };		// Flag to clear the output buffers to zeros next time GetOutput is called
		
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
			HighShelf(const int sampleRate) : HighShelf(1000.0, 1.0, sampleRate) {};

			/**
			* @brief Constructor that initialises an 1st order high shelf filter with a given cut off frequency and shelf gain
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			HighShelf(const Real fc, const Real gain, const int sampleRate) : IIRFilter1(sampleRate),
				targetFc(fc), targetGain(gain), currentFc(fc), currentGain(gain)
			{
				assert(fc < static_cast<Real>(sampleRate) / 2.0); // Ensure cut off frequency is less than Nyquist frequency

				UpdateCoefficients(currentFc, currentGain);
				parametersEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			};

			/**
			* @brief Default deconstructor
			*/
			~HighShelf() {};

			/**
			* @brief Atomically sets the target parameters of the high shelf filter
			* 
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			*/
			inline void SetTargetParameters(const Real fc, const Real gain)
			{
				targetFc.store(fc, std::memory_order_release);
				targetGain.store(gain, std::memory_order_release);
				parametersEqual.store(false, std::memory_order_release);
			}

		private:
			/**
			* @brief Linearly interpolates the current fc and gain with the target fc and gain
			* 
			* @param lerpFactor The lerp factor for interpolation
			*/
			void InterpolateParameters(const Real lerpFactor) override;

			/**
			* @brief Updates the parameters of the high shelf filter
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			*/
			void UpdateCoefficients(const Real fc, const Real gain);

			std::atomic<Real> targetFc;		// Target cut off frequency
			std::atomic<Real> targetGain;	// Target shelf gain
			Real currentFc;					// Current cut off frequency
			Real currentGain;				// Current shelf gain
		};

		/**
		* @brief Class that implements a 1st order low pass IIR filter
		*/
		class LowPass1 : public IIRFilter1
		{
		public:
			/**
			* @brief Constructor that initialises a deafult 1st order low pass filter
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LowPass1(const int sampleRate) : LowPass1(1000.0, sampleRate) {};
			
			/**
			* @brief Constructor that initialises an 1st order low pass filter with a given cut off frequency
			*
			* @param fc The cut off frequency of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LowPass1(const Real fc, const int sampleRate) : IIRFilter1(sampleRate), targetFc(fc), currentFc(fc)
			{ 
				assert(fc < static_cast<Real>(sampleRate) / 2.0); // Ensure cut off frequency is less than Nyquist frequency

				UpdateCoefficients(currentFc);
				parametersEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			};

			/**
			* @brief Default deconstructor
			*/
			~LowPass1() {};

			/**
			* @brief Atomically sets the target cut-off frequency of the filter
			*
			* @param fc The cut off frequency of the filter
			*/
			inline void SetTargetFc(const Real fc) { targetFc.store(fc, std::memory_order_release); parametersEqual.store(false, std::memory_order_release); }

		private:
			/**
			* @brief Linearly interpolates the current fc with the target fc
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			void InterpolateParameters(const Real lerpFactor) override;

			/**
			* @brief Updates the parameters of the low pass filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateCoefficients(const Real fc);

			std::atomic<Real> targetFc;	// Target cut off frequency
			Real currentFc;				// Current cut off frequency
		};

		/**
		* @brief Class that implements a 2nd-order IIR filter with one parameter
		*/
		class IIRFilter2Param1 : public IIRFilter2
		{
		public:
			/**
			* @brief Constructor that intialises a default pass filter with a given cut off frequency and sample rate
			*
			* @param fc The cut off frequency of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			IIRFilter2Param1(const Real parameter, const int sampleRate) : IIRFilter2(sampleRate),
				current(parameter), target(parameter) {
			};

			virtual ~IIRFilter2Param1() {};

		protected:
			/**
			* @brief Atomically sets the target parameter of the filter
			*
			* @param parameter The target parameter of the filter
			*/
			inline void SetTargetParameter(const Real parameter) { target.store(parameter, std::memory_order_release); parametersEqual.store(false, std::memory_order_release); }

			/**
			* @brief Updates the parameters of the low pass filter
			*
			* @param parameter The control parameter of the filter
			*/
			virtual void UpdateCoefficients(const Real parameter) = 0;

		private:
			/**
			* @brief Linearly interpolates the current parameter with the target parameter
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			void InterpolateParameters(const Real lerpFactor) override;

			std::atomic<Real> target;	// Target filter parameter
			Real current;				// Current filter parameter
		};

		/**
		* @brief Class that implements a 2nd order high shelf IIR filter (used by GraphicEQ)
		*/
		class PeakHighShelf : public IIRFilter2Param1
		{
		public:
			/**
			* @brief Constructor that initialises an 2nd order high shelf filter with a given cut off frequency
			*
			* @param fc The cut off frequency of the filter
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakHighShelf(const Real fc, const Real Q, const int sampleRate) : PeakHighShelf(fc, 1.0, Q, sampleRate) {}

			/**
			* @brief Constructor that initialises an 2nd order high shelf filter with a given cut off frequency and shelf gain
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakHighShelf(const Real fc, const Real gain, const Real Q, const int sampleRate) : IIRFilter2Param1(gain, sampleRate),
				cosOmega(cos(PI_2 * fc * T)), alpha(sin(PI_2 * fc * T) / Q) // sin(omega) / (2 * Q) (factor of two cancelled out in UpdateGain)
			{ 
				assert(fc < static_cast<Real>(sampleRate) / 2.0); // Ensure cut off frequency is less than Nyquist frequency

				UpdateCoefficients(gain);
				parametersEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			}

			/**
			* @brief Default deconstructor
			*/
			~PeakHighShelf() {};

			/**
			* @brief Atomically sets the target gain of the filter
			*
			* @param parameter The target gain of the filter
			*/
			inline void SetTargetGain(const Real gain) { SetTargetParameter(gain); }

		private:
			/**
			* @brief Updates the parameters of the low pass filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateCoefficients(const Real gain) override;

			const Real cosOmega;		// Cos of the cut off frequency
			const Real alpha;			// Alpha value for the filter
		};

		/**
		* @brief Class that implements a 2nd order low shelf IIR filter (used by GraphicEQ)
		*/
		class PeakLowShelf : public IIRFilter2Param1
		{
		public:
			/**
			* @brief Constructor that initialises an 2nd order low shelf filter with a given cut off frequency
			*
			* @param fc The cut off frequency of the filter
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakLowShelf(const Real fc, const Real Q, const int sampleRate) : PeakLowShelf(fc, 1.0, Q, sampleRate) {}

			/**
			* @brief Constructor that initialises an 2nd order low shelf filter with a given cut off frequency and shelf gain
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakLowShelf(const Real fc, const Real gain, const Real Q, const int sampleRate) : IIRFilter2Param1(gain, sampleRate),
				cosOmega(cos(PI_2 * fc * T)), alpha(sin(PI_2 * fc * T) / Q) // sin(omega) / (2 * Q) (factor of two cancelled out in UpdateGain)
			{
				assert(fc < static_cast<Real>(sampleRate) / 2.0); // Ensure cut off frequency is less than Nyquist frequency

				UpdateCoefficients(gain);
				parametersEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			}

			/**
			* @brief Default deconstructor
			*/
			~PeakLowShelf() {};

			/**
			* @brief Atomically sets the target gain of the filter
			*
			* @param parameter The target gain of the filter
			*/
			inline void SetTargetGain(const Real gain) { SetTargetParameter(gain); }

		private:
			/**
			* @brief Updates the parameters of the low pass filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateCoefficients(const Real gain);

			const Real cosOmega;		// Cos of the cut off frequency
			const Real alpha;			// Alpha value for the filter
		};

		/**
		* @brief Class that implements a 2nd order peaking IIR filter (used by GraphicEQ)
		*/
		class PeakingFilter : public IIRFilter2Param1
		{
		public:
			/**
			* @brief Constructor that initialises an 2nd order peaking filter with a given cut off frequency
			*
			* @param fc The center frequency of the filter
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakingFilter(const Real fc, const Real Q, const int sampleRate) : PeakingFilter(fc, 1.0, Q, sampleRate) {}

			/**
			* @brief Constructor that initialises an 2nd order peaking filter with a given cut off frequency and gain
			*
			* @param fc The center frequency of the filter
			* @param gain The gain of the filter (linear)
			* @param Q The quality factor of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			PeakingFilter(const Real fc, const Real gain, const Real Q, const int sampleRate) : IIRFilter2Param1(gain, sampleRate),
				cosOmega(-2.0 * cos(PI_2 * fc * T)), alpha(sin(PI_2* fc* T) / (2.0 * Q))
			{
				assert(fc < static_cast<Real>(sampleRate) / 2.0); // Ensure cut off frequency is less than Nyquist frequency

				UpdateCoefficients(gain);
				parametersEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			}

			/**
			* @brief Default deconstructor
			*/
			~PeakingFilter() {};

			/**
			* @brief Atomically sets the target gain of the filter
			*
			* @param parameter The target gain of the filter
			*/
			inline void SetTargetGain(const Real gain) { SetTargetParameter(gain); }

		private:
			/**
			* @brief Updates the parameters of the peaking filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateCoefficients(const Real gain);

			const Real cosOmega;		// Cos of the cut off frequency
			const Real alpha;			// Alpha value for the filter
		};

		/**
		* @brief Class that implements a 2nd order IIR filter from poles, zeros and gain (used by NN models)
		*/
		class ZPKFilter : public IIRFilter2
		{
			using Parameters = Coefficients<std::array<Real, 5>>;
		public:
			/**
			* @brief Constructor that initialises a default second order IIRFilter with a given sample rate
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			ZPKFilter(const int sampleRate) : ZPKFilter(Parameters({ 0.25, -0.99, 0.99, -0.25, 0.0 }), sampleRate) {};
			
			/**
			* @brief Constructor that initialises a second order IIRFilter with a given sample rate
			*
			* @param zpk The poles, zeros and gain of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			ZPKFilter(const Parameters& zpk, const int& sampleRate) : IIRFilter2(sampleRate),
				currentZPK(zpk)
			{
				SetTargetParameters(zpk);

				a0 = 1.0;
				UpdateCoefficients(currentZPK);
				parametersEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			};

			/**
			* @brief Atomically sets the target ZPK parameters of the filter
			*
			* @param parameter The target ZPK parameters of the filter
			*/
			void SetTargetParameters(const Parameters& zpk);

			void SetTargetGain(const Real k);

		private:
			/**
			* @brief Linearly interpolates the current ZPK parameters with the target ZPK parameters
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			void InterpolateParameters(const Real lerpFactor) override;

			/**
			* @brief Updates the parameters of the low pass filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateCoefficients(const Parameters& zpk);

#ifdef __ANDROID__
			std::shared_ptr<Parameters> targetZPK;	// Target ZPK parameters
#else
			std::atomic<std::shared_ptr<Parameters>> targetZPK;	// Target ZPK parameters
#endif
			Parameters currentZPK;								// Current ZPK parameters

			static ReleasePool releasePool;		// ReleasePool for managing memory of shared pointers
		};

		/**
		* @brief Class that implements a low or high pass IIR filter (used by LinkwitzRiley filter)
		*/
		class LowPass : public IIRFilter2Param1
		{
		public:
			/**
			* @brief Constructor that intialises a default pass filter with a given sample rate
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LowPass(const int sampleRate) : LowPass(1000.0, sampleRate) {}
			

			/**
			* @brief Constructor that intialises a default pass filter with a given cut off frequency and sample rate
			*
			* @param fc The cut off frequency of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			LowPass(const Real fc, const int sampleRate) : IIRFilter2Param1(fc, sampleRate)
			{
				UpdateCoefficients(fc);
				parametersEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			}

			/**
			* @brief Default deconstructor
			*/
			~LowPass() {};

			/**
			* @brief Atomically sets the target cut-of frequency parameter of the filter
			*
			* @param parameter The target cut-off frequency of the filter
			*/
			inline void SetTargetFc(const Real fc) { SetTargetParameter(fc); }

		private:
			/**
			* @brief Updates the parameters of the high pass filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateCoefficients(const Real fc) override;
		};

		/**
		* @brief Class that implements a low or high pass IIR filter (used by LinkwitzRiley filter)
		*/
		class HighPass : public IIRFilter2Param1
		{
		public:
			/**
			* @brief Constructor that intialises a default pass filter with a given sample rate
			*
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			HighPass(const int sampleRate) : HighPass(1000.0, sampleRate) {};

			/**
			* @brief Constructor that intialises a default pass filter with a given cut off frequency and sample rate
			*
			* @param fc The cut off frequency of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			HighPass(const Real fc, const int sampleRate) : IIRFilter2Param1(fc, sampleRate)
			{
				UpdateCoefficients(fc);
				parametersEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			}

			/**
			* @brief Default deconstructor
			*/
			~HighPass() {};

			/**
			* @brief Atomically sets the target cut-of frequency parameter of the filter
			*
			* @param fc The target cut-off frequency of the filter
			*/
			inline void SetTargetFc(const Real fc) { SetTargetParameter(fc); }

		private:
			/**
			* @brief Updates the parameters of the high pass filter
			*
			* @param fc The cut off frequency of the filter
			*/
			void UpdateCoefficients(const Real fc) override;
		};

		/**
		* @brief Class that implements a 1st order high shelf IIR filter
		* 
		* @remarks Based on Matched One-Pole Digital Shelving Filters Vicanek M 2019.
		* Improved frequency match at large fc values compared to HighShelf class
		*/
		class HighShelfMatched : public IIRFilter1
		{
		public:

			/**
			* @brief Constructor that initialises an 1st order high shelf filter with a given cut off frequency and shelf gain
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			HighShelfMatched(const Real fc, const Real gain, const int sampleRate) : IIRFilter1(sampleRate),
				targetFc(fc), targetGain(gain), currentFc(fc), currentGain(gain)
			{
				assert(fc < static_cast<Real>(sampleRate) / 2.0); // Ensure cut off frequency is less than Nyquist frequency

				UpdateCoefficients(currentFc, currentGain);
				parametersEqual.store(true, std::memory_order_release);
				initialised.store(true, std::memory_order_release);
			};

			/**
			* @brief Default deconstructor
			*/
			~HighShelfMatched() {};

			/**
			* @brief Atomically sets the target parameters of the high shelf filter
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			*/
			inline void SetTargetParameters(const Real fc, const Real gain)
			{
				targetFc.store(fc, std::memory_order_release);
				targetGain.store(gain, std::memory_order_release);
				parametersEqual.store(false, std::memory_order_release);
			}

		private:
			/**
			* @brief Linearly interpolates the current fc and gain with the target fc and gain
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			void InterpolateParameters(const Real lerpFactor) override;

			/**
			* @brief Updates the parameters of the high shelf filter
			*
			* @param fc The cut off frequency of the filter
			* @param gain The shelf gain of the filter (linear)
			*/
			void UpdateCoefficients(const Real fc, const Real gain);

			std::atomic<Real> targetFc;		// Target cut off frequency
			std::atomic<Real> targetGain;	// Target shelf gain
			Real currentFc;					// Current cut off frequency
			Real currentGain;				// Current shelf gain
		};
	}
}

#endif // DSP_IIRFilter_h