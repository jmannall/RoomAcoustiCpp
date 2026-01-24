/*
* @class Room
*
* @brief Declaration of Room class
*
*/

//C++ headers
#include <algorithm>
#include <cmath>

// Common headers
#include "Common/Debug.h"

// Spatialiser headers
#include "Spatialiser/Room.h"
#include "Spatialiser/EdgeLocating.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Room class ////////////////////

		////////////////////////////////////////

		size_t Room::InitMaterial(const Coefficients<>& material)
		{
			size_t id;
			std::lock_guard<std::mutex> lock(mMaterialMutex);
			if (!mEmptyMaterialSlots.empty()) // Assign material to an existing ID
			{
				id = mEmptyMaterialSlots.back();
				mEmptyMaterialSlots.pop_back();
			}
			else // Create a new ID
				id = nextMaterial++;

			// Check if material exists (in case UpdateMaterial was used to set material ID manually)
			auto it = mMaterials.find(id);
			while (it != mMaterials.end()) // case: material already exists
			{
				id++;
				it = mMaterials.find(id);
			}

			mMaterials.insert_or_assign(id, CalculateReflectance(material));
			RecordChange();
			return id;
		}

		////////////////////////////////////////

		void Room::RemoveMaterial(size_t id)
		{
			std::lock_guard<std::mutex> lock(mMaterialMutex);
			auto it = mMaterials.find(id);

			if (it == mMaterials.end()) { return; } // case: material does not exist
			else // case: material does exist
			{
				mMaterials.erase(it);
				mEmptyMaterialSlots.push_back(id);
				RecordChange();
			}
		}

		////////////////////////////////////////

		size_t Room::AddWall(Wall& wall)
		{
			size_t id;
			std::lock_guard<std::mutex> lock(mWallMutex);
			if (mWalls.size() == 0) // TODO: Is this check obsolete now we have polygonID?
			{
				mEmptyWallSlots.clear();
				mWallTimers.clear();
				nextWall = 0;
			}
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
				// TODO: flag plane as changed
				// RemoveEdges(it->second.GetEdges(), id);
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

		void Room::CreateTriangleMeshSoA()
		{
			std::lock_guard<std::mutex> lock(mWallMutex);
			mTriangleMeshSoA.resize(ToInt(mWalls.size()));

			for (const auto& [i, wall] : mWalls)
			{
				Vertices vertices = wall.GetVertices();
				const Vec3& A = vertices[0];
				const Vec3& B = vertices[1];
				const Vec3& C = vertices[2];

				// ----- Anchor vertex A -----
				mTriangleMeshSoA.A[i] = A;

				// ----- Edges from A -----
				mTriangleMeshSoA.edge1[i] = B - A;
				mTriangleMeshSoA.edge2[i] = C - A;


				// ----- Plane parameters: normal n and plane constant d0 -----
				mTriangleMeshSoA.n[i] = wall.GetNormal();
				mTriangleMeshSoA.patchId[i] = static_cast<int>(wall.GetMaterialID());

				mTriangleMeshSoA.d0PlusEPS[i] = wall.GetD() + EPS_FACING;
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
				std::vector<size_t> edgeIDs = itP->second.GetEdges();
				for (size_t edgeID : edgeIDs)
					RemoveEdge(edgeID, planeID);

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

		void Room::AddEdge(const Edge& edge, Plane& planeA, Plane& planeB)
		{
			if (IsCoplanarEdge(edge))
			{
				RAC_DEBUG_LOG("Edge is coplanar, not adding", DebugType::Init);
				return;
			}

			RAC_DEBUG_LOG("Init Edge", DebugType::Init);

			size_t id;
			if (!mEmptyEdgeSlots.empty()) // Assign edge to an existing ID
			{
				id = mEmptyEdgeSlots.back();
				mEmptyEdgeSlots.pop_back();
			}
			else // Create a new ID
				id = nextEdge++;

			planeA.AddEdge(id);
			planeB.AddEdge(id);

			std::lock_guard<std::mutex> lock(mEdgeMutex);
			mEdges.insert_or_assign(id, edge);
			RecordChange();

			RAC_DEBUG_SENDPATH(ToString(id) + "e", edge.GetBase(), edge.GetTop());
		}

		////////////////////////////////////////

		/*void Room::InitEdges(const size_t id)
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
		}*/

		////////////////////////////////////////

		/*void Room::InitEdges(const size_t id, const std::vector<size_t>& wallIDs)
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
		}*/

		////////////////////////////////////////

		//void Room::FindEdges(const size_t idA, const size_t idB, std::vector<Edge>& edges, std::vector<size_t>& IDs)
		//{
		//	auto itA = mWalls.find(idA);
		//	auto itB = mWalls.find(idB);
		//	std::vector<size_t> IDsA = itA->second.GetEdges();
		//	std::vector<size_t> IDsB = itB->second.GetEdges();
		//	// Write common edge IDs from both walls to IDs. Used to check whether the number of located edge changes
		//	std::set_intersection(IDsA.begin(), IDsA.end(), IDsB.begin(), IDsB.end(), std::back_inserter(IDs));
		//	if (itA != mWalls.end() && itB != mWalls.end()) // case: walls exist
		//		FindEdges(itA->second, itB->second, idA, idB, edges);
		//}

		////////////////////////////////////////

		/*void Room::FindEdges(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges)
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
		}*/

		////////////////////////////////////////

		//void Room::FindParallelEdges(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges)
		//{
		//	if (abs(wallA.GetD() + wallB.GetD()) < 0.01)
		//	{
		//		Vertices verticesA = wallA.GetVertices();
		//		Vertices verticesB = wallB.GetVertices();

		//		int numA = static_cast<int>(verticesA.size());
		//		int numB = static_cast<int>(verticesB.size());

		//		for (int i = 0; i < numA; i += 2)
		//		{
		//			bool match = false;
		//			int j = 0;
		//			while (!match && j < numB)
		//			{
		//				match = verticesA[i] == verticesB[j];
		//				j++;
		//			}
		//			if (match)
		//			{
		//				j--;
		//				int idxA = i + 1;
		//				if (idxA == numA)
		//					idxA = 0;
		//				int idxB = j - 1;
		//				if (idxB < 0)
		//					idxB = numB - 1;
		//				bool validEdge = verticesA[idxA] == verticesB[idxB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down

		//				if (validEdge) // Planes not twisted
		//					edges.emplace_back(verticesA[i], verticesA[idxA], wallA.GetNormal(), wallB.GetNormal(), idA, idB, wallA.GetPlaneID(), wallB.GetPlaneID());

		//				if (i > 0)
		//				{
		//					idxA = i - 1;
		//					if (idxA < 0)
		//						idxA = numA - 1;
		//					int idxB = j + 1;
		//					if (idxB == numB)
		//						idxB = 0;
		//					bool validEdge = verticesA[idxA] == verticesB[idxB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down

		//					if (validEdge) // Planes not twisted
		//						edges.emplace_back(verticesA[i], verticesA[idxA], wallB.GetNormal(), wallA.GetNormal(), idB, idA, wallB.GetPlaneID(), wallA.GetPlaneID());
		//				}
		//			}
		//		}
		//	}
		//}

		////////////////////////////////////////

		// Coordinates defined using right hand rule.
		// Vertices are defined using a right hand curl around the direction of the normal
		// Edge face normals are defined using right hand curl rule around the direction of the edge (from base to top) that rotates from plane A to plane B through the exterior of the wedge.
		// Walls defined as triangles (can only have one valid edge)
		//void Room::FindEdge(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges)
		//{
		//	Vertices verticesA = wallA.GetVertices();
		//	Vertices verticesB = wallB.GetVertices();

		//	int numA = static_cast<int>(verticesA.size());
		//	int numB = static_cast<int>(verticesB.size());

		//	for (int i = 0; i < numA; i += 2)
		//	{
		//		bool match = false;
		//		int j = 0;
		//		while (!match && j < numB)
		//		{
		//			match = verticesA[i] == verticesB[j];
		//			j++;
		//		}
		//		if (match)
		//		{
		//			j--;
		//			int idxA = i - 1;
		//			if (idxA < 0)
		//				idxA = numA - 1;
		//			int idxB = j + 1;
		//			if (idxB == numB)
		//				idxB = 0;
		//			bool validEdge = verticesA[idxA] == verticesB[idxB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down

		//			if (!validEdge)
		//			{
		//				idxA = i + 1;
		//				if (idxA == numA)
		//					idxA = 0;
		//				idxB = j - 1;
		//				if (idxB < 0)
		//					idxB = numB - 1;
		//				validEdge = verticesA[idxA] == verticesB[idxB];
		//			}
		//			if (validEdge) // Planes not twisted
		//			{
		//				int check = 0;
		//				while (check == i || check == idxA)
		//					check++;

		//				// K won't equal zero as then planes would be parallel
		//				Real k = wallB.PointWallPosition(verticesA[check]);	// Only valid for convex shapes
		//				if (k < 0) // Check angle greater than 180
		//				{
		//					// Cross(a.GetNormal(), b.GetNormal()) gives vector in direction of the edge
		//					// verticesA[idxA] - verticesA[i] give vector from base to top of edge
		//					Vec3 test1 = Round((wallA.GetNormal().cross(wallB.GetNormal())).Normalised());
		//					Vec3 test2 = Round((verticesA[idxA] - verticesA[i]).Normalised());

		//					Vec3 test = test1 - test2; // If opposite sign, will tend to zero
		//					if (test.Normal() < 0.1)
		//					{

		//						edges.emplace_back(verticesA[i], verticesA[idxA], wallA.GetNormal(), wallB.GetNormal(), idA, idB, wallA.GetPlaneID(), wallB.GetPlaneID());
		//						return;
		//					}
		//					else if (test.Normal() > 1.9)
		//					{
		//						edges.emplace_back(verticesA[i], verticesA[idxA], wallB.GetNormal(), wallA.GetNormal(), idB, idA, wallB.GetPlaneID(), wallA.GetPlaneID());
		//						return;
		//					}
		//					// else case: edge is invalid (numerical issues caused k < 0)
		//				}
		//			}
		//		}
		//	}
		//}

		////////////////////////////////////////

		bool Room::UpdateEdge(const size_t id, const Edge& edge)
		{
			if (IsCoplanarEdge(edge))
			{
				RAC_DEBUG_LOG("Edge is coplanar, not updating", DebugType::Init);
				return true;
			}

			std::lock_guard<std::mutex> lock(mEdgeMutex);
			auto it = mEdges.find(id);
			if (it == mEdges.end()) { return false; } // case: edge does not exist
			else { it->second = edge; } // case: edge does exist

			RAC_DEBUG_SENDPATH(ToString(id) + "e", edge.GetBase(), edge.GetTop());
			return false;
		}

		////////////////////////////////////////

		bool Room::IsCoplanarEdge(const Edge& edge)
		{
			IDPair edgePlaneIDs = edge.GetPlaneIDs();
			{
				for (const auto& [planeID, plane] : mPlanes) // Plane and wall mutexes already locked in calling function UpdateEdges
				{
					if (planeID == edgePlaneIDs.first || planeID == edgePlaneIDs.second)
						continue;

					if (!IsApprox(plane.PointPlanePosition(edge.GetBase()), REAL_CONST(0.0)))
						continue;

					if (!IsApprox(plane.PointPlanePosition(edge.GetTop()), REAL_CONST(0.0)))
						continue;

					std::vector<size_t> wallIDs = plane.GetWalls();
					Vec3 start = edge.GetMidPoint() + plane.GetNormal();
					Vec3 end = edge.GetMidPoint() - plane.GetNormal();
					for (const size_t wallID : wallIDs)
					{
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

		void Room::RemoveEdge(const size_t edgeID)
		{
			RAC_DEBUG_LOG("Remove Edge", DebugType::Remove);

			std::lock_guard<std::mutex> lock(mEdgeMutex);
			mEdges.erase(edgeID);
			while (!mEdgeTimers.empty() && difftime(time(nullptr), mEdgeTimers.front().time) > 60)
			{
				mEmptyEdgeSlots.push_back(mEdgeTimers.front().id);
				mEdgeTimers.erase(mEdgeTimers.begin());
			}
			mEdgeTimers.push_back(TimerPair(edgeID));

			RAC_DEBUG_REMOVEPATH(ToString(edgeID) + "e");
		}

		////////////////////////////////////////

		void Room::RemoveEdge(const size_t edgeID, const size_t planeID)
		{
			auto itE = mEdges.find(edgeID);
			if (itE == mEdges.end())
				return;

			size_t id = itE->second.GetPlaneID(planeID);
			auto itP = mPlanes.find(id);
			if (itP != mPlanes.end()) // case: plane exists
				itP->second.RemoveEdge(edgeID);

			RemoveEdge(edgeID);
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
							{
								RAC_DEBUG_LOG("Wall no longer coplanar with plane, reassigning", DebugType::Init);
								freeWalls.insert_or_assign(planeID, wallID);
							}
						}
					}
				}
			}
			for (const auto& [planeID, wallID] : freeWalls)
			{
				RemoveWallFromPlane(planeID, wallID);
				AssignWallToPlane(wallID);
			}
			RecordChange();
		}

		////////////////////////////////////////
#pragma optimize("", off)
		void Room::FindSegmentsParallel(const Vec3& lineDir, const Vec3& linePoint, const std::vector<size_t>& wallIDs, const Plane& plane, std::vector<EdgeSegment>& segments)
		{
			std::vector<EdgeSegment> locatedSegments;
			for (size_t wallID : wallIDs)
			{
				auto it = mWalls.find(wallID);
				if (it == mWalls.end()) // case: wall does not exist
					continue;

				Vertices vertices = it->second.GetVertices();

				int result = LocateEdgeOnLine(vertices, lineDir, linePoint);

				if (result == 0)	// No valid edge found
					continue;

				bool foundNegative = result < 0;
				if (foundNegative)
					result = -result;

				int indexA = result < 5 ? 0 : 1; // 3 or 4 -> vertex 0, 5 -> vertex 1
				int indexB = result < 4 ? 1 : 2; // 3 -> vertex 1, 4 or 5 -> vertex 2

				Real kA = lineDir.dot(vertices[indexA]);
				Real kB = lineDir.dot(vertices[indexB]);

				locatedSegments.emplace_back(kA, kB, foundNegative);
			}

			if (locatedSegments.size() < 1)	// No segments found
				return;

			if (locatedSegments.size() < 2)	// Only one segment found
			{
				segments = locatedSegments;
				return;
			}

			segments = MergeSegments(locatedSegments);
		}

		void Room::FindSegments(const Vec3& lineDir, const std::vector<size_t>& wallIDs, const Plane& plane, std::vector<EdgeSegment>& segments)
		{
			std::vector<EdgeSegment> locatedSegments;
			for (size_t wallID : wallIDs)
			{
				auto it = mWalls.find(wallID);
				if (it == mWalls.end()) // case: wall does not exist
					continue;

				Vertices vertices = it->second.GetVertices();

				int result = LocateEdgeOnPlane(vertices, plane);

				if (result == 0)	// No valid edge found
					continue;

				bool foundNegative = result < 0;
				if (foundNegative)
					result = -result;

				int indexA = result < 5 ? 0 : 1; // 3 or 4 -> vertex 0, 5 -> vertex 1
				int indexB = result < 4 ? 1 : 2; // 3 -> vertex 1, 4 or 5 -> vertex 2

				Real kA = lineDir.dot(vertices[indexA]);
				Real kB = lineDir.dot(vertices[indexB]);

				locatedSegments.emplace_back(kA, kB, foundNegative);
			}

			if (locatedSegments.size() < 1)	// No segments found
				return;

			if (locatedSegments.size() < 2)	// Only one segment found
			{
				segments = locatedSegments;
				return;
			}

			segments = MergeSegments(locatedSegments);
		}

		void Room::FindExteriorEdges(const std::vector<size_t>& wallIDs)
		{
			if (wallIDs.size() < 2)
				return;

			int numWalls = ToInt(wallIDs.size());
			std::vector<std::pair<Vec3, Vec3>> edgePoints(numWalls);

			for (size_t wallID : wallIDs)
			{
				auto it = mWalls.find(wallID);
				if (it == mWalls.end()) // case: wall does not exist
					continue;

				Vertices vertices = it->second.GetVertices();
			}
		}


		//// Project points onto a plane; for coplanar triangles, we can pick any 2D basis.
		//// Here we assume a function that projects 3D points to 2D coordinates on the plane.
		//struct Vec2 {
		//	double u, v;
		//};

		//// Simple 2D line segment
		//struct Segment2D {
		//	Vec2 p1, p2;
		//};

		//// Return intersection of two 2D segments, if any
		//std::optional<Segment2D> segmentIntersection(const Segment2D& s1, const Segment2D& s2) {
		//	double x1 = s1.p1.u, y1 = s1.p1.v;
		//	double x2 = s1.p2.u, y2 = s1.p2.v;
		//	double x3 = s2.p1.u, y3 = s2.p1.v;
		//	double x4 = s2.p2.u, y4 = s2.p2.v;

		//	double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
		//	if (denom == 0) return std::nullopt; // parallel or collinear

		//	double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
		//	double u = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / denom;

		//	if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
		//		// Intersection point
		//		Vec2 intersect = { x1 + t * (x2 - x1), y1 + t * (y2 - y1) };
		//		return Segment2D{ intersect, intersect };
		//	}

		//	return std::nullopt;
		//}

		//// Function to project a 3D point to 2D plane coordinates
		//Vec2 ProjectTo2D(const Vec3& point, const Vec3& planePoint, const Vec3& planeNormal) {
		//	// Step 1: Define a local 2D coordinate system on the plane
		//	// Pick any vector not parallel to the normal
		//	Vec3 temp(1.0, 0.0, 0.0);
		//	if (std::abs(temp.dot(planeNormal)) > REAL_CONST(0.99))
		//		temp = Vec3(0.0, 1.0, 0.0);

		//	// First axis in plane
		//	Vec3 axisU = planeNormal.cross(temp);
		//	// Second axis in plane
		//	Vec3 axisV = planeNormal.cross(axisU);

		//	// Step 2: Compute vector from plane reference point to point
		//	Vec3 vec = point - planePoint;

		//	// Step 3: Project onto axes
		//	Real u = vec.dot(axisU);
		//	Real v = vec.dot(axisV);

		//	return Vec2(u, v);
		//}

		//// Convert triangles to edges
		//std::vector<Segment2D> triangleToEdges(const Vertices& triangle)
		//{
		//	std::vector<Segment2D> edges(3);
		//	edges[0] = { ProjectTo2D(triangle[0]), ProjectTo2D(triangle[1]) };
		//	edges[1] = { ProjectTo2D(triangle[1]), ProjectTo2D(triangle[2]) };
		//	edges[2] = { ProjectTo2D(triangle[2]), ProjectTo2D(triangle[0]) };
		//	return edges;
		//}

		//// Find all intersecting segments between two sets of triangles
		//std::vector<Segment2D> findIntersectingEdges(const Vertices& verticesA, const Vertices& verticesB)
		//{
		//	std::vector<Segment2D> intersections;

		//	// Convert all triangles to edges
		//	std::vector<Segment2D> edgesA, edgesB;
		//	for (auto& vertex : verticesA) {
		//		auto e = triangleToEdges(tri);
		//		edgesA.insert(edgesA.end(), e.begin(), e.end());
		//	}
		//	for (auto& vertex : verticesB) {
		//		auto e = triangleToEdges(tri);
		//		edgesB.insert(edgesB.end(), e.begin(), e.end());
		//	}

		//	// Check each pair of edges
		//	for (auto& ea : edgesA) {
		//		for (auto& eb : edgesB) {
		//			if (auto inter = segmentIntersection(ea, eb)) {
		//				intersections.push_back(*inter);
		//			}
		//		}
		//	}
		//	return intersections;
		//}




		//// Project a point onto the segment�s 1D param coordinate
		//double Project1D(const Vec3& origin, const Vec3& dir, const Vec3& p)
		//{
		//	return (p - origin).dot(dir); // Coordinate along the line dir
		//}

		//bool DirectionsColinear(const Vec3& a, const Vec3& b)
		//{
		//	Vec3 normalA = a.Normalised();
		//	Vec3 normalB = b.Normalised();
		//	return std::abs(normalA.dot(normalB)) >= EPS_ANGULAR; // Almost parallel or anti-parallel
		//}

		//bool AreParallel(const Vec3& A, const Vec3& B)
		//{
		//	return  IsApprox(A.x * B.y, A.y * B.x) &&
		//		IsApprox(A.x * B.z, A.z * B.x) &&
		//		IsApprox(A.y * B.z, A.z * B.y);
		//}

		//bool LinesAreParallel(const Vec3& dirA, const Vec3& pointA, const Vec3& dirB, const Vec3& pointB)
		//{
		//	if (!AreParallel(dirA, dirB))
		//		return false;

		//	Vec3 AB = pointB - pointA;
		//	return AreParallel(dirA, AB);
		//}
		//

		//// Compute overlap of colinear 3D segments AB and CD.
		//// Returns std::nullopt if no overlap.
		//// Otherwise returns the overlapping segment endpoints in 3D.
		//std::optional<std::pair<Vec3, Vec3>>
		//	SegmentOverlap(const Vertices& verticesA, const Vertices& verticesB)
		//{
		//	const int numA = 3;
		//	const int numB = 3;


		//	bool parallelEdges = AreParallel(verticesA[0] - verticesA[1], verticesB[0] - vertices[1]);

		//	for (int i = 0; i < numA; i += 2)
		//	{
		//		bool match = false;
		//		int j = 0;
		//		while (!match && j < numB)
		//		{
		//			match = verticesA[i] == verticesB[j];
		//			j++;
		//		}
		//		if (match)
		//		{
		//			j--;
		//			int idxA = i + 1;
		//			if (idxA == numA)
		//				idxA = 0;
		//			int idxB = j - 1;
		//			if (idxB < 0)
		//				idxB = numB - 1;
		//			bool validEdge = verticesA[idxA] == verticesB[idxB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down

		//			if (validEdge) // Planes not twisted
		//				edges.emplace_back(verticesA[i], verticesA[idxA], wallA.GetNormal(), wallB.GetNormal(), idA, idB, wallA.GetPlaneID(), wallB.GetPlaneID());

		//			if (i > 0)
		//			{
		//				idxA = i - 1;
		//				if (idxA < 0)
		//					idxA = numA - 1;
		//				int idxB = j + 1;
		//				if (idxB == numB)
		//					idxB = 0;
		//				bool validEdge = verticesA[idxA] == verticesB[idxB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down

		//				if (validEdge) // Planes not twisted
		//					edges.emplace_back(verticesA[i], verticesA[idxA], wallB.GetNormal(), wallA.GetNormal(), idB, idA, wallB.GetPlaneID(), wallA.GetPlaneID());
		//			}
		//		}
		//	}

		//	Vec3 AB = B - A;
		//	Vec3 CD = D - C;

		//	// Must be colinear direction-wise
		//	if (!DirectionsColinear(AB, CD))
		//		return std::nullopt;

		//	// Normalize direction for projection
		//	Vec3 dir = AB.Normalised();

		//	// Project all endpoints into 1D
		//	Real a0 = 0.0;                // project A onto itself = 0
		//	Real a1 = Project1D(A, dir, B);
		//	Real c0 = Project1D(A, dir, C);
		//	Real c1 = Project1D(A, dir, D);

		//	if (a1 < a0)
		//		std::swap(a0, a1); // ensure a0 <= a1

		//	if (c1 < c0)
		//		std::swap(c0, c1);

		//	// Compute interval overlap in 1D
		//	Real lo = std::max(a0, c0);
		//	Real hi = std::min(a1, c1);

		//	if (hi <= lo + EPS)
		//		return std::nullopt; // no overlap

		//	// Convert back to 3D
		//	Vec3 P = A + dir * lo;
		//	Vec3 Q = A + dir * hi;

		//	return std::make_pair(P, Q);
		//}

		void Room::FindEdgesParallel(const Vec3& lineDir, const Vec3& linePoint, const Plane& planeA, const Plane& planeB, std::vector<std::pair<Real, Real>>& edges)
		{
			const auto wallIDsA = planeA.GetWalls();
			const auto wallIDsB = planeB.GetWalls();

			std::vector<EdgeSegment> segmentsA;
			if (wallIDsA.size() < wallIDsB.size()) // Search smaller set first
				FindSegmentsParallel(lineDir, linePoint, wallIDsA, planeB, segmentsA);
			else
				FindSegmentsParallel(lineDir, linePoint, wallIDsB, planeA, segmentsA);

			if (segmentsA.size() < 1) // No edge possible
				return;

			std::vector<EdgeSegment> segmentsB;
			if (wallIDsA.size() < wallIDsB.size())
				FindSegmentsParallel(lineDir, linePoint, wallIDsB, planeA, segmentsB);
			else
				FindSegmentsParallel(lineDir, linePoint, wallIDsA, planeB, segmentsB);

			if (segmentsB.size() < 1) // No edge possible
				return;

			edges = FindSegmentIntersections(segmentsA, segmentsB);
		}

		Vec3 Room::FindEdges(const Plane& planeA, const Plane& planeB, std::vector<std::pair<Real, Real>>& edges)
		{
			Vec3 normalA = planeA.GetNormal();
			Vec3 normalB = planeB.GetNormal();

			if (normalA.dot(normalB) >= EPS_ANGULAR) // Parallel planes
				return Vec3();

			const auto wallIDsA = planeA.GetWalls();
			const auto wallIDsB = planeB.GetWalls();

			// Need to work out how to not leave early.
			// May need to move check earlier and create a separate function for parallel planes.
			// Needs to return line point as reconstruction function does not work for parallel planes.
			if (normalA.dot(-normalB) >= EPS_ANGULAR) // For now ignore oppositely parallel planes
			{
				for (auto wallID : wallIDsA)
				{
					auto it = mWalls.find(wallID);
					if (it == mWalls.end()) // case: wall does not exist
						continue;

					Vertices vertices = it->second.GetVertices();
					Vec3 lineDir = (vertices[0] - vertices[1]).Normalised();
					FindEdgesParallel(lineDir, vertices[0], planeA, planeB, edges);

					if (edges.size() > 0)
						return lineDir;

					lineDir = (vertices[1] - vertices[2]).Normalised();
					FindEdgesParallel(lineDir, vertices[1], planeA, planeB, edges);

					if (edges.size() > 0)
						return lineDir;

					lineDir = (vertices[2] - vertices[0]).Normalised();
					FindEdgesParallel(lineDir, vertices[2], planeA, planeB, edges);

					return lineDir;
				}
			}

			Vec3 lineDir = normalA.cross(normalB).Normalised();

			std::vector<EdgeSegment> segmentsA;
			if (wallIDsA.size() < wallIDsB.size()) // Search smaller set first
				FindSegments(lineDir, wallIDsA, planeB, segmentsA);
			else
				FindSegments(lineDir, wallIDsB, planeA, segmentsA);

			if (segmentsA.size() < 1) // No edge possible
				return lineDir;

			std::vector<EdgeSegment> segmentsB;
			if (wallIDsA.size() < wallIDsB.size())
				FindSegments(lineDir, wallIDsB, planeA, segmentsB);
			else
				FindSegments(lineDir, wallIDsA, planeB, segmentsB);

			if (segmentsB.size() < 1) // No edge possible
				return lineDir;

			edges = FindSegmentIntersections(segmentsA, segmentsB);
			return lineDir;
		}

		void Room::UpdateEdges()
		{
			std::lock_guard<std::mutex> lockWall(mWallMutex); // Must be locked first to avoid deadlock
			std::lock_guard<std::mutex> lockPlane(mPlaneMutex);
			RAC_DEBUG_LOG("Num planes: " + ToString(mPlanes.size()), DebugType::Parameter);

			for (auto& [planeIDA, planeA] : mPlanes)
			{
				auto wallIDsA = planeA.GetWalls();

				for (auto& [planeIDB, planeB] : mPlanes)
				{
					if (planeIDA >= planeIDB) // Avoid double checking and self-checking
						continue;

					std::vector<std::pair<Real, Real>> result;
					Vec3 lineDir = FindEdges(planeA, planeB, result);

					std::vector<size_t> edgeIDsA = planeA.GetEdges();
					std::vector<size_t> edgeIDsB = planeB.GetEdges();
					std::vector<size_t> edgeIDs;

					// Write common edge IDs from both planes to IDs. Used to check whether the number of located edge changes
					std::set_intersection(edgeIDsA.begin(), edgeIDsA.end(), edgeIDsB.begin(), edgeIDsB.end(), std::back_inserter(edgeIDs));

					int numEdgesFound = ToInt(result.size());
					if (numEdgesFound < 1) // No edges found
					{
						for (size_t edgeID : edgeIDs)
							RemoveEdge(edgeID, planeA, planeB);
						continue;
					}

					int numExistingEdges = ToInt(edgeIDs.size());

					for (int i = numEdgesFound; i < numExistingEdges; i++) // Remove any excess existing edges
						RemoveEdge(edgeIDs[i], planeA, planeB);

					Vec3 normalA = planeA.GetNormal();
					Vec3 normalB = planeB.GetNormal();
					Vec3 offset = normalA * planeA.GetD() + normalB * planeB.GetD();

					int numEdgesToUpdate = std::min(numEdgesFound, numExistingEdges);
					for (int i = 0; i < numEdgesToUpdate; i++) // Update existing edges
					{
						Vec3 base = ReconstructPoint(planeA, planeB, lineDir, result[i].first);
						Vec3 top = ReconstructPoint(planeA, planeB, lineDir, result[i].second);

						Edge edge(base, top, normalA, normalB, planeIDA, planeIDB);
						bool remove = UpdateEdge(edgeIDs[i], edge);
						if (remove)
							RemoveEdge(edgeIDs[i], planeA, planeB);
					}

					for (int i = numExistingEdges; i < numEdgesFound; i++) // Add new edges
					{
						Vec3 base = ReconstructPoint(planeA, planeB, lineDir, result[i].first);
						Vec3 top = ReconstructPoint(planeA, planeB, lineDir, result[i].second);

						Edge edge(base, top, normalA, normalB, planeIDA, planeIDB);
						AddEdge(edge, planeA, planeB);
					}
				}
			}

			//std::unordered_map<size_t, size_t> freeWalls;
			//std::lock_guard<std::mutex> lock(mWallMutex);
			//{
			//	std::vector<size_t> removeIDs;
			//	std::lock_guard<std::mutex> lock(mEdgeMutex);
			//	for (const auto& [edgeID, edge] : mEdges)		// Update existing edges
			//	{
			//		IDPair ids = edge.GetWallIDs();
			//		std::vector<Edge> edges;
			//		std::vector<size_t> IDs;
			//		FindEdges(ids.first, ids.second, edges, IDs);

			//		if (edges.size() == 1)
			//		{
			//			if (UpdateEdge(IDs[0], edges[0]))
			//				removeIDs.push_back(edgeID);
			//		}
			//		else if (edges.size() > 1)
			//		{
			//			std::vector toRemove = UpdateEdges(IDs, edges);
			//			removeIDs.insert(removeIDs.end(), toRemove.begin(), toRemove.end());
			//		}
			//		else
			//			removeIDs.push_back(edgeID);
			//	}
			//	for (size_t id : removeIDs)
			//		RemoveEdge(id);
			//}
			//for (const auto& [wallID, wall] : mWalls)	// Init edges for walls with no edges
			//{
			//	if (wall.EmptyEdges())
			//	{
			//		std::vector<size_t> edgeIDs = wall.GetEdges();
			//		std::vector<size_t> wallIDs;
			//		{
			//			std::lock_guard<std::mutex> lock(mEdgeMutex);
			//			for (const size_t edgeID : edgeIDs)
			//			{
			//				auto it = mEdges.find(edgeID);
			//				if (it != mEdges.end()) // case: edge does exist
			//					wallIDs.push_back(it->second.GetWallID(wallID));
			//			}
			//		}
			//		InitEdges(wallID, wallIDs);
			//	}
			//}
			RecordChange();
		}
#pragma optimize("", on)
		////////////////////////////////////////

		std::pair<Coefficients<>, Real> Room::CalculateAbsorptionSurfaceArea()
		{
			Coefficients<> absorption = Coefficients<>::Constant(numFrequencyBands, 1.0);
			Real surfaceArea = 0.0;
			MaterialMap absorptionFactors;

			{
				std::lock_guard<std::mutex> lock(mMaterialMutex);
				for (auto& [matID, material] : mMaterials)
					absorptionFactors[matID] = (material * material - 1.0);
			}
			{
				std::lock_guard<std::mutex> lock(mWallMutex);
				for (const auto& [wallID, wall] : mWalls)
				{
					int materialID = ToInt(wall.GetMaterialID());
					auto it = absorptionFactors.find(materialID);
					if (it == absorptionFactors.end()) continue;

					absorption -= it->second * wall.GetArea();
					surfaceArea += wall.GetArea();
				}
			}
			return { absorption, surfaceArea };
		}

		////////////////////////////////////////

		Coefficients<> Room::GetReverbTime()
		{
			std::lock_guard<std::mutex> lock(roomDataMutex);
			if (roomData.formula == ReverbFormula::Custom)
				return roomData.customT60;

			auto [absorption, surfaceArea] = CalculateAbsorptionSurfaceArea();
			switch (roomData.formula)
			{
			default:
			case ReverbFormula::Sabine:
				return Sabine(absorption);
			case ReverbFormula::Eyring:
				return Eyring(absorption, surfaceArea);
			case ReverbFormula::Custom:
				return roomData.customT60;
			}
		}

		////////////////////////////////////////

		Coefficients<> Room::Sabine(const Coefficients<>& absorption) const
		{
			Real factor = REAL_CONST(24.0) * std::log(REAL_CONST(10.0)) / SPEED_OF_SOUND;
			return factor * roomData.volume / absorption;
		}

		////////////////////////////////////////

		Coefficients<> Room::Eyring(const Coefficients<>& absorption, const Real& surfaceArea) const
		{
			Real factor = REAL_CONST(24.0) * std::log(REAL_CONST(10.0)) / SPEED_OF_SOUND;
			return -factor * roomData.volume / ((1 - absorption / surfaceArea).Log() * surfaceArea);
		}
	}
}