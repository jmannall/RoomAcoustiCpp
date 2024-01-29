/*
*
*  \FDN class
*
*/

#ifndef Spatialiser_FDN_h
#define Spatialiser_FDN_h

// C++ headers
#include <vector>
#include <mutex>

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Wall.h"

// Common headers
#include "Common/AudioManager.h"
#include "Common/Matrix.h"
#include "Common/Vec.h"

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// FDN Channel class ////////////////////

		class Channel
		{
		public:
			// Load and Destroy
			Channel(int fs);
			Channel(Real t, const FrequencyDependence& T60, int fs);
			~Channel() {};

			// Setters
			void SetParameters(const FrequencyDependence& T60, const Real& t);
			void SetAbsorption(const FrequencyDependence& T60);
			inline void SetDelay(const Real& t) { mT = t; SetDelay(); }
			inline void Reset()
			{ 
				idx = 0; 
				mBuffer.ResetBuffer();
				mAbsorptionFilter.ClearBuffers();
				mAirAbsorption.ClearBuffers();
			}

			// Getters
			Real GetOutput(const Real input);

		private:
			// Setters
			void SetAbsorption(Real g[]);
			void SetDelay();

			// Member variables
			Real mT;
			int sampleRate;
			Buffer mBuffer;
			ParametricEQ mAbsorptionFilter;
			LowPass mAirAbsorption;
			
			std::mutex* mBufferMutex;
			int idx;	// Read index
		};

		//////////////////// FDN class ////////////////////

		class FDN
		{
		public:
			// Load and Destroy
			FDN(size_t numChannels, int fs);
			FDN(const FrequencyDependence& T60, const vec& dimensions, size_t numChannels, int fs);
			~FDN() {}

			// Getters
			rowvec GetOutput(const Real* data, bool valid);

			// Setters
			void SetParameters(const FrequencyDependence& T60, const vec& dimensions);
			inline void Reset()
			{ 
				x.Reset(); y.Reset();  
				for (int i = 0; i < mNumChannels; i++)
					mChannels[i].Reset();
			}

		private:
			// Init
			void InitMatrix();
			void CalculateTimeDelay(const vec& dimensions, vec& t);

			// Process
			void ProcessMatrix() { Mult(y, mat, x); }//x = y * mat; }

			// Member variables
			size_t mNumChannels;
			std::vector<Channel> mChannels;
			rowvec x;
			rowvec y;
			matrix mat;
		};

#if(_ANDROID)
		inline int getStatusWord()
		{
			int result;
			asm volatile("mrs %[result], FPCR" : [result] "=r" (result));
			return result;
		}

		inline void setStatusWord(int a)
		{
			asm volatile("msr FPCR, %[src]" : : [src] "r" (a));
		}
#endif
	}
}

#endif