#pragma once

#include "vec3.h"
#include "Spatialiser/Wall.h"
#include "Spatialiser/Edge.h"
#include "Spatialiser/Diffraction/Path.h"
#include "Spatialiser/VirtualSource.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/VirtualSource.h"
#include "UnityGAPlugin.h"
#include <vector>
#include <thread>

namespace Spatialiser
{
	struct SourceData
	{
		vec3 position;
		bool visible;
		std::mutex mMutex;
		std::vector<VirtualSourceData> vSources;

		SourceData() : position(vec3()), visible(false), vSources() {}
		SourceData(vec3 _position) : position(_position), visible(false), vSources() {}
		//SourceData(const SourceData &data) : position(data.position), visible(data.visible), vSources(data.vSources) {}
	};

	class Room
	{
		using WallMap = std::unordered_map<size_t, Wall>;
		using EdgeMap = std::unordered_map<size_t, Edge>;
		using SourceMap = std::unordered_map<size_t, SourceData>;
		using VirtualSourceMap = std::unordered_multimap<size_t, VirtualSourceData>;
		using VirtualSourceVec = std::vector<VirtualSourceData>;
	public:
		Room() : mMaxOrder(0), mListenerPosition() {};
		Room(int maxOrder) : mMaxOrder(maxOrder), mListenerPosition() {};
		~Room() {};

		void UpdateISMConfig(const ISMConfig& config) { mISMConfig = config; }

		void UpdateListenerPosition(const vec3& position);

		size_t AddWall(const Wall& wall);
		inline Absorption UpdateWall(const size_t& id, const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption)
		{
			lock_guard <mutex> rLock(mWallMutex);
			auto it = mWalls.find(id);
			if (it == mWalls.end())		// case: wall does not exist
			{
				// Wall does not exist
				return Absorption();
			}
			else
			{
				// Update wall
				return it->second.Update(normal, vData, numVertices, absorption);
			}
		}
		inline Absorption RemoveWall(const size_t& id)
		{
			Absorption absorption = Absorption();
			lock_guard <mutex> nLock(mNextMutex);
			lock_guard <mutex> rLock(mRemoveMutex);
			lock_guard <mutex> wLock(mWallMutex);
			auto it = mWalls.find(id);
			if (it == mWalls.end())		// case: wall does not exist
			{
				// Wall does not exist
			}
			else
			{
				// Get absorption to return
				absorption = it->second.GetAbsorption();
				RemoveEdges(id, it->second);
			}
			mWalls.erase(id);
			mEmptyWallSlots.push_back(id);
			return absorption;
		}

		void InitEdges(const size_t& id);
		inline void RemoveEdges(const size_t& id, const Wall& wall)
		{
			std::vector<size_t> edgeIDs = wall.GetEdges();
			for (int i = 0; i < edgeIDs.size(); i++)
			{
				auto itE = mEdges.find(edgeIDs[i]);
				if (itE == mEdges.end())		// case: wall does not exist
				{
					// Edge does not exist
				}
				else
				{
					size_t wallID = itE->second.GetWallID(id);
					auto itW = mWalls.find(wallID);
					if (itW == mWalls.end())		// case: wall does not exist
					{
						// Wall does not exist
					}
					else
					{
						itW->second.RemoveEdge(edgeIDs[i]);
					}
					mEdges.erase(edgeIDs[i]);
					mEmptyEdgeSlots.push_back(edgeIDs[i]);
				}
			}
		}

		FrequencyDependence GetReverbTime(const float& volume);

		inline SourceData& UpdateSourcePosition(const size_t& id, const vec3& position)
		{
			lock_guard <mutex> lock(mSourceMutex);
			auto it = mSources.find(id);
			if (it == mSources.end())		// case: source does not exist
			{
				// Create entry if doesn't exist
				SourceData& newData = mSources.try_emplace(id).first->second;
				{
					lock_guard <mutex> iLock(newData.mMutex);
					newData.position = position;
				}
				return newData;
			}
			else
			{
				{
					lock_guard <mutex> iLock(it->second.mMutex);
					it->second.position = position;
				}
				return it->second;
			}
		}
		inline void RemoveSourcePosition(const size_t& id)
		{
			lock_guard <mutex> nLock(mNextMutex);
			lock_guard <mutex> rLock(mRemoveMutex);
			lock_guard <mutex> lock(mSourceMutex);
			mSources.erase(id);
		}

		bool LineRoomIntersection(const vec3& start, const vec3& end);
		void LineRoomIntersection(const vec3& start, const vec3& end, bool& obstruction);
		bool LineRoomIntersection(const vec3& start, const vec3& end, size_t currentWallID);
		void LineRoomIntersection(const vec3& start, const vec3& end, size_t currentWallID, bool& obstruction);
		bool LineRoomIntersection(const vec3& start, const vec3& end, size_t currentWallID1, size_t currentWallID2);

		void UpdateISM();
		bool ReflectPointInRoom(const vec3& point, std::vector<VirtualSourceData>& vSources);


		void SetMaxRefOrder(const int& maxOrder) { mMaxOrder = maxOrder; }

	private:
		void ParallelFindEdges(Wall& a, Wall& b, const size_t IDa, const size_t IDb);
		void FindEdges(Wall& a, Wall& b, const size_t IDa, const size_t IDb);

		// void SpecularDiffraction(const vec3& point, VirtualSourceMap& sp, VirtualSourceMap& edSp, VirtualSourceMap& spEd, VirtualSourceVec& vSources);
		void HigherOrderSpecularDiffraction(const vec3& point, VirtualSourceMap& sp, VirtualSourceMap& edSp, VirtualSourceMap& spEd, VirtualSourceVec& vSources);
		void FirstOrderDiffraction(const vec3& point, VirtualSourceMap& ed, VirtualSourceVec& vSources);
		void FirstOrderReflections(const vec3& point, VirtualSourceMap& sp, VirtualSourceVec& vSources);
		void HigherOrderReflections(const vec3& point, VirtualSourceMap& sp, VirtualSourceVec& vSources);

		size_t AddEdge(const Edge& edge);

		WallMap mWalls;
		std::vector<size_t> mEmptyWallSlots;
		EdgeMap mEdges;
		std::vector<size_t> mEmptyEdgeSlots;

		vec3 mListenerPosition;
		SourceMap mSources;
		int mMaxOrder;
		ISMConfig mISMConfig;

		std::mutex mWallMutex;
		std::mutex mSourceMutex;
		std::mutex mRemoveMutex;
		std::mutex mLowPrioMutex;
		std::mutex mNextMutex;
	};
}