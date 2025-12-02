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
			* @params dspConfig The spatialiser configuration
			*/
			SourceManager(Binaural::CCore* core, const std::shared_ptr<DSPConfig> dspConfig)
				: mCore(core), dspConfig(dspConfig), mImageSources(core, dspConfig), frequencyIndexing(1)
			{
				for (auto& sources : mSources)
					sources.emplace(core, mImageSources, dspConfig);
			};
			
			/**
			* @brief Default deconstructor
			*/
			~SourceManager() {};

			/**
			* @brief Updates the diffraction model
			* 
			* @params model The new diffraction model
			*/
			inline void UpdateDiffractionModel(const DiffractionModel model) { mImageSources.UpdateDiffractionModel(model, dspConfig->GetData().fs); }

			/**
			* @brief Updates the source directivity for a given source ID
			* 
			* @params id The source ID
			* @params directivity The new directivity
			*/
			inline void UpdateSourceDirectivity(const size_t id, const SourceDirectivity directivity)
			{
				mSources[id]->UpdateDirectivity(directivity, dspConfig->GetData().frequencyBands, dspConfig->GetData().numReverbSources);
			}

			/**
			* @brief Updates the size of source residues for all active sources
			*
			* @params indexing The new frequency band indexing
			* @params numFrames The number of frames per audio buffer
			*/
			inline void UpdateMoDARTParameters(const Vec<int>& indexing, int numFrames)
			{
				{
					std::lock_guard<std::mutex> lock(frequencyIndexingMutex);
					frequencyIndexing = indexing;
				}
				for (auto& source : mSources)
					source->UpdateMoDARTParameters(frequencyIndexing, numFrames);
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
				if (id >= MAX_SOURCES)
				{
					Debug::Log("SourceManager::Update: Invalid source ID", Colour::Red);
					return;
				}
				mSources[id]->Update(position, orientation, distance);
			}

			/**
			* @brief Removes a source from the source manager
			* 
			* @params id The ID of the source to remove
			*/
			inline void Remove(const size_t id)
			{
				if (id >= MAX_SOURCES)
				{
					Debug::Log("SourceManager::Remove: Invalid source ID", Colour::Red);
					return;
				}
				mSources[id]->Remove();
			}

			/**
			* @return Position, orientation and directivity data for all sources
			*/
			std::vector<Source::Data> GetSourceData(ThreadID id);

			/**
			* @brief Returns the position of the source with the given ID
			* 
			* @params id The ID of the source
			* @return The position of the source
			*/
			inline Vec3 GetSourcePosition(const size_t id) { return mSources[id]->GetPosition(); }

			/**
			* @brief Returns the position which was last used in ray-tracing for the source with the given ID
			* 
			* @params id The ID of the source
			* @return The last position of the source
			*/
			inline Vec3 GetLastRTMSourcePosition(const size_t id) { return mSources[id]->GetLastRTMPosition(); };
			/**
			* @brief Sets the position which was last used in ray-tracing for the source with the given ID
			*
			* @params id The ID of the source
			* @params The last position of the source
			*/
			inline void SetLastRTMSourcePosition(const size_t id, const Vec3 pos) { mSources[id]->SetLastRTMPosition(pos); };

			/**
			* @brief Updates the audio DSP parameters and image sources for a given source
			* 
			* @params id The ID of the source
			* @params source The source audio DSP parameters
			* @params vSources The new image sources
			*/
			inline void UpdateSourceData(const size_t id, const Source::DSPParameters source, ImageSourceDataMap& vSources)
			{
				PROFILE_UpdateAudioData
				mSources[id]->UpdateData(source, vSources, dspConfig);
			}

			/**
			* @brief Update listener residues for RAVES reverb
			*
			* @param id The ID of the FDN to update
			* @param residues The new source residues (size of numRAVESFDNs)
			*/
			inline void SetSourceTargetResidues(const size_t id, const Coefficients<>& residues)
			{
				mSources[id]->SetTargetResidues(residues);
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

			inline void ResetInputBuffers()
			{
				for (auto& source : mSources)	// Zero any input buffers for sources that are not in use (but may still have image sources)
					source->ResetInputBuffer();
			}

			inline void ProcessAudio(Buffer<>& outputBuffer, const AudioData& audioData)
			{
				PROFILE_EarlyReflections
				audioThreadPool->ProcessAllSources(mSources, mImageSources, outputBuffer, audioData);
				/*for (auto& source : mSources)
					source->ProcessAudio(outputBuffer, audioData);
				mImageSources.ProcessAudio(outputBuffer, audioData);*/
			}

			inline void ProcessLateReverbSend(Matrix<>& reverbInput, const AudioData& audioData)
			{
				PROFILE_LateReverb
				switch (audioData.lateReverbModel)
				{
				default:
				case LateReverbModel::none:
					return;
				case LateReverbModel::raves:
					for (auto& source : mSources)
						source->ProcessMoDARTSend(reverbInput, audioData.lerpFactor);
					break;
				case LateReverbModel::fdn:
					for (auto& source : mSources)
						source->ProcessSingleFDNSend(reverbInput, audioData.lerpFactor);
					mImageSources.ProcessSingleFDNSend(reverbInput, audioData.lerpFactor);
					break;
				}
			}

		private:
			Binaural::CCore* mCore;			// 3DTI processing core

			std::shared_ptr<DSPConfig> dspConfig;			// Spatialiser configuration

			std::array<std::optional<Source>, MAX_SOURCES> mSources;	// Sources for the audio thread
			ImageSourceManager mImageSources;							// Image sources for the audio thread

			std::mutex frequencyIndexingMutex;		// Mutex to protect frequency indexing
			Vec<int> frequencyIndexing;				// Frequency band indexing for MoDART source residues
		};
	}
}

#endif
