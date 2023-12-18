#pragma once

#include "UnityGAPlugin.h"
#include "DiffractionGeometry.h"

#pragma region WedgeManager
class WedgeManager
{
public:
	WedgeManager() : mWedges(), mEmptySlots() {};
	~WedgeManager() { Reset(); };

	size_t Init(const Wedge& wedge);
	inline void Remove(const size_t& id) { mEmptySlots.push_back(id); mFullSlots[id] = false; }
	inline Wedge& GetData(const size_t& id)	{ return mWedges[id]; }
	
	std::vector<bool> mFullSlots;
private:
	inline void Reset() { mWedges.clear(); mEmptySlots.clear(); }

	std::vector<Wedge> mWedges;
	std::vector<size_t> mEmptySlots;
};
#pragma endregion