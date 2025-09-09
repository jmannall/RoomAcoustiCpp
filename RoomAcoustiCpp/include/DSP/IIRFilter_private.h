

#ifndef RoomAcoustiCpp_IIRFilter_private_h
#define RoomAcoustiCpp_IIRFilter_private_h

#include "Common/Types.h"

namespace RAC
{
	using namespace Common;
	namespace DSP
	{
		/**
		* @brief Class that implements a second order Infinite Impulse Response filter
		*
		* @details Uses the Direct-Form-II implementation
		*/
		template<typename In = Real>
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
			In GetOutput(const In input, const Real lerpFactor);

			/**
			* @brief Set internal buffers to zeros
			*/
			inline void ClearBuffers() { y0 = 0.0; y1 = 0.0; }

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
			In y0{ 0.0 }, y1{ 0.0 };					// Outputs (should only be accessed from the audio thread)

			std::atomic<bool> parametersEqual{ false };		// True if the current parameters are known to be equal to the target parameters
			std::atomic<bool> initialised{ false };			// True if the filter has been initialised, false otherwise

		private:
			/**
			* @brief Pure virtual function to lineraly interpolates filter parameters. Must be overloaded in the derived classes
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			virtual void InterpolateParameters(const Real lerpFactor) = 0;
		};

		/**
		* @brief Class that implements a 2nd-order IIR filter with one parameter
		*/
		template<typename In = Real>
		class IIRFilter2Param1 : public IIRFilter2<In>
		{
		public:
			/**
			* @brief Constructor that intialises a default pass filter with a given cut off frequency and sample rate
			*
			* @param fc The cut off frequency of the filter
			* @param sampleRate The sample rate for calculating filter coefficients
			*/
			IIRFilter2Param1(const Real parameter, const int sampleRate) : IIRFilter2<In>(sampleRate),
				current(parameter), target(parameter) {
			};

			virtual ~IIRFilter2Param1() {};

		protected:
			/**
			* @brief Atomically sets the target parameter of the filter
			*
			* @param parameter The target parameter of the filter
			*/
			inline void SetTargetParameter(const Real parameter) { target.store(parameter, std::memory_order_release); this->parametersEqual.store(false, std::memory_order_release); }

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
		template<typename In = Real>
		class PeakHighShelf : public IIRFilter2Param1<In>
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
			PeakHighShelf(const Real fc, const Real gain, const Real Q, const int sampleRate) : IIRFilter2Param1<In>(gain, sampleRate),
				cosOmega(cos(PI_2 * fc * this->T)), alpha(sin(PI_2 * fc * this->T) / Q) // sin(omega) / (2 * Q) (factor of two cancelled out in UpdateGain)
			{
				assert(fc < static_cast<Real>(sampleRate) / 2.0); // Ensure cut off frequency is less than Nyquist frequency

				UpdateCoefficients(gain);
				this->parametersEqual.store(true, std::memory_order_release);
				this->initialised.store(true, std::memory_order_release);
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
			inline void SetTargetGain(const Real gain) { this->SetTargetParameter(gain); }

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
		template<typename In = Real>
		class PeakLowShelf : public IIRFilter2Param1<In>
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
			PeakLowShelf(const Real fc, const Real gain, const Real Q, const int sampleRate) : IIRFilter2Param1<In>(gain, sampleRate),
				cosOmega(cos(PI_2 * fc * this->T)), alpha(sin(PI_2 * fc * this->T) / Q) // sin(omega) / (2 * Q) (factor of two cancelled out in UpdateGain)
			{
				assert(fc < static_cast<Real>(sampleRate) / 2.0); // Ensure cut off frequency is less than Nyquist frequency

				UpdateCoefficients(gain);
				this->parametersEqual.store(true, std::memory_order_release);
				this->initialised.store(true, std::memory_order_release);
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
			inline void SetTargetGain(const Real gain) { this->SetTargetParameter(gain); }

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
		template<typename In = Real>
		class PeakingFilter : public IIRFilter2Param1<In>
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
			PeakingFilter(const Real fc, const Real gain, const Real Q, const int sampleRate) : IIRFilter2Param1<In>(gain, sampleRate),
				cosOmega(-2.0 * cos(PI_2 * fc * this->T)), alpha(sin(PI_2 * fc * this->T) / (2.0 * Q))
			{
				assert(fc < static_cast<Real>(sampleRate) / 2.0); // Ensure cut off frequency is less than Nyquist frequency

				UpdateCoefficients(gain);
				this->parametersEqual.store(true, std::memory_order_release);
				this->initialised.store(true, std::memory_order_release);
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
			inline void SetTargetGain(const Real gain) { this->SetTargetParameter(gain); }

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
	}
}

#endif // RoomAcoustiCpp_IIRFilter_private_h