/*
* @class Reverb, ReverbSource
*
* @brief Declaration of Reverb and ReverbSource classes
*
*/

#ifndef RoomAcoustiCpp_Reverb_h
#define RoomAcoustiCpp_Reverb_h

// C++ headers
#include <mutex>
#include <numeric>
#include <unordered_set>

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"
#include "Common/ReleasePool.h"
#include "Common/Vec.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/FDN.h"

// DSP headers
#include "DSP/GraphicEQ.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		/**
		* @brief Handles spatialisation processing for a single late reverberation channel
		*/
		class ReverbSource
		{
		public:
			/**
			* @brief Constructor that intialises a reverb source with a given position offset
			*
			* @params core The 3DTI processing core
			* @params dspConfig The spatialiser configuration
			* @params shift The position offset relative to the listener
			* @params inBuffer Pointer to the input buffer to read from
			*/
			ReverbSource(Binaural::CCore* core, const std::shared_ptr<DSPConfig> dspConfig, const Vec3& shift, const Buffer<>* inBuffer);

			/**
			* @brief Default deconstructor
			*/
			~ReverbSource();

			/**
			* @return The position shift relative to the listener
			*/
			inline Vec3 GetShift() const { return mShift; }

			/**
			* @brief Updates the absolute position of the reverb source to maintain a correct shift relative to the listener
			*
			* @params listenerPosition The current listener position
			*/
			void UpdatePosition(const Vec3& listenerPosition);

			/**
			* @brief Process the current reverb source audio buffer
			*
			* @params outputBuffer The output buffer to write to
			* @params audioData Data relevant to audio processing
			*/
			void ProcessAudio(Buffer<>& outputBuffer, const AudioData& audioData);

		private:
			/**
			* @brief Initialises the reverb source
			* 
			* @params dspConfig The spatialiser configuration
			*/
			void InitSource(const std::shared_ptr<DSPConfig>& dspConfig);

			/**
			* @brief Update the spatialisation mode for the HRTF processing
			*
			* @params New spatialisation mode
			*/
			void SetSpatialisationMode(const SpatialisationMode mode);

			const Vec3 mShift;		// Position shift relative to the listener

			Binaural::CCore* mCore;									// 3DTI core
			shared_ptr<Binaural::CSingleSourceDSP> mSource;			// 3DTI source
#ifdef __ANDROID__
			std::shared_ptr<CTransform> transform;	// 3DTI source transform
#else
			std::atomic<std::shared_ptr<const CTransform>> transform;	// 3DTI source transform
#endif
			CMonoBuffer<float> bInput;								// 3DTI Input buffer	
			CEarPair<CMonoBuffer<float>> bOutput;					// 3DTI Output buffer

			const Buffer<>* inputBuffer{ nullptr };		// Pointer to the input buffer

			SpatialisationMode currentSpatialisationMode{ SpatialisationMode::quality };	// Current spatialisation mode

			static ReleasePool releasePool;		// Garbage collector for shared pointers after atomic replacement
		};

		/**
		* @brief Handles late reverberation processing
		*/
		class Reverb
		{
		public:
			/**
			* @brief Constructor that intialises a default late reverberation with a 1s T60
			*
			* @params core The 3DTI processing core
			* @params dspConfig The spatialiser configuration
			*/
			Reverb(Binaural::CCore* core, const std::shared_ptr<DSPConfig> dspConfig) : reverbSourceInputs(dspConfig->GetData().numReverbSources, Buffer<>(dspConfig->GetData().numFrames))
			{
				int numReverbSources = dspConfig->GetData().numReverbSources;
				const std::vector<Vec3> points = CalculateSourcePositions(numReverbSources);
				mReverbSources.reserve(numReverbSources);
				for (int i = 0; i < numReverbSources; i++)
					mReverbSources.emplace_back(std::make_unique<ReverbSource>(core, dspConfig, points[i], &reverbSourceInputs[i]));
			}

			/**
			* @brief Constructor that intialises late reveberation with a target T60 and given primary room dimensions
			*
			* @params core The 3DTI processing core
			* @params dspConfig The spatialiser configuration
			* @params dimensions Primary room dimensions that determine delay line lengths
			* @params T60 Target decay time
			*/
			// Reverb(Binaural::CCore* core, const DSPConfig& dspConfig, const Vec& dimensions, const Coefficients& T60) {}

			/**
			* @brief Default deconstructor
			*/
			~Reverb() {}

			/**
			* @brief Updates the reverb source positions relative to the listener
			*
			* @params listenerPosition The listener position
			*/
			void UpdateReverbSourcePositions(const Vec3& listenerPosition);

			/**
			* @brief Processes a single audio buffer
			*
			* @params data Multichannel audio data input
			* @params ouputBuffer Stereo output buffer to write to
			*/
			void ProcessAudio(const Matrix<>& data, Buffer<>& outputBuffer, const AudioData& audioData);

			virtual void ProcessReverberator(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData) = 0;

			/**
			 * @brief Assign delay line lengths to a set of FDNs, such that
			 * - across all FDNs, all lines have different lengths
			 * - across all FDNs, all lines lengths are within given upper/lower bounds
			 * - within each FDN, all pairs of line lengths are co-prime
			 * - within each FDN, all line lengths are at least a minimum difference apart
			 *
			 * @param delayLineLengths Integer-valued matrix to fill out (size numFDNs x fdnSize)
			 * @param fs Audio sample rate
			 * @param minDiffSeconds Minimum difference between delay line lengths, in seconds
			 * @param minLineSeconds Minimum delay line length, in seconds
			 * @param maxLineSeconds Maximum delay line length, in seconds
			 */
			static void buildDelaySets(Matrix<int>& delayLineLengths, int fs,
				Real minDiffSeconds = 2e-4, Real minLineSeconds = 3e-2, Real maxLineSeconds = 3e-1);

			inline void SetEigenvectors (const std::vector<Vec<>>& rightEigenvectors, const std::vector<Vec<>>& leftEigenvectors)
			{
				this->rightEigenvectors = rightEigenvectors;
				this->leftEigenvectors = leftEigenvectors;
			}

			inline const Vec<>& GetRightEigenvector(const int id)
			{
				assert(id < rightEigenvectors.size());
				return rightEigenvectors[id];
			}

			inline const Vec<>& GetLeftEigenvector(const int id)
			{
				assert(id < leftEigenvectors.size());
				return leftEigenvectors[id];
			}

			virtual void SetTargetT60(const Coefficients<>& T60) { /*Do nothing*/ }

			/**
			* @brief Update reflection filters for directional dependent reverberation level
			*
			* @params absorptions New reflection filter target gains
			* @params running True if including late reveberation in audio prcoessing, false otherwise
			*/
			virtual void SetTargetOutputFilters(const std::vector<Coefficients<>>& gains) { /*Do Nothing*/ }

			/**
			* @brief Update listener residues for RAVES reverb
			* 
			* @param id The ID of the FDN to update
			* @param residues The new listener residues (size of numReverbSources)
			*/
			virtual void SetTargetListenerResidues(size_t id, const Coefficients<>& residues) { /*Do Nothing*/ }

			/**
			* @brief Update length of preceding delay for RAVES reverb
			*
			* @param delay The new length (in seconds) of the delay
			* @param fs The sample rate
			*/
			virtual void SetPrecedingDelay(const Real delay, int fs) { /*Do Nothing*/ }

			/**
			* @brief Update the minimum reverberation time to model for RAVES reverb
			*
			* @param T60 The minimum reverberation time in seconds
			*/
			virtual void SetMinimumT60(const Real T60) { /*Do Nothing*/ }

			/**
			* @return The length (in seconds) of the preceding delay
			*/
			inline Real GetPrecedingDelay() const { return precedingDelayLength; }

			/**
			* @brief Calculate the end limits for reverb source directions
			*
			* @params directions The vector to add reverb source directions to
			*/
			inline void GetReverbSourceDirections(std::vector<Vec3>& directions) const
			{
				directions.reserve(mReverbSources.size());
				for (const auto& reverbSource : mReverbSources)
					directions.emplace_back(100.0 * reverbSource->GetShift());
			}

			/**
			 * @brief Returns if this filter is valid.
			 *
			 * @return true if the valid is valid and GetOutput() can be called.
			 */
			bool IsValid() const { return initialised.load(std::memory_order_acquire); }

		protected:
			std::atomic<bool> initialised{ false };		// True if T60 > 0.0 and T60 < 20.0 seconds
			std::atomic<bool> running{ false };			// True if audio thread should process late reverberation

			static ReleasePool releasePool;			// Garbage collector for shared pointers after atomic replacement

			int delayOffset{ 0 };					// Offset (in samples) to apply to the preceding delay to account for octave band filtering
			Real precedingDelayLength{ 0.0 };		// Length (in seconds) of the delay which precedes the FDNs in MoDART
		private:
			std::vector<Vec3> CalculateSourcePositions(const int numReverbSources) const;
			
			std::vector<Buffer<>> reverbSourceInputs;						// Input buffers for each reverb source
			std::vector<std::unique_ptr<ReverbSource>> mReverbSources;		// Reverb sources to binauralise the FDN output

			std::vector<Vec<>> rightEigenvectors;	// Right eigenvectors for MoDART
			std::vector<Vec<>> leftEigenvectors;	// Left eigenvectors for MoDART
		};

		class SingleFDN : public Reverb
		{
		public:
			SingleFDN(Binaural::CCore* core, const Coefficients<>& t60, const Vec<> roomDimensions, const LateReverbData& data, const std::shared_ptr<DSPConfig> dspConfig) : Reverb(core, dspConfig)
			{
				InitLateReverb(t60, roomDimensions, data, dspConfig);
			}

			/**
			* @brief Updates the target T60
			*
			* @params T60 The new decay time
			*/
			void SetTargetT60(const Coefficients<>& T60) override;

			/**
			 * @brief Returns if this filter is valid.
			 *
			 * @return true if the valid is valid and GetOutput() can be called.
			 */
			bool IsValid() const { return initialised.load(std::memory_order_acquire); }

			inline void ProcessReverberator(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData) override
			{
				assert(IsValid());
#ifdef __ANDROID__
				std::atomic_load(&mFDN)->ProcessAudio(data, outputBuffers, audioData);
#else
				mFDN.load()->ProcessAudio(data, outputBuffers, audioData);
#endif
			}

			/**
			* @brief Update reflection filters for directional dependent reverberation level
			*
			* @params absorptions New reflection filter target gains
			*/
			void SetTargetOutputFilters(const std::vector<Coefficients<>>& gains);

		private:
			void InitLateReverb(const Coefficients<>& T60, const Vec<>& delayLineLengths, const LateReverbData& data, const std::shared_ptr<DSPConfig>& dspConfig);

#ifdef __ANDROID__
			std::shared_ptr<FDN<>> mFDN;		// FDN for late reverberation processing
#else
			std::atomic<std::shared_ptr<FDN<>>> mFDN;	// FDN for late reverberation processing
#endif
		};

		class RAVES : public Reverb
		{
		public:
			RAVES(Binaural::CCore* core, const MoDARTData& data, const std::shared_ptr<DSPConfig> dspConfig) : Reverb(core, dspConfig)
			{
				InitLateReverb(data, dspConfig);
				SetEigenvectors(data.rightEigenvectors, data.leftEigenvectors);
			}

			/**
			* @brief Update listener residues for RAVES reverb
			*
			* @param id The ID of the FDN to update
			* @param residues The new listener residues (size of numReverbSources)
			*/
			void SetTargetListenerResidues(size_t id, const Coefficients<>& residues) override;

			/**
			* @brief Update length of preceding delay for RAVES reverb
			*
			* @param delay The new length (in seconds) of the delay
			* @param fs The sample rate
			*/
			inline void SetPrecedingDelay(const Real delay, int fs) override
			{
				int samples = static_cast<int>(std::max(0, static_cast<int>(delay * fs) - delayOffset));
				precedingDelayLength = samples > 0 ? delay : static_cast<Real>(delayOffset) / fs;
#ifdef __ANDROID__
				auto fdns = std::atomic_load(&mFDNs);
#else
				auto fdns = mFDNs.load();
#endif
				for (int i = 0; i < fdns->size(); i++)
					fdns->at(i)->SetPrecedingDelay(samples);
			}

			inline void SetMinimumT60(const Real T60) override
			{
#ifdef __ANDROID__
				auto fdns = std::atomic_load(&mFDNs);
#else
				auto fdns = mFDNs.load();
#endif
				for (int i = 0; i < fdns->size(); i++)
					fdns->at(i)->SetMinimumReverbTime(T60);
			}

			void ProcessReverberator(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData) override;

		private:
			void InitLateReverb(const MoDARTData& data, const std::shared_ptr<DSPConfig>& dspConfig);

			using FDNPtr = std::shared_ptr<std::vector<std::unique_ptr<FDN<Complex>>>>;

#ifdef __ANDROID__
			FDNPtr mFDNs;
#else
			std::atomic<FDNPtr> mFDNs;
#endif
		};
	}
}

#endif