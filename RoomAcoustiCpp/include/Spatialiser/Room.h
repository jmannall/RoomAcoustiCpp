/*
* @class Room
*
* @brief Declaration of Room class
* 
*/

#ifndef RoomAcoustiCpp_Room_h
#define RoomAcoustiCpp_Room_h

// C++ headers
#include <vector>
#include <mutex>

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/Source.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/Wall.h"
#include "Spatialiser/Edge.h"
#include "Spatialiser/Diffraction/Path.h"
#include "Spatialiser/VirtualSource.h"
#include "Spatialiser/VirtualSource.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Room class ////////////////////
		class Room
		{
		public:

			// Load and Destroy
			Room(const size_t numBands) : nextPlane(0), nextWall(0), nextEdge(0), reverbTime(ReverbTime::Sabine), mVolume(0.0), numAbsorptionBands(numBands), hasChanged(true) {}
			~Room() {};

			// Image source model
			inline Coefficients UpdateReverbTimeModel(const ReverbTime model) { reverbTime = model; return GetReverbTime(); }

			// Wall
			size_t AddWall(Wall& wall);

			inline void UpdateWall(const size_t id, const vec3& normal, const Real* vData)
			{
				lock_guard<std::mutex> lock(mWallMutex);
				auto it = mWalls.find(id);
				if (it == mWalls.end()) { return; } // case: wall does not exist
				else { it->second.Update(normal, vData); RecordChange(); } // case: wall does exist
			}

			inline void UpdateWallAbsorption(const size_t id, const Absorption& absorption)
			{
				lock_guard<std::mutex> lock(mWallMutex);
				auto it = mWalls.find(id);
				if (it == mWalls.end()) { return; } // case: wall does not exist
				else { it->second.Update(absorption); RecordChange(); } // case: wall does exist
			}

			inline void RemoveWall(const size_t id)
			{
				lock_guard<std::mutex> lock(mWallMutex);
				auto it = mWalls.find(id);

				if (it == mWalls.end()) { return; } // case: wall does not exist
				else // case: wall does exist
				{
					RemoveEdges(it->second.GetEdges(), id);
					RemoveWallFromPlane(it->second.GetPlaneID(), id);

					mWalls.erase(it);
					while (!mWallTimers.empty() && difftime(time(nullptr), mWallTimers.front().time) > 60)
					{
						mEmptyWallSlots.push_back(mWallTimers.front().id);
						mWallTimers.erase(mWallTimers.begin());
					}
					mWallTimers.push_back(TimerPair(id, time(nullptr)));
					RecordChange();
				}
			}

			void InitEdges(const size_t id);

			void UpdatePlanes();

			void UpdateEdges();

			Coefficients GetReverbTime();
			Coefficients GetReverbTime(const Real volume) { mVolume = volume; return GetReverbTime(); }

			void RecordChange() { lock_guard<std::mutex> lock(mChangeMutex); hasChanged = true; }
			bool HasChanged() {
				lock_guard<std::mutex> lock(mChangeMutex);
				if (hasChanged) { hasChanged = false; return true; }
				else { return false; }
			}

			PlaneMap GetPlanes() { lock_guard<std::mutex> lock(mPlaneMutex); return mPlanes; }
			WallMap GetWalls() { lock_guard<std::mutex> lock(mWallMutex); return mWalls; }
			EdgeMap GetEdges() { lock_guard<std::mutex> lock(mEdgeMutex); return mEdges; }

		private:
			void AssignWallToPlane(const size_t id);
			void AssignWallToPlane(const size_t wallID, Wall& wall);

			inline void RemoveWallFromPlane(const size_t idP, const size_t idW)
			{
				lock_guard<std::mutex> lock(mPlaneMutex);
				auto itP = mPlanes.find(idP);

				if (itP != mPlanes.end()) // case: plane does exist
				{
					if (itP->second.RemoveWall(idW)) // If plane contains no other walls
					{
						mPlanes.erase(itP);
						while (!mPlaneTimers.empty() && difftime(time(nullptr), mPlaneTimers.front().time) > 60)
						{
							mEmptyPlaneSlots.push_back(mPlaneTimers.front().id);
							mPlaneTimers.erase(mPlaneTimers.begin());
						}
						mPlaneTimers.push_back(TimerPair(idP, time(nullptr)));
					}
				}
			}

			// Edge
			void AddEdge(const Edge& edge);

			void InitEdges(const size_t id, const std::vector<size_t>& IDsW);

			void FindEdges(const size_t idA, const size_t idB, std::vector<Edge>& edges, std::vector<size_t>& IDs);
			void FindEdges(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges);

			void FindParallelEdges(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges);
			void FindEdge(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges);

			inline void UpdateEdge(const size_t id, const Edge& edge)
			{ 
				if (IsCoplanarEdge(edge))
				{
					RemoveEdge(id);
					return;
				}

				auto it = mEdges.find(id);
				if (it == mEdges.end()) { return; } // case: edge does not exist
				else { it->second.Update(edge); } // case: edge does exist
			}

			inline void UpdateEdges(const std::vector<size_t>& IDs, const std::vector<Edge>& edges)
			{
				int i = 0;
				for (auto& edge : edges)
				{
					UpdateEdge(IDs[i], edge);
					i++;
				}
			}

			inline bool IsCoplanarEdge(const Edge& edge)
			{
				IDPair planeIds = edge.GetPlaneIDs();
				// Vec3Pair edgeNormals = edge.GetFaceNormals();
				{
					lock_guard<std::mutex> lock(mPlaneMutex);
					for (auto& itP : mPlanes)
					{
						if (itP.first == planeIds.first || itP.first == planeIds.second)
							continue;

						/*vec3 planeNormal = itP.second.GetNormal();
						if (planeNormal == edgeNormals.first || planeNormal == edgeNormals.second)
							continue;

						if (planeNormal == -edgeNormals.first || planeNormal == -edgeNormals.second)
							continue;*/

						if (itP.second.PointPlanePosition(edge.GetMidPoint()) != 0.0)
							continue;

						if (itP.second.PointPlanePosition(edge.GetEdgeCoord(EPS)) == 0.0)
							return true;
					}
				}
				return false;
			}

			inline void RemoveEdges(const std::vector<size_t>& IDsE, const size_t idW)
			{
				lock_guard<std::mutex> lock(mEdgeMutex);
				for (auto& idE : IDsE)
					RemoveEdge(idE, idW);
			}

			inline void RemoveEdge(const size_t idE, const size_t idW)
			{
				auto itE = mEdges.find(idE);
				if (itE != mEdges.end())
				{
					size_t id = itE->second.GetWallID(idW);
					auto itW = mWalls.find(id);
					if (itW != mWalls.end()) // case: wall exists
						itW->second.RemoveEdge(idE);

					mEdges.erase(idE);
					while (!mEdgeTimers.empty() && difftime(time(nullptr), mEdgeTimers.front().time) > 60)
					{
						mEmptyEdgeSlots.push_back(mEdgeTimers.front().id);
						mEdgeTimers.erase(mEdgeTimers.begin());
					}
					mEdgeTimers.push_back(TimerPair(idE, time(nullptr)));
				}
			}

			inline void RemoveEdge(const size_t idE)
			{
				auto itE = mEdges.find(idE);
				if (itE != mEdges.end())
				{
					IDPair ids = itE->second.GetWallIDs();
					auto itW = mWalls.find(ids.first);
					if (itW != mWalls.end()) // case: wall exists
						itW->second.RemoveEdge(idE);

					itW = mWalls.find(ids.second);
					if (itW != mWalls.end()) // case: wall exists
						itW->second.RemoveEdge(idE);

					mEdges.erase(idE);
					while (!mEdgeTimers.empty() && difftime(time(nullptr), mEdgeTimers.front().time) > 60)
					{
						mEmptyEdgeSlots.push_back(mEdgeTimers.front().id);
						mEdgeTimers.erase(mEdgeTimers.begin());
					}
					mEdgeTimers.push_back(TimerPair(idE, time(nullptr)));
				}
			}

			Coefficients Sabine(const Coefficients& absorption);
			Coefficients Eyring(const Coefficients& absorption, const Real& surfaceArea);

			bool hasChanged;

			Real mVolume;
			ReverbTime reverbTime;
			size_t numAbsorptionBands;

			WallMap mWalls;
			std::vector<size_t> mEmptyWallSlots;
			std::vector<TimerPair> mWallTimers;
			size_t nextWall;

			PlaneMap mPlanes;
			std::vector<size_t> mEmptyPlaneSlots;
			std::vector<TimerPair> mPlaneTimers;
			size_t nextPlane;

			EdgeMap mEdges;
			std::vector<size_t> mEmptyEdgeSlots;
			std::vector<TimerPair> mEdgeTimers;
			size_t nextEdge;
			EdgeIDMap oldEdgeIDs;

			std::mutex mWallMutex; // Must always be locked before Plane and Edge
			std::mutex mPlaneMutex; // Can be locked after Edge (Not before)
			std::mutex mEdgeMutex; // Cannot be locked after Plane
			std::mutex mChangeMutex;
		};
	}
}

#endif