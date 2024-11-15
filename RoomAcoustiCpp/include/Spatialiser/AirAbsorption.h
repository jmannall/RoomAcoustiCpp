/*
* @class AirAbsorption
* 
* @brief Declaration of AirAbsorption class
*
* @remark Based after Implementation and perceptual evaluation of a simulation method for coupled rooms in higher order ambisonics. Grimm G et al. 2014
* 
*/

#ifndef RoomAcoustiCpp_AirAbsorption_h
#define RoomAcoustiCpp_AirAbsorption_h

// Common headers
#include "Common/Types.h"

// DSP headers
#include "DSP/Buffer.h"

namespace RAC
{
	using namespace DSP;
	namespace Spatialiser
	{
		/**
		* @brief Class that implements an air absorption filter
		*/
		class AirAbsorption
		{
		public:
			/**
			* @brief Constructor that initialises the AirAbsorption with a given sample rate
			*
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			AirAbsorption(const int sampleRate) : AirAbsorption(0.0, sampleRate) {}

			/**
			* @brief Constructor that initialises the AirAbsorption with a given distance and sample rate
			*
			* @param distance The distance for calculating the filter coefficients
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			AirAbsorption(const Real distance, const int sampleRate) : x(0), y(0), currentDistance(distance), targetDistance(distance), a(0.0), b(0.0), equal(true)
			{
				constant = static_cast<Real>(sampleRate) / (SPEED_OF_SOUND * 7782.0);
				UpdateParameters();
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
			inline void SetDistance(const Real distance)
			{ 
				targetDistance = distance;
				if (currentDistance != targetDistance)
					equal = false;
			}

			/**
			* @brief Updates the filter coefficients
			*/
			inline void UpdateParameters()
			{
				b = exp(-currentDistance * constant);
				a = 1 - b;
			}

			/**
			* @brief Gets the current distance
			*
			* @return The current distance
			*/
			inline Real GetDistance() const { return currentDistance; }

			/**
			* @brief Returns the output of the AirAbsorption filter given an input
			*
			* @param input The input to the AirAbsorption filter
			* @return The output of the AirAbsorption filter
			*/
			Real GetOutput(const Real input);

			/**
			* @brief Processes an input buffer and updates the output buffer
			*
			* @param inBuffer The input buffer
			* @param outBuffer The output buffer
			* @param numFrames The number of frames in the buffer
			* @param lerpFactor The linear interpolation factor
			*/
			void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor);
						
			/**
			* @brief Resets the filter buffers
			*/
			inline void ClearBuffers() { x = 0.0; y = 0.0; }

		private:

			Real constant;		// Air absorption constant

			Real x;		// Previous input
			Real y;		// Previous output
			Real b;		// Numerator coefficient
			Real a;		// Denominator coefficient

			Real currentDistance;		// Current distance
			Real targetDistance;		// Target distance
			bool equal;					// True if the current and target distances are equal
		};
	}
}

#endif
