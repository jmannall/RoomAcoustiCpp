
#include "WedgeManager.h"

size_t WedgeManager::Init(const Wedge& wedge)
{
	if (!mEmptySlots.empty())
	{
		size_t next = mEmptySlots.back();
		mEmptySlots.pop_back();
		mWedges[next] = wedge;
		mFullSlots[next] = true;
		return next;
	}
	// case a new ID needs to be generated
	else
	{
		size_t next = mWedges.size();
		mWedges.push_back(wedge);
		mFullSlots.push_back(true);
		return next;
	}
}