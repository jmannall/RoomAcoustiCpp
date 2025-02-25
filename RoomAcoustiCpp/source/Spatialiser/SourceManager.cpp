/*
* @class SourceManager
*
* @brief Declaration of SourceManager class
*
*/

// Spatialiser headers
#include "Spatialiser/SourceManager.h"

namespace RAC
{
	namespace Spatialiser
	{

		//////////////////// SourceManager class ////////////////////

		////////////////////////////////////////

		size_t SourceManager::Init()
		{
			size_t id;
			unique_lock<shared_mutex> lock(mSourceMutex);

			// Source source = Source(mCore, mConfig);
			if (!mEmptySlots.empty()) // Assign source to an existing ID
			{
				id = mEmptySlots.back();
				mEmptySlots.pop_back();
			}
			else // Create a new ID
				id = nextSource++;

			mSources.try_emplace(id, mCore, mConfig);
			// mSources.insert_or_assign(id, source);
			sourceData.emplace_back();
			return id;
		}

		////////////////////////////////////////

		void SourceManager::Remove(const size_t id)
		{
			unique_lock<shared_mutex> lock(mSourceMutex);

			size_t removed = mSources.erase(id);
			while (!mTimers.empty() && difftime(time(nullptr), mTimers.front().time) > 60)
			{
				mEmptySlots.push_back(mTimers.front().id);
				mTimers.erase(mTimers.begin());
			}

			if (removed == 0)
				return;

			sourceData.pop_back();
			mTimers.push_back(TimerPair(id));
		}

		////////////////////////////////////////

		void SourceManager::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			mConfig.spatialisationMode = mode;
			shared_lock<shared_mutex> lock(mSourceMutex);
			for (auto& [sourceID, source] : mSources)
				source.UpdateSpatialisationMode(mode);
		}

		////////////////////////////////////////

		void SourceManager::UpdateImpulseResponseMode(const Real lerpFactor, const bool mode)
		{
			mConfig.lerpFactor = lerpFactor;
			shared_lock<shared_mutex> lock(mSourceMutex);
			for (auto& [sourceID, source] : mSources)
				source.UpdateImpulseResponseMode(lerpFactor, mode);
		}

		////////////////////////////////////////

		void SourceManager::UpdateDiffractionModel(const DiffractionModel model)
		{
			mConfig.diffractionModel = model;
			shared_lock<shared_mutex> lock(mSourceMutex);
			for (auto& [sourceID, source] : mSources)
				source.UpdateDiffractionModel(model);
		}

		////////////////////////////////////////

		std::vector<SourceData> SourceManager::GetSourceData()
		{
			shared_lock<shared_mutex> lock(mSourceMutex);
			assert(sourceData.size() == mSources.size()); // Ensure sourceData is up to date (size matches mSources)
			int i = 0;
			Vec4 orientation;
			for (auto& [sourceID, source] : mSources)
			{
				sourceData[i].id = sourceID;
				sourceData[i].position = source.GetPosition();
				sourceData[i].orientation = source.GetOrientation();
				sourceData[i].forward = sourceData[i].orientation.Forward();
				sourceData[i].directivity = source.GetDirectivity();
				sourceData[i].hasChanged = source.HasChanged();
				i++;
			}
			return sourceData;
		}
	}
}