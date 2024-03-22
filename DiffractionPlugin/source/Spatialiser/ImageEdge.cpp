/*
*
*  \ImageEdge class
*
*/

// Spatialiser headers
#include "Spatialiser/ImageEdge.h"

// Unity headers
#include "Unity/UnityInterface.h"

namespace UIE
{
	namespace Spatialiser
	{

		ImageEdge::ImageEdge(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const size_t& numBands) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb), numAbsorptionBands(numBands)
		{
			reverbDirections = mReverb->GetReverbSourceDirections();
			reverbAbsorptions = std::vector<Absorption>(reverbDirections.size(), Absorption(numAbsorptionBands, 0.0));
		}

		void ImageEdge::RunIEM()
		{
			mPlanes = mRoom->GetPlanes();
			mWalls = mRoom->GetWalls();
			mEdges = mRoom->GetEdges();

			mSources.clear();
			mSourceManager->GetSourceData(mSources);
			{
				lock_guard<std::mutex> lock(mMutex);
				UpdateRValid();
				mListenerPosition = mListenerPositionStore;
				mISMConfig = mISMConfigStore;
			}

			for (SourceData& source : mSources)
			{
				source.vSources.clear();
				source.visible = ReflectPointInRoom(source.mPosition, source.vSources);

#ifdef DEBUG_ISM_THREAD
				Debug::Log("Source visible: " + BoolToStr(visible), Colour::White);
				Debug::Log("Source " + IntToStr((int)it->first) + " has " + IntToStr(vSources.size()) + " visible virtual sources", Colour::White);
#endif
				mSourceManager->UpdateSourceData(source);
			}
		}

		void ImageEdge::UpdateRValid()
		{
			// Determine if receiver is in front or behind plane face
			for (auto& it : mPlanes)
				it.second.SetRValid(it.second.ReflectPointInPlane(mListenerPosition));
		}

		bool ImageEdge::LineRoomIntersection(const vec3& start, const vec3& end)
		{
			return LineRoomIntersection(start, end, -1);
		}

		void ImageEdge::LineRoomIntersection(const vec3& start, const vec3& end, bool& obstruction)
		{
			LineRoomIntersection(start, end, -1, obstruction);
		}

		bool ImageEdge::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentPlaneId)
		{
			vec3 intersection;
			for (auto& itP : mPlanes)
			{
				if (itP.first != currentPlaneId)
				{
					if (itP.second.LinePlaneIntersection(intersection, start, end))
					{
						if (FindWallIntersection(intersection, start, end, itP.second))
							return true;
					}
				}
			}
			return false;
		}

		void ImageEdge::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentPlaneId, bool& obstruction)
		{
			if (obstruction)
				return;

			vec3 intersection;
			for (auto& itP : mPlanes)
			{
				if (itP.first != currentPlaneId)
				{
					if (itP.second.LinePlaneIntersection(intersection, start, end))
					{
						obstruction = FindWallIntersection(intersection, start, end, itP.second);
						if (obstruction)
							return;
					}
				}
			}
		}

		bool ImageEdge::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentPlaneId1, size_t currentPlaneId2)
		{
			vec3 intersection;
			for (auto& itP : mPlanes)
			{
				if (itP.first != currentPlaneId1 && itP.first != currentPlaneId2)
				{
					if (itP.second.LinePlaneIntersection(intersection, start, end))
					{
						if (FindWallIntersection(intersection, start, end, itP.second))
							return true;
					}
				}
			}
			return false;
		}

		bool ImageEdge::FindWallIntersection(const vec3& intersection, const vec3& start, const vec3& end, const Plane& plane) const
		{
			Absorption absorption;
			return FindWallIntersection(absorption, intersection, start, end, plane);
		}

		bool ImageEdge::FindWallIntersection(Absorption& absorption, const vec3& intersection, const vec3& start, const vec3& end, const Plane& plane) const
		{
			for (auto& idW : plane.GetWalls())
			{
				auto itW = mWalls.find(idW);
				if (itW != mWalls.end()) // case: wall exists
				{
					if (itW->second.LineWallIntersection(intersection, start, end))
					{
						absorption = itW->second.GetAbsorption();
						return true;
					}
				}
			}
			return false;
		}

		bool ImageEdge::FindIntersection(vec3& intersection, Absorption& absorption, const vec3& start, const vec3& end, const Plane& plane)
		{
			if (plane.LinePlaneIntersection(intersection, start, end))
			{
				if (FindWallIntersection(absorption, intersection, start, end, plane))
					return true;
			}
			else
				return false;
		}

		bool ImageEdge::FindIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx)
		{
			return FindIntersections(intersections, vSource, bounceIdx, mListenerPosition);
		}

		bool ImageEdge::FindIntersectionsSpEd(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx)
		{
			return FindIntersections(intersections, vSource, bounceIdx - 1, intersections[bounceIdx]);
		}

		bool ImageEdge::FindIntersectionsEdSp(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx)
		{
			return FindRIntersections(intersections, vSource, bounceIdx - 1, intersections[bounceIdx]);
		}

		bool ImageEdge::FindIntersectionsSpEdSp(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, int edgeIdx)
		{
			Plane plane; Absorption absorption = Absorption(numAbsorptionBands);
			vSource.ResetAbsorption();
			int idx1 = edgeIdx - 1;
			int idx2 = edgeIdx;
			bool valid = true;
			{
				while (valid && idx1 >= 0)
				{
					// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path
					auto it = mPlanes.find(vSource.GetID(idx1));
					if (it != mPlanes.end()) // case: wall exists
					{
						valid = FindIntersection(intersections[idx1], absorption, intersections[idx2], vSource.GetPosition(idx1), it->second);
						vSource.AddAbsorption(absorption);
						idx1--; idx2--;
					}
					else
						valid = false;
				}

				idx1 = edgeIdx + 1;
				idx2 = edgeIdx;
				int idx3 = bounceIdx - idx1;
				while (valid && idx1 <= bounceIdx)
				{
					// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path
					auto it = mPlanes.find(vSource.GetID(idx1));
					if (it != mPlanes.end()) // case: wall exists
					{
						valid = FindIntersection(intersections[idx1], absorption, intersections[idx2], vSource.GetRPosition(idx3), it->second);
						vSource.AddAbsorption(absorption);
						idx1++; idx2++; idx3--;
					}
					else
						valid = false;
				}
			}
			return valid;
		}

		bool ImageEdge::FindIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const vec3& start)
		{
			Absorption absorption = Absorption(numAbsorptionBands);
			vSource.ResetAbsorption();
			bool valid = true;
			auto it = mPlanes.find(vSource.GetID(bounceIdx));
			if (it != mPlanes.end()) // case: wall exists
				valid = FindIntersection(intersections[bounceIdx], absorption, start, vSource.GetPosition(bounceIdx), it->second);

			int idx1 = bounceIdx - 1;
			int idx2 = bounceIdx;
			while (valid && idx1 >= 0)
			{
				vSource.AddAbsorption(absorption);
				// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path

				it = mPlanes.find(vSource.GetID(idx1));
				if (it != mPlanes.end()) // case: wall exists
				{
					valid = FindIntersection(intersections[idx1], absorption, intersections[idx2], vSource.GetPosition(idx1), it->second);
					idx1--; idx2--;
				}
				else
					valid = false;
			}
			vSource.AddAbsorption(absorption);
			return valid;
		}

		bool ImageEdge::FindRIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const vec3& start)
		{
			Absorption absorption = Absorption(numAbsorptionBands);
			vSource.ResetAbsorption();
			bool valid = true;
			auto it = mPlanes.find(vSource.GetID(bounceIdx));
			if (it != mPlanes.end()) // case: wall exists
				valid = FindIntersection(intersections[bounceIdx], absorption, start, vSource.GetRPosition(bounceIdx), it->second);

			int idx1 = bounceIdx - 1;
			int idx2 = bounceIdx;
			while (valid && idx1 >= 0)
			{
				vSource.AddAbsorption(absorption);
				// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path
				it = mPlanes.find(vSource.GetID(idx1));
				if (it != mPlanes.end()) // case: wall exists
				{
					valid = FindIntersection(intersections[idx1], absorption, intersections[idx2], vSource.GetRPosition(idx1), it->second);
					idx1--; idx2--;
				}
				else
					valid = false;
			}
			vSource.AddAbsorption(absorption);
			return valid;
		}

		// Image Source Model


		void ImageEdge::UpdateLateReverbFilters()
		{
			Real k;
			vec3 normal, point, intersection;
			Plane plane;
			Absorption absorption = Absorption(numAbsorptionBands);
			for (int j = 0; j < reverbDirections.size(); j++)
			{
				std::vector<Plane> planes;
				std::vector<Real> ks;
				point = mListenerPosition + 100.0 * reverbDirections[j];
				for (auto& it : mPlanes)
				{
					plane = it.second;
					k = plane.PointPlanePosition(point);
					if (plane.GetRValid() && k < 0) // receiver in front of wall and point behind wall
					{
						auto iterK = ks.begin();
						auto iterP = planes.begin();
						bool inserted = false;
						while (!inserted && iterK != ks.end())
						{
							if (k > *iterK)
							{
								ks.insert(iterK, k);
								planes.insert(iterP, it.second);
								inserted = true;
							}
							iterK++;
							iterP++;
						}
						if (!inserted)
						{
							ks.push_back(k);
							planes.push_back(it.second);
						}
					}
				}
				bool valid = false;
				int i = 0;
				while (!valid && i < planes.size())
				{
					valid = FindIntersection(intersection, absorption, mListenerPosition, point, planes[i]);
					if (valid)
						reverbAbsorptions[j] = absorption;
					i++;
				}
				if (!valid)
					reverbAbsorptions[j] = Absorption(numAbsorptionBands, 0.0);
			}
			mReverb->UpdateReflectionFilters(reverbAbsorptions, mISMConfig.lateReverb);
		}

		bool ImageEdge::ReflectPointInRoom(const vec3& point, VirtualSourceDataMap& vSources)
		{

#ifdef PROFILE_BACKGROUND_THREAD
			BeginIEM();
#endif

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
#ifdef PROFILE_BACKGROUND_THREAD
			EndIEM();
#endif
			return lineOfSight;
		}

		// Diffraction
		void ImageEdge::HigherOrderSpecularDiffraction(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataStore& edSp, VirtualSourceDataStore& spEd, VirtualSourceDataMap& vSources)
		{
			// Check for reflections in sp
			if (sp.size() == 0)
				return;

			Plane plane; Edge edge; vec3 position, vPosition; Diffraction::Path path; size_t idP, idW, idE; bool valid;

			for (int j = 1; j < mISMConfig.order; j++) // only handle up to 1st order diffraction
			{
				int order = j + 1;
				int refIdx = j - 1;
				bool feedsFDN = mISMConfig.order == order;
				std::vector<vec3> intersections;
				size_t capacity = intersections.capacity();
				intersections.reserve(j);
				std::fill_n(std::back_inserter(intersections), order - capacity, vec3());

				spEd.push_back(std::vector<VirtualSourceData>());
				edSp.push_back(std::vector<VirtualSourceData>());
				size_t numReflectionPaths = sp[j - 1].size();
				// auto bIdx = sp.bucket(j);

				// Check bIdx is not null
				if (numReflectionPaths == 0)
					continue;

				// auto numReflectionPaths = sp.bucket_size(bIdx);

				// auto vs = sp.begin(bIdx);
				for (VirtualSourceData vSourceStore : sp[j - 1])
					// for (int i = 0; i < numReflectionPaths; i++, vs++)
				{
					//VirtualSourceData vSourceStore = vs->second;

					auto itP = mPlanes.find(vSourceStore.GetID());

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
												LineRoomIntersection(intersections[0], point, vSource.GetID(0), obstruction);

												int p = 0;
												while (!obstruction && p < j)
												{
													if (vSource.IsReflection(p) && vSource.IsReflection(p + 1))
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
													else
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p));
													p++;
												}
												if (!obstruction)
												{
													vSource.SetDistance(mListenerPosition);
													vSource.Visible(feedsFDN);
													vSources.insert_or_assign(vSource.GetKey(), vSource);
												}
											}
										}
									}

									spEd[refIdx].push_back(vSource);
									//spEd.emplace(j, vSource);
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
										auto temp = mPlanes.find(vSource.GetID(k));
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
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
													else
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p));
													p++;
												}
												if (!obstruction)
												{
													vSource.SetDistance(mListenerPosition);
													vSource.Visible(feedsFDN);
													vSources.insert_or_assign(vSource.GetKey(), vSource);
												}
											}
										}
									}
									edSp[refIdx].push_back(vSource);
									//edSp.emplace(j, vSource);
								}
							}
						}
					}
				}
				// spedsp paths
				for (int i = 1; i < j; i++)
				{
					size_t numSpEd = spEd[i - 1].size();
					if (numSpEd == 0)
						continue;

					for (VirtualSourceData start : spEd[i - 1])
					{
						if (start.GetSValid())
						{
							size_t numSpEd = edSp[j - i - 1].size();
							if (numSpEd == 0)
								continue;

							for (VirtualSourceData end : edSp[j - i - 1])
							{
								if (end.GetRValid())
								{
									if (start.GetID() == end.GetID(0))
									{
										VirtualSourceData vSource = start;
										vSource.Reset();

										vSource.SetRPositions(end.GetRPositions());
										for (int k = 0; k < j - i; k++)
											vSource.AddPlaneID(end.GetID(k));

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
													vSource.AddPlaneIDs(end.GetPlaneIDs());
													vSource.AddAbsorption(end.GetAbsorption());

													vPosition = mListenerPosition + (path.sData.d + path.rData.d) * UnitVector(end.GetTransformPosition() - mListenerPosition);
													vSource.UpdateTransform(vPosition);
													if (start.visible && end.visible)
													{
														vSource.SetDistance(mListenerPosition);
														vSource.Visible(feedsFDN);
														vSources.insert_or_assign(vSource.GetKey(), vSource);
													}

												}
												else
												{
													vPosition = path.CalculateVirtualPostion();
													for (int k = 0; k < end.GetOrder() - 1; k++)
													{
														auto temp = mPlanes.find(end.GetID(k));
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
														bool obstruction = LineRoomIntersection(point, intersections[0], vSource.GetID(0));
														LineRoomIntersection(mListenerPosition, intersections[j], vSource.GetID(j), obstruction);

														int p = 0;
														while (!obstruction && p < j)
														{
															if (vSource.IsReflection(p) && vSource.IsReflection(p + 1))
																obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
															else
																obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p));
															p++;
														}
														if (!obstruction)
														{
															vSource.SetDistance(mListenerPosition);
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

		void ImageEdge::FirstOrderDiffraction(const vec3& point, VirtualSourceDataStore& ed, VirtualSourceDataMap& vSources)
		{

#ifdef DEBUG_ISM_THREAD
			Debug::Log("Find 1st order diffraction", Colour::White);
#endif

			bool feedsFDN = mISMConfig.order == 1;
			Edge edge; vec3 position; Diffraction::Path path; size_t id;
			ed.push_back(std::vector<VirtualSourceData>());
			for (auto it : mEdges)
			{
				id = it.first;
				edge = it.second;

				path = Diffraction::Path(point, mListenerPosition, edge);
				position = path.CalculateVirtualPostion();

				VirtualSourceData vSource = VirtualSourceData(numAbsorptionBands);
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

						if (!obstruction) // Visible diffraction path
						{
							vSource.SetDistance(mListenerPosition);
							vSource.Visible(feedsFDN);
							vSources.insert_or_assign(vSource.GetKey(), vSource);
						}
					}
				}
				ed[0].push_back(vSource);
			}
		}

		// Reflection
		void ImageEdge::FirstOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
		{
			bool feedsFDN = mISMConfig.order == 1;
			Plane plane;  vec3 position, rPosition, intersection; Absorption absorption; size_t id; bool valid, rValid;
			sp.push_back(std::vector<VirtualSourceData>());
			for (auto it : mPlanes)
			{
				id = it.first;
				plane = it.second;

				VirtualSourceData vSource = VirtualSourceData(numAbsorptionBands);

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
						valid = FindIntersection(intersection, absorption, mListenerPosition, position, plane);

						if (valid)
						{
							vSource.AddAbsorption(absorption);
							bool obstruction = LineRoomIntersection(point, intersection, id);
							LineRoomIntersection(intersection, mListenerPosition, id, obstruction);
							if (!obstruction)
							{
								vSource.SetDistance(mListenerPosition);
								vSource.Visible(feedsFDN);
								vSources.insert_or_assign(vSource.GetKey(), vSource);
							}
						}
					}
				}
				sp[0].push_back(vSource);
			}
		}

		void ImageEdge::HigherOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
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
				sp.push_back(std::vector<VirtualSourceData>());
				size_t m = sp[j - 1].size();

				// Check n is not null
				if (m == 0)
					continue;

				for (auto it : mPlanes)
				{
					id = it.first;
					plane = it.second;

					for (VirtualSourceData vSource : sp[j - 1])
					{
						if (id != vSource.GetID(j - 1) && (vSource.valid || vSource.rValid))
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
							if (s)
							{
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
												LineRoomIntersection(intersections[0], point, vSource.GetID(0), obstruction);

												int p = 0;
												while (!obstruction && p < j)
												{
													obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
													p++;
												}
												if (!obstruction)
												{
													vSource.SetDistance(mListenerPosition);
													vSource.Visible(feedsFDN);
													vSources.insert_or_assign(vSource.GetKey(), vSource);
												}
											}
										}
									}
								}
							}
							if (s || r) // In theory this only needs to be if current source valid or rValid? - could then remove checks when finding next vSource
								sp[j].push_back(vSource);
						}
					}
				}
			}
		}
	}
}