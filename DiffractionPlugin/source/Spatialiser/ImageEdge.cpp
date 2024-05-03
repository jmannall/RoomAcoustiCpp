/*
*
*  \ImageEdge class
*
*/

// C++ headers
#include <algorithm>

// Spatialiser headers
#include "Spatialiser/ImageEdge.h"

// Unity headers
#include "Unity/UnityInterface.h"

namespace RAC
{
	namespace Spatialiser
	{

		ImageEdge::ImageEdge(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const size_t numBands) :
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
				mIEMConfig = mIEMConfigStore;
			}

			for (SourceData& source : mSources)
			{
				source.vSources.clear();
				source.visible = ReflectPointInRoom(source.mPosition, source.vSources);

#ifdef DEBUG_IEM_THREAD
				Debug::Log("Source visible: " + BoolToStr(visible), Colour::White);
				Debug::Log("Source " + std::to_string((int)it->first) + " has " + std::to_string(vSources.size()) + " visible virtual sources", Colour::White);
#endif
				mSourceManager->UpdateSourceData(source);
			}
		}

		void ImageEdge::UpdateRValid()
		{
			// Determine if receiver is in front or behind plane face
			for (auto& it : mPlanes)
				it.second.SetRValid(it.second.ReflectPointInPlane(mListenerPosition));

			// Determine where receiver lies around an edge
			for (auto& it : mEdges)
			{
				EdgeZone zone = it.second.FindEdgeZone(mListenerPosition);
				it.second.SetRZone(zone);
			}
		}

		bool ImageEdge::LineRoomObstruction(const vec3& start, const vec3& end)
		{
			return LineRoomObstruction(start, end, -1);
		}

		void ImageEdge::LineRoomObstruction(const vec3& start, const vec3& end, bool& obstruction)
		{
			LineRoomObstruction(start, end, -1, obstruction);
		}

		bool ImageEdge::LineRoomObstruction(const vec3& start, const vec3& end, size_t currentPlaneId)
		{
			vec3 intersection;
			for (auto& itP : mPlanes)
			{
				if (itP.first != currentPlaneId)
				{
					if (itP.second.LinePlaneObstruction(intersection, start, end))
					{
						if (FindWallIntersection(intersection, start, end, itP.second))
							return true;
					}
				}
			}
			return false;
		}

		void ImageEdge::LineRoomObstruction(const vec3& start, const vec3& end, size_t currentPlaneId, bool& obstruction)
		{
			if (obstruction)
				return;

			vec3 intersection;
			for (auto& itP : mPlanes)
			{
				if (itP.first != currentPlaneId)
				{
					if (itP.second.LinePlaneObstruction(intersection, start, end))
					{
						obstruction = FindWallIntersection(intersection, start, end, itP.second);
						if (obstruction)
							return;
					}
				}
			}
		}

		bool ImageEdge::LineRoomObstruction(const vec3& start, const vec3& end, size_t currentPlaneId1, size_t currentPlaneId2)
		{
			vec3 intersection;
			for (auto& itP : mPlanes)
			{
				if (itP.first != currentPlaneId1 && itP.first != currentPlaneId2)
				{
					if (itP.second.LinePlaneObstruction(intersection, start, end))
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
						if (start == intersection) // case: point on edge (consecutive inersections are identical)
							absorption *= 0.5;
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
			return false;
		}

		bool ImageEdge::FindIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx)
		{
			return FindIntersections(intersections, vSource, bounceIdx, mListenerPosition);
		}

		//bool ImageEdge::FindIntersectionsSpEd(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx)
		//{
		//	return FindIntersections(intersections, vSource, bounceIdx - 1, intersections[bounceIdx]);
		//}

		//bool ImageEdge::FindIntersectionsEdSp(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx)
		//{
		//	return FindRIntersections(intersections, vSource, bounceIdx - 1, intersections[bounceIdx]);
		//}

		//bool ImageEdge::FindIntersectionsSpEdSp(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, int edgeIdx)
		//{
		//	Plane plane; Absorption absorption = Absorption(numAbsorptionBands);
		//	vSource.ResetAbsorption();
		//	int idx1 = edgeIdx - 1;
		//	int idx2 = edgeIdx;
		//	bool valid = true;
		//	{
		//		while (valid && idx1 >= 0)
		//		{
		//			// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path
		//			auto it = mPlanes.find(vSource.GetID(idx1));
		//			if (it != mPlanes.end()) // case: wall exists
		//			{
		//				valid = FindIntersection(intersections[idx1], absorption, intersections[idx2], vSource.GetPosition(idx1), it->second);
		//				vSource.AddAbsorption(absorption);
		//				idx1--; idx2--;
		//			}
		//			else
		//				valid = false;
		//		}

		//		idx1 = edgeIdx + 1;
		//		idx2 = edgeIdx;
		//		int idx3 = bounceIdx - idx1;
		//		while (valid && idx1 <= bounceIdx)
		//		{
		//			// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path
		//			auto it = mPlanes.find(vSource.GetID(idx1));
		//			if (it != mPlanes.end()) // case: wall exists
		//			{
		//				valid = FindIntersection(intersections[idx1], absorption, intersections[idx2], vSource.GetRPosition(idx3), it->second);
		//				vSource.AddAbsorption(absorption);
		//				idx1++; idx2++; idx3--;
		//			}
		//			else
		//				valid = false;
		//		}
		//	}
		//	return valid;
		//}

		bool ImageEdge::FindIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const vec3& start)
		{
			Absorption absorption = Absorption(numAbsorptionBands);
			vSource.ResetAbsorption();
			bool valid = true;
			if (vSource.IsReflection(bounceIdx))
			{
				auto it = mPlanes.find(vSource.GetID(bounceIdx));
				if (it != mPlanes.end()) // case: wall exists
					valid = FindIntersection(intersections[bounceIdx], absorption, start, vSource.GetPosition(bounceIdx), it->second);
			}
			else
				intersections[bounceIdx] = vSource.GetApex();

			int idx1 = bounceIdx - 1;
			int idx2 = bounceIdx;
			while (valid && idx1 >= 0)
			{
				vSource.AddAbsorption(absorption);
				// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path

				if (vSource.IsReflection(idx1))
				{
					auto it = mPlanes.find(vSource.GetID(idx1));
					if (it != mPlanes.end()) // case: wall exists
					{
						valid = FindIntersection(intersections[idx1], absorption, intersections[idx2], vSource.GetPosition(idx1), it->second);
						idx1--; idx2--;
					}
					else
						valid = false;
				}
				else
				{
					auto it = mEdges.find(vSource.GetID(idx1));
					if (it != mEdges.end()) // case: edge exists
					{
						intersections[idx1] = it->second.GetEdgeCoord(vSource.mDiffractionPath.zA);
						idx1--; idx2--;
					}
				}
			}
			vSource.AddAbsorption(absorption);
			return valid;
		}

		//bool ImageEdge::FindRIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const vec3& start)
		//{
		//	Absorption absorption = Absorption(numAbsorptionBands);
		//	vSource.ResetAbsorption();
		//	bool valid = true;
		//	auto it = mPlanes.find(vSource.GetID(bounceIdx));
		//	if (it != mPlanes.end()) // case: wall exists
		//		valid = FindIntersection(intersections[bounceIdx], absorption, start, vSource.GetRPosition(bounceIdx), it->second);

		//	int idx1 = bounceIdx - 1;
		//	int idx2 = bounceIdx;
		//	while (valid && idx1 >= 0)
		//	{
		//		vSource.AddAbsorption(absorption);
		//		// (idx1) Current bounce (idx2) Previous bounce in code and next bounce in path
		//		it = mPlanes.find(vSource.GetID(idx1));
		//		if (it != mPlanes.end()) // case: wall exists
		//		{
		//			valid = FindIntersection(intersections[idx1], absorption, intersections[idx2], vSource.GetRPosition(idx1), it->second);
		//			idx1--; idx2--;
		//		}
		//		else
		//			valid = false;
		//	}
		//	vSource.AddAbsorption(absorption);
		//	return valid;
		//}

		// Image Source Model

		typedef std::pair<Real, int> mypair;
		bool comparator(const mypair& l, const mypair& r)
		{
			return l.first < r.first;
		}

		void ImageEdge::UpdateLateReverbFilters()
		{
			Real k;
			vec3 normal, point, intersection;
			Plane plane;
			Absorption absorption = Absorption(numAbsorptionBands);
			for (int j = 0; j < reverbDirections.size(); j++)
			{
				std::vector<mypair> ks = std::vector(mPlanes.size(), mypair(0.0, -1));
				point = mListenerPosition + 100.0 * reverbDirections[j];
				int i = 0;
				for (auto& it : mPlanes)
				{
					k = it.second.PointPlanePosition(point);
					if (it.second.GetRValid() && k < 0) // receiver in front of wall and point behind wall
						ks[i] = mypair(k, it.first); // A more negative k means the plane is closer to the receiver
					i++;
				}
				std::sort(ks.begin(), ks.end());
				bool valid = false;
				i = 0;
				while (!valid && i < ks.size())
				{
					if (ks[i].first < 0.0)
					{
						auto itP = mPlanes.find(ks[i].second);
						if (itP != mPlanes.end()) // case: plane exists
						{
							valid = FindIntersection(intersection, absorption, mListenerPosition, point, itP->second);
							if (valid)
								reverbAbsorptions[j] = absorption;
						}
					}
					i++;
				}
				if (!valid)
					reverbAbsorptions[j] = Absorption(numAbsorptionBands, 0.0);
			}
			mReverb->UpdateReflectionFilters(reverbAbsorptions, mIEMConfig.lateReverb);
		}

		bool ImageEdge::ReflectPointInRoom(const vec3& point, VirtualSourceDataMap& vSources)
		{

#ifdef PROFILE_BACKGROUND_THREAD
			BeginIEM();
#endif

			bool lineOfSight = false;

			// Direct sound
			switch (mIEMConfig.direct)
			{
				case DirectSound::doCheck:
					lineOfSight = !LineRoomObstruction(mListenerPosition, point);
					break;
				case DirectSound::alwaysTrue:
					lineOfSight = true;
					break;
				default:
					lineOfSight = false;
			}

			if (mIEMConfig.order < 1)
				return lineOfSight;

			VirtualSourceDataStore sp = std::vector<std::vector<VirtualSourceData>>();
			sp.push_back(std::vector<VirtualSourceData>());

			if ((mIEMConfig.diffraction != DiffractionSound::none || mIEMConfig.reflectionDiffraction != DiffractionSound::none) && mEdges.size() > 0)
				FirstOrderDiffraction(point, sp, vSources);

			// Reflections
			if ((mIEMConfig.reflection || mIEMConfig.reflectionDiffraction != DiffractionSound::none) && mWalls.size() > 0)
			{
				FirstOrderReflections(point, sp, vSources);

				if (mIEMConfig.order < 2)
					return lineOfSight;

				HigherOrderReflections(point, sp, vSources);

				/*if (mIEMConfig.reflectionDiffraction != DiffractionSound::none && mEdges.size() > 0)
					HigherOrderSpecularDiffraction(point, sp, vSources);*/
			}

#ifdef PROFILE_BACKGROUND_THREAD
			EndIEM();
#endif
			return lineOfSight;
		}

		// Diffraction
		//void ImageEdge::HigherOrderSpecularDiffraction(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
		//{
		//	// Check for reflections in sp
		//	if (sp.size() == 0)
		//		return;

		//	Plane plane; Edge edge; vec3 position, vPosition; Diffraction::Path path; size_t idP, idE; bool valid;

		//	VirtualSourceDataStore edSp;
		//	VirtualSourceDataStore spEd;
		//	//VirtualSourceDataStore edSp = std::vector<std::vector<VirtualSourceData>>(mIEMConfig.order - 1);
		//	//VirtualSourceDataStore spEd = std::vector<std::vector<VirtualSourceData>>(mIEMConfig.order - 1);

		//	for (int j = 1; j < mIEMConfig.order; j++) // only handle up to 1st order diffraction
		//	{
		//		int order = j + 1;
		//		int refIdx = j - 1;
		//		bool feedsFDN = mIEMConfig.order == order;
		//		std::vector<vec3> intersections;
		//		size_t capacity = intersections.capacity();
		//		intersections.reserve(j);
		//		std::fill_n(std::back_inserter(intersections), order - capacity, vec3());

		//		/*spEd[refIdx].reserve(sp[refIdx].size());
		//		edSp[refIdx].reserve(sp[refIdx].size());*/
		//		spEd.push_back(std::vector<VirtualSourceData>());
		//		edSp.push_back(std::vector<VirtualSourceData>());
		//		size_t numReflectionPaths = sp[j - 1].size();
		//		// auto bIdx = sp.bucket(j);	 

		//		// Check bIdx is not null
		//		if (numReflectionPaths == 0)
		//			continue;

		//		// auto numReflectionPaths = sp.bucket_size(bIdx);

		//		// auto vs = sp.begin(bIdx);
		//		for (VirtualSourceData vSourceStore : sp[j - 1])
		//			// for (int i = 0; i < numReflectionPaths; i++, vs++)
		//		{
		//			//VirtualSourceData vSourceStore = vs->second;

		//			auto itP = mPlanes.find(vSourceStore.GetID());

		//			idP = itP->first;
		//			plane = itP->second;

		//			for (auto itE : mEdges)
		//			{
		//				idE = itE.first;

		//				if (edge.AttachedToPlane(plane.GetWalls()))
		//					continue;

		//				valid = plane.ReflectEdgeInPlane(itE.second); // Check edge in front of plane

		//				if (!valid)
		//					continue;

		//				// sped
		//				SpecularDiffraction(vSourceStore, itE.second);
		//				//if (vSourceStore.valid) // valid source route
		//				//{
		//				//	VirtualSourceData vSource = vSourceStore;
		//				//	vSource.Reset();
		//				//	// Check for sp - ed
		//				//	position = vSource.GetPosition();
		//				//	path = Diffraction::Path(position, mListenerPosition, edge);

		//				//	if (path.valid)
		//				//	{
		//				//		vSource.Valid();

		//				//		if (path.inShadow || mIEMConfig.reflectionDiffraction == DiffractionSound::allZones)
		//				//		{
		//				//			vPosition = path.CalculateVirtualPostion();
		//				//			vSource.SetTransform(position, vPosition);

		//				//			intersections[j] = path.GetApex();
		//				//			valid = FindIntersectionsSpEd(intersections, vSource, j);
		//				//			vSource.AddEdgeID(idE, path);

		//				//			if (valid)
		//				//			{
		//				//				// Check for obstruction
		//				//				bool obstruction = LineRoomObstruction(mListenerPosition, intersections[j]);
		//				//				LineRoomObstruction(intersections[0], point, vSource.GetID(0), obstruction);

		//				//				int p = 0;
		//				//				while (!obstruction && p < j)
		//				//				{
		//				//					if (vSource.IsReflection(p) && vSource.IsReflection(p + 1))
		//				//						obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
		//				//					else
		//				//						obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p));
		//				//					p++;
		//				//				}
		//				//				if (!obstruction)
		//				//				{
		//				//					vSource.SetDistance(mListenerPosition);
		//				//					vSource.Visible(feedsFDN);
		//				//					vSources.insert_or_assign(vSource.GetKey(), vSource);
		//				//				}
		//				//			}
		//				//		}
		//				//	}

		//				//	spEd[refIdx].push_back(vSource);
		//				//	//spEd.emplace(j, vSource);
		//				//}

		//				// edsp
		//				if (vSourceStore.rValid) // valid receiver route
		//				{
		//					VirtualSourceData vSource = vSourceStore;
		//					vSource.Reset();
		//					vSource.FlipPath();

		//					// Check for ed - sp
		//					position = vSource.GetRPosition();
		//					path = Diffraction::Path(point, position, edge);

		//					vPosition = path.CalculateVirtualPostion();
		//					for (int k = 0; k < j; k++)
		//					{
		//						auto temp = mPlanes.find(vSource.GetID(k));
		//						if (temp != mPlanes.end())
		//						{
		//							temp->second.ReflectPointInPlaneNoCheck(vPosition); // Need to reflect regardless of whether vPosition behind wall...
		//						}
		//						else
		//						{
		//							// Wall does not exist
		//						}
		//					}
		//					vSource.SetRTransform(position, vPosition);
		//					if (path.valid) // Would be more effcient to save sValid and rValid for each edge
		//					{
		//						vSource.RValid();
		//						if (path.inShadow || mIEMConfig.reflectionDiffraction == DiffractionSound::allZones)
		//						{


		//							intersections[j] = path.GetApex();
		//							valid = FindIntersectionsEdSp(intersections, vSource, j);
		//							// Flip plane ids l to r?
		//							vSource.AddEdgeIDToStart(idE, path);

		//							if (valid)
		//							{
		//								// Check for obstruction
		//								bool obstruction = LineRoomObstruction(point, intersections[j]);
		//								LineRoomObstruction(intersections[0], mListenerPosition, idP, obstruction);

		//								int p = 0;
		//								while (!obstruction && p < j)
		//								{
		//									if (vSource.IsReflection(p) && vSource.IsReflection(p + 1))
		//										obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
		//									else
		//										obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p));
		//									p++;
		//								}
		//								if (!obstruction)
		//								{
		//									vSource.SetDistance(mListenerPosition);
		//									vSource.Visible(feedsFDN);
		//									vSources.insert_or_assign(vSource.GetKey(), vSource);
		//								}
		//							}
		//						}
		//					}
		//					edSp[refIdx].push_back(vSource);
		//					//edSp.emplace(j, vSource);
		//				}
		//			}
		//		}
		//		// spedsp paths
		//		for (int i = 1; i < j; i++)
		//		{
		//			size_t numSpEd = spEd[i - 1].size();
		//			if (numSpEd == 0)
		//				continue;

		//			for (VirtualSourceData start : spEd[i - 1])
		//			{
		//				if (start.GetSValid())
		//				{
		//					size_t numSpEd = edSp[j - i - 1].size();
		//					if (numSpEd == 0)
		//						continue;

		//					for (VirtualSourceData end : edSp[j - i - 1])
		//					{
		//						if (end.GetRValid())
		//						{
		//							if (start.GetID() == end.GetID(0))
		//							{
		//								VirtualSourceData vSource = start;
		//								vSource.Reset();

		//								vSource.SetRPositions(end.GetRPositions());
		//								for (int k = 0; k < j - i; k++)
		//									vSource.AddPlaneID(end.GetID(k));

		//								edge = vSource.GetEdge();
		//								path = Diffraction::Path(vSource.GetPosition(), end.GetRPosition(), edge);
		//								vSource.UpdateDiffractionPath(path);

		//								if (path.valid) // Should be valid as previous paths were valid (unless apex has moved)
		//								{
		//									vSource.Valid();
		//									vSource.RValid();

		//									if (path.inShadow || mIEMConfig.reflectionDiffraction == DiffractionSound::allZones)
		//									{
		//										if (path.GetApex() == start.GetApex() && path.GetApex() == end.GetApex())
		//										{
		//											vSource.AddPlaneIDs(end.GetPlaneIDs());
		//											vSource.AddAbsorption(end.GetAbsorption());

		//											vPosition = mListenerPosition + (path.sData.d + path.rData.d) * UnitVector(end.GetTransformPosition() - mListenerPosition);
		//											vSource.UpdateTransform(vPosition);
		//											if (start.visible && end.visible)
		//											{
		//												vSource.SetDistance(mListenerPosition);
		//												vSource.Visible(feedsFDN);
		//												vSources.insert_or_assign(vSource.GetKey(), vSource);
		//											}

		//										}
		//										else
		//										{
		//											vPosition = path.CalculateVirtualPostion();
		//											for (int k = 0; k < end.GetOrder() - 1; k++)
		//											{
		//												auto temp = mPlanes.find(end.GetID(k));
		//												if (temp != mPlanes.end())
		//												{
		//													temp->second.ReflectPointInPlaneNoCheck(vPosition); // Need to reflect regardless of whether vPosition behind wall...
		//												}
		//												else
		//												{
		//													// Plane does not exist
		//												}
		//											}
		//											vSource.UpdateTransform(vPosition);

		//											intersections[i] = path.GetApex();

		//											valid = FindIntersectionsSpEdSp(intersections, vSource, j, i);

		//											if (valid)
		//											{
		//												// Check for obstruction
		//												bool obstruction = LineRoomObstruction(point, intersections[0], vSource.GetID(0));
		//												LineRoomObstruction(mListenerPosition, intersections[j], vSource.GetID(j), obstruction);

		//												int p = 0;
		//												while (!obstruction && p < j)
		//												{
		//													if (vSource.IsReflection(p) && vSource.IsReflection(p + 1))
		//														obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
		//													else
		//														obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p));
		//													p++;
		//												}
		//												if (!obstruction)
		//												{
		//													vSource.SetDistance(mListenerPosition);
		//													vSource.Visible(feedsFDN);
		//													vSources.insert_or_assign(vSource.GetKey(), vSource);
		//												}
		//											}
		//										}
		//									}
		//								}
		//							}
		//						}
		//					}
		//				}
		//			}
		//		}
		//	}
		//}

		void ImageEdge::FirstOrderDiffraction(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
		{
			bool feedsFDN = mIEMConfig.order == 1;
			vec3 position;
			for (const auto& it : mEdges)
			{
				// Source checks
				EdgeZone zone = it.second.FindEdgeZone(point);

				if (zone == EdgeZone::Invalid)
					continue;

				if (mIEMConfig.diffraction == DiffractionSound::shadowZone && zone == EdgeZone::NonShadowed)
					continue;

				VirtualSourceData vSource = VirtualSourceData(numAbsorptionBands);

				vSource.Valid();
				vSource.AddEdgeID(it.first);
				vSource.UpdateDiffractionPath(point, mListenerPosition, it.second);

				if (mIEMConfig.diffraction == DiffractionSound::none)
				{
					sp[0].push_back(vSource);
					continue;
				}

				// Receiver checks
				if (it.second.GetRZone() == EdgeZone::Invalid)
				{
					sp[0].push_back(vSource);
					continue;
				}

				if (mIEMConfig.diffraction == DiffractionSound::shadowZone && it.second.GetRZone() == EdgeZone::NonShadowed)
				{
					sp[0].push_back(vSource);
					continue;
				}

				if (!vSource.mDiffractionPath.valid)
				{
					sp[0].push_back(vSource);
					continue;
				}

				if (!vSource.mDiffractionPath.inShadow && mIEMConfig.diffraction == DiffractionSound::shadowZone)
				{
					sp[0].push_back(vSource);
					continue;
				}

				bool obstruction = LineRoomObstruction(point, vSource.GetApex());
				LineRoomObstruction(vSource.GetApex(), mListenerPosition, obstruction);

				if (!obstruction)
				{
					vSource.SetDistance(mListenerPosition);
					vSource.Visible(feedsFDN);
					vSources.insert_or_assign(vSource.GetKey(), vSource);
				}

				sp[0].push_back(vSource);
			}
		}

		// Reflection
		void ImageEdge::FirstOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
		{
			bool feedsFDN = mIEMConfig.order == 1;
			vec3 position, intersection; Absorption absorption;
			for (const auto& it : mPlanes)
			{
				bool valid = it.second.ReflectPointInPlane(position, point);

				if (!valid)
					continue;

				VirtualSourceData vSource = VirtualSourceData(numAbsorptionBands);

				vSource.Valid();
				vSource.AddPlaneID(it.first);
				vSource.SetTransform(position);

				if (!mIEMConfig.reflection)
				{
					sp[0].push_back(vSource);
					continue;
				}

				if (!it.second.GetRValid())
				{
					sp[0].push_back(vSource);
					continue;
				}

				valid = FindIntersection(intersection, absorption, mListenerPosition, position, it.second);

				if (!valid)
				{
					sp[0].push_back(vSource);
					continue;
				}

				vSource.AddAbsorption(absorption);
				bool obstruction = LineRoomObstruction(point, intersection, it.first);
				LineRoomObstruction(intersection, mListenerPosition, it.first, obstruction);
				if (!obstruction)
				{
					vSource.SetDistance(mListenerPosition);
					vSource.Visible(feedsFDN);
					vSources.insert_or_assign(vSource.GetKey(), vSource);
				}

				sp[0].push_back(vSource);
			}
		}

		void ImageEdge::HigherOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
		{
			// Check for first order reflections in sp
			if (sp.size() == 0)
				return;

			vec3 position;
			for (int refIdx = 1; refIdx < mIEMConfig.order; refIdx++)
			{
				int refOrder = refIdx + 1;
				int prevRefIdx = refIdx - 1;
				bool feedsFDN = mIEMConfig.order == refOrder;
				std::vector<vec3> intersections;
				size_t capacity = intersections.capacity();
				intersections.reserve(refOrder);
				std::fill_n(std::back_inserter(intersections), refOrder - capacity, vec3());
				sp.push_back(std::vector<VirtualSourceData>());
				size_t m = sp[prevRefIdx].size();

				// Check m is not null
				if (m == 0)
					continue;

				for (const auto& it : mPlanes)
				{
					for (VirtualSourceData vSource : sp[prevRefIdx])
					{
						if (!vSource.diffraction)
						{
							// Can't reflect in same plane twice
							if (it.first == vSource.GetID(prevRefIdx))
								continue;

							vSource.Reset();

							bool valid = it.second.ReflectPointInPlane(position, vSource.GetPosition(prevRefIdx));

							if (!valid)
								continue;

							vSource.Valid();
							vSource.AddPlaneID(it.first);
							vSource.SetTransform(position);

							if (!mIEMConfig.reflection)
							{
								sp[refIdx].push_back(vSource);
								continue;
							}

							if (!it.second.GetRValid())
							{
								sp[refIdx].push_back(vSource);
								continue;
							}

							// Check valid intersections
							valid = FindIntersections(intersections, vSource, refIdx);

							if (!valid)
							{
								sp[refIdx].push_back(vSource);
								continue;
							}

							// Check for obstruction
							bool obstruction = LineRoomObstruction(intersections[refIdx], mListenerPosition, it.first);
							LineRoomObstruction(intersections[0], point, vSource.GetID(0), obstruction);

							int p = 0;
							while (!obstruction && p < refIdx)
							{
								obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
								p++;
							}
							if (!obstruction)
							{
								vSource.SetDistance(mListenerPosition);
								vSource.Visible(feedsFDN);
								vSources.insert_or_assign(vSource.GetKey(), vSource);
							}

							sp[refIdx].push_back(vSource);
						}
						else if (mIEMConfig.reflectionDiffraction != DiffractionSound::none)
						{
							Edge edge = vSource.GetEdge();

							if (vSource.IsReflection(prevRefIdx))
							{
								// Can't reflect in same plane twice
								if (it.first == vSource.GetID(prevRefIdx))
									continue;
							}
							else
							{
								// Can't reflect in plane attached to edge
								if (edge.AttachedToPlane(it.second.GetWalls()))
									continue;
							}

							vSource.Reset();

							bool valid = it.second.ReflectEdgeInPlane(edge); // Check edge in front of plane

							if (!valid)
								continue;

							vSource.Valid();
							vSource.AddPlaneID(it.first);

							position = vSource.GetPosition(prevRefIdx);
							it.second.ReflectPointInPlaneNoCheck(position);
							vSource.UpdateDiffractionPath(position, mListenerPosition, edge);

							if (!it.second.GetRValid())
							{
								sp[refIdx].push_back(vSource);
								continue;
							}

							EdgeZone zone = edge.FindEdgeZone(mListenerPosition);

							// Receiver checks
							if (zone == EdgeZone::Invalid)
							{
								sp[refIdx].push_back(vSource);
								continue;
							}

							if (mIEMConfig.reflectionDiffraction == DiffractionSound::shadowZone && zone == EdgeZone::NonShadowed)
							{
								sp[refIdx].push_back(vSource);
								continue;
							}

							if (!vSource.mDiffractionPath.valid)
							{
								sp[refIdx].push_back(vSource);
								continue;
							}

							if (!vSource.mDiffractionPath.inShadow && mIEMConfig.reflectionDiffraction == DiffractionSound::shadowZone)
							{
								sp[refIdx].push_back(vSource);
								continue;
							}

							// Check valid intersections
							valid = FindIntersections(intersections, vSource, refIdx);

							if (!valid)
							{
								sp[refIdx].push_back(vSource);
								continue;
							}

							// Check for obstruction
							bool obstruction = LineRoomObstruction(mListenerPosition, intersections[refIdx]);
							LineRoomObstruction(intersections[0], point, vSource.GetID(0), obstruction);

							int p = 0;
							while (!obstruction && p < refIdx)
							{
								if (vSource.IsReflection(p))
								{
									if (vSource.IsReflection(p + 1))
										obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
									else
										obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p));
								}
								else
									obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p + 1));
								p++;
							}
							if (!obstruction)
							{
								vSource.SetDistance(mListenerPosition);
								vSource.Visible(feedsFDN);
								vSources.insert_or_assign(vSource.GetKey(), vSource);
							}

							sp[refIdx].push_back(vSource);
						}
					}
				}

				if (mIEMConfig.reflectionDiffraction == DiffractionSound::none)
					continue;

				for (const auto& it : mEdges)
				{
					for (VirtualSourceData vSource : sp[prevRefIdx])
					{
						if (vSource.diffraction)
							continue;

						// Source checks
						EdgeZone zone = it.second.FindEdgeZone(vSource.GetPosition());

						if (zone == EdgeZone::Invalid)
							continue;

						if (mIEMConfig.reflectionDiffraction == DiffractionSound::shadowZone && zone == EdgeZone::NonShadowed)
							continue;

						vSource.Valid();
						vSource.AddEdgeID(it.first);
						vSource.UpdateDiffractionPath(vSource.GetPosition(), mListenerPosition, it.second);

						// Receiver checks
						if (it.second.GetRZone() == EdgeZone::Invalid)
						{
							sp[refIdx].push_back(vSource);
							continue;
						}

						if (mIEMConfig.reflectionDiffraction == DiffractionSound::shadowZone && it.second.GetRZone() == EdgeZone::NonShadowed)
						{
							sp[refIdx].push_back(vSource);
							continue;
						}

						if (!vSource.mDiffractionPath.valid)
						{
							sp[refIdx].push_back(vSource);
							continue;
						}

						if (!vSource.mDiffractionPath.inShadow && mIEMConfig.reflectionDiffraction == DiffractionSound::shadowZone)
						{
							sp[refIdx].push_back(vSource);
							continue;
						}

						// Check valid intersections
						bool valid = FindIntersections(intersections, vSource, refIdx);

						if (!valid)
						{
							sp[refIdx].push_back(vSource);
							continue;
						}

						// Check for obstruction
						bool obstruction = LineRoomObstruction(mListenerPosition, intersections[refIdx]);
						LineRoomObstruction(intersections[0], point, vSource.GetID(0), obstruction);

						int p = 0;
						while (!obstruction && p < refIdx)
						{
							if (vSource.IsReflection(p))
							{
								if (vSource.IsReflection(p + 1))
									obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
								else
									obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p));
							}
							else
								obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p + 1));
							p++;
						}
						if (!obstruction)
						{
							vSource.SetDistance(mListenerPosition);
							vSource.Visible(feedsFDN);
							vSources.insert_or_assign(vSource.GetKey(), vSource);
						}

						sp[refIdx].push_back(vSource);
					}
				}
			}
		}




		// Reflection
		//void ImageEdge::FirstOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
		//{
		//	bool feedsFDN = mIEMConfig.order == 1;
		//	Plane plane;  vec3 position, rPosition, intersection; Absorption absorption; size_t id; bool valid, rValid;
		//	sp.push_back(std::vector<VirtualSourceData>());
		//	for (auto it : mPlanes)
		//	{
		//		id = it.first;
		//		plane = it.second;

		//		VirtualSourceData vSource = VirtualSourceData(numAbsorptionBands);

		//		vSource.AddPlaneID(id);

		//		valid = plane.ReflectPointInPlane(position, point);
		//		rValid = plane.ReflectPointInPlane(rPosition, mListenerPosition);

		//		if (rValid)
		//		{
		//			vSource.RValid();
		//			vSource.SetRPosition(rPosition);
		//		}
		//		if (valid)
		//		{
		//			vSource.Valid();
		//			vSource.SetTransform(position);

		//			if (mIEMConfig.reflection)
		//			{
		//				valid = FindIntersection(intersection, absorption, mListenerPosition, position, plane);

		//				if (valid)
		//				{
		//					vSource.AddAbsorption(absorption);
		//					bool obstruction = LineRoomObstruction(point, intersection, id);
		//					LineRoomObstruction(intersection, mListenerPosition, id, obstruction);
		//					if (!obstruction)
		//					{
		//						vSource.SetDistance(mListenerPosition);
		//						vSource.Visible(feedsFDN);
		//						vSources.insert_or_assign(vSource.GetKey(), vSource);
		//					}
		//				}
		//			}
		//		}
		//		sp[0].push_back(vSource);
		//	}
		//}

		//void ImageEdge::HigherOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources)
		//{
		//	// Check for first order reflections in sp
		//	if (sp.size() == 0)
		//		return;

		//	Plane plane; Wall wall; vec3 position, rPosition; size_t id; bool r, s, valid, rValid;
		//	for (int j = 1; j < mIEMConfig.order; j++)
		//	{
		//		int refOrder = j + 1;
		//		bool feedsFDN = mIEMConfig.order == refOrder;
		//		std::vector<vec3> intersections;
		//		size_t capacity = intersections.capacity();
		//		intersections.reserve(refOrder);
		//		std::fill_n(std::back_inserter(intersections), refOrder - capacity, vec3());
		//		sp.push_back(std::vector<VirtualSourceData>());
		//		size_t m = sp[j - 1].size();

		//		// Check n is not null
		//		if (m == 0)
		//			continue;

		//		for (auto it : mPlanes)
		//		{
		//			id = it.first;
		//			plane = it.second;

		//			for (VirtualSourceData vSource : sp[j - 1])
		//			{
		//				if (id != vSource.GetID(j - 1) && (vSource.valid || vSource.rValid))
		//				{
		//					vSource.AddPlaneID(id);

		//					r = vSource.rValid;
		//					s = vSource.valid;
		//					vSource.Reset();
		//					if (r)
		//					{
		//						rValid = plane.ReflectPointInPlane(rPosition, vSource.GetRPosition(j - 1)); // Need to add rValid checks for ed - sp
		//						if (rValid)
		//						{
		//							vSource.RValid();
		//							vSource.SetRPosition(rPosition);
		//						}
		//					}
		//					if (s)
		//					{
		//						valid = plane.ReflectPointInPlane(position, vSource.GetPosition(j - 1));

		//						if (valid)
		//						{
		//							vSource.Valid();
		//							vSource.SetTransform(position);

		//							if (mIEMConfig.reflection)
		//							{
		//								if (plane.GetRValid()) // wall.GetRValid() returns if mListenerPosition in front of current wall
		//								{
		//									// Check valid intersections
		//									valid = FindIntersections(intersections, vSource, j);

		//									if (valid)
		//									{
		//										// Check for obstruction
		//										bool obstruction = LineRoomObstruction(intersections[j], mListenerPosition, id);
		//										LineRoomObstruction(intersections[0], point, vSource.GetID(0), obstruction);

		//										int p = 0;
		//										while (!obstruction && p < j)
		//										{
		//											obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
		//											p++;
		//										}
		//										if (!obstruction)
		//										{
		//											vSource.SetDistance(mListenerPosition);
		//											vSource.Visible(feedsFDN);
		//											vSources.insert_or_assign(vSource.GetKey(), vSource);
		//										}
		//									}
		//								}
		//							}
		//						}
		//					}
		//					if (s || r) // In theory this only needs to be if current source valid or rValid? - could then remove checks when finding next vSource
		//						sp[j].push_back(vSource);
		//				}
		//			}
		//		}
		//	}
		//}










	}
}