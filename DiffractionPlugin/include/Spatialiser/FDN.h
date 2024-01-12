/*
*
*  \FDN class
*
*/

#ifndef Spatialiser_FDN_h
#define Spatialiser_FDN_h

// C++ headers
#include <vector>

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
			void SetDelay(const Real& t) { mT = t; SetDelay(); }

			// Getters
			Real GetOutput(const Real input);

		private:
			// Setters
			void SetAbsorption(Real g[]);
			void SetDelay();

			// Member variables
			Real mT;
			int sampleRate;
			size_t mDelay;
			Buffer mBuffer;
			ParametricEQ mAbsorptionFilter;
			LowPass mAirAbsorption;
			
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

		private:
			// Init
			void InitMatrix();
			void CalculateTimeDelay(const vec& dimensions, vec& t);

			// Process
			void ProcessMatrix() { x = y * mat; }

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