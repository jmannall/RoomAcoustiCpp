/*
* @class AirAbsorption
* 
* @brief Declaration of AirAbsorption class
*
*/

#ifndef RoomAcoustiCpp_AirAbsorption_h
#define RoomAcoustiCpp_AirAbsorption_h

// Common headers
#include "Common/Types.h"

// Spatialsier headers
#include "Spatialiser/Types.h"

// DSP headers
#include "DSP/IIRFilter.h"
#include "DSP/Interpolate.h"

namespace RAC
{
	using namespace DSP;
	namespace Spatialiser
	{
		class AirAbsorption
		{
		public:
			AirAbsorption(const int sampleRate) : x(0), y(0), currentD(0.0), targetD(0.0), a(0.0), b(0.0)
			{
				constant = static_cast<Real>(sampleRate) / (SPEED_OF_SOUND * 7782.0);
				UpdateParameters();
			}
			AirAbsorption(const Real distance, const int sampleRate) : x(0), y(0), currentD(distance), targetD(distance), a(0.0), b(0.0)
			{
				constant = static_cast<Real>(sampleRate) / (SPEED_OF_SOUND * 7782.0);
				UpdateParameters();
			}
			~AirAbsorption() {}

			inline void SetDistance(const Real distance) { targetD = distance; }

			inline void UpdateParameters()
			{
				b = exp(-currentD * constant);
				a = 1 - b;
			}

			inline Real GetDistance() const { return currentD; }

			inline void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor)
			{
				if (currentD == targetD)
				{
					for (int i = 0; i < numFrames; i++)
						outBuffer[i] = GetOutput(inBuffer[i]);
				}
				else
				{
					for (int i = 0; i < numFrames; i++)
					{
						outBuffer[i] = GetOutput(inBuffer[i]);
						Lerp(currentD, targetD, lerpFactor);
						UpdateParameters();
					}
				}
			}

			inline Real GetOutput(const Real input)
			{
				FlushDenormals();
				Real v = input;
				Real output = 0.0;

				v += y * a;
				y = v;
				output += v * b;

				NoFlushDenormals();
				return output;
			}
			
		private:
			Real constant;

			Real x;
			Real y;
			Real b;
			Real a;
			Real currentD;
			Real targetD;;
		};

	}
}

#endif
