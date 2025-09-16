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
			FDNChannel(const int delayLength, const Coefficients<>& T60, const std::shared_ptr<DSPConfig> dspConfig) requires std::is_same_v<T, Real> :
				mT(static_cast<Real>(delayLength) / dspConfig->GetData().fs), mBuffer(delayLength),
				absorption(CalculateFilterGains(T60)[0]), mAbsorptionFilter(CalculateFilterGains(T60), dspConfig->GetData().frequencyBands, dspConfig->GetData().Q, dspConfig->GetData().fs),
				mReflectionFilter(dspConfig->GetData().frequencyBands, dspConfig->GetData().Q, dspConfig->GetData().fs) {}

			template <typename U = T, std::enable_if_t<std::is_same_v<U, Complex>, bool> = true>
			FDNChannel(const int delayLength, const Coefficients<>& T60, const std::shared_ptr<DSPConfig> dspConfig) requires std::is_same_v<T, Complex> :
				mT(static_cast<Real>(delayLength) / dspConfig->GetData().fs), mBuffer(delayLength),
				absorption(CalculateFilterGains(T60)[0]), mAbsorptionFilter(CalculateFilterGains(T60), dspConfig->GetData().frequencyBands, dspConfig->GetData().Q, dspConfig->GetData().fs) {}

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
				absorption.store(CalculateFilterGains(T60)[0], std::memory_order_release);
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
				mBuffer.Reset();
				mAbsorptionFilter.ClearBuffers();
				mReflectionFilter.ClearBuffers();
			}

			inline void Reset() requires std::is_same_v<T, Complex>
			{
				mBuffer.Reset();
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
			int idx{ 0 };		// Current delay line read index

			std::atomic<Real> absorption;
			GraphicEQ<T> mAbsorptionFilter;		// The absorption filter to match the target decay time
			std::conditional_t<std::is_same_v<T, Real>,
				GraphicEQ<Real>, std::nullptr_t> mReflectionFilter;		// The reflection filter on the FDN output
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
			* @param dspConfig The spatialiser configuration
			*/
			FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig>& dspConfig) : FDN(T60, dimensions, dspConfig, InitMatrix(dspConfig->GetData().fdnSize)) {}

			/**
			* @brief Initialises an FDN with a target T60 and given delay line lengths
			* @details Initialises with a default householder matrix
			*
			* @param T60 Target decay time
			* @param delayLengths Delay line lengths (in samples)
			* @param dspConfig The spatialiser configuration
			*/
			FDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig>& dspConfig) : FDN(T60, delayLengths, dspConfig, InitMatrix(dspConfig->GetData().fdnSize)) {}

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

			inline void SetPrecedingDelay(Real delay, int offset, int fs)
			requires std::is_same_v<T, Complex>
			{
				precedingDelayBuffer.ResizeBuffer(std::max(0, static_cast<int>(delay * fs) - offset));
				precedingDelayBuffer.Reset(); // TODO: Do we want to avoid resetting it?
			}

			inline void SetMinimumReverbTime(Real T60)
				requires std::is_same_v<T, Complex>
			{
				if (mT60 > T60)
				{
					if (!enabled.load(std::memory_order_acquire))
						Reset();
					enabled.store(true, std::memory_order_release);
				}
				else
					enabled.store(false, std::memory_order_release);
			}

			inline void SubmitAudio(const Real* input)
			requires std::is_same_v<T, Complex> 
			{ 
				inputData = reinterpret_cast<const Complex*>(input);
			}

			/**
			* @brief Processes a single audio buffer
			*
			* @param data Multichannel audio data input (numChannels x numFrames)
			* @param outputBuffers Output buffers to write to
			*/
			void ProcessAudio(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData);

			void ProcessAudio(std::vector<Buffer<>>& outputBuffers, const AudioData& audioData);

		protected:
			/**
			* @brief Initialises an FDN with a target T60 and given primary room dimensions
			*
			* @param T60 Target decay time
			* @param dimensions Primary room dimensions that determine delay line lengths
			* @param config The spatialiser configuration
			* @param matrix The feedback matrix to use for the FDN
			*/
			FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix);

			/**
			* @brief Initialises an FDN with a target T60 and given delay line lengths
			*
			* @param T60 Target decay time
			* @param delayLengths Delay line lengths (in samples)
			* @param config The spatialiser configuration
			* @param matrix The feedback matrix to use for the FDN
			*/
			FDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix);

			/**
			* @brief Processes a square feedback matrix
			*/
			void ProcessSquare();

			Rowvec<T> x;	// Next input audio buffer
			Rowvec<T> y;	// Previous output audio buffer

		private:

			inline void Reset();

			/**
			* @brief Calculate a sample delay based on given distances
			*
			* @param dimensions Primary room dimensions
			* @param fdnSize The number of channels in the FDN
			* @param fs The sample rate
			* @return Sample delays for each FDN channel
			*/
			std::vector<int> CalculateTimeDelay(const Vec<>& dimensions, const int fdnSize, const int fs);

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
					matrix(i, i) = 1.0;		// Initialise diagonal to 1.0
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

			// TODO: Update to consider numReverbSources is different from fdnSize
			/* The MoD-ART model relies on the assumption that each FDN's power output starts from a value of 1 at time 0.
			 * The `powerNormalization` value is used to ensure that is the case. The following is an explanation of what's going on.
			 * 
			 * Let N be the number of delay lines in the FDN, and let M be the total number of taps in the FDN (sum of delay line lengths).
			 * Say the FDN receives a single, unit-magnitude impulse as input. The value "1" is copied into the first sample of each delay line.
			 * The FDN's state now consists of N taps set to 1, and M-N taps set to 0. This (N) is the power which will be circulated through the FDN.
			 * If the FDN was lossless, this initial power would be circulated indefinitely.
			 * **IF** the feedback matrix provides good mixing **AND** the delay line lengths provide decorrelated signals, the power would eventually
			 * be spread uniformly over all taps: each tap would, on average, have a steady power of N/M.
			 * This power-per-tap is equal to the power-per-sample leaving each delay line: the output signal of a line is just the value of its last tap.
			 * In other words, the power output of each individual delay line is N/M, and the power output of the whole FDN is N*N/M.
			 * In the case of a lossy FDN (finite T60), these are the starting power values, which then decay exponentially.
			 * 
			 * Each (directional) listener residue takes the output of a single delay line, and assumes that its power starts from a value of 1 at time 0.
			 * In order to achieve this, the FDN's power needs to be multiplied by M/N.
			 * The value of `powerNormalization` is therefore sqrt(M/N), because it is applied to the audio signal, not to its power.
			 */
			Real powerNormalization;
			
			std::conditional_t<std::is_same_v<T, Complex>,
				std::vector<RAVESListenerResidue>, std::nullptr_t> ravesResidues; // Residues for the RAVES algorithm

			// Delay which precedes the entire late reverberation block:
			std::conditional_t<std::is_same_v<T, Complex>,
				Buffer<Complex>, std::nullptr_t> precedingDelayBuffer;	// Buffer implementing the delay
			std::conditional_t<std::is_same_v<T, Complex>,
				int, std::nullptr_t> precedingDelayCursor{ 0 };			// Position of the read/write cursor within the buffer

			const std::conditional_t<std::is_same_v<T, Complex>,
				Real, std::nullptr_t> mT60;

			std::conditional_t<std::is_same_v<T, Complex>,
				std::atomic<bool>, std::nullptr_t> enabled;

			std::conditional_t<std::is_same_v<T, Complex>,
				const Complex*, std::nullptr_t> inputData{ nullptr };
		};

		inline void FDN<Real>::Reset()
		{
			x.Reset();
			y.Reset();
			for (auto& channel : mChannels)
				channel->Reset();
		}

		inline void FDN<Complex>::Reset()
		{
			x.Reset();
			y.Reset();
			precedingDelayBuffer.Reset();
			for (auto& channel : mChannels)
				channel->Reset();
		}

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
			* @param dspConfig The spatialiser configuration
			*/
			HouseHolderFDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig>& dspConfig)
				: FDN<T>(T60, dimensions, dspConfig, Matrix()), houseHolderFactor(2.0 / static_cast<Real>(dspConfig->GetData().fdnSize)) {}
			
			/**
			* @brief Initialises an FDN with a target T60 and given delay line lengths
			* @details Initialises with a householder matrix
			*
			* @param T60 Target decay time
			* @param delayLengths Delay line lengths (in samples)
			* @param dspConfig The spatialiser configuration
			*/
			HouseHolderFDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig>& dspConfig)
				: FDN<T>(T60, delayLengths, dspConfig, Matrix()), houseHolderFactor(2.0 / static_cast<Real>(dspConfig->GetData().fdnSize)) {}

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
			* @param dspConfig The spatialiser configuration
			*/
			RandomOrthogonalFDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig>& dspConfig)
				: FDN<T>(T60, dimensions, dspConfig, InitMatrix(dspConfig->GetData().fdnSize)) {}

			/**
			* @brief Initialises an FDN with a target T60 and given delay line lengths
			* @details Initialises with a random orthogonal matrix
			*
			* @param T60 Target decay time
			* @param delayLengths Delay line lengths (in samples)
			* @param dspConfig The spatialiser configuration
			*/
			RandomOrthogonalFDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig>& dspConfig)
				: FDN<T>(T60, delayLengths, dspConfig, InitMatrix(dspConfig->GetData().fdnSize)) {}

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