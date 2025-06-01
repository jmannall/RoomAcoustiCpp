/*
* @class AirAbsorption
* 
* @brief Declaration of AirAbsorption class
*
* @remark Based after Implementation and perceptual evaluation of a simulation method for coupled rooms in higher order ambisonics. Grimm G et al. 2014
* @note Error in paper eq (1). should be y_k = a_1 * x_k - (1 - a_1) * y_k
*
*/

#ifndef RoomAcoustiCpp_AirAbsorption_h
#define RoomAcoustiCpp_AirAbsorption_h

// Common headers
#include "Common/Types.h"

// DSP headers
#include "DSP/Buffer.h"
#include "DSP/IIRFilter.h"

namespace RAC
{
	using namespace DSP;
	namespace Spatialiser
	{
		/**
		* @brief Class that implements an air absorption filter
		*/
		class AirAbsorption : public IIRFilter1
		{
		public:

			/**
			* @brief Constructor that initialises the AirAbsorption with a given distance and sample rate
			*
			* @param distance The distance for calculating the filter coefficients
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			AirAbsorption(const Real distance, const int sampleRate) : IIRFilter1(sampleRate), currentDistance(distance), targetDistance(distance),
				constant(static_cast<Real>(sampleRate) / (SPEED_OF_SOUND * 7782.0))
			{
				a0 = 1.0; b1 = 0.0; // Not used by this filter
				UpdateCoefficients(distance);

				parametersEqual.store(true);
				initialised.store(true);
			}

			/**
			* @brief Default deconstructor
			*/
			~AirAbsorption() {}

			/**
			* @brief Updates the target distance
			*
			* @param distance The new target distance
			*/
			inline void SetTargetDistance(const Real distance)
			{ 
				assert(distance > 0.0);

				targetDistance.store(distance);
				parametersEqual.store(false);
			}

			/**
			* @brief Processes an input buffer and updates the output buffer
			*
			* @param inBuffer The input buffer
			* @param outBuffer The output buffer
			* @param numFrames The number of frames in the buffer
			* @param lerpFactor The interpolation factor (0.0 to 1.0)
			*/
			void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor);

		private:
			/**
			* @brief Updates the filter coefficients
			*/
			inline void UpdateCoefficients(Real distance)
			{
				b0 = exp(-distance * constant);
				a1 = b0 - 1;
			}

			/**
			 * @brief Interpolates between the current distance and target distance using linear interpolation
			 * 
			 * @param lerpFactor The interpolation factor (0.0 to 1.0)
			 */
			void InterpolateParameters(const Real lerpFactor) override;

			const Real constant;		// Constant used for calculating filter coefficients

			std::atomic<Real> targetDistance;		// Target distance
			Real currentDistance;					// Current distance (should only be accessed from the audio thread)
		};
	}
}

#endif
