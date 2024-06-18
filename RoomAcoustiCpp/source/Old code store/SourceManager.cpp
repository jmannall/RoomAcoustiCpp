
#include "SourceManager.h"

size_t SourceManager::Init(const vec3& position)
{
	Source source = Source(position);
	if (!mEmptySlots.empty())
	{
		size_t next = mEmptySlots.back();
		mEmptySlots.pop_back();
		mSources[next] = source;
		mFullSlots[next] = true;
		return next;
	}
	// case a new ID needs to be generated
	else
	{
		size_t next = mSources.size();
		mSources.push_back(source);
		mFullSlots.push_back(true);
		return next;
	}
}