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

// Spatialsier headers
#include "Spatialiser/Types.h"

// DSP headers
#include "DSP/Buffer.h"

namespace RAC
{
	using namespace DSP;
	namespace Spatialiser
	{
		/**
		* Class that implements an air absorption filter
		*/
		class AirAbsorption
		{
		public:
			/**
			* Constructor that initialises the AirAbsorption with a given sample rate
			*
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			AirAbsorption(const int sampleRate) : x(0), y(0), currentD(0.0), targetD(0.0), a(0.0), b(0.0)
			{
				constant = static_cast<Real>(sampleRate) / (SPEED_OF_SOUND * 7782.0);
				UpdateParameters();
			}

			/**
			* Constructor that initialises the AirAbsorption with a given distance and sample rate
			*
			* @param distance The distance for calculating the filter coefficients
			* @param sampleRate The sample rate for calculating the filter coefficients
			*/
			AirAbsorption(const Real distance, const int sampleRate) : x(0), y(0), currentD(distance), targetD(distance), a(0.0), b(0.0)
			{
				constant = static_cast<Real>(sampleRate) / (SPEED_OF_SOUND * 7782.0);
				UpdateParameters();
			}

			/**
			* Default deconstructor
			*/
			~AirAbsorption() {}

			/**
			* Updates the target distance
			*
			* @param distance The new target distance
			*/
			inline void SetDistance(const Real distance) { targetD = distance; }

			/**
			* Updates the filter coefficients
			*/
			inline void UpdateParameters()
			{
				b = exp(-currentD * constant);
				a = 1 - b;
			}

			/**
			* Updates the filter coefficients
			* 
			* @param distance The new distance
			*/
			inline void UpdateParameters(const Real distance)
			{
				targetD = distance;
				currentD = distance;
				UpdateParameters();
			}

			/**
			* Gets the current distance
			*
			* @return The current distance
			*/
			inline Real GetDistance() const { return currentD; }

			/**
			* Returns the output of the AirAbsorption filter given an input
			*
			* @param input The input to the AirAbsorption filter
			* @return The output of the AirAbsorption filter
			*/
			Real GetOutput(const Real input);

			/**
			* Processes an input buffer and updates the output buffer
			*
			* @param inBuffer The input buffer
			* @param outBuffer The output buffer
			* @param numFrames The number of frames in the buffer
			* @param lerpFactor The linear interpolation factor
			*/
			void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor);
						
			/**
			* Resets the filter buffers
			*/
			inline void ClearBuffers() { x = 0.0; y = 0.0; }

		private:
			/**
			* Air absorption constant
			*/
			Real constant;

			/**
			* Previous input and output values
			*/
			Real x;
			Real y;

			/**
			* Filter coefficients
			*/
			Real b;
			Real a;

			/**
			* Current and target distances
			*/
			Real currentD;
			Real targetD;;
		};
	}
}

#endif
