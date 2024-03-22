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

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Room class ////////////////////

		class Room
		{
		public:

			// Load and Destroy
			Room(const size_t numBands, const std::vector<vec3>& reverbSourceDirections) : nextPlane(0), nextWall(0), nextEdge(0), reverbTime(ReverbTime::Sabine), mVolume(0.0), numAbsorptionBands(numBands) {}
			~Room() {};

			// Image source model
			inline Coefficients UpdateReverbTimeModel(const ReverbTime& model) { reverbTime = model; return GetReverbTime(); }

			// Wall
			size_t AddWall(Wall& wall);

			inline void UpdateWall(const size_t& id, const vec3& normal, const Real* vData, size_t numVertices)
			{
				lock_guard<std::mutex> lock(mWallMutex);
				auto it = mWalls.find(id);
				if (it == mWalls.end()) { return; } // case: wall does not exist
				else { it->second.Update(normal, vData, numVertices); } // case: wall does exist
			}

			inline void FreeWallId(const size_t& id)
			{ 
				lock_guard<std::mutex> lock(mWallMutex);
				mEmptyWallsSlots.push_back(id);
				auto it = oldEdgeIDs.find(id);
				if (it != oldEdgeIDs.end())		// case: attached edges
				{
					for (auto edgeID : it->second)
						mEmptyEdgeSlots.push_back(edgeID);
				}
			}
			inline void RemoveWall(const size_t& id)
			{
				lock_guard<std::mutex> lock(mWallMutex);
				auto it = mWalls.find(id);

				if (it == mWalls.end()) { return; } // case: wall does not exist
				else // case: wall does exist
				{
					RemoveEdges(it->second.GetEdges(), id);
					RemoveWallFromPlane(it->second.GetPlaneID(), id);
					mWalls.erase(it);
				}
			}

			void InitEdges(const size_t& id);

			void UpdatePlanes();

			void UpdateEdges();

			inline void GetWallVertices(int id, float** wallVertices)
			{
				lock_guard<std::mutex> lock(mWallMutex);
				auto it = mWalls.find(id);

				if (it != mWalls.end()) { return; } // case: wall does not exist
				else { it->second.GetVertices(wallVertices); } // case: wall does exist
			}

			Coefficients GetReverbTime();
			Coefficients GetReverbTime(const Real& volume) { mVolume = volume; return GetReverbTime(); }

			PlaneMap GetPlanes() { lock_guard<std::mutex> lock(mPlaneMutex); return mPlanes; }
			WallMap GetWalls() { lock_guard<std::mutex> lock(mWallMutex); return mWalls; }
			EdgeMap GetEdges() { lock_guard<std::mutex> lock(mEdgeMutex); return mEdges; }

		private:
			inline void RemoveWallFromPlane(const size_t& idP, const size_t& idW)
			{
				lock_guard<std::mutex> lock(mPlaneMutex);
				auto itP = mPlanes.find(idP);

				if (itP != mPlanes.end()) // case: plane does exist
				{
					if (itP->second.RemoveWall(idW)) // If plane contains no other walls
					{
						mEmptyPlanesSlots.push_back(itP->first);
						mPlanes.erase(itP);
					}
				}
			}

			void AssignWallToPlane(const size_t& id);
			void AssignWallToPlane(const size_t& wallID, Wall& wall);

			// Edge
			void AddEdge(const EdgeData& data);

			void InitEdges(const size_t& id, const std::vector<size_t>& IDsW);

			void FindEdges(const size_t& idA, const size_t& idB, std::vector<EdgeData>& data, std::vector<size_t>& IDs);
			void FindEdges(const Wall& wallA, const Wall& wallB, const size_t& idA, const size_t& idB, std::vector<EdgeData>& data);

			void FindParallelEdges(const Wall& wallA, const Wall& wallB, const size_t& idA, const size_t& idB, std::vector<EdgeData>& data);
			void FindEdge(const Wall& wallA, const Wall& wallB, const size_t& idA, const size_t& idB, std::vector<EdgeData>& data);

			inline void UpdateEdge(const size_t& id, const EdgeData& edge)
			{ 
				auto it = mEdges.find(id);
				if (it == mEdges.end()) { return; } // case: edge does not exist
				else { it->second.Update(edge); } // case: edge does exist
			}

			inline void UpdateEdges(const std::vector<size_t>& IDs, const std::vector<EdgeData>& data)
			{
				int i = 0;
				for (auto& edge : data)
				{
					UpdateEdge(IDs[i], edge);
					i++;
				}
			}

			inline void RemoveEdges(const std::vector<size_t>& IDsE, const size_t& idW)
			{
				lock_guard<std::mutex> lock(mEdgeMutex);
				oldEdgeIDs.insert_or_assign(idW, IDsE);
				for (auto& idE : IDsE)
					RemoveEdge(idE, idW);
			}

			inline void RemoveEdge(const size_t& idE, const size_t& idW)
			{
				auto itE = mEdges.find(idE);
				if (itE != mEdges.end())
				{
					size_t id = itE->second.GetWallID(idW);
					auto itW = mWalls.find(id);
					if (itW != mWalls.end())
						itW->second.RemoveEdge(idE);
					mEdges.erase(idE);
				}
			}



			/*void FindEdges(const size_t& id);
			bool UpdateEdge(const size_t& id, Wall& a, Wall& b);*/

			

			Coefficients Sabine(const Coefficients& absorption);
			Coefficients Eyring(const Coefficients& absorption, const Real& surfaceArea);

			Real mVolume;
			ReverbTime reverbTime;
			size_t numAbsorptionBands;

			WallMap mWalls;
			std::vector<size_t> mEmptyWallsSlots;
			size_t nextWall;

			PlaneMap mPlanes;
			std::vector<size_t> mEmptyPlanesSlots;
			size_t nextPlane;

			EdgeMap mEdges;
			std::vector<size_t> mEmptyEdgeSlots;
			size_t nextEdge;
			EdgeIDMap oldEdgeIDs;

			std::mutex mWallMutex; // Must always be locked before Plane and Edge
			std::mutex mPlaneMutex; // Cannot be locked with Edge
			std::mutex mEdgeMutex; // Cannot be locked with Plane
		};
	}
}

#endif