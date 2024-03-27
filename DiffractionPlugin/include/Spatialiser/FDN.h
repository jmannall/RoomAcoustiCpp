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

// Unity headers
#include "Unity/Profiler.h"
#include "Unity/Debug.h"

// Common headers
#include "Common/AudioManager.h"
#include "Common/Matrix.h"
#include "Common/Vec.h"
#include "Common/Coefficients.h"

// DSP headers
#include "DSP/Buffer.h"
#include "DSP/GraphicEQ.h"

namespace UIE
{
	using namespace Common;
	using namespace DSP;
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// FDN Channel class ////////////////////

		class Channel
		{
		public:
			// Load and Destroy
			Channel(const Config& config);
			Channel(Real t, const Coefficients& T60, const Config& config);
			~Channel() { /*delete mBufferMutex;*/ };

			// Setters
			// void UpdateT60(const Coefficients& T60);
			void SetParameters(const Coefficients& T60, const Real& t);
			void SetAbsorption();
			void SetAbsorption(const Coefficients& T60);
			inline void SetDelay(const Real& t) { mT = t; SetDelay(); }
			inline void Reset()
			{ 
				idx = 0; 
				mBuffer.ResetBuffer();
				mAbsorptionFilter.ClearBuffers();
			}

			// Getters
			Real GetOutput(const Real input);

		private:
			// Setters
			void SetDelay();

			// Member variables
			Real mT;
			Config mConfig;
			Buffer mBuffer;
			GraphicEQ mAbsorptionFilter;
			
			//std::mutex* mBufferMutex;
			std::shared_ptr<std::mutex> mBufferMutex;
			int idx;	// Read index
		};

		//////////////////// FDN class ////////////////////

		class FDN
		{
		public:
			// Load and Destroy
			FDN(const Config& config);
			FDN(const Coefficients& T60, const vec& dimensions, const Config& config);
			~FDN() {}

			// Getters
			void ProcessOutput(const std::vector<Real>& data, const Real& gain);
			inline Real GetOutput(const int& i) const { return y.GetEntry(i); }

			// Setters
			void UpdateT60(const Coefficients& T60);
			void SetParameters(const Coefficients& T60, const vec& dimensions);
			inline void Reset()
			{ 
				x.Reset(); y.Reset();  
				for (int i = 0; i < mConfig.numFDNChannels; i++)
					mChannels[i].Reset();
			}
			inline void SetFDNModel(const FDNMatrix& model)
			{
				switch (model)
				{
				case FDNMatrix::householder:
				{ 
					mat = matrix(mConfig.numFDNChannels, 1);
					Init = &FDN::InitHouseHolder;
					Process = &FDN::ProcessHouseholder;
					break;
				}
				case FDNMatrix::randomOrthogonal:
				{ 
					mat = matrix(mConfig.numFDNChannels, mConfig.numFDNChannels);
					Init = &FDN::InitRandomOrthogonal;
					Process = &FDN::ProcessSquare;
					break;
				}
				}
				InitMatrix();
			}

		private:
			// Init
			void InitMatrix() { (this->*Init)(); };
			void (FDN::* Init)();
			vec CalculateTimeDelay(const vec& dimensions);

			// Process
			inline void ProcessMatrix() { (this->*Process)(); };
			void (FDN::* Process)();

			inline void InitHouseHolder() { houseHolderFactor = 2.0 / mConfig.numFDNChannels; }
			void InitRandomOrthogonal();

			inline void ProcessHouseholder()
			{
				// x.Reset();
				/*for (int i = 0; i < y.Cols(); i++)
					x.AddEntry(houseHolderFactor - y.GetEntry(i), i);*/
				x.Reset();
				Real entry = houseHolderFactor * y.Sum();
				for (int i = 0; i < y.Cols(); i++)
					x.AddEntry(entry - y.GetEntry(i), i);

				/*Real entry = 0.0;
				for (int i = 0; i < y.Cols(); i++)
					entry += y.GetEntry(i) * mat.GetEntry(i, 0);

				entry *= 2.0;
				for (int i = 0; i < y.Cols(); i++)
					x.AddEntry(y.GetEntry(i) - mat.GetEntry(i, 0) * entry, i);*/
			}

			inline void ProcessSquare()
			{
				x.Reset();
				for (int j = 0; j < mat.Cols(); ++j)
				{
					for (int k = 0; k < mat.Rows(); ++k)
					{
						x.IncreaseEntry(y.GetEntry(k) * mat.GetEntry(k, j), j);
					}
				}
			}

			// Member variables
			Config mConfig;
			std::vector<Channel> mChannels;
			rowvec x;
			rowvec y;
			matrix mat;

			FDNMatrix mModel;
			Real houseHolderFactor;
		};
	}
}

#endif