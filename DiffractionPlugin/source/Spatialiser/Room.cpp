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

			// Determine if receiver is in front or behind wall face
			for (auto it = mWalls.begin(); it != mWalls.end(); it++)
			{
				it->second.SetRValid(it->second.ReflectPointInWall(mListenerPosition));
			}
			//for (auto it = mEdges.begin(); it != mEdges.end(); it++)
			//{
			//	it->second.SetRValid(mListenerPosition);
			//}
		}

		size_t Room::AddWall(const Wall& wall)
		{
			lock_guard <mutex> wLock(mWallMutex);
			if (!mEmptyWallSlots.empty()) // Assign source to an existing ID
			{
				size_t next = mEmptyWallSlots.back();
				mEmptyWallSlots.pop_back();
				mWalls.insert_or_assign(next, wall);
				InitEdges(next);
				return next;
			}
			else // Create a new ID
			{
				// size_t next = mWalls.size();
				size_t next = nextWall;
				nextWall++;
				mWalls.insert_or_assign(next, wall);
				InitEdges(next);
				return next;
			}
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
			if (b.PointWallPosition(verticesA[0]) == 0)
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
#ifdef DEBUG_INIT
	Debug::Log("Init Edge", Colour::Green);
#endif

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

				for (auto wall : mWalls)
				{
					absorption += (1.0 - wall.second.GetAbsorption() * wall.second.GetAbsorption()) * wall.second.GetArea();
					surfaceArea += wall.second.GetArea();
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
		bool Room::LineRoomIntersectionDiff(const vec3& start, const vec3& end)
		{
			bool obstruction = false;
			LineRoomIntersectionDiff(start, end, -1, obstruction);
			return obstruction;
		}

		bool Room::LineRoomIntersectionDiff(const vec3& start, const vec3& end, bool& obstruction)
		{
			LineRoomIntersectionDiff(start, end, -1, obstruction);
			return obstruction;
		}

		void Room::LineRoomIntersectionDiff(const vec3& start, const vec3& end, size_t currentWallID, bool& obstruction)
		{
			auto it = mWalls.begin();
			while (!obstruction && it != mWalls.end())
			{
				size_t id = it->first;
				Wall wall = it->second;
				if (id != currentWallID)
				{
					Real kS = wall.PointWallPosition(start);
					Real kE = wall.PointWallPosition(end);
					if (kS * kE < 0)	// point lies on plane when kS || kE == 0. Therefore not obstructed
					{
						obstruction = wall.LineWallIntersection(start, end);
					}
				}
				it++;
			}
		}




		bool Room::LineRoomIntersection(const vec3& start, const vec3& end)
		{
			return LineRoomIntersection(start, end, -1);
		}

		void Room::LineRoomIntersection(const vec3& start, const vec3& end, bool& obstruction)
		{
			LineRoomIntersection(start, end, -1, obstruction);
		}

		bool Room::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentWallID)
		{
			bool obstruction = false;
			LineRoomIntersection(start, end, currentWallID, obstruction);
			return obstruction;
		}

		void Room::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentWallID, bool& obstruction)
		{
			auto it = mWalls.begin();
			while (!obstruction && it != mWalls.end())
			{
				size_t id = it->first;
				Wall wall = it->second;
				if (id != currentWallID)
				{
					Real kS = wall.PointWallPosition(start);
					Real kE = wall.PointWallPosition(end);
					if (kS * kE < 0)	// point lies on plane when kS || kE == 0. Therefore not obstructed
					{
						obstruction = wall.LineWallIntersection(start, end);
					}
				}
				it++;
			}
		}

		bool Room::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentWallID1, size_t currentWallID2)
		{
			bool obstruction = false;
			auto it = mWalls.begin();
			while (!obstruction && it != mWalls.end())
			{
				size_t id = it->first;
				Wall wall = it->second;
				if (id != currentWallID1 && id != currentWallID2)
				// if (true)
				{
					Real kS = wall.PointWallPosition(start);
					Real kE = wall.PointWallPosition(end);
					if (kS * kE < 0)
					{
						obstruction = wall.LineWallIntersection(start, end);
					}
				}
				it++;
			}
			return obstruction;
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
#if DEBUG_ISM_THREAD
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

#if DEBUG_ISM_THREAD
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

					auto itW = mWalls.find(vSourceStore.GetID());

					size_t idW = itW->first;
					Wall wall = itW->second;

					for (auto itE : mEdges)
					{
						size_t idE = itE.first;
						Edge edge = itE.second;

						if (!edge.AttachedToPlane(idW))
						{
							bool valid = wall.ReflectEdgeInWall(edge); // Check edge in front of wall

							if (valid)
							{
								// sped
								if (vSourceStore.valid) // valid source route
								{
									VirtualSourceData vSource = vSourceStore;
									vSource.Reset();
									// Check for sp - ed
									vec3 position = vSource.GetPosition();
									Diffraction::Path path = Diffraction::Path(position, mListenerPosition, edge);
									vSource.AddEdgeID(idE, path);

									if (path.valid)
									{
										vSource.Valid();

										if (path.inShadow || mISMConfig.specularDiffraction)
										{
											vec3 vPosition = path.CalculateVirtualPostion();
											vSource.SetTransform(position, vPosition);

											intersections[j] = path.GetApex();

											// Check valid intersections
											int n = 0;
											while (valid && n < j)
											{
												int idx = j - (n + 1); // Current bounce (j - n is previous bounce in code and next bounce in path)
												Wall previousWall = mWalls.find(vSource.GetID(idx))->second;
												valid = previousWall.LineWallIntersection(intersections[idx], intersections[idx + 1], vSource.GetPosition(idx));
												n++;
											}
											if (valid)
											{
												// Check for obstruction
												bool obstruction = LineRoomIntersection(mListenerPosition, intersections[j]);
												LineRoomIntersection(intersections[0], point, vSource.GetID(0), obstruction);

												int p = 0;
												while (!obstruction && p < j)
												{
													if (vSource.IsReflection(p))
													{
														if (vSource.IsReflection(p + 1))
															obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
														else
															obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p));
													}
													else
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p + 1));
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
									vec3 position = vSource.GetRPosition();
									Diffraction::Path path = Diffraction::Path(point, position, edge);
									vSource.AddEdgeIDToStart(idE, path);

									vec3 vPosition = path.CalculateVirtualPostion();
									for (int k = 1; k < order; k++)
									{
										auto temp = mWalls.find(vSource.GetID(k));
										if (temp != mWalls.end())
										{
											temp->second.ReflectPointInWallNoCheck(vPosition); // Need to reflect regardless of whether vPosition behind wall...
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

											int n = 0;
											valid = true; // In case changed in previous if statement
											while (valid && n < j)
											{
												int idx = j - n; // Current bounce (j - n is previous bounce in code and next bounce in path)
												Wall previousWall = mWalls.find(vSource.GetID(idx))->second;
												valid = previousWall.LineWallIntersection(intersections[idx - 1], intersections[idx], vSource.GetRPosition(idx));
												n++;
											}
											if (valid)
											{
												// Check for obstruction
												bool obstruction = LineRoomIntersection(point, intersections[j]);
												LineRoomIntersection(intersections[0], mListenerPosition, idW, obstruction);

												int p = 0;
												while (!obstruction && p < j)
												{
													if (vSource.IsReflection(p))
													{
														if (vSource.IsReflection(p + 1))
															obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
														else
															obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p));
													}
													else
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p + 1));
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

										Absorption absorption;
										end.GetAbsorption(absorption);
										vSource.AddWallIDs(end.GetWallIDs(), absorption);

										vSource.SetRPositions(end.GetRPositions());

										Edge edge = vSource.GetEdge();
										Diffraction::Path path = Diffraction::Path(vSource.GetPosition(), end.GetRPosition(), edge);
										vSource.UpdateDiffractionPath(path);

										if (path.valid) // Should be valid as previous paths were valid (unless apex has moved)
										{
											vSource.Valid();
											vSource.RValid();

											if (path.inShadow || mISMConfig.specularDiffraction)
											{
												if (path.GetApex() == start.GetApex() && path.GetApex() == end.GetApex())
												{
													vec3 vPosition = mListenerPosition + (path.sData.d + path.rData.d) * UnitVector(end.GetTransformPosition() - mListenerPosition);
													vSource.UpdateTransform(vPosition);
													if (start.visible && end.visible)
													{
														vSource.Visible(feedsFDN);
														vSources.insert_or_assign(vSource.GetKey(), vSource);
													}

												}
												else
												{
													vec3 vPosition = path.CalculateVirtualPostion();
													for (int k = 1; k < end.GetOrder(); k++)
													{
														auto temp = mWalls.find(end.GetID(k));
														if (temp != mWalls.end())
														{
															temp->second.ReflectPointInWallNoCheck(vPosition); // Need to reflect regardless of whether vPosition behind wall...
														}
														else
														{
															// Wall does not exist
														}
													}
													vSource.UpdateTransform(vPosition);

													intersections[i] = path.GetApex();

													int n = 0;
													bool valid = true;
													while (valid && n < i)
													{
														int idx = i - (n + 1); // Current bounce (i - n is previous bounce in code and next bounce in path)
														Wall previousWall = mWalls.find(vSource.GetID(idx))->second;
														valid = previousWall.LineWallIntersection(intersections[idx], intersections[i - n], vSource.GetPosition(idx));
														n++;
													}

													n = 0;
													while (valid && n < j - i)
													{
														int idx = i + n + 1; // Current bounce (j - n is previous bounce in code and next bounce in path)
														Wall previousWall = mWalls.find(vSource.GetID(idx))->second;
														valid = previousWall.LineWallIntersection(intersections[idx], intersections[i + n], vSource.GetRPosition(j - idx));
														n++;
													}

													if (valid)
													{
														// Check for obstruction
														bool obstruction = LineRoomIntersection(point, intersections[0], vSource.GetID(0));
														LineRoomIntersection(mListenerPosition, intersections[j], vSource.GetID(j), obstruction);

														int p = 0;
														while (!obstruction && p < j)
														{
															if (vSource.IsReflection(p))
															{
																if (vSource.IsReflection(p + 1))
																	obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
																else
																	obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p));
															}
															else // assume p + 1 is reflection
																obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p + 1));
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
#if DEBUG_ISM_THREAD
	Debug::Log("Find 1st order diffraction", Colour::White);
#endif

			bool feedsFDN = mISMConfig.order == 1;
			for (auto it : mEdges)
			{
				size_t id = it.first;
				Edge edge = it.second;

				Diffraction::Path path = Diffraction::Path(point, mListenerPosition, edge);
				vec3 position = path.CalculateVirtualPostion();

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
			for (auto it : mWalls)
			{
				size_t id = it.first;
				Wall wall = it.second;

				VirtualSourceData vSource;

				vSource.AddWallID(id, wall.GetAbsorption());

				vec3 position;
				bool valid = wall.ReflectPointInWall(position, point);

				vec3 rPosition;
				bool rValid = wall.ReflectPointInWall(rPosition, mListenerPosition);

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
						vec3 intersection;
						valid = wall.LineWallIntersection(intersection, mListenerPosition, position);

						if (valid)
						{
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

				for (auto it : mWalls)
				{
					size_t id = it.first;
					Wall wall = it.second;

					n = sp.bucket(j);
					auto vs = sp.begin(n);
					for (int i = 0; i < m; i++, vs++)
					{
						VirtualSourceData vSource = vs->second;
						if (id != vSource.GetID(j - 1) && (vSource.valid || vSource.rValid))
						{
							vSource.AddWallID(id, wall.GetAbsorption());

							bool r = vSource.rValid;
							bool s = vSource.valid;
							vSource.Reset();
							if (r)
							{
								vec3 rPosition;
								bool rValid = wall.ReflectPointInWall(rPosition, vSource.GetRPosition(j - 1)); // Need to add rValid checks for ed - sp
								if (rValid)
								{
									vSource.RValid();
									vSource.SetRPosition(rPosition);
								}
							}
							if (s) // Could add precomputed visibility between walls
							{
								// vSource.Reset(); // Resets validity and visibility

								vec3 position;
								bool valid = wall.ReflectPointInWall(position, vSource.GetPosition(j - 1));

								if (valid)
								{
									vSource.Valid();
									vSource.SetTransform(position);

									if (mISMConfig.reflection)
									{
										if (wall.GetRValid()) // wall.GetRValid() returns if mListenerPosition in front of current wall
										{
											// Check valid intersections
											valid = wall.LineWallIntersection(intersections[j], mListenerPosition, position);

											int n = 0;
											while (valid && n < j)
											{
												int idx = j - (n + 1); // Current bounce (j - n is previous bounce in code and next bounce in path)
												Wall previousWall = mWalls.find(vSource.GetID(idx))->second;
												valid = previousWall.LineWallIntersection(intersections[idx], intersections[j - n], vSource.GetPosition(idx));
												n++;
											}
											if (valid)
											{
												// Check for obstruction
												bool obstruction = LineRoomIntersection(intersections[j], mListenerPosition, id);
												LineRoomIntersection(intersections[0], point, vSource.GetID(0), obstruction);

												int p = 0;
												while (!obstruction && p < j)
												{
													obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
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