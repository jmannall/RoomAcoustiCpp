/*
* @class SourceManager
*
* @brief Declaration of SourceManager class
*
*/

#ifndef RoomAcoustiCpp_SourceManager_h
#define RoomAcoustiCpp_SourceManager_h

// C++ headers
#include <mutex>
#include <shared_mutex>

// Common headers
#include "Common/Types.h"
#include "Common/RACProfiler.h"

// Spatialiser headers
#include "Spatialiser/Globals.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/Source.h"
#include "Spatialiser/ImageSourceManager.h"

// 3DTI headers
#include "BinauralSpatializer/Core.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{
		//////////////////// SourceManager class ////////////////////

		class SourceManager
		{
		public:

			/**
			* @brief Constructor that initialises the source manager with the given configuration.
			* 
			* @params core The 3DTI processing core
			* @params config The spatialiser configuration
			*/
			SourceManager(Binaural::CCore* core, const std::shared_ptr<Config> config) : mCore(core), mConfig(config), mImageSources(core)
			{
				for (auto& sources : mSources)
					sources.emplace(core, mImageSources, config);
			};
			
			/**
			* @brief Default deconstructor
			*/
			~SourceManager() {};

			/**
			* @brief Updates the spatialisation mode for the HRTF processing
			* 
			* @params mode New spatialisation mode
			*/
			inline void UpdateSpatialisationMode(const SpatialisationMode mode)
			{
				mImageSources.UpdateSpatialisationMode(mode);
				for (auto& source : mSources)
					source->UpdateSpatialisationMode(mode);
			}

			/**
			* @brief Updates the interpolation settings for recording impulse responses
			*
			* @params mode True if disable AttuenationSmoothing, false otherwise
			*/
			inline void UpdateImpulseResponseMode(const bool mode)
			{
				mImageSources.UpdateImpulseResponseMode(mode);
				for (auto& source : mSources)
					source->UpdateImpulseResponseMode(mode);
			}

			/**
			* @brief Updates the diffraction model
			* 
			* @params model The new diffraction model
			*/
			inline void UpdateDiffractionModel(const DiffractionModel model) { mImageSources.UpdateDiffractionModel(model, mConfig->fs); }

			/**
			* @brief Updates the source directivity for a given source ID
			* 
			* @params id The source ID
			* @params directivity The new directivity
			*/
			inline void UpdateSourceDirectivity(const size_t id, const SourceDirectivity directivity)
			{
				mSources[id]->UpdateDirectivity(directivity, mConfig->frequencyBands, mConfig->numLateReverbChannels);
			}

			inline int NextID() const
			{
				int nextID = 0;
				for (const auto& source : mSources)
				{
					if (source.has_value() && source->CanEdit() && source->IsReset())
						return nextID;
					nextID++;
				}
				return -1;
			}
			/**
			* @brief Initialises a new source
			* 
			* @return The ID of the new source
			*/
			int Init();

			/**
			* @brief Updates the position and orientation of a source
			* 
			* @params id The ID of the source
			* @params position The new position of the source
			* @params orientation The new orientation of the source
			* @params distance The distance of the source from the listener
			*/
			inline void Update(const size_t id, const Vec3& position, const Vec4& orientation, Real& distance)
			{
				mSources[id]->Update(position, orientation, distance);
			}

			inline void Remove(const size_t id) { mSources[id]->Remove(); }

			/**
			* @return Position, orientation and directivity data for all sources
			*/
			std::vector<Source::Data> GetSourceData();

			/**
			* @brief Returns the position of the source with the given ID
			* 
			* @params id The ID of the source
			* @return The position of the source
			*/
			inline Vec3 GetSourcePosition(const size_t id) { return mSources[id]->GetPosition(); }

			/**
			* @brief Updates the audio DSP parameters and image sources for a given source
			* 
			* @params id The ID of the source
			* @params source The source audio DSP parameters
			* @params vSources The new image sources
			*/
			inline void UpdateSourceData(const size_t id, const Source::AudioData source, const ImageSourceDataMap& vSources)
			{
				PROFILE_UpdateAudioData
				mSources[id]->UpdateData(source, vSources, mConfig);
			}

			/**
			* @brief Resets any unused sources
			*/
			inline void ResetUnusedSources()
			{
				for (auto& source : mSources)
					source->Reset();
				mImageSources.Reset();
			}

			/**
			* @brief Process a single audio frame for a given source
			* 
			* @params id The ID of the source
			* @params data The input audio buffer
			* @params reverbInput The reverb input matrix to write to
			* @params outputBuffer The output audio buffer to write to
			*/
			inline void SetInputBuffer(const size_t id, const Buffer<>& data) { mSources[id]->SetInputBuffer(data); }

			inline void ProcessAudio(Buffer<>& outputBuffer, Matrix& reverbInput, const Real lerpFactor)
			{
				PROFILE_EarlyReflections
				for (auto& source : mSources)	// Zero any input buffers for sources that are not in use (but may still have image sources)
					source->ResetInputBuffer();
				audioThreadPool->ProcessAllSources(mSources, mImageSources, outputBuffer, reverbInput, lerpFactor);
				/*for (auto& source : mSources)
					source->ProcessAudio(outputBuffer, reverbInput, lerpFactor);
				mImageSources.ProcessAudio(outputBuffer, reverbInput, lerpFactor);*/
			}

		private:
			Binaural::CCore* mCore;			// 3DTI processing core

			std::shared_ptr<Config> mConfig;			// Spatialiser configuration

			std::array<std::optional<Source>, MAX_SOURCES> mSources;	// Sources for the audio thread
			ImageSourceManager mImageSources;							// Image sources for the audio thread

		};
	}
}

#endif
