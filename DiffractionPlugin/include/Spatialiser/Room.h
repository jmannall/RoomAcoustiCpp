/*
*
*  \Room class
*
*/

#ifndef Spatialiser_Room_h
#define Spatialiser_Room_h

// C++ headers
#include <vector>
#include <mutex>

// Common headers
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Wall.h"
#include "Spatialiser/Edge.h"
#include "Spatialiser/Source.h"

#include "Spatialiser/Diffraction/Path.h"
#include "Spatialiser/VirtualSource.h"
#include "Spatialiser/VirtualSource.h"

namespace UIE
{
	namespace Spatialiser
	{

		//////////////////// Room class ////////////////////

		class Room
		{
		public:

			// Load and Destroy
			Room() : mISMConfig(), mListenerPosition() {};
			Room(int order) : mISMConfig(), mListenerPosition() { mISMConfig.order = order; };
			~Room() {};

			// Image source model
			void UpdateISMConfig(const ISMConfig& config) { mISMConfig = config; }

			// Listener
			void UpdateListenerPosition(const vec3& position);

			// Wall
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
					absorption.area = -absorption.area;
					RemoveEdges(id, it->second);
				}
				mWalls.erase(id);
				mEmptyWallSlots.push_back(id);
				return absorption;
			}

			// Edge
			void InitEdges(const size_t& id);
			inline void RemoveEdges(const size_t& id, const Wall& wall)
			{
				std::vector<size_t> edgeIDs = wall.GetEdges();
				for (int i = 0; i < edgeIDs.size(); i++)
				{
					auto itE = mEdges.find(edgeIDs[i]);
					if (itE == mEdges.end())		// case: edge does not exist
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

			// Source
			inline SourceData UpdateSourcePosition(const size_t& id, const vec3& position)
			{
				lock_guard <mutex> lock(mSourceMutex);
				auto it = mSources.find(id);
				if (it == mSources.end())		// case: source does not exist
				{
					// Create entry if doesn't exist
					SourceData ret;
					SourceData& newData = mSources.try_emplace(id).first->second;
					{
						lock_guard <mutex> iLock(newData.mMutex);
						newData.position = position;
						ret = newData;
					}
					return ret;
				}
				else
				{
					SourceData ret;
					{
						lock_guard <mutex> iLock(it->second.mMutex);
						it->second.position = position;
						ret = it->second;
					}
					return ret;
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
			bool ReflectPointInRoom(const vec3& point, VirtualSourceDataMap& vSources);

			void SetMaxRefOrder(const int& order) { mISMConfig.order = order; }

		private:
			void ParallelFindEdges(Wall& a, Wall& b, const size_t IDa, const size_t IDb);
			void FindEdges(Wall& a, Wall& b, const size_t IDa, const size_t IDb);

			// void SpecularDiffraction(const vec3& point, VirtualSourceMap& sp, VirtualSourceMap& edSp, VirtualSourceMap& spEd, VirtualSourceVec& vSources);
			void HigherOrderSpecularDiffraction(const vec3& point, VirtualSourceMap& sp, VirtualSourceMap& edSp, VirtualSourceMap& spEd, VirtualSourceStore& vSources);
			void FirstOrderDiffraction(const vec3& point, VirtualSourceMap& ed, VirtualSourceStore& vSources);
			void FirstOrderReflections(const vec3& point, VirtualSourceMap& sp, VirtualSourceStore& vSources);
			void HigherOrderReflections(const vec3& point, VirtualSourceMap& sp, VirtualSourceStore& vSources);

			size_t AddEdge(const Edge& edge);

			WallMap mWalls;
			std::vector<size_t> mEmptyWallSlots;
			EdgeMap mEdges;
			std::vector<size_t> mEmptyEdgeSlots;

			vec3 mListenerPosition;
			SourceDataMap mSources;
			ISMConfig mISMConfig;

			std::mutex mWallMutex;
			std::mutex mSourceMutex;
			std::mutex mRemoveMutex;
			std::mutex mLowPrioMutex;
			std::mutex mNextMutex;
		};
	}
}

#endif