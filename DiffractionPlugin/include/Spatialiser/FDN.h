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
#include "Spatialiser/Main.h"

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

		inline void HouseholderMult(const matrix& u, const matrix& v, matrix& out)
		{
			out.Reset();

			Real entry = 0.0;
			for (int i = 0; i < u.Cols(); i++)
				entry += u.GetEntry(0, i) * v.GetEntry(i, 0);

			entry *= 2.0;
			for (int i = 0; i < u.Cols(); i++)
				out.AddEntry(u.GetEntry(0, i) - v.GetEntry(i, 0) * entry, 0, i);
		}

		inline void Mult(const matrix& u, const matrix& v, matrix& out)
		{
			out.Reset();
			for (int i = 0; i < u.Rows(); ++i)
			{
				for (int j = 0; j < v.Cols(); ++j)
				{
					for (int k = 0; k < u.Cols(); ++k)
					{
						out.IncreaseEntry(u.GetEntry(i, k) * v.GetEntry(k, j), i, j);
					}
				}
			}
		}

		class FDN
		{
		public:
			// Load and Destroy
			FDN(const Config& config);
			FDN(const FrequencyDependence& T60, const vec& dimensions, const Config& config);
			~FDN() {}

			// Getters
			rowvec GetOutput(const std::vector<Real>& data, Real gain, bool valid);

			// Setters
			void SetParameters(const FrequencyDependence& T60, const vec& dimensions);
			inline void Reset()
			{ 
				x.Reset(); y.Reset();  
				for (int i = 0; i < mConfig.numFDNChannels; i++)
					mChannels[i].Reset();
			}

		private:
			// Init
			void InitMatrix();
			void CalculateTimeDelay(const vec& dimensions, vec& t);

			// Process
			inline void ProcessMatrix() //x = y * mat
			{ 
				HouseholderMult(y, houseMat, x);
				// Mult(y, mat, x);
			}

			// Member variables
			Config mConfig;
			std::vector<Channel> mChannels;
			rowvec x;
			rowvec y;
			matrix mat;
			matrix houseMat;
		};
	}
}

#endif