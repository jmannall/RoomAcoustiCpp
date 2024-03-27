/*
*
*  \Source manager class
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

namespace UIE
{
	namespace Spatialiser
	{

		//////////////////// SourceManager class ////////////////////

		size_t SourceManager::Init()
		{
			size_t id;
			std::lock(updateMutex, processAudioMutex);
			std::lock_guard<std::mutex> lk1(updateMutex, std::adopt_lock);
			std::lock_guard<std::mutex> lk2(processAudioMutex, std::adopt_lock);

			Source source = Source(mCore, mConfig);
			if (!mEmptySlots.empty()) // Assign source to an existing ID
			{
				id = mEmptySlots.back();
				mEmptySlots.pop_back();
			}
			else // Create a new ID
				id = nextSource++;

			mSources.insert_or_assign(id, source);
			source.Deactivate();
			return id;
		}

		void SourceManager::UpdateSpatialisationMode(const SPATConfig& config)
		{
			mConfig.spatConfig = config;
			lock_guard <mutex> lock(updateMutex);
			for (auto& it : mSources)
				it.second.UpdateSpatialisationMode(config);
		}

		void SourceManager::GetSourceData(std::vector<SourceData>& data)
		{
			lock_guard <mutex> lock(updateMutex);
			for (auto& it : mSources)
				data.push_back(it.second.GetData(it.first));
		}

		void SourceManager::UpdateSourceData(const SourceData& data)
		{
			lock_guard <mutex> lock(updateMutex);
			auto it = mSources.find(data.id);
			if (it != mSources.end()) // case: source does exist
			{ it->second.UpdateData(data); }
		}

		void SourceManager::ProcessAudio(const size_t& id, const Buffer& data, matrix& reverbInput, Buffer& outputBuffer)
		{
			lock_guard <mutex> lock(processAudioMutex);
			auto it = mSources.find(id);
			if (it != mSources.end()) // case: source does exist
			{ it->second.ProcessAudio(data, reverbInput, outputBuffer); }
		}
	}
}