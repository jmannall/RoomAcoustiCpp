#pragma once

#include "UnityGAPlugin.h"
#include "DiffractionGeometry.h"
#include "DiffractionModel.h"
#include "AudioManager.h"
#include "WedgeManager.h"
#include "SourceManager.h"
#include <vector>
#include <unordered_map>

class Geometry;

inline size_t GetID(const size_t& sID, const size_t& wID) { return (size_t)(0.5f * (sID + wID) * (sID + wID + 1.0f) + sID); }

class EmissionManager;

#pragma region PathManager
class PathManager
{
	using PathMap = std::unordered_map<size_t, DiffractionPath>;
public:
	PathManager(SourceManager* sources_, Receiver* receiver_, WedgeManager* wedges_, EmissionManager* emissions_) :
		sources(sources_), receiver(receiver_), wedges(wedges_), emissions(emissions_) {};
	~PathManager() {};


	inline void UpdatePaths();

	inline DiffractionPath& GetData(const size_t& id) 
	{ 
		auto it = mPaths.find(id);
		return it->second;
	} // Could invert to then call GetDta(sID, wID); - This function should only be called when already exists

	inline DiffractionPath& GetData(const size_t& sID, const size_t& wID)
	{
		size_t id = GetID(sID, wID);
		auto it = mPaths.find(id);
		if (it == mPaths.end())		// case: path does not exist
		{
			// Create new path
			auto newPath = mPaths.insert_or_assign(id, DiffractionPath(&sources->GetData(sID), receiver, &wedges->GetData(wID)));
			return newPath.first->second;
		}
		else
		{
			return it->second;
		}
	}

	// int GetSize() { return mPaths.size(); }

private:
	inline void UpdatePath(const size_t& sID, const size_t& wID)
	{
		size_t id = GetID(sID, wID);
		auto it = mPaths.find(id);
		if (it == mPaths.end())
		{
			// Create new path
			mPaths.insert_or_assign(id, DiffractionPath(&sources->GetData(sID), receiver, &wedges->GetData(wID)));
		}
		else
		{
			// Update existing path
			DiffractionPath& path = it->second;
			path.UpdateParameters(&sources->GetData(sID), receiver, &wedges->GetData(wID));
		}
	}

	inline void InvalidatePath(const size_t& id)
	{
		auto it = mPaths.find(id);
		if (it != mPaths.end())
		{
			// Invalidate path
			DiffractionPath& path = it->second;
			path.valid = false;
		}
		// else path doesn't exist
	}

	PathMap mPaths;

	SourceManager* sources;
	Receiver* receiver;
	WedgeManager* wedges;
	EmissionManager* emissions;
};
#pragma endregion

#pragma region EmissionManager
class Emission
{
public:
	Emission(DiffractionPath* path, int _fs) : attenuate(path), lpf(path, _fs), udfa(path, _fs), udfai(path, _fs), nnBest(path), nnSmall(path), utd(path, _fs), btm(path, _fs), fs(_fs) {};
	~Emission() {};

	void UpdateParameters();

	Attenuate attenuate;
	LPF lpf;
	UDFA udfa;
	UDFAI udfai;
	NNBest nnBest;
	NNSmall nnSmall;
	UTD utd;
	BTM btm;
	
private:
	int fs;
};

class EmissionManager
{
	using EmissionMap = std::unordered_map<size_t, Emission>;

public:
	EmissionManager(int samplingRate) : fs(samplingRate), paths(nullptr), mEmissions() {};
	~EmissionManager() { Reset(); };

	inline void SetPathManager(PathManager* paths_) { paths = paths_; }

	inline void UpdateDSPParameters(const size_t& id)
	{
		auto it = mEmissions.find(id);
		if (it == mEmissions.end())
		{
			mEmissions.insert_or_assign(id, Emission(&paths->GetData(id), fs));
		}
		else
		{
			Emission& emission = it->second;
			emission.UpdateParameters();
		}
	}

	inline Emission& GetDSPParameters(const size_t& sID, const size_t& wID)
	{
		size_t id = GetID(sID, wID);
		// finds id in the map, if doesn't exist makes a new one - assumes path exists
		auto it = mEmissions.find(id);
		if (it == mEmissions.end())
		{
			std::cout << "EmissionSamplingRate: " << fs << "\n";
			auto newEmission = mEmissions.insert_or_assign(id, Emission(&paths->GetData(sID, wID), fs));
			return newEmission.first->second;
		}
		else
		{
			return it->second;
		}
	}

private:
	inline void Reset() { mEmissions.clear(); }

	int fs;
	PathManager* paths;
	EmissionMap mEmissions;
};
#pragma endregion

inline void PathManager::UpdatePaths()
{
	for (int i = 0; i < sources->mFullSlots.size(); i++)
		for (int j = 0; j < wedges->mFullSlots.size(); j++)
		{
			size_t id = GetID((size_t)i, (size_t)j);
			if (sources->mFullSlots[i] & wedges->mFullSlots[j])
			{
				UpdatePath((size_t)i, (size_t)j);
				emissions->UpdateDSPParameters(id);
			}
			else
			{
				InvalidatePath(id);
			}
		}
}