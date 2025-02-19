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

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Source.h"

// 3DTI headers
#include "BinauralSpatializer/Core.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
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
			SourceManager(Binaural::CCore* core, const Config& config) : mSources(), mEmptySlots(), mCore(core), mConfig(config), nextSource(0) {};
			
			/**
			* @brief Default deconstructor
			*/
			~SourceManager() { unique_lock<shared_mutex> lock(mSourceMutex); Reset(); };

			/**
			* @brief Updates the spatialisation mode for the HRTF processing
			* 
			* @params mode New spatialisation mode
			*/
			void UpdateSpatialisationMode(const SpatialisationMode mode);

			/**
			* @brief Updates the interpolation settings for recording impulse responses
			*
			* @params lerpFactor New interpolation factor
			* @params mode True if disable AttuenationSmoothing, false otherwise
			*/
			void UpdateImpulseResponseMode(const Real lerpFactor, const bool mode);

			/**
			* @brief Updates the diffraction model
			* 
			* @params model The new diffraction model
			*/
			void UpdateDiffractionModel(const DiffractionModel model);

			/**
			* @brief Updates the source directivity for a given source ID
			* 
			* @params id The source ID
			* @params directivity The new directivity
			*/
			inline void UpdateSourceDirectivity(const size_t id, const SourceDirectivity directivity)
			{
				shared_lock<shared_mutex> lock(mSourceMutex);
				auto it = mSources.find(id);
				if (it != mSources.end()) // case: source does exist
					it->second.UpdateDirectivity(directivity);
			}

			/**
			* @brief Initialises a new source
			* 
			* @return The ID of the new source
			*/
			size_t Init();

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
				shared_lock<shared_mutex> lock(mSourceMutex);
				auto it = mSources.find(id);
				if (it != mSources.end()) { it->second.Update(position, orientation, distance); } // case: source does exist
			}

			void Remove(const size_t id);

			/**
			* @return Position, orientation and directivity data for all sources
			*/
			std::vector<SourceData> GetSourceData();

			/**
			* @brief Returns the position of the source with the given ID
			* 
			* @params id The ID of the source
			* @return The position of the source
			*/
			inline Vec3 GetSourcePosition(const size_t id)
			{
				shared_lock<shared_mutex> lock(mSourceMutex);
				auto it = mSources.find(id);
				if (it != mSources.end()) { return it->second.GetPosition(); } // case: source does exist
				return Vec3();
			}

			/**
			* @brief Updates the audio DSP parameters and image sources for a given source
			* 
			* @params id The ID of the source
			* @params source The source audio DSP parameters
			* @params vSources The new image sources
			*/
			inline void UpdateSourceData(const size_t id, const SourceAudioData source, const ImageSourceDataMap& vSources)
			{
				shared_lock<shared_mutex> lock(mSourceMutex);
				auto it = mSources.find(id);
				if (it != mSources.end()) { it->second.UpdateData(source, vSources); } // case: source does exist
			}

			/**
			* @brief Process a single audio frame for a given source
			* 
			* @params id The ID of the source
			* @params data The input audio buffer
			* @params reverbInput The reverb input matrix to write to
			* @params outputBuffer The output audio buffer to write to
			*/
			inline void ProcessAudio(const size_t id, const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer)
			{
				shared_lock<shared_mutex> lock(mSourceMutex);
				auto it = mSources.find(id);
				if (it != mSources.end()) // case: source does exist
					it->second.ProcessAudio(data, reverbInput, outputBuffer);
			}

		private:
			/**
			* @brief Removes all sources
			*/
			inline void Reset() { mSources.clear(); mEmptySlots.clear(); }

			Config mConfig;			// Spatialiser configuration

			SourceMap mSources;							// Stored sources
			std::vector<size_t> mEmptySlots;			// Available source IDs
			std::vector<TimerPair> mTimers;				// Source IDs waiting to be made available
			size_t nextSource;							// Next source ID if none available
			std::vector<SourceData> sourceData;			// Stores position, orientation and directivity data for all sources

			Binaural::CCore* mCore;			// 3DTI processing core

			std::shared_mutex mSourceMutex;		// Protects mSources
		};
	}
}

#endif
