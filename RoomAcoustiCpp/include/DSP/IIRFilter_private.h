#ifndef RoomAcoustiCpp_IIRFilter_private_h
#define RoomAcoustiCpp_IIRFilter_private_h

#include <cassert>

#include "Common/Types.h"
#include "include/Common/Definitions.h"

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
			IIRFilter2(const int sampleRate) : T(REAL_CONST(1.0) / static_cast<Real>(sampleRate)) {};

			/**
			* @brief Default deconstructor
			*/
			virtual ~IIRFilter2() {};

			/**
			* @brief Returns the output of the IIRFilter given an input
			*
			* @param input The input to the IIRFilter
			* @param lerpFactor The linear interpolation factor for parameters
			* @return The output of the IIRFilter
			*/
			In GetOutput(const In input, const Real lerpFactor);

			/**
			* @brief Returns the output of the IIRFilter given an input
			*
			* @param input The input to the IIRFilter
			* @param output The output to the filter
			* @param lerpFactor The linear interpolation factor for parameters
			*/
			void GetOutput(const In& input, In& output, const Real lerpFactor);

			/**
			* @brief Processes a batch of inputs
			*
			* @param input The input to the IIRFilter
			* @param output The output to the filter
			* @param inputOutputLength The length of the data arrays
			* @param lerpFactor The linear interpolation factor for parameters
			*/
			void GetOutputBatch(const In* input, In* output, int inputOutputLength, const Real lerpFactor);

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

			/**
			 * @brief Returns if this filter is valid.
			 *
			 * @return true if the valid is valid and GetOutput() can be called.
			 */
			bool IsValid() const { return initialised.load(std::memory_order_acquire); }

			static void GetOutputFromMultipleFilters(IIRFilter2** filters, int numFilters, const In& input, In& output, const Real lerpFactor);

		protected:
			// to ensure alignment for AVX, these are in a slightly different order

			// a -> Denominator coefficients (should only be accessed from the audio thread)
			// b -> Numerator coefficients (should only be accessed from the audio thread)
			// y -> Outputs (should only be accessed from the audio thread)

#if USE_AVX
			alignas(16)
#endif
			Real a1{ 0.0 };
			Real a2{ 0.0 };
			Real b1{ 0.0 };
			Real b2{ 0.0 };
			In   y0{ 0.0 };
			In   y1{ 0.0 };

			Real a0{ 0.0 };
			Real b0{ 0.0 };

			const Real T;				// Sample rate time period

			std::atomic<bool> parametersEqual{ false };		// True if the current parameters are known to be equal to the target parameters
			std::atomic<bool> initialised{ false };			// True if the filter has been initialised, false otherwise

		private:
			/**
			* @brief Pure virtual function to lineraly interpolates filter parameters. Must be overloaded in the derived classes
			*
			* @param lerpFactor The lerp factor for interpolation
			*/
			virtual void InterpolateParameters(const Real lerpFactor) = 0;

			void GetOutputInternal(const In& input, In& output);
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
			inline void SetTargetParameter(const Real parameter)
			{
				target.store(parameter, std::memory_order_release);
				this->parametersEqual.store(false, std::memory_order_release);
			}

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
				cosOmega(REAL_CONST(-2.0)* cos(PI_2* fc* this->T)), alpha(sin(PI_2* fc* this->T) / (REAL_CONST(2.0) * Q))
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

		////////////////////////////////////////

		// Declare these here to allow potential inlining

#if USE_AVX

#if DATA_TYPE_DOUBLE

		template <>
		RAC_FORCE_INLINE void IIRFilter2<double>::GetOutputInternal(const double& input, double& output)
		{
			// make sure they are aligned
			assert(IsAligned16(&this->a1));
			assert(IsAligned16(&this->b1));
			assert(IsAligned16(&this->y0));

			// In v = input - y0 * a1 + y1 * a2 --> input - y[0:1] . a[0:1] -> sub(input, y.a)
			__m128d y = _mm_load_pd(&this->y0);
			__m128d aDotY = _mm_dp_pd(_mm_load_pd(&this->a1), y, 0x31);
			// v[low] has the result v[high] is not used
			__m128d v = _mm_sub_sd(_mm_set_sd(input), aDotY);

			// In output	 = y0 * b1 + y1 * b2 + v * b0 --> y[0:1] . b[0:1] + v * b0 --> b0 * v + (y[0:1] . b[0:1]) --> fmadd( b0, v, y.b )
			__m128d bDotY = _mm_dp_pd(_mm_load_pd(&this->b1), y, 0x31);
			// output[low] has the result; output[high] is not used
			_mm_store_sd(&output, _mm_fmadd_pd(v, _mm_set_sd(this->b0), bDotY));

			// shift the filter y1 = y0, y0 = v
			__m128d newY = _mm_move_sd(_mm_movedup_pd(y), v);

			// save it
			_mm_store_pd(&this->y0, newY);
		}

		template <>
		RAC_FORCE_INLINE void IIRFilter2<double>::GetOutputFromMultipleFilters(IIRFilter2<double>** filters, int numFilters, const double& input, double& output, const Real lerpFactor)
		{
			// update parameters first
			for (int filterIndex = 0; filterIndex < numFilters; ++filterIndex)
			{
				if (!filters[filterIndex]->parametersEqual.load(std::memory_order_acquire))
					filters[filterIndex]->InterpolateParameters(lerpFactor);
			}

			// load the input. we will use this for all values
			__m128d working = _mm_set_sd(input);

			for (int filterIndex = 0; filterIndex < numFilters; ++filterIndex)
			{
				IIRFilter2<double>* currentFilter = filters[filterIndex];

				// In v = input - y0 * a1 + y1 * a2 --> input - y[0:1] . a[0:1] -> sub(input, y.a)
				__m128d y = _mm_load_pd(&currentFilter->y0);
				__m128d aDotY = _mm_dp_pd(_mm_load_pd(&currentFilter->a1), y, 0x31);
				// v[low] has the result v[high] is not used
				__m128d v = _mm_sub_sd(working, aDotY);

				// In output	 = y0 * b1 + y1 * b2 + v * b0 --> y[0:1] . b[0:1] + v * b0 --> b0 * v + (y[0:1] . b[0:1]) --> fmadd( b0, v, y.b )
				__m128d bDotY = _mm_dp_pd(_mm_load_pd(&currentFilter->b1), y, 0x31);
				// output[low] has the result; output[high] is not used
				working = _mm_fmadd_pd(v, _mm_set_sd(currentFilter->b0), bDotY);

				// shift the filter y1 = y0, y0 = v
				__m128d newY = _mm_move_sd(_mm_movedup_pd(y), v);

				// save the new filter arguments
				_mm_store_pd(&currentFilter->y0, newY);

				// the working result doesn't need to be written back yet
			}

			// after all filters are processed, save the final result
			_mm_store_sd(&output, working);
		}

#else

		template <>
		RAC_FORCE_INLINE void IIRFilter2<float>::GetOutputInternal(const float& input, float& output)
		{
#if CHECK_ALIGNMENT
			// make sure they are aligned
			assert((reinterpret_cast<ptrdiff_t>(&this->a1) & 0xf) == 0);
			assert((reinterpret_cast<ptrdiff_t>(&this->y1) & 0xf) == 0);
#endif

			__m128 a12b12 = _mm_load_ps(&this->a1);
			__m128 y = _mm_load_ps(&this->y0);			// we only use y01
			y = _mm_movelh_ps(y, y);					// duplicate y01 into the high and low

			// In v = input - y0 * a1 + y1 * a2 --> input - y[0:1] . a[0:1] -> sub(input, y.a)
			__m128 aDotY = _mm_dp_ps(a12b12, y, 0x31);
			// v[low] has the result v[low+1] is not used
			__m128 v = _mm_sub_ss(_mm_set_ss(input), aDotY);

			// In output	 = y0 * b1 + y1 * b2 + v * b0 --> y[0:1] . b[0:1] + v * b0 --> b0 * v + (y[0:1] . b[0:1]) --> fmadd( b0, v, y.b )
			__m128 bDotY = _mm_dp_ps(a12b12, y, 0xc1);
			// output[low] has the result; output[high] is not used
			_mm_store_ss(&output, _mm_fmadd_ps(v, _mm_set_ss(this->b0), bDotY));

			// shift the filter y1 = y0, y0 = v
			__m128 shifted = _mm_move_ss(_mm_shuffle_ps(y, y, _MM_SHUFFLE(3, 2, 0, 1)), v);

			// store it
			_mm_storel_pi(reinterpret_cast<__m64*>(&this->y0), shifted);
		}

		template <>
		RAC_FORCE_INLINE void IIRFilter2<float>::GetOutputFromMultipleFilters(IIRFilter2<float>** filters, int numFilters, const float& input, float& output, const Real lerpFactor)
		{
			// update parameters first
			for (int filterIndex = 0; filterIndex < numFilters; ++filterIndex)
			{
				if (!filters[filterIndex]->parametersEqual.load(std::memory_order_acquire))
					filters[filterIndex]->InterpolateParameters(lerpFactor);
			}

			// load the input. we will use this for all values
			__m128 working = _mm_set_ss(input);

			for (int filterIndex = 0; filterIndex < numFilters; ++filterIndex)
			{
				IIRFilter2<float>* currentFilter = filters[filterIndex];

				__m128 a12b12 = _mm_load_ps(&currentFilter->a1);

				// In v = input - y0 * a1 + y1 * a2 --> input - y[0:1] . a[0:1] -> sub(input, y.a)
				__m128 y = _mm_load_ps(&currentFilter->y0);			// we only use y01
				y = _mm_movelh_ps(y, y);					// duplicate y01 into the high and low

				// In v = input - y0 * a1 + y1 * a2 --> input - y[0:1] . a[0:1] -> sub(input, y.a)
				__m128 aDotY = _mm_dp_ps(a12b12, y, 0x31);
				// v[low] has the result v[low+1] is not used
				__m128 v = _mm_sub_ss(working, aDotY);

				// In output	 = y0 * b1 + y1 * b2 + v * b0 --> y[0:1] . b[0:1] + v * b0 --> b0 * v + (y[0:1] . b[0:1]) --> fmadd( b0, v, y.b )
				__m128 bDotY = _mm_dp_ps(a12b12, y, 0xc1);
				// output[low] has the result; output[high] is not used
				working = _mm_fmadd_ps(v, _mm_set_ss(currentFilter->b0), bDotY);

				// shift the filter y1 = y0, y0 = v
				__m128 shifted = _mm_move_ss(_mm_shuffle_ps(y, y, _MM_SHUFFLE(3, 2, 0, 1)), v);

				// store it
				_mm_storel_pi(reinterpret_cast<__m64*>(&currentFilter->y0), shifted);

				// the working result doesn't need to be written back yet
			}

			// after all filters are processed, save the final result
			_mm_store_ss(&output, working);
		}

#endif

#endif

		template <typename In>
		RAC_FORCE_INLINE void IIRFilter2<In>::GetOutputInternal(const In& input, In& output)
		{
			In v = input;
			output = 0.0;

			v -= y0 * a1;
			output += y0 * b1;

			v -= y1 * a2;
			output += y1 * b2;

			y1 = y0;
			y0 = v;

			output += v * b0;
		}

		template<typename In>
		RAC_FORCE_INLINE void IIRFilter2<In>::GetOutput(const In& input, In& output, const Real lerpFactor)
		{
			assert(IsValid());

			if (!parametersEqual.load(std::memory_order_acquire))
				InterpolateParameters(lerpFactor);

			GetOutputInternal(input, output);
		}

		template<typename In>
		RAC_FORCE_INLINE In IIRFilter2<In>::GetOutput(const In input, const Real lerpFactor)
		{
			assert(IsValid());

			if (!parametersEqual.load(std::memory_order_acquire))
				InterpolateParameters(lerpFactor);

			In output;
			GetOutputInternal(input, output);
			return output;
		}

		template <typename In>
		RAC_FORCE_INLINE void IIRFilter2<In>::GetOutputFromMultipleFilters(IIRFilter2** filters, int numFilters, const In& input, In& output, const Real lerpFactor)
		{
			// update parameters first
			for (int filterIndex = 0; filterIndex < numFilters; ++filterIndex)
			{
				if (!filters[filterIndex]->parametersEqual.load(std::memory_order_acquire))
					filters[filterIndex]->InterpolateParameters(lerpFactor);
			}

			// process the data
			for (int filterIndex = 0; filterIndex < numFilters; ++filterIndex)
				filters[filterIndex]->GetOutputInternal(input, output);
		}

		////////////////////////////////////////
	}
}

#endif // RoomAcoustiCpp_IIRFilter_private_h