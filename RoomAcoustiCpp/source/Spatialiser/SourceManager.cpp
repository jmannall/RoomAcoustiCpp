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
			mSources[id]->Init(mConfig);
			return id;
		}

		////////////////////////////////////////

		std::vector<Source::Data> SourceManager::GetSourceData()
		{
			std::vector<Source::Data> sourceData;
			sourceData.reserve(mSources.size());
			int i = 0;
			for (auto& source : mSources)
			{
				if (auto data = source->GetData(); data.has_value())
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