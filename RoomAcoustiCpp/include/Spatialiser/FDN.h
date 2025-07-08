/*
* @class FDN, FDNChannel
*
* @brief Declaration of FDN and FDNChannel classes
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
		class FDNChannel
		{
		public:
			/**
			* @brief Constructor that initialises an FDN Channel with a set delay length and T60
			* 
			* @param delayLength delay in samples
			* @param T60 Target decay time
			* @param config Configuration of the spatialiser
			*/
			FDNChannel(const int delayLength, const Coefficients<>& T60, const std::shared_ptr<Config> config) :
				mT(static_cast<Real>(delayLength) / config->fs), mBuffer(delayLength),
				mAbsorptionFilter(CalculateFilterGains(T60), config->frequencyBands, config->Q, config->fs),
				mReflectionFilter(config->frequencyBands, config->Q, config->fs), idx(0) {}

			/**
			* @brief Default deconstructor
			*/
			~FDNChannel() {};

			/**
			* @brief Sets the target T60 and updates the absorption filter gains
			*
			* @param T60 The new target decay time
			*/
			inline void SetTargetT60(const Coefficients<>& T60)
			{
				mAbsorptionFilter.SetTargetGains(CalculateFilterGains(T60));
			}
			
			/**
			* @brief Sets the target reflection filter gains
			* 
			* @param gains The target reflection filter gains
			* @return True if all current and target reflection gains are zero, false otherwise
			*/
			inline bool SetTargetReflectionFilter(const Coefficients<>& gains)
			{
				return mReflectionFilter.SetTargetGains(gains);
			}

			/**
			* @brief Resets the internal buffers to zero
			*/
			inline void Reset()
			{ 
				clearBuffers.store(true, std::memory_order_release);
				mAbsorptionFilter.ClearBuffers();
				mReflectionFilter.ClearBuffers();
			}

			/**
			* @brief Process the output reflection filter
			* 
			* @param data The input audio data to process
			* @param outptuBuffer The output buffer to write to
			* @param numFrames The number of frames in the input data
			* @param lerpFactor The linear interpolation factor
			*/
			inline void ProcessOutput(const Buffer<>& data, Buffer<>& outputBuffer, const Real lerpFactor)
			{
				mReflectionFilter.ProcessAudio(data, outputBuffer, lerpFactor);
			}

			/**
			* @brief Processes a single sample output
			* 
			* @brief input The next audio sample input to the delay line
			* @return The next output from the channel
			*/
			Real GetOutput(const Real input, const Real lerpFactor);

		private:

			/**
			* @brief Calculates the filter gain coefficients required for a give T60
			*
			* @param T60 The target decay time
			* @return The required filter gain coefficients
			*/
			inline Coefficients<> CalculateFilterGains(const Coefficients<>& T60) const { return (-3.0 * mT / T60).Pow10(); } // 20 * log10(H(f)) = -60 * t / t60(f);

			const Real mT;		// The current delay in seconds
			Buffer<> mBuffer;		// The internal delay line
			int idx;			// Current delay line read index

			GraphicEQ mAbsorptionFilter;		// The absorption filter to match the target decay time
			GraphicEQ mReflectionFilter;		// The reflection filter on the FDN output

			std::atomic<bool> clearBuffers{ false };		// Flag to clear buffers to zeros next time GetOutput is called

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
			* @param T60 Target decay time
			* @param dimensions Primary room dimensions that determine delay line lengths
			* @param config The spatialiser configuration
			*/
			FDN(const Coefficients<>& T60, const Vec& dimensions, const std::shared_ptr<Config> config) : FDN(T60, dimensions, config, InitMatrix(config->numLateReverbChannels)) {}

			/**
			* @brief Default deconstructor
			*/
			virtual ~FDN() {}

			/**
			* @brief Updates the target T60
			* 
			* @param T60 The new decay time
			*/
			void SetTargetT60(const Coefficients<>& T60);

			/**
			* @brief Sets the target reflection filters for each channel
			* 
			* @param gains The target reflection filter gains for each channel
			* @return True if all channels have zero target reflection gains, false otherwise
			*/
			inline bool SetTargetReflectionFilters(const std::vector<Absorption<>>& gains)
			{
				assert(gains.size() == mChannels.size());

				bool isZero = true;
				for (size_t i = 0; i < mChannels.size(); i++)
					isZero = mChannels[i]->SetTargetReflectionFilter(gains[i]) && isZero;
				return isZero;
			}

			/**
			* @brief Processes a single audio buffer
			* 
			* @param data Multichannel audio data input (numChannels x numFrames)
			* @param outputBuffers Output buffers to write to
			*/
			void ProcessAudio(const Matrix& data, std::vector<Buffer<>>& outputBuffers, const Real lerpFactor);

			/**
			* @brief Resets all internal FDN buffers to zero
			*/
			inline void Reset()
			{ 
				clearBuffers.store(true, std::memory_order_release);
				for (auto& channel : mChannels)
					channel->Reset();
			}

		protected:
			/**
			* @brief Initialises an FDN with a target T60 and given primary room dimensions
			*
			* @param T60 Target decay time
			* @param dimensions Primary room dimensions that determine delay line lengths
			* @param config The spatialiser configuration
			* @param matrix The feedback matrix to use for the FDN
			*/
			FDN(const Coefficients<>& T60, const Vec& dimensions, const std::shared_ptr<Config> config, const Matrix& matrix);

			/**
			* @brief Processes a square feedback matrix
			*/
			void ProcessSquare();

			Rowvec x;	// Next input audio buffer
			Rowvec y;	// Previous output audio buffer

		private:
			/**
			* @brief Calculate a sample delay based on given distances
			* 
			* @param dimensions Primary room dimensions
			* @return Sample delays for each FDN channel
			*/
			std::vector<int> CalculateTimeDelay(const Vec& dimensions, const int numLateReverbChannels, const int fs);

			/**
			* @brief Initialises a default diagonal matrix
			* 
			* @param numChannels The number of channels in the FDN
			* @return A diagonal matrix with ones on the diagonal
			*/
			static inline Matrix InitMatrix(const size_t numChannels)
			{
				Matrix matrix = Matrix(numChannels, numChannels);
				for (int i = 0; i < matrix.Rows(); i++)
					matrix[i][i] = 1.0;		// Initialise diagonal to 1.0
				return matrix;
			}

			/**
			* @brief Runs the currently selected matrix Process function
			*/
			virtual inline void ProcessMatrix() { ProcessSquare(); };

			/**
			* @brief Checks if a set of numbers is mutually prime
			* 
			* @param numbers The set of numbers to check
			*/
			static bool IsSetMutuallyPrime(const std::vector<int>& numbers);

			/**
			* @brief Checks if a single entry in a set of numbers is mutually prime with all other entries
			* 
			* @param numbers The set of numbers to check
			* @param idx The index of the entry to check
			*/
			static bool IsEntryMutuallyPrime(const std::vector<int>& numbers, int idx);

			/**
			* @brief Makes a set of numbers mutually prime by iteratively adjusting each entry
			* 
			* @param numbers The set of numbers to adjust
			*/
			static void MakeSetMutuallyPrime(std::vector<int>& numbers);

			const Matrix feedbackMatrix;	// Feedback matrix

			std::vector<std::unique_ptr<FDNChannel>> mChannels;		// Internal delay line channels

			std::atomic<bool> clearBuffers{ false };		// Flag to clear buffers to zeros next time GetOutput is called
		};

		class HouseHolderFDN : public FDN
		{
		public:
			/**
			* @brief Initialises an FDN with a target T60 and given primary room dimensions
			* @details Initialises with a householder matrix
			*
			* @param T60 Target decay time
			* @param dimensions Primary room dimensions that determine delay line lengths
			* @param config The spatialiser configuration
			*/
			HouseHolderFDN(const Coefficients<>& T60, const Vec& dimensions, const std::shared_ptr<Config> config)
				: FDN(T60, dimensions, config, Matrix()), houseHolderFactor(2.0 / static_cast<Real>(config->numLateReverbChannels))
			{}

			/**
			* @brief Default deconstructor
			*/
			~HouseHolderFDN() {}

			/**
			* @brief Processes a householder matrix
			*/
			inline void ProcessMatrix() override
			{
				Real entry = houseHolderFactor * y.Sum();
				for (int i = 0; i < y.Cols(); i++)
					x[i] = entry - y[i];
			}

		private:
			Real houseHolderFactor;		// Precomputed factor for processing a householder matrix

		};

		class RandomOrthogonalFDN : public FDN
		{
		public:
			/**
			* @brief Initialises an FDN with a target T60 and given primary room dimensions
			* @details Initialises with a random orthogonal matrix
			*
			* @param T60 Target decay time
			* @param dimensions Primary room dimensions that determine delay line lengths
			* @param config The spatialiser configuration
			*/
			RandomOrthogonalFDN(const Coefficients<>& T60, const Vec& dimensions, const std::shared_ptr<Config> config)
				: FDN(T60, dimensions, config, InitMatrix(config->numLateReverbChannels)) {}

			/**
			* @brief Default deconstructor
			*/
			~RandomOrthogonalFDN() {}

			/**
			* @brief Initialises a random orthogonal matrix
			* 
			* @param numChannels The number of channels in the FDN
			* @return A random orthogonal matrix
			*/
			static Matrix InitMatrix(const size_t numChannels);
		};
	}
}

#endif