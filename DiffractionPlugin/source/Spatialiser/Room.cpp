/*
*
*  \Room class
*
*/

// Unity headers
#include "Unity/Debug.h"

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/Room.h"

namespace UIE
{
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// Room class ////////////////////

		void Room::UpdateListenerPosition(const vec3& position)
		{
			mListenerPosition = position;
			lock_guard <mutex> wLock(mWallMutex);
			// Determine if receiver is in front or behind plane face
			for (auto& it : mPlanes)
			{
				it.second.SetRValid(it.second.ReflectPointInPlane(mListenerPosition));
			}

			//for (auto it = mEdges.begin(); it != mEdges.end(); it++)
			//{
			//	it->second.SetRValid(mListenerPosition);
			//}
		}

		size_t Room::AddWall(Wall& wall)
		{
			size_t next;
			lock_guard <mutex> wLock(mWallMutex);
			if (!mEmptyWallsSlots.empty()) // Assign source to an existing ID
			{
				next = mEmptyWallsSlots.back();
				mEmptyWallsSlots.pop_back();
			}
			else // Create a new ID
			{
				next = nextWall;
				nextWall++;
			}

			for (auto& it : mPlanes)
			{
				if (it.second.IsCoplanar(wall))
				{
					it.second.AddWall(next);
					wall.SetPlaneID(it.first);
					mWalls.insert_or_assign(next, wall);
					InitEdges(next);
					return next;
				}
			}

			//bool match = false;
			//auto it = mPlanes.begin();
			//while (!match && it != mPlanes.end())
			//{
			//	if (it->second.IsCoplanar(wall))
			//	{
			//		it->second.AddWall(next);
			//		wall.SetPlaneID(it->first);
			//		match = true;
			//	}
			//	it++;
			//}

			if (!mEmptyPlanesSlots.empty()) // Assign source to an existing ID
			{
				size_t id = mEmptyPlanesSlots.back();
				mEmptyPlanesSlots.pop_back();
				mPlanes.insert_or_assign(id, Plane(next, wall));
				wall.SetPlaneID(id);
			}
			else // Create a new ID
			{
				mPlanes.insert_or_assign(nextPlane, Plane(next, wall));
				wall.SetPlaneID(nextPlane);
				nextPlane++;
			}

			mWalls.insert_or_assign(next, wall);
			InitEdges(next);
			return next;
		}

		// Edges
		void Room::InitEdges(const size_t& id)
		{
			auto itA = mWalls.find(id);
			if (itA == mWalls.end())
			{
				// Wall doesn't exist
			}
			else
			{
				for (auto itB = mWalls.begin(); itB != mWalls.end(); itB++)
				{
					if (itA->first != itB->first)
					{
						vec3 normalA = itA->second.GetNormal();
						vec3 normalB = itB->second.GetNormal();
						if (normalA != normalB)
						{
							if (normalA == -normalB)
								ParallelFindEdges(itA->second, itB->second, itA->first, itB->first);
							else
								FindEdges(itA->second, itB->second, itA->first, itB->first);
						}
					}
				}
			}
		}

		void Room::ParallelFindEdges(Wall& a, Wall& b, const size_t IDa, const size_t IDb)
		{
			std::vector<vec3> verticesA = a.GetVertices();

			// walls must be coplanar
			//if (b.PointWallPosition(verticesA[0]) == 0)
			if (a.GetD() == b.GetD())
			{
				std::vector<vec3> verticesB = b.GetVertices();

				int numA = static_cast<int>(verticesA.size());
				int numB = static_cast<int>(verticesB.size());

				bool evenNum = numA % 2 == 0;
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
						int idxA[2] = { i, (i + 1) % numA };
						bool validEdge = verticesA[idxA[1]] == verticesB[(j - 1) % numB];

						if (validEdge)
						{
							Edge edge = Edge(verticesA[idxA[0]], verticesA[idxA[1]], a.GetNormal(), b.GetNormal(), IDa, IDb);
							size_t id = AddEdge(edge);
							a.AddEdge(id);
							b.AddEdge(id);
						}

						if (i > 0 || evenNum)
						{
							idxA[1] = (i - 1) % numA;
							validEdge = verticesA[idxA[1]] == verticesB[(j + 1) % numB];

							if (validEdge)
							{
								Edge edge = Edge(verticesA[idxA[0]], verticesA[idxA[1]], a.GetNormal(), b.GetNormal(), IDa, IDb);
								size_t id = AddEdge(edge);
								a.AddEdge(id);
								b.AddEdge(id);
							}
						}
					}
				}
			}
		}

		// Coordinates defined using right hand rule.
		// Vertices are defined using a right hand curl around the direction of the normal
		// Edge face normals are defined using right hand curl rule around the direction of the edge (from base to top) that rotates from plane A to plane B through the exterior of the wedge.
		
		void Room::FindEdges(Wall& a, Wall& b, const size_t IDa, const size_t IDb)
		{
			std::vector<vec3> verticesA = a.GetVertices();
			std::vector<vec3> verticesB = b.GetVertices();

			size_t numA = verticesA.size();
			size_t numB = verticesB.size();

			for (int i = 0; i < numA; i += 2)
			{
				bool match = false;
				int j = 0;
				while (!match && j < numB)
				{
					if (verticesA[i].x == verticesB[j].x)
					{
						if (verticesA[i].y == verticesB[j].y)
						{
							if (verticesA[i].z == verticesB[j].z)
								match = true;
						}
					}

					j++;
				}
				if (match)
				{
					j--;
					int idxA = (i + 1) % numA;
					bool validEdge = verticesA[idxA] == verticesB[(j - 1) % numB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down

					if (!validEdge)
					{
						idxA = (i - 1) % numA;
						validEdge = verticesA[idxA] == verticesB[(j + 1) % numB];
					}
					if (validEdge) // Planes not twisted
					{
						int check = 0;
						while (check == i || check == idxA)
							check++;

						// K won't equal zero as then planes would be parallel
						Real k = b.PointWallPosition(verticesA[check]);	// Only valid for convex shapes
						if (k < 0) // Check angle greater than 180
						{
							// Cross(a.GetNormal(), b.GetNormal()) gives vector in direction of the edge
							// verticesA[idxA] - verticesA[i] give vector from base to top of edge
							bool reflexAngle = UnitVectorRound(Cross(a.GetNormal(), b.GetNormal())) == UnitVectorRound(verticesA[idxA] - verticesA[i]);

							if (reflexAngle) // Check returns correct angle type
							{
								Edge edge = Edge(verticesA[i], verticesA[idxA], a.GetNormal(), b.GetNormal(), IDa, IDb);
								size_t id = AddEdge(edge);
								a.AddEdge(id);
								b.AddEdge(id);
							}
							else
							{
								Edge edge = Edge(verticesA[i], verticesA[idxA], b.GetNormal(), a.GetNormal(), IDb, IDa);
								size_t id = AddEdge(edge);
								a.AddEdge(id);
								b.AddEdge(id);
							}
#ifdef DEBUG_INIT
	Debug::Log("Init Edge: " + RealToStr(edge.t), Colour::Green);
#endif
						}
					}
				}
			}
		}

		size_t Room::AddEdge(const Edge& edge)
		{
			if (!mEmptyEdgeSlots.empty()) // Assign source to an existing ID
			{
				size_t next = mEmptyEdgeSlots.back();
				mEmptyEdgeSlots.pop_back();
				mEdges.insert_or_assign(next, edge);
				return next;
			}
			else // Create a new ID
			{
				//  next = mEdges.size();
				size_t next = nextEdge;
				nextEdge++;
				mEdges.insert_or_assign(next, edge);
				return next;
			}
		}

		// Reverb
		FrequencyDependence Room::GetReverbTime(const Real& volume)
		{
			FrequencyDependence absorption = FrequencyDependence(0.0, 0.0, 0.0, 0.0, 0.0);
			Real surfaceArea = 0.0;

			{
				lock_guard <mutex> wLock(mWallMutex);
				Debug::Log("Number of walls: " + IntToStr(mWalls.size()), Colour::Orange);

				for (auto& it : mWalls)
				{
					absorption += (1.0 - it.second.GetAbsorption() * it.second.GetAbsorption()) * it.second.GetArea();
					surfaceArea += it.second.GetArea();
				}
			}
			Real temp[5];
			absorption.GetValues(&temp[0]);

			Real factor = 24.0 * log(10.0) / SPEED_OF_SOUND;
			// Sabine
			// FrequencyDependence sabine = factor * volume / absorption;

			// eyring
			FrequencyDependence eyring = factor * volume / (-surfaceArea * (1 - absorption / surfaceArea).Log());
			return eyring;
		}

		// Geometry checks
		//bool Room::LineRoomIntersectionDiff(const vec3& start, const vec3& end)
		//{
		//	bool obstruction = false;
		//	LineRoomIntersectionDiff(start, end, -1, obstruction);
		//	return obstruction;
		//}

		//bool Room::LineRoomIntersectionDiff(const vec3& start, const vec3& end, bool& obstruction)
		//{
		//	LineRoomIntersectionDiff(start, end, -1, obstruction);
		//	return obstruction;
		//}

		//void Room::LineRoomIntersectionDiff(const vec3& start, const vec3& end, size_t currentWallID, bool& obstruction)
		//{
		//	auto it = mWalls.begin();
		//	while (!obstruction && it != mWalls.end())
		//	{
		//		size_t id = it->first;
		//		Wall wall = it->second;
		//		if (id != currentWallID)
		//		{
		//			Real kS = wall.PointWallPosition(start);
		//			Real kE = wall.PointWallPosition(end);
		//			if (kS * kE < 0)	// point lies on plane when kS || kE == 0. Therefore not obstructed
		//			{
		//				obstruction = wall.LineWallIntersection(start, end);
		//			}
		//		}
		//		it++;
		//	}
		//}

		bool Room::LineRoomIntersection(const vec3& start, const vec3& end)
		{
			return LineRoomIntersection(start, end, -1);
		}

		void Room::LineRoomIntersection(const vec3& start, const vec3& end, bool& obstruction)
		{
			LineRoomIntersection(start, end, -1, obstruction);
		}

		bool Room::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentPlaneId)
		{
			Plane plane; Wall wall; Real kS, kE;  size_t id;
			//lock_guard <mutex> wLock(mWallMutex);
			for (auto& it : mPlanes)
			{
				id = it.first;
				plane = it.second;
				if (id != currentPlaneId)
				{
					kS = plane.PointPlanePosition(start);
					kE = plane.PointPlanePosition(end);

					if (kS * kE < 0)	// point lies on plane when kS || kE == 0. Therefore not obstructed
					{
						for (size_t& id : plane.GetWalls())
						{
							auto it = mWalls.find(id);
							if (it != mWalls.end()) // case: wall exists
							{
								if (it->second.LineWallIntersection(start, end))
									return true;
							}
							/*wall = mWalls.find(id)->second;
							if (wall.LineWallIntersection(start, end))
								return true;*/
						}
					}
				}
			}
			return false;
		}

		void Room::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentPlaneId, bool& obstruction)
		{
			if (obstruction)
				return;

			Plane plane; Wall wall; Real kS, kE; size_t id;
			//lock_guard <mutex> wLock(mWallMutex);
			for (auto& it : mPlanes)
			{
				id = it.first;
				plane = it.second;
				if (id != currentPlaneId)
				{
					kS = plane.PointPlanePosition(start);
					kE = plane.PointPlanePosition(end);

					if (kS * kE < 0)	// point lies on plane when kS || kE == 0. Therefore not obstructed
					{
						for (size_t& id : plane.GetWalls())
						{
							auto it = mWalls.find(id);
							if (it != mWalls.end()) // case: wall exists
							{
								obstruction = it->second.LineWallIntersection(start, end);
								if (obstruction)
									return;
							}
							/*wall = mWalls.find(id)->second;
							obstruction = wall.LineWallIntersection(start, end);
							if (obstruction)
								return;*/
						}
					}
				}
			}
		}

		bool Room::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentPlaneId1, size_t currentPlaneId2)
		{
			Plane plane; Wall wall; Real kS, kE; size_t id;
			//lock_guard <mutex> wLock(mWallMutex);
			for (auto& it : mPlanes)
			{
				id = it.first;
				plane = it.second;
				if (id != currentPlaneId1 && id != currentPlaneId2)
				{
					kS = plane.PointPlanePosition(start);
					kE = plane.PointPlanePosition(end);

					if (kS * kE < 0)	// point lies on plane when kS || kE == 0. Therefore not obstructed
					{
						for (size_t& id : plane.GetWalls())
						{
							auto it = mWalls.find(id);
							if (it != mWalls.end()) // case: wall exists
							{
								if (it->second.LineWallIntersection(start, end))
									return true;
							}
							/*wall = mWalls.find(id)->second;
							if (wall.LineWallIntersection(start, end))
								return true;*/
						}
					}
				}
			}
			return false;
		}

		bool Room::FindIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx)
		{
			return FindIntersections(intersections, vSource, bounceIdx, mListenerPosition);
		}

		bool Room::FindIntersectionsSpEd(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx)
		{
			return FindIntersections(intersections, vSource, bounceIdx - 1, intersections[bounceIdx]);
		}

		bool Room::FindIntersectionsEdSp(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx)
		{
			return FindRIntersections(intersections, vSource, bounceIdx - 1, intersections[bounceIdx]);
		}

		// CHECK THIS (Likely cause of crash)
		bool Room::FindIntersectionsSpEdSp(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, int edgeIdx)
		{
			Plane plane; Wall wall; size_t idW;
			vSource.RemoveWallIDs();
			int idx1 = edgeIdx - 1;
			int idx2 = edgeIdx;
			bool valid = true;
			{
				lock_guard <mutex> wLock(mWallMutex);
				while (valid && idx1 >= 0)
				{
					// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path
					auto it = mPlanes.find(vSource.GetPlaneID(idx1));
					if (it != mPlanes.end()) // case: wall exists
					{
						valid = FindIntersection(intersections[idx1], wall, idW, intersections[idx2], vSource.GetPosition(idx1), it->second);
						vSource.AddWallIDToStart(idW, wall.GetAbsorption());
						idx1--; idx2--;
					}
					else
						valid = false;

					/*plane = mPlanes.find(vSource.GetPlaneID(idx))->second;
					valid = FindIntersection(intersections[idx], wall, idW, intersections[idx + 1], vSource.GetPosition(idx), plane);
					vSource.AddWallIDToStart(idW, wall.GetAbsorption());
					n++;*/
				}

				idx1 = edgeIdx + 1;
				idx2 = edgeIdx;
				int idx3 = bounceIdx - idx1;
				while (valid && idx1 <= bounceIdx)
				{
					// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path
					auto it = mPlanes.find(vSource.GetPlaneID(idx2));
					if (it != mPlanes.end()) // case: wall exists
					{
						valid = FindIntersection(intersections[idx1], wall, idW, intersections[idx2], vSource.GetRPosition(idx3), it->second);
						vSource.AddWallID(idW, wall.GetAbsorption());
						idx1++; idx2++; idx3--;
					}
					else
						valid = false;
					
					/*plane = mPlanes.find(vSource.GetPlaneID(idx))->second;
					valid = FindIntersection(intersections[idx], wall, idW, intersections[idx - 1], vSource.GetRPosition(bounceIdx - idx), plane);
					vSource.AddWallID(idW, wall.GetAbsorption());
					n++;*/
				}
			}
			return valid;
		}

		bool Room::FindIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const vec3& start)
		{
			Wall wall; size_t idW;
			vSource.RemoveWallIDs();
			//lock_guard <mutex> wLock(mWallMutex);
			// Plane plane = mPlanes.find(vSource.GetPlaneID(bounceIdx))->second;
			bool valid = true;
			auto it = mPlanes.find(vSource.GetPlaneID(bounceIdx));
			if (it != mPlanes.end()) // case: wall exists
			{
				valid = FindIntersection(intersections[bounceIdx], wall, idW, start, vSource.GetPosition(bounceIdx), it->second);
			}
			//bool valid = FindIntersection(intersections[bounceIdx], wall, idW, start, vSource.GetPosition(bounceIdx), plane);

			int idx1 = bounceIdx - 1;
			int idx2 = bounceIdx;
			while (valid && idx1 >= 0)
			{
				vSource.AddWallIDToStart(idW, wall.GetAbsorption());
				// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path

				auto it = mPlanes.find(vSource.GetPlaneID(idx1));
				if (it != mPlanes.end()) // case: wall exists
				{
					valid = FindIntersection(intersections[idx1], wall, idW, intersections[idx2], vSource.GetPosition(idx1), it->second);
					idx1--; idx2--;
				}
				else
					valid = false;

				/*plane = mPlanes.find(vSource.GetPlaneID(idx))->second;
				valid = FindIntersection(intersections[idx], wall, idW, intersections[idx + 1], vSource.GetPosition(idx), plane);
				n++;*/
			}
			vSource.AddWallIDToStart(idW, wall.GetAbsorption());
			return valid;
		}

		bool Room::FindRIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const vec3& start)
		{
			Wall wall; size_t idW;
			vSource.RemoveWallIDs();
			// lock_guard <mutex> wLock(mWallMutex);
			// Plane plane = mPlanes.find(vSource.GetPlaneID(bounceIdx))->second;
			bool valid = true;
			auto it = mPlanes.find(vSource.GetPlaneID(bounceIdx));
			if (it != mPlanes.end()) // case: wall exists
				valid = FindIntersection(intersections[bounceIdx], wall, idW, start, vSource.GetRPosition(bounceIdx), it->second);

			//bool valid = FindIntersection(intersections[bounceIdx], wall, idW, start, vSource.GetRPosition(bounceIdx), plane);

			int idx1 = bounceIdx - 1;
			int idx2 = bounceIdx;
			while (valid && idx1 >= 0)
			{
				vSource.AddWallID(idW, wall.GetAbsorption());
				// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path
				auto it = mPlanes.find(vSource.GetPlaneID(idx1));
				if (it != mPlanes.end()) // case: wall exists
				{
					valid = FindIntersection(intersections[idx1], wall, idW, intersections[idx2], vSource.GetRPosition(idx1), it->second);
					idx1--; idx2--;
				}
				else
					valid = false;
				
				/*plane = mPlanes.find(vSource.GetPlaneID(idx))->second;
				valid = FindIntersection(intersections[idx], wall, idW, intersections[idx + 1], vSource.GetRPosition(idx), plane);
				n++;*/
			}
			vSource.AddWallID(idW, wall.GetAbsorption());
			return valid;
		}

		bool Room::FindIntersection(vec3& intersection, Wall& wall, size_t& idW, const vec3& start, const vec3& end, const Plane& plane)
		{
			for (size_t& id : plane.GetWalls())
			{
				auto it = mWalls.find(id);
				if (it != mWalls.end()) // case: wall exists
				{
					if (it->second.LineWallIntersection(intersection, start, end))
					{
						idW = id;
						return true;
					}
				}
				/*wall = mWalls.find(id)->second;
				if (wall.LineWallIntersection(intersection, start, end))
				{
					idW = id;
					return true;
				}*/
			}
			return false;
		}

		// Image Source Model
		void Room::UpdateISM()
		{
			size_t oldEndPtr = mSources.size();
			auto endPtr = oldEndPtr;
			for (auto it = mSources.begin(); it != mSources.end(); it++)
			{
				{
					lock_guard <mutex> lLock(mLowPrioMutex);
					unique_lock <mutex> nLock(mNextMutex);
					lock_guard <mutex> rLock(mRemoveMutex); // Prevents current source being removed
					nLock.unlock();
					endPtr = mSources.size();

					if (endPtr != oldEndPtr) // Check the number of sources hasn't changed during this iteration
					{
						return;
					}
					oldEndPtr = endPtr;

					VirtualSourceDataMap vSources;
					vec3 position(it->second.position);

					bool visible = ReflectPointInRoom(position, vSources);
					{
						lock_guard <mutex> iLock(it->second.mMutex);
						it->second.visible = visible;
						it->second.vSources = vSources;
					}		
#ifdef DEBUG_ISM_THREAD
	Debug::Log("Source visible: " + BoolToStr(visible), Colour::White);
	Debug::Log("Source " + IntToStr((int)it->first) + " has " + IntToStr(vSources.size()) + " visible virtual sources", Colour::White);
#endif
				}
			}
		}

		bool Room::ReflectPointInRoom(const vec3& point, VirtualSourceDataMap& vSources)
		{
			bool lineOfSight = false;

			// Direct sound
			if (mISMConfig.direct)
				lineOfSight = !LineRoomIntersection(point, mListenerPosition);

			if (mISMConfig.order < 1)
				return lineOfSight;

			VirtualSourceDataStore sp;
			VirtualSourceDataStore edSp;
			VirtualSourceDataStore spEd;
			VirtualSourceDataStore ed;

			if (mISMConfig.diffraction && mEdges.size() > 0)
				FirstOrderDiffraction(point, ed, vSources);

			// Reflections
			if ((mISMConfig.reflection || mISMConfig.reflectionDiffraction) && mWalls.size() > 0)
			{
				FirstOrderReflections(point, sp, vSources);

				if (mISMConfig.order < 2)
					return lineOfSight;

				HigherOrderReflections(point, sp, vSources);

				if (mISMConfig.reflectionDiffraction && mEdges.size() > 0)
					HigherOrderSpecularDiffraction(point, sp, edSp, spEd, vSources);
			}

#ifdef DEBUG_ISM_THREAD
	Debug::Log("Reflections: " + IntToStr((int)sp.size()), Colour::White);
	Debug::Log("Diffraction: " + IntToStr((int)ed.size()), Colour::White);
	Debug::Log("RefDiff: " + IntToStr((int)spEd.size()), Colour::White);
	Debug::Log("DiffRef: " + IntToStr((int)edSp.size()), Colour::White);
#endif

			return lineOfSight;
		}

		// Diffraction
		void Room::HigherOrderSpecularDiffraction(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataStore& edSp, VirtualSourceDataStore& spEd, VirtualSourceDataMap& vSources)
		{
			// Check for reflections in sp
			if (sp.size() == 0)
				return;

			Plane plane; Edge edge; vec3 position, vPosition; Diffraction::Path path; Absorption absorption; size_t idP, idW, idE; bool valid;

			for (int j = 1; j < mISMConfig.order; j++) // only handle up to 1st order diffraction
			{
				int order = j + 1;
				int refIdx = j - 1;
				bool feedsFDN = mISMConfig.order == order;
				std::vector<vec3> intersections;
				size_t capacity = intersections.capacity();
				intersections.reserve(j);
				std::fill_n(std::back_inserter(intersections), order - capacity, vec3());
				auto bIdx = sp.bucket(j);

				// Check bIdx is not null
				if (bIdx == NULL)
					continue;

				auto numReflectionPaths = sp.bucket_size(bIdx);

				auto vs = sp.begin(bIdx);
				for (int i = 0; i < numReflectionPaths; i++, vs++)
				{
					VirtualSourceData vSourceStore = vs->second;

					auto itP = mPlanes.find(vSourceStore.GetPlaneID());

					idP = itP->first;
					plane = itP->second;

					for (auto itE : mEdges)
					{
						idE = itE.first;
						edge = itE.second;

						if (!edge.AttachedToPlane(plane.GetWalls()))
						{
							valid = plane.ReflectEdgeInPlane(edge); // Check edge in front of plane

							if (valid)
							{
								// sped
								if (vSourceStore.valid) // valid source route
								{
									VirtualSourceData vSource = vSourceStore;
									vSource.Reset();
									// Check for sp - ed
									position = vSource.GetPosition();
									path = Diffraction::Path(position, mListenerPosition, edge);

									if (path.valid)
									{
										vSource.Valid();

										if (path.inShadow || mISMConfig.specularDiffraction)
										{
											vPosition = path.CalculateVirtualPostion();
											vSource.SetTransform(position, vPosition);

											intersections[j] = path.GetApex();
											valid = FindIntersectionsSpEd(intersections, vSource, j);
											vSource.AddEdgeID(idE, path);

											if (valid)
											{
												// Check for obstruction
												bool obstruction = LineRoomIntersection(mListenerPosition, intersections[j]);
												LineRoomIntersection(intersections[0], point, vSource.GetPlaneID(0), obstruction);

												int p = 0;
												while (!obstruction && p < j)
												{
													if (vSource.IsReflection(p) && vSource.IsReflection(p + 1))
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetPlaneID(p), vSource.GetPlaneID(p + 1));		
													else
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetPlaneID(p));
													p++;
												}
												if (!obstruction)
												{
													vSource.Visible(feedsFDN);
													vSources.insert_or_assign(vSource.GetKey(), vSource);
												}
											}
										}
									}

									spEd.emplace(j, vSource);
								}

								// edsp
								if (vSourceStore.rValid) // valid receiver route
								{
									VirtualSourceData vSource = vSourceStore;
									vSource.Reset();
									// Check for ed - sp
									position = vSource.GetRPosition();
									path = Diffraction::Path(point, position, edge);

									vPosition = path.CalculateVirtualPostion();
									for (int k = 0; k < j; k++)
									{
										auto temp = mPlanes.find(vSource.GetPlaneID(k));
										if (temp != mPlanes.end())
										{
											temp->second.ReflectPointInPlaneNoCheck(vPosition); // Need to reflect regardless of whether vPosition behind wall...
										}
										else
										{
											// Wall does not exist
										}
									}
									vSource.SetRTransform(position, vPosition);
									if (path.valid) // Would be more effcient to save sValid and rValid for each edge
									{
										vSource.RValid();
										if (path.inShadow || mISMConfig.specularDiffraction)
										{
									

											intersections[j] = path.GetApex();
											valid = FindIntersectionsEdSp(intersections, vSource, j);
											vSource.AddEdgeIDToStart(idE, path);

											if (valid)
											{
												// Check for obstruction
												bool obstruction = LineRoomIntersection(point, intersections[j]);
												LineRoomIntersection(intersections[0], mListenerPosition, idP, obstruction);

												int p = 0;
												while (!obstruction && p < j)
												{
													if (vSource.IsReflection(p) && vSource.IsReflection(p + 1))
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetPlaneID(p), vSource.GetPlaneID(p + 1));
													else
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetPlaneID(p));
													p++;
												}
												if (!obstruction)
												{
													vSource.Visible(feedsFDN);
													vSources.insert_or_assign(vSource.GetKey(), vSource);
												}
											}
										}
									}

									edSp.emplace(j, vSource);
								}
							}
						}
					}
				}
				// spedsp paths
				for (int i = 1; i < j; i++)
				{
					auto idxSpEd = spEd.bucket(i);
					if (idxSpEd == NULL)
						continue;

					auto vsSpEd = spEd.begin(idxSpEd);

					auto numSpEd = spEd.bucket_size(idxSpEd);

					for (int x = 0; x < numSpEd; x++, vsSpEd++)
					{
						VirtualSourceData start = vsSpEd->second;

						if (start.GetSValid())
						{
							auto idxEdSp = edSp.bucket(j - i);
							if (idxEdSp == NULL)
								continue;

							auto vsEdSp = edSp.begin(idxEdSp);
							auto numEdSp = edSp.bucket_size(idxEdSp);

							for (int y = 0; y < numEdSp; y++, vsEdSp++)
							{
								VirtualSourceData end = vsEdSp->second;

								if (end.GetRValid())
								{
									if (start.GetID() == end.GetID(0))
									{
										VirtualSourceData vSource = start;
										vSource.Reset();

										// bool recheckVisibility = vSource.AppendVSource(end, mListenerPosition);

										vSource.SetRPositions(end.GetRPositions());
										for (int k = 0; k < j - i; k++)
											vSource.AddPlaneID(end.GetPlaneID(k));

										edge = vSource.GetEdge();
										path = Diffraction::Path(vSource.GetPosition(), end.GetRPosition(), edge);
										vSource.UpdateDiffractionPath(path);

										if (path.valid) // Should be valid as previous paths were valid (unless apex has moved)
										{
											vSource.Valid();
											vSource.RValid();

											if (path.inShadow || mISMConfig.specularDiffraction)
											{
												if (path.GetApex() == start.GetApex() && path.GetApex() == end.GetApex())
												{
													end.GetAbsorption(absorption);
													vSource.AddWallIDs(end.GetWallIDs(), absorption);

													vPosition = mListenerPosition + (path.sData.d + path.rData.d) * UnitVector(end.GetTransformPosition() - mListenerPosition);
													vSource.UpdateTransform(vPosition);
													if (start.visible && end.visible)
													{
														vSource.Visible(feedsFDN);
														vSources.insert_or_assign(vSource.GetKey(), vSource);
													}

												}
												else
												{
													vPosition = path.CalculateVirtualPostion();
													for (int k = 0; k < end.GetOrder() - 1; k++)
													{
														auto temp = mPlanes.find(end.GetPlaneID(k));
														if (temp != mPlanes.end())
														{
															temp->second.ReflectPointInPlaneNoCheck(vPosition); // Need to reflect regardless of whether vPosition behind wall...
														}
														else
														{
															// Plane does not exist
														}
													}
													vSource.UpdateTransform(vPosition);

													intersections[i] = path.GetApex();

													valid = FindIntersectionsSpEdSp(intersections, vSource, j, i);

													if (valid)
													{
														// Check for obstruction
														bool obstruction = LineRoomIntersection(point, intersections[0], vSource.GetPlaneID(0));
														LineRoomIntersection(mListenerPosition, intersections[j], vSource.GetPlaneID(j), obstruction);

														int p = 0;
														while (!obstruction && p < j)
														{
															if (vSource.IsReflection(p) && vSource.IsReflection(p + 1))
																obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetPlaneID(p), vSource.GetPlaneID(p + 1));
															else
																obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetPlaneID(p));
															p++;
														}
														if (!obstruction)
														{
															vSource.Visible(feedsFDN);
															vSources.insert_or_assign(vSource.GetKey(), vSource);
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		void Room::FirstOrderDiffraction(const vec3& point, VirtualSourceDataStore& ed, VirtualSourceDataMap& vSources)
		{

#ifdef DEBUG_ISM_THREAD
	Debug::Log("Find 1st order diffraction", Colour::White);
#endif

			bool feedsFDN = mISMConfig.order == 1;
			Edge edge; vec3 position; Diffraction::Path path; size_t id;
			for (auto it : mEdges)
			{
				id = it.first;
				edge = it.second;

				path = Diffraction::Path(point, mListenerPosition, edge);
				position = path.CalculateVirtualPostion();

				VirtualSourceData vSource;
				vSource.AddEdgeID(id, path);
				vSource.SetTransform(point, position);

				if (path.valid)
				{
					// Valid diffraction path
					vSource.Valid();

					if (path.inShadow || mISMConfig.specularDiffraction)
					{
						bool obstruction = LineRoomIntersection(point, path.GetApex());
						LineRoomIntersection(path.GetApex(), mListenerPosition, obstruction);

						//bool obstruction = LineRoomIntersectionDiff(point, path.GetApex());
						//LineRoomIntersectionDiff(path.GetApex(), mListenerPosition, obstruction);

						if (!obstruction) // Visible diffraction path
						{
							vSource.Visible(feedsFDN);
							vSources.insert_or_assign(vSource.GetKey(), vSource);
						}
					}
				}
				ed.emplace(1, vSource);
			}
		}

		// Reflection
		void Room::FirstOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
		{
			bool feedsFDN = mISMConfig.order == 1;
			Plane plane;  vec3 position, rPosition, intersection; Wall wall; size_t id, idW; bool valid, rValid;
			for (auto it : mPlanes)
			{
				id = it.first;
				plane = it.second;

				VirtualSourceData vSource;

				vSource.AddPlaneID(id);

				valid = plane.ReflectPointInPlane(position, point);
				rValid = plane.ReflectPointInPlane(rPosition, mListenerPosition);

				if (rValid)
				{
					vSource.RValid();
					vSource.SetRPosition(rPosition);
				}
				if (valid)
				{
					vSource.Valid();
					vSource.SetTransform(position);

					if (mISMConfig.reflection)
					{
						valid = FindIntersection(intersection, wall, idW, mListenerPosition, position, plane);

						if (valid)
						{
							vSource.AddWallID(idW, wall.GetAbsorption());
							bool obstruction = LineRoomIntersection(point, intersection, id);
							LineRoomIntersection(intersection, mListenerPosition, id, obstruction);
							if (!obstruction)
							{
								vSource.Visible(feedsFDN);
								vSources.insert_or_assign(vSource.GetKey(), vSource);
							}
						}
					}
				}
				sp.emplace(1, vSource); // In theory only need to do this if either valid or rValid. Could then remove later checks for next vSource
			}
		}

		void Room::HigherOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
		{
			// Check for first order reflections in sp
			if (sp.size() == 0)
				return;

			Plane plane; Wall wall; vec3 position, rPosition; size_t id, idW; bool r, s, valid, rValid;
			for (int j = 1; j < mISMConfig.order; j++)
			{
				int refOrder = j + 1;
				bool feedsFDN = mISMConfig.order == refOrder;
				std::vector<vec3> intersections;
				size_t capacity = intersections.capacity();
				intersections.reserve(refOrder);
				std::fill_n(std::back_inserter(intersections), refOrder - capacity, vec3());
				auto n = sp.bucket(j);

				// Check n is not null
				if (n == NULL)
					continue;

				auto m = sp.bucket_size(n);

				for (auto it : mPlanes)
				{
					id = it.first;
					plane = it.second;

					n = sp.bucket(j);
					auto vs = sp.begin(n);
					for (int i = 0; i < m; i++, vs++)
					{
						VirtualSourceData vSource = vs->second;
						if (id != vSource.GetPlaneID(j - 1) && (vSource.valid || vSource.rValid))
						{
							vSource.AddPlaneID(id);

							r = vSource.rValid;
							s = vSource.valid;
							vSource.Reset();
							if (r)
							{
								rValid = plane.ReflectPointInPlane(rPosition, vSource.GetRPosition(j - 1)); // Need to add rValid checks for ed - sp
								if (rValid)
								{
									vSource.RValid();
									vSource.SetRPosition(rPosition);
								}
							}
							if (s) // Could add precomputed visibility between walls
							{
								// vSource.Reset(); // Resets validity and visibility

								valid = plane.ReflectPointInPlane(position, vSource.GetPosition(j - 1));

								if (valid)
								{
									vSource.Valid();
									vSource.SetTransform(position);

									if (mISMConfig.reflection)
									{
										if (plane.GetRValid()) // wall.GetRValid() returns if mListenerPosition in front of current wall
										{
											// Check valid intersections

											valid = FindIntersections(intersections, vSource, j);

											if (valid)
											{
												// Check for obstruction
												bool obstruction = LineRoomIntersection(intersections[j], mListenerPosition, id);
												LineRoomIntersection(intersections[0], point, vSource.GetPlaneID(0), obstruction);

												int p = 0;
												while (!obstruction && p < j)
												{
													obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetPlaneID(p), vSource.GetPlaneID(p + 1));
													p++;
												}
												if (!obstruction)
												{
													vSource.Visible(feedsFDN);
													vSources.insert_or_assign(vSource.GetKey(), vSource);
												}
											}
										}
									}
								}
							}
							if (s || r) // In theory this only needs to be if current source valid or rValid? - could then remove checks when finding next vSource
							{
								sp.emplace(refOrder, vSource); // Need rValid for edsp paths
							}
						}
					}
				}
			}
		}
	}
}