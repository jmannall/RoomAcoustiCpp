/*
* @brief AirAbsorption class
*
*/

#ifndef Spatialiser_AirAbsorption_h
#define Spatialiser_AirAbsorption_h

// Common headers
#include "Common/Types.h";

// Spatialsier headers
#include "Spatialiser/Types.h"

// DSP headers
#include "DSP/IIRFilter.h"
#include "DSP/Interpolate.h"

namespace UIE
{
	using namespace DSP;
	namespace Spatialiser
	{
		class AirAbsorption
		{
		public:
			AirAbsorption(const int& sampleRate) : x(0), y(0), currentD(0.0), targetD(0.0), a(0.0), b(0.0)
			{
				constant = static_cast<Real>(sampleRate) / (SPEED_OF_SOUND * 7782.0);
				UpdateParameters();
			}
			AirAbsorption(const Real& distance, const int& sampleRate) : x(0), y(0), currentD(distance), targetD(distance), a(0.0), b(0.0)
			{
				constant = static_cast<Real>(sampleRate) / (SPEED_OF_SOUND * 7782.0);
				UpdateParameters();
			}
			~AirAbsorption() {}

			inline void SetDistance(const Real& distance) { targetD = distance; }

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

			inline Real GetOutput(const Real& input)
			{
#if(_WINDOWS)
				_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#elif(_ANDROID)

				unsigned m_savedCSR = getStatusWord();
				// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
				setStatusWord(m_savedCSR | (1 << 24));
#endif
				Real v = input;
				Real output = 0.0;

				v += y * a;
				y = v;
				output += v * b;
#if(_WINDOWS)
				_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
#elif(_ANDROID)

				m_savedCSR = getStatusWord();
				// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
				setStatusWord(m_savedCSR | (0 << 24));
#endif
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
