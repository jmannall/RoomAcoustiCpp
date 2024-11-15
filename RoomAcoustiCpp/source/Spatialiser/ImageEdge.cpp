/*
*
*  \ImageEdge class
*
*/

// C++ headers
#include <algorithm>

// Common headers
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/ImageEdge.h"

// Unity headers
#include "Unity/UnityInterface.h"

namespace RAC
{
	namespace Spatialiser
	{

		ImageEdge::ImageEdge(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const int numBands) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb), numAbsorptionBands(numBands), currentCycle(false), configChanged(true)
		{
			sp = std::vector<std::vector<VirtualSourceData>>();
			sp.push_back(std::vector<VirtualSourceData>());

			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			reverbDirections = sharedReverb->GetReverbSourceDirections();
			reverbAbsorptions = std::vector<Absorption>(static_cast<int>(reverbDirections.size()), Absorption(numAbsorptionBands));
		}

		bool ImageEdge::RunIEM()
		{
			bool doIEM = false;

			shared_ptr<Room> sharedRoom = mRoom.lock();
			if (sharedRoom->HasChanged())
			{
				// Recopy room data (planes, walls, edges
				mPlanes = sharedRoom->GetPlanes();
				mWalls = sharedRoom->GetWalls();
				mEdges = sharedRoom->GetEdges();
				doIEM = true;
			}

			shared_ptr<SourceManager> sharedSource = mSourceManager.lock();
			mSources = sharedSource->GetSourceData();

#ifdef PROFILE_BACKGROUND_THREAD
			BeginIEM();
#endif
			{
				lock_guard<std::mutex> lock(mMutex);
				if (mListenerPosition != mListenerPositionStore)
				{
					mListenerPosition = mListenerPositionStore;
					doIEM = true;
				}
				if (configChanged)
				{
					mIEMConfig = mIEMConfigStore;
					configChanged = false;
					doIEM = true;
				}
				UpdateRValid();
			}
#ifdef PROFILE_BACKGROUND_THREAD
			EndIEM();
#endif

			if (mVSources.size() != mSources.size())
			{
				mVSources.resize(mSources.size());
				mSourceAudioDatas.resize(mSources.size());
				mCurrentCycles.resize(mSources.size());
				doIEM = true;
			}

			int i = 0;
			for (SourceData& source : mSources)
			{
				/*if (doIEM || source.hasChanged)
				{
					mCurrentCycles[i] = !mCurrentCycles[i];
					currentCycle = mCurrentCycles[i];
					mSourceAudioDatas[i] = ReflectPointInRoom(source, mVSources[i]);
				}*/

				mCurrentCycles[i] = !mCurrentCycles[i];
				currentCycle = mCurrentCycles[i];
				mSourceAudioDatas[i] = ReflectPointInRoom(source, mVSources[i]);

#ifdef DEBUG_IEM_THREAD
				Debug::Log("Source visible: " + BoolToStr(visible), Colour::White);
				Debug::Log("Source " + std::to_string((int)it->first) + " has " + std::to_string(vSources.size()) + " visible virtual sources", Colour::White);
#endif
#ifdef PROFILE_BACKGROUND_THREAD
				BeginUpdateAudioData();
#endif
				sharedSource->UpdateSourceData(source.id, mSourceAudioDatas[i], mVSources[i]);
#ifdef PROFILE_BACKGROUND_THREAD
				EndUpdateAudioData();
#endif
				++i;
			}			
			return doIEM;
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

		bool ImageEdge::LineRoomObstruction(const Vec3& start, const Vec3& end) const
		{
			return LineRoomObstruction(start, end, -1);
		}

		void ImageEdge::LineRoomObstruction(const Vec3& start, const Vec3& end, bool& obstruction) const
		{
			obstruction = LineRoomObstruction(start, end, -1);
		}

		bool ImageEdge::LineRoomObstruction(const Vec3& start, const Vec3& end, size_t currentPlaneId) const
		{
			for (auto& itP : mPlanes)
			{
				if (itP.first != currentPlaneId)
				{
					if (itP.second.LinePlaneObstruction(start, end))
					{
						if (FindWallObstruction(start, end, itP.second))
							return true;
					}
				}
			}
			return false;
		}

		void ImageEdge::LineRoomObstruction(const Vec3& start, const Vec3& end, size_t currentPlaneId, bool& obstruction) const
		{
			if (!obstruction)
				obstruction = LineRoomObstruction(start, end, currentPlaneId);
		}

		bool ImageEdge::LineRoomObstruction(const Vec3& start, const Vec3& end, size_t currentPlaneId1, size_t currentPlaneId2) const
		{
			for (auto& itP : mPlanes)
			{
				if (itP.first != currentPlaneId1 && itP.first != currentPlaneId2)
				{
					if (itP.second.LinePlaneObstruction(start, end))
					{
						if (FindWallObstruction(start, end, itP.second))
							return true;
					}
				}
			}
			return false;
		}

		void ImageEdge::LineRoomObstruction(const Vec3& start, const Vec3& end, size_t currentPlaneId1, size_t currentPlaneId2, bool& obstruction) const
		{
			if (!obstruction)
				obstruction = LineRoomObstruction(start, end, currentPlaneId1, currentPlaneId2);
		}

		bool ImageEdge::FindWallObstruction(const Vec3& start, const Vec3& end, const Plane& plane) const
		{
			for (auto& idW : plane.GetWalls())
			{
				auto itW = mWalls.find(idW);
				if (itW != mWalls.end()) // case: wall exists
				{
					if (itW->second.LineWallObstruction(start, end))
						return true;
				}
			}
			return false;
		}

		bool ImageEdge::FindWallIntersection(Absorption& absorption, Vec3& intersection, const Vec3& start, const Vec3& end, const Plane& plane) const
		{
			for (auto& idW : plane.GetWalls())
			{
				auto itW = mWalls.find(idW);
				if (itW != mWalls.end()) // case: wall exists
				{
					if (itW->second.LineWallIntersection(intersection, start, end))
					{
						absorption *= itW->second.GetAbsorption();
						if (start != intersection) // case: point on edge (consecutive intersections are identical)
							return true;

						if (itW->second.VertexMatch(intersection))  // case: point on corner (will be triggered twice)
							absorption *= INV_SQRT_3;
						else
							absorption *= 0.5;					
						return true;
					}
				}
			}
			return false;
		}

		bool ImageEdge::FindIntersection(Vec3& intersection, Absorption& absorption, const Vec3& start, const Vec3& end, const Plane& plane) const
		{
			if (plane.LinePlaneIntersection(start, end))
			{
				if (FindWallIntersection(absorption, intersection, start, end, plane))
					return true;
			}
			return false;
		}

		bool ImageEdge::FindIntersections(std::vector<Vec3>& intersections, VirtualSourceData& vSource, int bounceIdx) const
		{
			return FindIntersections(intersections, vSource, bounceIdx, mListenerPosition);
		}

		bool ImageEdge::FindIntersections(std::vector<Vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const Vec3& start) const
		{
			vSource.ResetAbsorption();
			Absorption& absorption = vSource.GetAbsorptionRef();
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
						return false;
				}
				else
				{
					auto it = mEdges.find(vSource.GetID(idx1));
					if (it != mEdges.end()) // case: edge exists
					{
						intersections[idx1] = it->second.GetEdgeCoord(vSource.mDiffractionPath.zA);
						idx1--; idx2--;
					}
					else
						return false;
				}
			}

			if (!valid)
				return false;

			if (intersections.size() < 4)
				return valid;

			int count = 0;
			for (int i = 0; i < intersections.size() - 1; i++)
			{
				if (intersections[i] == intersections[i + 1])
					count++;
				else
					count = 0;

				if (count > 2)
					return false;
			}

			return valid;
		}

		// Image Source Model

		typedef std::pair<Real, size_t> PlaneDistanceID;
		bool comparator(const PlaneDistanceID& l, const PlaneDistanceID& r)
		{
			return l.first < r.first;
		}

		void ImageEdge::UpdateLateReverbFilters()
		{
#ifdef PROFILE_BACKGROUND_THREAD
			BeginLateReverb();
#endif
			Real k;
			Vec3 point, intersection;
			for (int j = 0; j < reverbDirections.size(); j++)
			{
				std::vector<PlaneDistanceID> ks = std::vector(mPlanes.size(), PlaneDistanceID(0.0, -1));
				point = mListenerPosition + reverbDirections[j];
				int i = 0;
				for (auto& it : mPlanes)
				{
					k = it.second.PointPlanePosition(point);
					if (it.second.GetRValid() && k < 0) // receiver in front of wall and point behind wall
						ks[i] = PlaneDistanceID(k, it.first); // A more negative k means the plane is closer to the receiver
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
							reverbAbsorptions[j] = 1.0;
							valid = FindIntersection(intersection, reverbAbsorptions[j], mListenerPosition, point, itP->second);
						}
					}
					i++;
				}
				if (!valid)
					reverbAbsorptions[j] = 0.0;
			}

#ifdef PROFILE_BACKGROUND_THREAD
			EndLateReverb();
#endif
#ifdef PROFILE_BACKGROUND_THREAD
			BeginUpdateAudioData();
#endif
			std::shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->UpdateReflectionFilters(reverbAbsorptions, mIEMConfig.lateReverb);
#ifdef PROFILE_BACKGROUND_THREAD
			EndUpdateAudioData();
#endif
		}

		SourceAudioData ImageEdge::ReflectPointInRoom(const SourceData& source, VirtualSourceDataMap& vSources)
		{

#ifdef PROFILE_BACKGROUND_THREAD
			BeginIEM();
			BeginDirect();
#endif

			SourceAudioData direct = { 0.0, false };
			bool lineOfSight = false;
			// Direct sound
			switch (mIEMConfig.direct)
			{
				case DirectSound::doCheck:
					lineOfSight = !LineRoomObstruction(mListenerPosition, source.position);
					break;
				case DirectSound::alwaysTrue:
					lineOfSight = true;
					break;
				default:
					lineOfSight = false;
			}

			if (lineOfSight)
				direct.first = CalculateDirectivity(source, mListenerPosition);

#ifdef PROFILE_BACKGROUND_THREAD
			EndDirect();
#endif

			if (mIEMConfig.order < 1 || (!mIEMConfig.reflections && mIEMConfig.diffraction == DiffractionSound::none && mIEMConfig.diffractedReflections == DiffractionSound::none))
			{
				direct.second = mIEMConfig.lateReverb;
				EraseOldEntries(vSources);
#ifdef PROFILE_BACKGROUND_THREAD
				EndIEM();
#endif
				return direct;
			}

			if (sp.size() != mIEMConfig.order)
				sp.resize(mIEMConfig.order, std::vector<VirtualSourceData>());

			size_t counter = 0;

			if ((mIEMConfig.diffraction != DiffractionSound::none || mIEMConfig.diffractedReflections != DiffractionSound::none) && mEdges.size() > 0)
				counter = FirstOrderDiffraction(source, vSources);

			// Reflections
			if ((mIEMConfig.reflections || mIEMConfig.diffractedReflections != DiffractionSound::none) && mWalls.size() > 0)
			{
				counter = FirstOrderReflections(source, vSources, counter);

				sp[0].resize(counter, VirtualSourceData(numAbsorptionBands));

				if (mIEMConfig.order < 2)
				{
					EraseOldEntries(vSources);
#ifdef PROFILE_BACKGROUND_THREAD
					EndIEM();
#endif
					return direct;
				}

				HigherOrderReflections(source, vSources);
			}
			else
				sp[0].resize(counter, VirtualSourceData(numAbsorptionBands));

			EraseOldEntries(vSources);
#ifdef PROFILE_BACKGROUND_THREAD
			EndIEM();
#endif
			return direct;
		}

		size_t ImageEdge::FirstOrderDiffraction(const SourceData& source, VirtualSourceDataMap& vSources)
		{
#ifdef PROFILE_BACKGROUND_THREAD
			BeginFirstOrderDiff();
#endif
			size_t size = sp[0].size();
			size_t counter = 0;

			bool feedsFDN = mIEMConfig.order == 1 && mIEMConfig.lateReverb;
			Vec3 position;
			for (const auto& it : mEdges)
			{
				if (it.second.zW < mIEMConfig.minEdgeLength)
					continue;

				// Source checks
				EdgeZone zone = it.second.FindEdgeZone(source.position);

				if (zone == EdgeZone::Invalid)
					continue;

				if (mIEMConfig.diffraction == DiffractionSound::shadowZone && zone == EdgeZone::NonShadowed)
					continue;


				VirtualSourceData& vSource = counter < size ? sp[0][counter] : sp[0].emplace_back(numAbsorptionBands);
				
				if (counter < size)
					vSource.Clear();
				counter++;

				vSource.Valid();
				vSource.AddEdgeID(it.first);
				vSource.UpdateDiffractionPath(source.position, mListenerPosition, it.second);

				if (mIEMConfig.diffraction == DiffractionSound::none)
					continue;

				// Receiver checks
				if (it.second.GetRZone() == EdgeZone::Invalid)
					continue;

				if (mIEMConfig.diffraction == DiffractionSound::shadowZone && it.second.GetRZone() == EdgeZone::NonShadowed)
					continue;

				if (!vSource.mDiffractionPath.valid)
					continue;

				if (!vSource.mDiffractionPath.inShadow && mIEMConfig.diffraction == DiffractionSound::shadowZone)
					continue;

				IDPair planeIds = it.second.GetPlaneIDs();
				bool obstruction = LineRoomObstruction(source.position, vSource.GetApex(), planeIds.first, planeIds.second);
				LineRoomObstruction(vSource.GetApex(), mListenerPosition, planeIds.first, planeIds.second, obstruction);

				if (!obstruction)
					InitVSource(source, vSource.GetApex(), vSource, vSources, feedsFDN);
			}
#ifdef PROFILE_BACKGROUND_THREAD
			EndFirstOrderDiff();
#endif
			return counter;
		}

		// Reflection
		size_t ImageEdge::FirstOrderReflections(const SourceData& source, VirtualSourceDataMap& vSources, size_t counter)
		{
#ifdef PROFILE_BACKGROUND_THREAD
			BeginFirstOrderRef();
#endif
			size_t size = sp[0].size();

			bool feedsFDN = mIEMConfig.order == 1 && mIEMConfig.lateReverb;
			Vec3 position, intersection;
			for (const auto& it : mPlanes)
			{
				if (!it.second.ReflectPointInPlane(position, source.position))
					continue;

				VirtualSourceData& vSource = counter < size ? sp[0][counter] : sp[0].emplace_back(numAbsorptionBands);

				vSource.SetPreviousPlane(Vec4(it.second.GetD(), it.second.GetNormal()));

				if (counter < size)
					vSource.Clear();
				counter++;

				vSource.Valid();
				vSource.AddPlaneID(it.first);
				vSource.SetTransform(position);

				if (!mIEMConfig.reflections)
					continue;

				if (!it.second.GetRValid())
					continue;

				Absorption& absorption = vSource.GetAbsorptionRef();

				if (!FindIntersection(intersection, absorption, mListenerPosition, position, it.second))
					continue;

				bool obstruction = LineRoomObstruction(source.position, intersection, it.first);
				LineRoomObstruction(intersection, mListenerPosition, it.first, obstruction);
				if (!obstruction)
					InitVSource(source, intersection, vSource, vSources, feedsFDN);
			}
#ifdef PROFILE_BACKGROUND_THREAD
			EndFirstOrderRef();
#endif
			return counter;
		}

		void ImageEdge::HigherOrderReflections(const SourceData& source, VirtualSourceDataMap& vSources)
		{
			// Check for first order reflections in sp
			if (sp[0].size() == 0)
				return;

			Vec3 position;
			for (int refIdx = 1; refIdx < mIEMConfig.order; refIdx++)
			{
				int refOrder = refIdx + 1;
				int prevRefIdx = refIdx - 1;
				bool feedsFDN = mIEMConfig.order == refOrder && mIEMConfig.lateReverb;

				std::vector<Vec3> intersections;
				size_t capacity = intersections.capacity();
				intersections.reserve(refOrder);
				std::fill_n(std::back_inserter(intersections), refOrder - capacity, Vec3());

				// Check m is not null
				if (sp[prevRefIdx].size() == 0)
					return;

				size_t size = sp[refIdx].size();
				size_t counter = 0;
#ifdef PROFILE_BACKGROUND_THREAD
				switch (refOrder)
				{
				case 2:
					BeginSecondOrderRef();
					break;
				case 3:
					BeginThirdOrderRef();
					break;
				case 4:
					BeginFourthOrderRef();
					break;
				default:
					BeginHigherOrderRef();
				}
#endif
				for (const auto& it : mPlanes)
				{
					bool rValid = it.second.GetRValid();
					for (const VirtualSourceData& vS : sp[prevRefIdx])
					{
						// HOD reflections
						if (!vS.diffraction)
						{
							// Can't reflect in same plane twice
							if (it.first == vS.GetID(prevRefIdx))
								continue;

							Vec4 previousPlaneData = vS.GetPreviousPlane();
							Vec4 planeData = Vec4(it.second.GetD(), it.second.GetNormal());

							// Can't reflect in parallel plane
							if (planeData.x == previousPlaneData.x && planeData.y == previousPlaneData.y && planeData.z == previousPlaneData.z)
								continue;

							// Can't reflect in coplanar plane
							if (planeData == -previousPlaneData)
								continue;

							if (!it.second.ReflectPointInPlane(position, vS.GetPosition(prevRefIdx)))
								continue;

							VirtualSourceData& vSource = counter < size ? sp[refIdx][counter] : sp[refIdx].emplace_back(vS);

							vSource.SetPreviousPlane(planeData);

							if (counter < size)
								sp[refIdx][counter].Update(vS);
							counter++;

							vSource.Reset();
							vSource.Valid();
							vSource.AddPlaneID(it.first);
							vSource.SetTransform(position);

							if (!mIEMConfig.reflections)
								continue;

							if (!rValid)
								continue;

							// Check valid intersections
							if (!FindIntersections(intersections, vSource, refIdx))
								continue;

							// Check for obstruction
							bool obstruction = LineRoomObstruction(intersections[refIdx], mListenerPosition, it.first);
							LineRoomObstruction(intersections[0], source.position, vSource.GetID(0), obstruction);

							int p = 0;
							while (!obstruction && p < refIdx)
							{
								obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
								p++;
							}
							if (!obstruction)
								InitVSource(source, intersections[0], vSource, vSources, feedsFDN);
						}
						// HOD reflections (post diffraction)
						else if (mIEMConfig.diffractedReflections != DiffractionSound::none)
						{
							const Edge& edge = vS.GetEdge();

							Vec4 planeData = Vec4(it.second.GetD(), it.second.GetNormal());

							if (vS.IsReflection(prevRefIdx))
							{
								// Can't reflect in same plane twice
								if (it.first == vS.GetID(prevRefIdx))
									continue;

								Vec4 previousPlaneData = vS.GetPreviousPlane();

								// Can't reflect in parallel plane
								if (planeData.x == previousPlaneData.x && planeData.y == previousPlaneData.y && planeData.z == previousPlaneData.z)
									continue;

								// Can't reflect in coplanar plane
								if (planeData == -previousPlaneData)
									continue;
							}
							else
							{
								// Can't reflect in plane attached to edge
								if (edge.IncludesPlane(it.first))
									continue;
							}

							// Check edge in front of plane
							if (!it.second.EdgePlanePosition(edge))
								continue;

							VirtualSourceData& vSource = counter < size ? sp[refIdx][counter] : sp[refIdx].emplace_back(vS);

							vSource.SetPreviousPlane(planeData);

							if (counter < size)
								sp[refIdx][counter].Update(vS);
							counter++;

							vSource.Reset();
							vSource.Valid();
							vSource.AddPlaneID(it.first);

							position = vSource.GetPosition(prevRefIdx);
							it.second.ReflectPointInPlaneNoCheck(position);
							vSource.UpdateDiffractionPath(position, mListenerPosition, it.second);

							if (!rValid)
								continue;

							EdgeZone zone = vSource.GetEdge().FindEdgeZone(mListenerPosition);

							// Receiver checks
							if (zone == EdgeZone::Invalid)
								continue;

							if (mIEMConfig.diffractedReflections == DiffractionSound::shadowZone && zone == EdgeZone::NonShadowed)
								continue;

							if (!vSource.mDiffractionPath.valid)
								continue;

							if (!vSource.mDiffractionPath.inShadow && mIEMConfig.diffractedReflections == DiffractionSound::shadowZone)
								continue;

							// Check valid intersections
							if (!FindIntersections(intersections, vSource, refIdx))
								continue;

							// Check for obstruction
							bool obstruction = LineRoomObstruction(mListenerPosition, intersections[refIdx]);
							LineRoomObstruction(intersections[0], source.position, vSource.GetID(0), obstruction);

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
								InitVSource(source, intersections[0], vSource, vSources, feedsFDN);
						}
					}
				}

#ifdef PROFILE_BACKGROUND_THREAD
				switch (refOrder)
				{
				case 2:
					EndSecondOrderRef();
					break;
				case 3:
					EndThirdOrderRef();
					break;
				case 4:
					EndFourthOrderRef();
					break;
				default:
					EndHigherOrderRef();
				}
#endif
				if (mIEMConfig.diffractedReflections == DiffractionSound::none)
				{
					sp[refIdx].resize(counter, VirtualSourceData(numAbsorptionBands));
					continue;
				}
#ifdef PROFILE_BACKGROUND_THREAD
				switch (refOrder)
				{
				case 2:
					BeginSecondOrderRefDiff();
					break;
				case 3:
					BeginThirdOrderRefDiff();
					break;
				case 4:
					BeginFourthOrderRefDiff();
					break;
				default:
					BeginHigherOrderRefDiff();
				}
#endif
				for (const auto& it : mEdges)
				{
					if (it.second.zW < mIEMConfig.minEdgeLength)
						continue;

					EdgeZone rZone = it.second.GetRZone();
					for (const VirtualSourceData& vS : sp[prevRefIdx])
					{
						if (vS.diffraction)
							continue;

						// Can't diffract in edge attached to plane
						if (it.second.IncludesPlane(vS.GetID()))
							continue;

						// Source checks
						EdgeZone zone = it.second.FindEdgeZone(vS.GetPosition());

						if (zone == EdgeZone::Invalid)
							continue;

						if (mIEMConfig.diffractedReflections == DiffractionSound::shadowZone && zone == EdgeZone::NonShadowed)
							continue;

						VirtualSourceData& vSource = counter < size ? sp[refIdx][counter] : sp[refIdx].emplace_back(vS);

						if (counter < size)
							sp[refIdx][counter].Update(vS);
						counter++;

						vSource.Reset();
						vSource.Valid();
						vSource.AddEdgeID(it.first);
						vSource.UpdateDiffractionPath(vSource.GetPosition(), mListenerPosition, it.second);

						// Receiver checks
						if (rZone == EdgeZone::Invalid)
							continue;

						if (mIEMConfig.diffractedReflections == DiffractionSound::shadowZone && it.second.GetRZone() == EdgeZone::NonShadowed)
							continue;

						if (!vSource.mDiffractionPath.valid)
							continue;

						if (!vSource.mDiffractionPath.inShadow && mIEMConfig.diffractedReflections == DiffractionSound::shadowZone)
							continue;

						// Check valid intersections
						if (!FindIntersections(intersections, vSource, refIdx))
							continue;

						// Check for obstruction
						bool obstruction = LineRoomObstruction(mListenerPosition, intersections[refIdx]);
						LineRoomObstruction(intersections[0], source.position, vSource.GetID(0), obstruction);

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
							InitVSource(source, intersections[0], vSource, vSources, feedsFDN);
					}
				}
#ifdef PROFILE_BACKGROUND_THREAD
				switch (refOrder)
				{
				case 2:
					EndSecondOrderRefDiff();
					break;
				case 3:
					EndThirdOrderRefDiff();
					break;
				case 4:
					EndFourthOrderRefDiff();
					break;
				default:
					EndHigherOrderRefDiff();
				}
#endif
				sp[refIdx].resize(counter, VirtualSourceData(numAbsorptionBands));
			}
		}

		void ImageEdge::InitVSource(const SourceData& source, const Vec3& intersection, VirtualSourceData& vSource, VirtualSourceDataMap& vSources, bool feedsFDN)
		{
			Real directivity = CalculateDirectivity(source, intersection);
			vSource.AddDirectivity(directivity);

			vSource.SetDistance(mListenerPosition);
			vSource.Visible(feedsFDN);
			vSource.lastUpdatedCycle = currentCycle;
			vSources.insert_or_assign(vSource.GetKey(), vSource);
		}

		Real ImageEdge::CalculateDirectivity(const SourceData& source, const Vec3& intersection) const
		{
			switch (source.directivity)
			{
			case SourceDirectivity::omni:
				return 1.0;
			case SourceDirectivity::cardioid:
			{
				Real angle = acos(Dot(source.forward, UnitVector(intersection - source.position)));
				Real ret = 0.5 * (1 + cos(angle));
				if (ret < EPS)
					return EPS;
				return ret;
			}
			default:
				return 1.0;
			}
		}

		void ImageEdge::EraseOldEntries(VirtualSourceDataMap& vSources)
		{
			for (auto it = vSources.begin(); it != vSources.end();)
			{
				if (it->second.lastUpdatedCycle != currentCycle)
					it = vSources.erase(it);
				else
					++it;
			}
		}
	}
}