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

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"
#include "Common/ThreadPool.h"
#include "Common/ReleasePool.h"

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
			* @params config The spatialiser configuration
			* @params shift The position offset relative to the listener
			*/
			ReverbSource(Binaural::CCore* core, const Config& config, const Vec3& shift);

			/**
			* @brief Default deconstructor
			*/
			~ReverbSource();

			/**
			* @brief Update the spatialisation mode for the HRTF processing
			*
			* @params mode New spatialisation mode
			*/
			void UpdateSpatialisationMode(const SpatialisationMode mode);

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
			*/
			void ProcessAudio(const Buffer& data, Buffer& outputBuffer);

			/**
			* @brief Reset the internal buffers to zero
			*/
			inline void Reset() { clearBuffers.store(true); }

		private:
			/**
			* @brief Initialises the reverb source
			*/
			void InitSource();

			const Vec3 mShift;		// Position shift relative to the listener

			Binaural::CCore* mCore;								// 3DTI core
			shared_ptr<Binaural::CSingleSourceDSP> mSource;		// 3DTI source
			std::atomic<shared_ptr<const CTransform>> transform;		// 3DTI source transform
			CMonoBuffer<float> bInput;							// 3DTI Input buffer	
			CEarPair<CMonoBuffer<float>> bOutput;				// 3DTI Output buffer

			std::atomic<bool> clearBuffers{ false };		// Flag to clear buffers to zeros next time ProcessAudio is called

			static ReleasePool releasePool;
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
			* @params config The spatialiser configuration
			*/
			Reverb(Binaural::CCore* core, const Config& config) : mConfig(config)
			{
				InitSources(core);
			}

			/**
			* @brief Constructor that intialises late reveberation with a target T60 and given primary room dimensions
			*
			* @params core The 3DTI processing core
			* @params config The spatialiser configuration
			* @params dimensions Primary room dimensions that determine delay line lengths
			* @params T60 Target decay time
			*/
			// Reverb(Binaural::CCore* core, const Config& config, const Vec& dimensions, const Coefficients& T60) {}

			/**
			* @brief Default deconstructor
			*/
			~Reverb() {}

			/**
			* @brief Update the spatialisation mode for the HRTF processing
			*
			* @params mode New spatialisation mode
			*/
			void UpdateSpatialisationMode(const SpatialisationMode mode);

			/**
			* @brief Update the interpolation factor for interpolations
			*
			* @params lerpFactor New interpolation factor
			*/
			void UpdateLerpFactor(const Real lerpFactor);

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
			void ProcessAudio(const Matrix& data, Buffer& outputBuffer);

			/**
			* @brief Resets the FDN and ReverbSources internal buffers to zero
			*/
			inline void Reset()
			{
				for (auto& reverbSource : mReverbSources)
					reverbSource->Reset();
				mFDN.load()->Reset();
			}

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

			inline void SetReverbGain(const Real gain) { /*reverbGain = gain;*/ }

			/**
			* @brief Updates the target T60
			*
			* @params T60 The new decay time
			*/
			void SetTargetT60(const Coefficients& T60);

			void InitLateReverb(const Coefficients& T60, const Vec& dimensions, const FDNMatrix matrix);

			/**
			* @brief Update reflection filters for directional dependent reverberation level
			*
			* @params absorptions New reflection filter target gains
			* @params running True if including late reveberation in audio prcoessing, false otherwise
			*/
			void UpdateReflectionFilters(const std::vector<Absorption>& absorptions);

		private:
			/**
			* @brief Initialse the ReverbSources
			*/
			void InitSources(Binaural::CCore* core);

			std::atomic<std::shared_ptr<FDN>> mFDN;		// FDN for late reverberation processing

			Config mConfig;									// Spatialiser configuration
			// Coefficients mT60;								// Target decay time
			std::vector<std::unique_ptr<ReverbSource>> mReverbSources;		// Reverb sources to binauralise the FDN output

			std::atomic<bool> initialised{ false };		// True if T60 > 0.0 and T60 < 20.0 seconds
			std::atomic<bool> running{ false };			// True if audio thread should process late reverberation

			std::vector<Buffer> threadResults;
			std::vector<Buffer> reverbOutputs;

			static ReleasePool releasePool;
		};
	}
}

#endif