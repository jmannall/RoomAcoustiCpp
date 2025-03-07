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

		void Room::AssignWallToPlane(const size_t wallID, Wall& wall)
		{
			std::lock_guard<std::mutex> lock(mPlaneMutex);
			for (auto& [planeID, plane] : mPlanes)
			{
				if (plane.IsCoplanar(wall))
				{
					plane.AddWall(wallID);
					wall.SetPlaneID(planeID);
					return;
				}
			}

			size_t planeID;
			// Initialise a new plane
			if (!mEmptyPlaneSlots.empty()) // Assign plane to an existing ID
			{
				planeID = mEmptyPlaneSlots.back();
				mEmptyPlaneSlots.pop_back();
			}
			else // Create a new ID
				planeID = nextPlane++;

			mPlanes.insert_or_assign(planeID, Plane(wallID, wall));
			wall.SetPlaneID(planeID);
			return;
		}

		////////////////////////////////////////

		void Room::RemoveWallFromPlane(const size_t planeID, const size_t wallID)
		{
			std::lock_guard<std::mutex> lock(mPlaneMutex);
			auto itP = mPlanes.find(planeID);

			if (itP == mPlanes.end()) // case: plane does not exist
				return;

			if (itP->second.RemoveWall(wallID)) // If plane contains no other walls
			{
				mPlanes.erase(itP);
				while (!mPlaneTimers.empty() && difftime(time(nullptr), mPlaneTimers.front().time) > 60)
				{
					mEmptyPlaneSlots.push_back(mPlaneTimers.front().id);
					mPlaneTimers.erase(mPlaneTimers.begin());
				}
				mPlaneTimers.push_back(TimerPair(planeID));
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

#ifdef DEBUG_GEOMETRY
			Debug::send_path(std::to_string(id) + "e", { edge.GetBase() }, edge.GetTop());
#endif
		}

		////////////////////////////////////////

		void Room::InitEdges(const size_t id)
		{
			std::vector<Edge> edges;
			std::lock_guard<std::mutex> lock(mWallMutex);
			const auto itA = mWalls.find(id);
			if (itA != mWalls.end())
			{
				for (const auto& [wallID, wall] : mWalls)
					FindEdges(itA->second, wall, itA->first, wallID, edges);
				for (const Edge& edge : edges)
					AddEdge(edge);
			}
		}

		////////////////////////////////////////

		void Room::InitEdges(const size_t id, const std::vector<size_t>& wallIDs)
		{
			std::vector<Edge> edges;
			auto itA = mWalls.find(id);
			if (itA != mWalls.end())
			{
				for (const auto& [wallID, wall] : mWalls)
				{
					if (std::find(wallIDs.begin(), wallIDs.end(), wallID) == wallIDs.end())
						FindEdges(itA->second, wall, itA->first, wallID, edges);
				}
				for (const Edge& edge : edges)
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

		bool Room::UpdateEdge(const size_t id, const Edge& edge)
		{
			if (IsCoplanarEdge(edge))
				return true;

			auto it = mEdges.find(id);
			if (it == mEdges.end()) { return false; } // case: edge does not exist
			else { it->second = edge; } // case: edge does exist

#ifdef DEBUG_GEOMETRY
			Debug::send_path(std::to_string(id) + "e", { edge.GetBase() }, edge.GetTop());
#endif
			return false;
		}

		////////////////////////////////////////

		bool Room::IsCoplanarEdge(const Edge& edge)
		{
			IDPair edgeWallIDs = edge.GetWallIDs();
			{
				std::lock_guard<std::mutex> lock(mPlaneMutex);
				for (const auto& [planeID, plane] : mPlanes)
				{
					if (plane.PointPlanePosition(edge.GetBase()) != 0.0)
						continue;

					if (plane.PointPlanePosition(edge.GetTop()) != 0.0)
						continue;

					std::vector<size_t> wallIDs = plane.GetWalls();
					Vec3 start = edge.GetMidPoint() + plane.GetNormal();
					Vec3 end = edge.GetMidPoint() - plane.GetNormal();
					for (const size_t wallID : wallIDs)
					{
						if (wallID == edgeWallIDs.first || wallID == edgeWallIDs.second)
							continue;

						auto itW = mWalls.find(wallID);
						if (itW != mWalls.end())
						{
							if (itW->second.LineWallObstruction(start, end))
								return true;
						}
					}
				}
			}
			return false;
		}

		////////////////////////////////////////

		void Room::RemoveEdge(const size_t edgeID, const size_t wallID)
		{
			auto itE = mEdges.find(edgeID);
			if (itE != mEdges.end())
			{
#ifdef DEBUG_REMOVE
				Debug::Log("Remove Edge", Colour::Red);
#endif
				size_t id = itE->second.GetWallID(wallID);
				auto itW = mWalls.find(id);
				if (itW != mWalls.end()) // case: wall exists
					itW->second.RemoveEdge(edgeID);

#ifdef DEBUG_IEM
				Debug::remove_path(std::to_string(edgeID) + "e");
#endif
				mEdges.erase(edgeID);
				while (!mEdgeTimers.empty() && difftime(time(nullptr), mEdgeTimers.front().time) > 60)
				{
					mEmptyEdgeSlots.push_back(mEdgeTimers.front().id);
					mEdgeTimers.erase(mEdgeTimers.begin());
				}
				mEdgeTimers.push_back(TimerPair(edgeID));
			}
		}

		////////////////////////////////////////

		void Room::RemoveEdge(const size_t edgeID)
		{
			auto itE = mEdges.find(edgeID);
			if (itE != mEdges.end())
			{
				IDPair wallIDs = itE->second.GetWallIDs();
				auto itW = mWalls.find(wallIDs.first);
				if (itW != mWalls.end()) // case: wall exists
					itW->second.RemoveEdge(edgeID);

				RemoveEdge(edgeID, wallIDs.first);
			}
		}

		////////////////////////////////////////

		void Room::UpdatePlanes()
		{
			std::unordered_map<size_t, size_t> freeWalls;
			std::lock_guard<std::mutex> lock(mWallMutex);
			{
				std::lock_guard<std::mutex> lock(mPlaneMutex);
				for (auto& [planeID, plane] : mPlanes)
				{
					bool updatePlane = true;
					std::vector<size_t> wallIDs = plane.GetWalls();
					for (const size_t wallID : wallIDs)
					{
						auto itW = mWalls.find(wallID);
						if (itW == mWalls.end()) // case: wall does not exist
							freeWalls.insert_or_assign(planeID, wallID);
						else // case: wall does exist
						{
							if (updatePlane)
							{
								plane.Update(itW->second);
								updatePlane = false;
							}
							// If not coplanar - Add to freeWalls
							else if (!plane.IsCoplanar(itW->second))
								freeWalls.insert_or_assign(planeID, wallID);
						}
					}
				}
			}
			for (const auto& [wallID, wall] : freeWalls)
			{
				RemoveWallFromPlane(wallID, wall);
				AssignWallToPlane(wall);
			}
			RecordChange();
		}

		////////////////////////////////////////

		void Room::UpdateEdges()
		{
			std::unordered_map<size_t, size_t> freeWalls;
			std::lock_guard<std::mutex> lock(mWallMutex);
			{
				std::vector<size_t> removeIDs;
				std::lock_guard<std::mutex> lock(mEdgeMutex);
				for (const auto& [edgeID, edge] : mEdges)
				{
					IDPair ids = edge.GetWallIDs();
					std::vector<Edge> edges;
					std::vector<size_t> IDs;
					FindEdges(ids.first, ids.second, edges, IDs);

					if (edges.size() == 1)
					{
						if (UpdateEdge(IDs[0], edges[0]))
							removeIDs.push_back(edgeID);
					}
					else if (edges.size() > 1)
					{
						std::vector toRemove = UpdateEdges(IDs, edges);
						removeIDs.insert(removeIDs.end(), toRemove.begin(), toRemove.end());
					}
					else
						removeIDs.push_back(edgeID);
				}
				for (size_t id : removeIDs)
					RemoveEdge(id);
			}
			for (const auto& [wallID, wall] : mWalls)
			{
				if (wall.EmptyEdges())
				{
					std::vector<size_t> edgeIDs = wall.GetEdges();
					std::vector<size_t> wallIDs;
					{
						std::lock_guard<std::mutex> lock(mEdgeMutex);
						for (const size_t edgeID : edgeIDs)
						{

							auto it = mEdges.find(edgeID);
							if (it != mEdges.end()) // case: edge does exist
								wallIDs.push_back(it->second.GetWallID(wallID));
						}
					}
					InitEdges(wallID, wallIDs);
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
				for (const auto& [wallID, wall] : mWalls)
				{
					absorption -= (wall.GetAbsorption() * wall.GetAbsorption() - 1.0) * wall.GetArea();
					surfaceArea += wall.GetArea();
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