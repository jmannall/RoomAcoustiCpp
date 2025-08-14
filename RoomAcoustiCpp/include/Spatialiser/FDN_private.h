/*
* @class FDN, FDNChannel
*
* @brief Declaration of FDN and FDNChannel classes
*
*/

#ifndef RoomAcoustiCpp_FDN_private_h
#define RoomAcoustiCpp_FDN_private_h

// C++ headers
#include <vector>
#include <mutex>
#include <cassert>

// Spatialiser headers
#include "Spatialiser/Types.h"
// RAVES headers
#include"Spatialiser/RAVESResidue.h"

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
		template <typename T = Real>
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
			// template <typename U = T, std::enable_if_t<std::is_same_v<U, Real>, bool> = true>
			FDNChannel(const int delayLength, const Coefficients<>& T60, const std::shared_ptr<Config> config) requires std::is_same_v<T, Real> :
				mT(static_cast<Real>(delayLength) / config->fs), mBuffer(delayLength),
				mAbsorptionFilter(CalculateFilterGains(T60), config->frequencyBands, config->Q, config->fs),
				mReflectionFilter(config->frequencyBands, config->Q, config->fs) {}

			template <typename U = T, std::enable_if_t<std::is_same_v<U, Complex>, bool> = true>
			FDNChannel(const int delayLength, const Coefficients<>& T60, const std::shared_ptr<Config> config) requires std::is_same_v<T, Complex> :
				mT(static_cast<Real>(delayLength) / config->fs), mBuffer(delayLength),
				mAbsorptionFilter(CalculateFilterGains(T60), config->frequencyBands, config->Q, config->fs) {}

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
				absorption.store(T60[0], std::memory_order_release);
				mAbsorptionFilter.SetTargetGains(CalculateFilterGains(T60));
			}

			/**
			* @brief Sets the target reflection filter gains
			*
			* @param gains The target reflection filter gains
			* @return True if all current and target reflection gains are zero, false otherwise
			*/
			inline bool SetTargetReflectionFilter(const Coefficients<>& gains)
			requires std::is_same_v<T, Real>
			{
				return mReflectionFilter.SetTargetGains(gains);
			}

			/**
			* @brief Resets the internal buffers to zero
			*/
			inline void Reset() requires std::is_same_v<T, Real>
			{
				clearBuffers.store(true, std::memory_order_release);
				mAbsorptionFilter.ClearBuffers();
				mReflectionFilter.ClearBuffers();
			}

			inline void Reset() requires std::is_same_v<T, Complex>
			{
				clearBuffers.store(true, std::memory_order_release);
				mAbsorptionFilter.ClearBuffers();
			}

			/**
			* @brief Process the output reflection filter
			*
			* @param data The input audio data to process
			* @param outptuBuffer The output buffer to write to
			* @param numFrames The number of frames in the input data
			* @param lerpFactor The linear interpolation factor
			*/
			inline void ProcessOutput(const Buffer<T>& data, Buffer<T>& outputBuffer, const Real lerpFactor)
			requires std::is_same_v<T, Real>
			{
				mReflectionFilter.ProcessAudio(data, outputBuffer, lerpFactor);
			}

			/**
			* @brief Processes a single sample output
			*
			* @brief input The next audio sample input to the delay line
			* @return The next output from the channel
			*/
			T GetOutput(const T input, const Real lerpFactor);

		private:

			/**
			* @brief Calculates the filter gain coefficients required for a give T60
			*
			* @param T60 The target decay time
			* @return The required filter gain coefficients
			*/
			inline Coefficients<> CalculateFilterGains(const Coefficients<>& T60) const { return (-3.0 * mT / T60).Pow10(); } // 20 * log10(H(f)) = -60 * t / t60(f);

			const Real mT;		// The current delay in seconds
			Buffer<T> mBuffer;	// The internal delay line
			int idx{ 0 };			// Current delay line read index

			std::atomic<Real> absorption;
			GraphicEQ<T> mAbsorptionFilter;		// The absorption filter to match the target decay time
			std::conditional_t<std::is_same_v<T, Real>,
				GraphicEQ<Real>, std::nullptr_t> mReflectionFilter;		// The reflection filter on the FDN output

			std::atomic<bool> clearBuffers{ false };		// Flag to clear buffers to zeros next time GetOutput is called
		};

		/**
		* @brief Implements a feedback delay network with modifiable T60 and delay line lengths
		*/
		template <typename T = Real>
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
			FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<Config> config) : FDN(T60, dimensions, config, InitMatrix(config->numReverbSources)) {}

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

			inline void SetTargetResidues(const Coefficients<>& residues)
			requires std::is_same_v<T, Complex>
			{
				assert(residues.Length() == ravesResidues.size());
				for (int i = 0; i < ravesResidues.size(); i++)
					ravesResidues[i].SetTargetEnergy(residues[i]);
			}

			/**
			* @brief Sets the target reflection filters for each channel
			*
			* @param gains The target reflection filter gains for each channel
			* @return True if all channels have zero target reflection gains, false otherwise
			*/
			inline bool SetTargetReflectionFilters(const std::vector<Absorption<>>& gains)
			requires std::is_same_v<T, Real>
			{
				// assert(gains.size() == mChannels.size());

				bool isZero = true;
				for (int i = 0; i < mChannels.size(); i++)
					isZero = mChannels[i]->SetTargetReflectionFilter(gains[i]) && isZero;
				return isZero;
			}

			inline void SetTimeDelay(Real delay, int fs)
			requires std::is_same_v<T, Complex>
			{
				delayBuffer.ResizeBuffer(static_cast<int>(delay * fs));
			}

			inline void SubmitAudio(const std::vector<Real>& input)
			requires std::is_same_v<T, Complex> 
			{ 
				inputData = reinterpret_cast<const Complex*>(input.data());
			}

			/**
			* @brief Processes a single audio buffer
			*
			* @param data Multichannel audio data input (numChannels x numFrames)
			* @param outputBuffers Output buffers to write to
			*/
			void ProcessAudio(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const Real lerpFactor);

			void ProcessAudio(std::vector<Buffer<>>& outputBuffers, const Real lerpFactor);

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
			FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<Config> config, const Matrix<>& matrix);

			/**
			* @brief Processes a square feedback matrix
			*/
			void ProcessSquare();

			Rowvec<T> x;	// Next input audio buffer
			Rowvec<T> y;	// Previous output audio buffer

		private:
			/**
			* @brief Calculate a sample delay based on given distances
			*
			* @param dimensions Primary room dimensions
			* @return Sample delays for each FDN channel
			*/
			std::vector<int> CalculateTimeDelay(const Vec<>& dimensions, const int numReverbSources, const int fs);

			/**
			* @brief Initialises a default diagonal matrix
			*
			* @param numChannels The number of channels in the FDN
			* @return A diagonal matrix with ones on the diagonal
			*/
			static inline Matrix<> InitMatrix(const size_t numChannels)
			{
				Matrix<> matrix = Matrix<>(numChannels, numChannels);
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

			const Matrix<> feedbackMatrix;	// Feedback matrix

			std::vector<std::unique_ptr<FDNChannel<T>>> mChannels;		// Internal delay line channels
			
			std::conditional_t<std::is_same_v<T, Complex>,
				std::vector<RAVESListenerResidue>, std::nullptr_t> ravesResidues; // Residues for the RAVES algorithm

			std::conditional_t<std::is_same_v<T, Complex>,
				Buffer<Complex>, std::nullptr_t> delayBuffer;

			std::conditional_t<std::is_same_v<T, Complex>,
				int, std::nullptr_t> idx{ 0 };

			std::conditional_t<std::is_same_v<T, Complex>,
				const Complex*, std::nullptr_t> inputData{ nullptr };

			std::atomic<bool> clearBuffers{ false };		// Flag to clear buffers to zeros next time GetOutput is called
		};

		template <typename T = Real>
		class HouseHolderFDN : public FDN<T>
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
			HouseHolderFDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<Config> config)
				: FDN<T>(T60, dimensions, config, Matrix()), houseHolderFactor(2.0 / static_cast<Real>(config->numReverbSources)) {}

			/**
			* @brief Default deconstructor
			*/
			~HouseHolderFDN() {}

			/**
			* @brief Processes a householder matrix
			*/
			inline void ProcessMatrix() override
			{
				T entry = houseHolderFactor * this->y.Sum();
				for (int i = 0; i < this->y.Cols(); i++)
					this->x[i] = entry - this->y[i];
			}

		private:
			Real houseHolderFactor;		// Precomputed factor for processing a householder matrix

		};

		template <typename T = Real>
		class RandomOrthogonalFDN : public FDN<T>
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
			RandomOrthogonalFDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<Config> config)
				: FDN<T>(T60, dimensions, config, InitMatrix(config->numReverbSources)) {}

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
			static Matrix<> InitMatrix(const size_t numChannels);
		};
	}
}

#endif // RoomAcoustiCpp_FDN_private_h