/*
* @class Room
*
* @brief Declaration of Room class
*
*/

//C++ headers
#include <algorithm>
#include <cmath>

// Unity headers
#include "Unity/Debug.h"
#include "Unity/UnityInterface.h"

// Spatialiser headers
#include "Spatialiser/Room.h"

namespace RAC
{
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// Room class ////////////////////

		////////////////////////////////////////

		size_t Room::AddWall(Wall& wall)
		{
			size_t id;
			std::lock_guard<std::mutex> lock(mWallMutex);
			if (!mEmptyWallSlots.empty()) // Assign wall to an existing ID
			{
				id = mEmptyWallSlots.back();
				mEmptyWallSlots.pop_back();
			}
			else // Create a new ID
				id = nextWall++;

			AssignWallToPlane(id, wall);
			mWalls.insert_or_assign(id, wall);
			RecordChange();
			return id;
		}

		////////////////////////////////////////

		void Room::RemoveWall(const size_t id)
		{
			std::lock_guard<std::mutex> lock(mWallMutex);
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
				mWallTimers.push_back(TimerPair(id));
				RecordChange();
			}
		}

		////////////////////////////////////////

		void Room::AssignWallToPlane(const size_t id)
		{
			auto it = mWalls.find(id);
			if (it != mWalls.end())
				AssignWallToPlane(id, it->second);
		}

		////////////////////////////////////////

		void Room::AssignWallToPlane(const size_t idW, Wall& wall)
		{
			std::lock_guard<std::mutex> lock(mPlaneMutex);
			for (auto& it : mPlanes)
			{
				if (it.second.IsCoplanar(wall))
				{
					it.second.AddWall(idW);
					wall.SetPlaneID(it.first);
					return;
				}
			}

			size_t idP;
			// Initialise a new plane
			if (!mEmptyPlaneSlots.empty()) // Assign plane to an existing ID
			{
				idP = mEmptyPlaneSlots.back();
				mEmptyPlaneSlots.pop_back();
			}
			else // Create a new ID
				idP = nextPlane++;

			mPlanes.insert_or_assign(idP, Plane(idW, wall));
			wall.SetPlaneID(idP);
			return;
		}

		////////////////////////////////////////

		void Room::RemoveWallFromPlane(const size_t idP, const size_t idW)
		{
			std::lock_guard<std::mutex> lock(mPlaneMutex);
			auto itP = mPlanes.find(idP);

			if (itP == mPlanes.end()) // case: plane does not exist
				return;

			if (itP->second.RemoveWall(idW)) // If plane contains no other walls
			{
				mPlanes.erase(itP);
				while (!mPlaneTimers.empty() && difftime(time(nullptr), mPlaneTimers.front().time) > 60)
				{
					mEmptyPlaneSlots.push_back(mPlaneTimers.front().id);
					mPlaneTimers.erase(mPlaneTimers.begin());
				}
				mPlaneTimers.push_back(TimerPair(idP));
			}
		}

		////////////////////////////////////////

		void Room::AddEdge(const Edge& edge)
		{
			size_t id;
			std::lock_guard<std::mutex> lock(mEdgeMutex);

			if (IsCoplanarEdge(edge))
				return;

			if (!mEmptyEdgeSlots.empty()) // Assign edge to an existing ID
			{
				id = mEmptyEdgeSlots.back();
				mEmptyEdgeSlots.pop_back();
			}
			else // Create a new ID
				id = nextEdge++;

			IDPair ids = edge.GetWallIDs();
			auto it = mWalls.find(ids.first);
			if (it == mWalls.end()) { return; } // case: wall does not exist
			else { it->second.AddEdge(id); } // case: wall does exist

			it = mWalls.find(ids.second);
			if (it == mWalls.end()) { return; } // case: wall does not exist
			else { it->second.AddEdge(id); } // case: wall does exist

#ifdef DEBUG_INIT
			Debug::Log("Init Edge", Colour::Green);
#endif

			mEdges.insert_or_assign(id, edge);
			RecordChange();
		}

		////////////////////////////////////////

		void Room::InitEdges(const size_t id)
		{
			std::vector<Edge> edges;
			std::lock_guard<std::mutex> lock(mWallMutex);
			auto itA = mWalls.find(id);
			if (itA != mWalls.end())
			{
				for (auto& itB : mWalls)
					FindEdges(itA->second, itB.second, itA->first, itB.first, edges);
				for (auto& edge : edges)
					AddEdge(edge);
			}
		}

		////////////////////////////////////////

		void Room::InitEdges(const size_t id, const std::vector<size_t>& IDsW)
		{
			std::vector<Edge> edges;
			auto itA = mWalls.find(id);
			if (itA != mWalls.end())
			{
				for (auto& itB : mWalls)
				{
					if (std::find(IDsW.begin(), IDsW.end(), itB.first) == IDsW.end())
						FindEdges(itA->second, itB.second, itA->first, itB.first, edges);
				}
				for (auto& edge : edges)
					AddEdge(edge);
			}
		}

		////////////////////////////////////////

		void Room::FindEdges(const size_t idA, const size_t idB, std::vector<Edge>& edges, std::vector<size_t>& IDs)
		{
			auto itA = mWalls.find(idA);
			auto itB = mWalls.find(idB);
			std::vector<size_t> IDsA = itA->second.GetEdges();
			std::vector<size_t> IDsB = itB->second.GetEdges();
			// Write common edge IDs from both walls to IDs. Used to check whether the number of located edge changes
			std::set_intersection(IDsA.begin(), IDsA.end(), IDsB.begin(), IDsB.end(), std::back_inserter(IDs));
			if (itA != mWalls.end() && itB != mWalls.end()) // case: walls exist
				FindEdges(itA->second, itB->second, idA, idB, edges);
		}

		////////////////////////////////////////

		void Room::FindEdges(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges)
		{
			if (idA != idB)
			{
				Vec3 normalA = wallA.GetNormal();
				Vec3 normalB = wallB.GetNormal();
				if (normalA != normalB)
				{
					if (normalA == -normalB)
						FindParallelEdges(wallA, wallB, idA, idB, edges);
					else
						FindEdge(wallA, wallB, idA, idB, edges);
				}
			}
		}

		////////////////////////////////////////

		void Room::FindParallelEdges(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges)
		{
			if (abs(wallA.GetD() + wallB.GetD()) < 0.01)
			{
				Vertices verticesA = wallA.GetVertices();
				Vertices verticesB = wallB.GetVertices();

				int numA = static_cast<int>(verticesA.size());
				int numB = static_cast<int>(verticesB.size());

				for (int i = 0; i < numA; i += 2)
				{
					bool match = false;
					int j = 0;
					while (!match && j < numB)
					{
						match = verticesA[i] == verticesB[j];
						j++;
					}
					if (match)
					{
						j--;
						int idxA = i + 1;
						if (idxA == numA)
							idxA = 0;
						int idxB = j - 1;
						if (idxB < 0)
							idxB = numB - 1;
						bool validEdge = verticesA[idxA] == verticesB[idxB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down

						if (validEdge) // Planes not twisted
							edges.emplace_back(verticesA[i], verticesA[idxA], wallA.GetNormal(), wallB.GetNormal(), idA, idB, wallA.GetPlaneID(), wallB.GetPlaneID());

						if (i > 0)
						{
							idxA = i - 1;
							if (idxA < 0)
								idxA = numA - 1;
							int idxB = j + 1;
							if (idxB == numB)
								idxB = 0;
							bool validEdge = verticesA[idxA] == verticesB[idxB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down

							if (validEdge) // Planes not twisted
								edges.emplace_back(verticesA[i], verticesA[idxA], wallB.GetNormal(), wallA.GetNormal(), idB, idA, wallB.GetPlaneID(), wallA.GetPlaneID());
						}
					}
				}
			}
		}

		////////////////////////////////////////

		// Coordinates defined using right hand rule.
		// Vertices are defined using a right hand curl around the direction of the normal
		// Edge face normals are defined using right hand curl rule around the direction of the edge (from base to top) that rotates from plane A to plane B through the exterior of the wedge.
		// Walls defined as triangles (can only have one valid edge)
		void Room::FindEdge(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges)
		{
			Vertices verticesA = wallA.GetVertices();
			Vertices verticesB = wallB.GetVertices();

			int numA = static_cast<int>(verticesA.size());
			int numB = static_cast<int>(verticesB.size());

			for (int i = 0; i < numA; i += 2)
			{
				bool match = false;
				int j = 0;
				while (!match && j < numB)
				{
					match = verticesA[i] == verticesB[j];
					j++;
				}
				if (match)
				{
					j--;
					int idxA = i - 1;
					if (idxA < 0)
						idxA = numA - 1;
					int idxB = j + 1;
					if (idxB == numB)
						idxB = 0;
					bool validEdge = verticesA[idxA] == verticesB[idxB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down
					
					if (!validEdge)
					{
						idxA = i + 1;
						if (idxA == numA)
							idxA = 0;
						idxB = j - 1;
						if (idxB < 0)
							idxB = numB - 1;
						validEdge = verticesA[idxA] == verticesB[idxB];
					}
					if (validEdge) // Planes not twisted
					{
						int check = 0;
						while (check == i || check == idxA)
							check++;

						// K won't equal zero as then planes would be parallel
						Real k = wallB.PointWallPosition(verticesA[check]);	// Only valid for convex shapes
						if (k < 0) // Check angle greater than 180
						{
							// Cross(a.GetNormal(), b.GetNormal()) gives vector in direction of the edge
							// verticesA[idxA] - verticesA[i] give vector from base to top of edge
							Vec3 test1 = UnitVectorRound(Cross(wallA.GetNormal(), wallB.GetNormal()));
							Vec3 test2 = UnitVectorRound(verticesA[idxA] - verticesA[i]);
							
							if (test1 == test2) // Check returns correct angle type
							{
								edges.emplace_back(verticesA[i], verticesA[idxA], wallA.GetNormal(), wallB.GetNormal(), idA, idB, wallA.GetPlaneID(), wallB.GetPlaneID());
								return;
							}
							else if (test1 == -test2)
							{
								edges.emplace_back(verticesA[i], verticesA[idxA], wallB.GetNormal(), wallA.GetNormal(), idB, idA, wallB.GetPlaneID(), wallA.GetPlaneID());
								return;
							}
							// else case: edge is invalid (numerical issues caused k < 0)
						}
					}
				}
			}
		}

		////////////////////////////////////////

		void Room::UpdateEdge(const size_t id, const Edge& edge)
		{
			if (IsCoplanarEdge(edge))
			{
				RemoveEdge(id);
				return;
			}

			auto it = mEdges.find(id);
			if (it == mEdges.end()) { return; } // case: edge does not exist
			else { it->second = edge; } // case: edge does exist
		}

		////////////////////////////////////////

		bool Room::IsCoplanarEdge(const Edge& edge)
		{
			IDPair planeIds = edge.GetPlaneIDs();
			// Vec3Pair edgeNormals = edge.GetFaceNormals();
			{
				std::lock_guard<std::mutex> lock(mPlaneMutex);
				for (auto& itP : mPlanes)
				{
					if (itP.first == planeIds.first || itP.first == planeIds.second)
						continue;

					if (itP.second.PointPlanePosition(edge.GetMidPoint()) != 0.0)
						continue;

					if (itP.second.PointPlanePosition(edge.GetEdgeCoord(EPS)) == 0.0)
						return true;
				}
			}
			return false;
		}

		////////////////////////////////////////

		void Room::RemoveEdge(const size_t idE, const size_t idW)
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
				mEdgeTimers.push_back(TimerPair(idE));
			}
		}

		////////////////////////////////////////

		void Room::RemoveEdge(const size_t idE)
		{
			auto itE = mEdges.find(idE);
			if (itE != mEdges.end())
			{
				IDPair ids = itE->second.GetWallIDs();
				auto itW = mWalls.find(ids.first);
				if (itW != mWalls.end()) // case: wall exists
					itW->second.RemoveEdge(idE);

				RemoveEdge(idE, ids.first);
			}
		}

		////////////////////////////////////////

		void Room::UpdatePlanes()
		{
			std::unordered_map<size_t, size_t> freeWalls;
			std::lock_guard<std::mutex> lock(mWallMutex);
			{
				std::lock_guard<std::mutex> lock(mPlaneMutex);
				for (auto& itP : mPlanes)
				{
					bool updatePlane = true;
					std::vector<size_t> ids = itP.second.GetWalls();
					for (auto& id : ids)
					{
						auto itW = mWalls.find(id);
						if (itW == mWalls.end()) // case: wall does not exist
							freeWalls.insert_or_assign(itP.first, id);
						else // case: wall does exist
						{
							if (updatePlane)
							{
								itP.second.Update(itW->second);
								updatePlane = false;
							}
							// If not coplanar - Add to freeWalls
							else if (!itP.second.IsCoplanar(itW->second))
								freeWalls.insert_or_assign(itP.first, id);
						}
					}
				}
			}
			for (auto& it : freeWalls)
			{
				RemoveWallFromPlane(it.first, it.second);
				AssignWallToPlane(it.second);
			}
			RecordChange();
		}

		////////////////////////////////////////

		void Room::UpdateEdges()
		{
			std::unordered_map<size_t, size_t> freeWalls;
			std::lock_guard<std::mutex> lock(mWallMutex);
			{
				std::lock_guard<std::mutex> lock(mEdgeMutex);
				for (auto& itE : mEdges)
				{
					IDPair ids = itE.second.GetWallIDs();
					std::vector<Edge> edges;
					std::vector<size_t> IDs;
					FindEdges(ids.first, ids.second, edges, IDs);

					if (edges.size() == 1)
						UpdateEdge(IDs[0], edges[0]);
					else if (edges.size() > 1)
						UpdateEdges(IDs, edges);
					else
						RemoveEdge(itE.first);
				}
			}
			for (auto& itW : mWalls)
			{
				if (itW.second.EmptyEdges())
				{
					std::vector<size_t> IDs = itW.second.GetEdges();
					std::vector<size_t> IDsW;
					{
						std::lock_guard<std::mutex> lock(mEdgeMutex);
						for (auto& id : IDs)
						{

							auto it = mEdges.find(id);
							if (it != mEdges.end()) // case: edge does exist
								IDsW.push_back(it->second.GetWallID(itW.first));
						}
					}
					InitEdges(itW.first, IDsW);
				}
			}
			RecordChange();
		}

		////////////////////////////////////////
		
		Coefficients Room::GetReverbTime()
		{
			if (mVolume <= 0.0)
				return Coefficients(numAbsorptionBands);

			Coefficients absorption = Coefficients(numAbsorptionBands);
			Real surfaceArea = 0.0;

			{
				std::lock_guard<std::mutex> lock(mWallMutex);
				for (const auto& it : mWalls)
				{
					absorption -= (it.second.GetAbsorption() * it.second.GetAbsorption() - 1.0) * it.second.GetArea();
					surfaceArea += it.second.GetArea();
				}
			}

			switch (reverbFormula)
			{
				case(ReverbFormula::Sabine):
					return Sabine(absorption);
				case(ReverbFormula::Eyring):
					return Eyring(absorption, surfaceArea);
				default:
					return Coefficients(numAbsorptionBands);
			}
		}

		////////////////////////////////////////

		Coefficients Room::Sabine(const Coefficients& absorption)
		{
			Real factor = 24.0 * log(10.0) / SPEED_OF_SOUND;
			return factor * mVolume / absorption;
		}

		////////////////////////////////////////

		Coefficients Room::Eyring(const Coefficients& absorption, const Real& surfaceArea)
		{
			Real factor = 24.0 * log(10.0) / SPEED_OF_SOUND;
			return -factor * mVolume / ((1 - absorption / surfaceArea).Log() * surfaceArea);
		}
	}
}