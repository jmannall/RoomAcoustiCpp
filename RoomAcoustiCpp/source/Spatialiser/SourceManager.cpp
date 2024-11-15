/*
* @class SourceManager
*
* @brief Declaration of SourceManager class
*
*/

// C++ headers
#include <unordered_map>
#include <mutex>

// Common headers
#include "Common/Types.h" 

// Spatialiser headers
#include "Spatialiser/Source.h"
#include "Spatialiser/SourceManager.h"

namespace RAC
{
	namespace Spatialiser
	{

		//////////////////// SourceManager class ////////////////////

		size_t SourceManager::Init()
		{
			size_t id;
			lock(updateMutex, processAudioMutex);
			unique_lock<shared_mutex> lk1(updateMutex, std::adopt_lock);
			lock_guard<mutex> lk2(processAudioMutex, std::adopt_lock);

			Source source = Source(mCore, mConfig);
			if (!mEmptySlots.empty()) // Assign source to an existing ID
			{
				id = mEmptySlots.back();
				mEmptySlots.pop_back();
			}
			else // Create a new ID
				id = nextSource++;

			mSources.insert_or_assign(id, source);
			sourceData.emplace_back();
			source.Deactivate();
			return id;
		}

		void SourceManager::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			mConfig.spatialisationMode = mode;
			shared_lock<shared_mutex> lock(updateMutex);
			for (auto& it : mSources)
				it.second.UpdateSpatialisationMode(mode);
		}

		void SourceManager::UpdateDiffractionModel(const DiffractionModel model)
		{
			mConfig.diffractionModel = model;
			shared_lock<shared_mutex> lock(updateMutex);
			for (auto& it : mSources)
				it.second.UpdateDiffractionModel(model);
		}

		void SourceManager::UpdateSourceDirectivity(const size_t id, const SourceDirectivity directivity)
		{
			shared_lock<shared_mutex> lock(updateMutex);
			auto it = mSources.find(id);
			if (it != mSources.end()) // case: source does exist
			{ it->second.UpdateDirectivity(directivity); }
		}

		std::vector<SourceData> SourceManager::GetSourceData()
		{
			shared_lock<shared_mutex> lock(updateMutex);
			assert(sourceData.size() == mSources.size()); // Ensure sourceData is up to date (size matches mSources)
			int i = 0;
			Vec4 orientation;
			for (auto& it : mSources)
			{
				sourceData[i].id = it.first;
				sourceData[i].position = it.second.GetPosition();
				sourceData[i].orientation = it.second.GetOrientation();
				sourceData[i].forward = sourceData[i].orientation.Forward();
				sourceData[i].directivity = it.second.GetDirectivity();
				sourceData[i].hasChanged = it.second.HasChanged();
				i++;
			}
			return sourceData;
		}

		void SourceManager::ProcessAudio(const size_t id, const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer)
		{
			lock_guard <mutex> lock(processAudioMutex);
			auto it = mSources.find(id);
			if (it != mSources.end()) // case: source does exist
			{ it->second.ProcessAudio(data, reverbInput, outputBuffer); }
		}
	}
}