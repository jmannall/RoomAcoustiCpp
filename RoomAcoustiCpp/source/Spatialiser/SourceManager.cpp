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

		int SourceManager::Init()
		{
			int id = NextID();
			if (id < 0)
				return id;
			std::lock_guard<std::mutex> lock(frequencyIndexingMutex);
			mSources[id]->Init(dspConfig, frequencyIndexing);
			return id;
		}

		////////////////////////////////////////

		std::vector<Source::Data> SourceManager::GetSourceData(ThreadID id)
		{
			std::vector<Source::Data> sourceData;
			sourceData.reserve(mSources.size());
			int i = 0;
			for (auto& source : mSources)
			{
				if (auto data = source->GetData(id); data.has_value())
				{
					data->id = i;
					sourceData.push_back(*data);
				}
				i++;
			}
			return sourceData;
		}
	}
}