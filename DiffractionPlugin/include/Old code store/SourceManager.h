#pragma once

#include "UnityGAPlugin.h"
#include "AudioManager.h"

#pragma region SourceManager

class Source;

class SourceManager
{
public:
	SourceManager() : mSources(), mEmptySlots() {};
	~SourceManager() { Reset(); };

	size_t Init(const vec3& position);
	inline void Remove(const size_t& id) { mEmptySlots.push_back(id); mFullSlots[id] = false; }
	inline Source& GetData(const size_t& id) { return mSources[id]; }

	std::vector<bool> mFullSlots;
private:
	inline void Reset() { mSources.clear(); mEmptySlots.clear(); }

	std::vector<Source> mSources;
	std::vector<size_t> mEmptySlots;
};

using Listener = Source;
using Receiver = Source;
#pragma endregion