/*
* @class FDN, Channel
*
* @brief Declaration of FDN and Channel classes
*
*/

#ifndef RoomAcoustiCpp_FDN_h
#define RoomAcoustiCpp_FDN_h

// C++ headers
#include <vector>
#include <mutex>
#include <cassert>

// Spatialiser headers
#include "Spatialiser/Types.h"

// Common headers
#include "Common/Types.h"
#include "Common/Matrix.h"
#include "Common/Vec.h"
#include "Common/Coefficients.h"

// DSP headers
#include "DSP/Buffer.h"
#include "DSP/GraphicEQ.h"

namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{
		/**
		* @brief Implements an FDN Channel with a delay and absorption
		*/
		class Channel
		{
		public:
			/**
			* @brief Constructor that initialises an FDN Channel with a set delay length and T60
			* 
			* @params delayLength delay in samples
			* @params T60 Target decay time
			* @params config Configuration of the spatialiser
			*/
			Channel(const int delayLength, const Coefficients& T60, const Config& config);

			/**
			* @brief Default deconstructor
			*/
			~Channel() {};

			/**
			* @brief Updates the delay line length
			* @details Currently no delay line interpolation implemented
			*
			* @params delayLength The new delay in samples
			*/
			inline void UpdateDelayLine(const int delayLength) { InitDelay(delayLength); }

			/**
			* @brief Initialises the absorption filter with a given target T60
			* @details No interpolation of the absorption filter coefficients
			* 
			* @params T60 The target decay time
			*/
			inline void InitAbsorption(const Coefficients& T60) { mAbsorptionFilter.InitParameters(CalcGain(T60)); }

			/**
			* @brief Updates the absorption filter for a given target T60
			*
			* @params T60 The new target decay time
			*/
			inline void UpdateAbsorption(const Coefficients& T60) { mAbsorptionFilter.SetGain(CalcGain(T60)); }
			
			/**
			* @brief Initialises the delay line
			* 
			* @params delayLength The delay line length in samples
			*/
			inline void InitDelay(const int delayLength)
			{ 
				mT = static_cast<Real>(delayLength) / mConfig.fs;
				mBuffer.ResizeBuffer(delayLength);
			}

			/**
			* @brief Resets the internal buffers to zero
			*/
			inline void Reset()
			{ 
				idx = 0; 
				mBuffer.ResetBuffer();
				mAbsorptionFilter.ClearBuffers();
			}

			/**
			* @brief Processes a single sample output
			* 
			* @brief input The next audio sample input to the delay line
			* @return The next output from the channel
			*/
			Real GetOutput(const Real input);

		private:

			/**
			* @brief Calculates the filter gain coefficients required for a give T60
			*
			* @params T60 The target decay time
			* @return The required filter gain coefficients
			*/
			inline Coefficients CalcGain(const Coefficients& T60) { return (-3.0 * mT / T60).Pow10(); } // 20 * log10(H(f)) = -60 * t / t60(f);

			Config mConfig;									// The spatialiser configuration
			Buffer mBuffer;									// The internal delay line
			GraphicEQ mAbsorptionFilter;					// The absorption filter to match the target decay time

			Real mT;		// The current delay in seconds
			int idx;		// Current delay line read index

		};

		/**
		* @brief Implements a feedback delay network with modifiable T60 and delay line lengths
		*/
		class FDN
		{
		public:
			/**
			* @brief Initialises an FDN with a target T60 and given primary room dimensions
			* @details Initialises with a default householder matrix
			* 
			* @params T60 Target decay time
			* @params dimensions Primary room dimensions that determine delay line lengths
			* @params config The spatialiser configuration
			*/
			FDN(const Coefficients& T60, const Vec& dimensions, const Config& config);

			/**
			* @brief Default deconstructor
			*/
			~FDN() {}

			/**
			* @brief Process a single FDN sample
			* 
			* @params data Multichannel audio input data
			* @params gain Late reverberation gain
			*/
			void ProcessOutput(const std::vector<Real>& data, const Real gain);

			/**
			* @brief Retrieve the last processed output for a given FDN channel
			*
			* @params i The channel number
			* @return The last processed sample
			*/
			inline Real GetOutput(const int i) const { assert(i < y.Cols()); return y[i]; }

			/**
			* @brief Updates the target T60
			* 
			* @params T60 The new decay time
			*/
			void UpdateT60(const Coefficients& T60);

			/**
			* @brief Updates the delay line lengths
			* @details Currently no delay line interpolation implemented
			* 
			* @params dimensions The new primary room dimensions that determine the delay line lengths
			*/
			void UpdateDelayLines(const Vec& dimensions);

			/**
			* @brief Resets all internal FDN buffers to zero
			*/
			inline void Reset()
			{ 
				x.Reset(); y.Reset();  
				for (Channel& channel : mChannels)
					channel.Reset();
			}

			/**
			* @brief Initialises the FDN matrix
			* 
			* @params matrixType The new FDN matrix type
			*/
			inline void InitFDNMatrix(const FDNMatrix& matrixType)
			{
				switch (matrixType)
				{
				case FDNMatrix::householder:
				{ 
					Init = &FDN::InitHouseHolder;
					Process = &FDN::ProcessHouseholder;
					break;
				}
				case FDNMatrix::randomOrthogonal:
				{ 
					Init = &FDN::InitRandomOrthogonal;
					Process = &FDN::ProcessSquare;
					break;
				}
				}
				InitMatrix();
			}

		private:
			/**
			* @brief Calculate a sample delay based on given distances
			* 
			* @params dimensions Primary room dimensions
			* @return Sample delays for each FDN channel
			*/
			std::vector<int> CalculateTimeDelay(const Vec& dimensions);

			/**
			* @brief Runs the currently selected matrix Init function
			*/
			void InitMatrix() { (this->*Init)(); };

			/**
			* @brief Init matrix function pointer
			*/
			void (FDN::* Init)();

			/**
			* @brief Runs the currently selected matrix Process function
			*/
			inline void ProcessMatrix() { (this->*Process)(); };

			/**
			* @brief Process matrix function pointer
			*/
			void (FDN::* Process)();

			/**
			* @brief Initialises the householder factor for effciently processing a householder matrix
			*/
			inline void InitHouseHolder() { houseHolderFactor = 2.0 / mConfig.numFDNChannels; }

			/**
			* @brief Initialises a random orthogonal matrix
			*/
			void InitRandomOrthogonal();

			/**
			* @brief Optimises processing of a householder feedback matrix
			*/
			inline void ProcessHouseholder()
			{
				Real entry = houseHolderFactor * y.Sum();
				for (int i = 0; i < y.Cols(); i++)
					x[i] = entry - y[i];
			}

			/**
			* @brief Processes a square feedback matrix
			*/
			void ProcessSquare();

			Config mConfig;						// Spatialiser configuration
			std::vector<Channel> mChannels;		// Internal delay line channels

			Rowvec x;					// Next input audio buffer
			Rowvec y;					// Previous output audio buffer
			Matrix feedbackMatrix;		// Feedback matrix
			Real houseHolderFactor;		// Precomputed factor for processing a householder matrix
		};
	}
}

#endif