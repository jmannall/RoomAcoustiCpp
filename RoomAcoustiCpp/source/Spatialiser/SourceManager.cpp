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

		void SourceManager::UpdateSpatialisationMode(const SpatMode mode)
		{
			mConfig.spatMode = mode;
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

		std::vector<IDPositionPair> SourceManager::GetSourceData()
		{
			shared_lock<shared_mutex> lock(updateMutex);
			assert(sourceData.size() == mSources.size()); // Ensure sourceData is up to date (size matches mSources)
			int i = 0;
			for (auto& it : mSources)
			{
				sourceData[i].first = it.first;
				sourceData[i].second = it.second.GetPosition();
				i++;
			}
			return sourceData;
		}

		void SourceManager::UpdateSourceData(const size_t id, const bool visible, const VirtualSourceDataMap& vSources)
		{
			BeginReverb();
			shared_lock<shared_mutex> lock(updateMutex);
			EndReverb();
			auto it = mSources.find(id);
			if (it != mSources.end()) // case: source does exist
			{ it->second.UpdateData(visible, vSources); }
		}

		void SourceManager::ProcessAudio(const size_t id, const Buffer& data, matrix& reverbInput, Buffer& outputBuffer)
		{
			lock_guard <mutex> lock(processAudioMutex);
			auto it = mSources.find(id);
			if (it != mSources.end()) // case: source does exist
			{ it->second.ProcessAudio(data, reverbInput, outputBuffer); }
		}
	}
}