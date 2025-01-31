/*
* @class ImageEdge
*
* @brief Declaration of ImageEdge class
*
* @remarks Image edge model based after The image edge model. Armin E, Jonas S, and Michael V. 2015
*
*/

// C++ headers
#include <algorithm>

// Spatialiser headers
#include "Spatialiser/ImageEdge.h"
#include "Spatialiser/Directivity.h"

// Unity headers
#include "Unity/UnityInterface.h"
#include "Unity/Debug.h"

namespace RAC
{
	using namespace Unity;
	namespace Spatialiser
	{
		//////////////////// ImageEdge Class ////////////////////

		////////////////////////////////////////

		ImageEdge::ImageEdge(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const Coefficients& frequencyBands) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb), frequencyBands(frequencyBands), currentCycle(false), configChanged(true)
		{
			sp = std::vector<std::vector<ImageSourceData>>();
			sp.push_back(std::vector<ImageSourceData>());

			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->GetReverbSourceDirections(reverbDirections);
			reverbAbsorptions = std::vector<Absorption>(static_cast<int>(reverbDirections.size()), Absorption(frequencyBands.Length()));
		}

		////////////////////////////////////////

		void ImageEdge::RunIEM()
		{
			bool doIEM = false;

			shared_ptr<Room> sharedRoom = mRoom.lock();
			if (sharedRoom->HasChanged())
			{
				// Recopy room data (planes, walls, edges)
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
				lock_guard<std::mutex> lock(dataStoreMutex);
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

			if (imageSources.size() != mSources.size())
			{
				imageSources.resize(mSources.size());
				mSourceAudioDatas.resize(mSources.size(), SourceAudioData(frequencyBands.Length(), false));
				mCurrentCycles.resize(mSources.size());
				doIEM = true;
			}

			int i = 0;
			for (SourceData& source : mSources)
			{
				if (doIEM || source.hasChanged)
				{
					mCurrentCycles[i] = !mCurrentCycles[i];
					currentCycle = mCurrentCycles[i];
					ReflectPointInRoom(source, mSourceAudioDatas[i], imageSources[i]);
				}

#ifdef PROFILE_BACKGROUND_THREAD
				BeginUpdateAudioData();
#endif
				sharedSource->UpdateSourceData(source.id, mSourceAudioDatas[i], imageSources[i]);
#ifdef PROFILE_BACKGROUND_THREAD
				EndUpdateAudioData();
#endif
#ifdef IEM_FLAG
				Debug::IEMFlag(source.id);
#endif
				++i;
			}	

			if (doIEM)
				UpdateLateReverbFilters();

#ifdef IEM_FLAG
			Debug::IEMFlag(-1);
#endif
		}

		////////////////////////////////////////

		void ImageEdge::UpdateRValid()
		{
			// Determine if receiver is in front or behind plane face
			for (auto& it : mPlanes)
				it.second.SetReceiverValid(mListenerPosition);

			// Determine where receiver lies around an edge
			for (auto& it : mEdges)
				it.second.SetReceiverZone(mListenerPosition);
		}

		////////////////////////////////////////

		bool ImageEdge::FindIntersections(ImageSourceData& imageSource, std::vector<Vec3>& intersections) const
		{
			int bounceIdx = static_cast<int>(intersections.size()) - 1;
			imageSource.ResetAbsorption();
			Absorption& absorption = imageSource.GetAbsorption();
			bool valid = false;
			if (imageSource.IsReflection(bounceIdx))
			{
				auto it = mPlanes.find(imageSource.GetID(bounceIdx));
				if (it != mPlanes.end()) // case: plane exists
					valid = LinePlaneIntersection(mListenerPosition, imageSource.GetPosition(bounceIdx), it->second, absorption, intersections[bounceIdx]);
			}
			else
			{
				valid = true;
				intersections[bounceIdx] = imageSource.GetApex();
			}

			int bounceIdxTakeOne = bounceIdx - 1;
			while (valid && bounceIdxTakeOne >= 0)
			{
				// (bounceIdxTakeOne) Reflection intersection being found (bounceIdx) Previous reflection intersection in code and next intersection in path
				if (imageSource.IsReflection(bounceIdxTakeOne))
				{
					auto it = mPlanes.find(imageSource.GetID(bounceIdxTakeOne));
					if (it != mPlanes.end()) // case: plane exists
						valid = LinePlaneIntersection(intersections[bounceIdx], imageSource.GetPosition(bounceIdxTakeOne), it->second, absorption, intersections[bounceIdxTakeOne]);
					else
						return false;
				}
				else
					intersections[bounceIdxTakeOne] = imageSource.GetApex();

				bounceIdxTakeOne--; bounceIdx--;
			}

			if (!valid)
				return false;

			// Check for more than three identical intersection points in a row
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

		////////////////////////////////////////

		bool ImageEdge::LinePlaneIntersection(const Vec3& start, const Vec3& end, const Plane& plane, Absorption& absorption, Vec3& intersection) const
		{
			if (plane.LinePlaneIntersection(start, end))
			{
				if (LineWallIntersection(start, end, plane.GetWalls(), absorption, intersection))
					return true;
			}
			return false;
		}

		////////////////////////////////////////

		bool ImageEdge::LineWallIntersection(const Vec3& start, const Vec3& end, const std::vector<size_t>& wallIDs, Absorption& absorption, Vec3& intersection) const
		{
			for (auto& idW : wallIDs)
			{
				auto itW = mWalls.find(idW);
				if (itW == mWalls.end()) // case: wall doesn't exist
					continue;

				if (itW->second.LineWallIntersection(start, end, intersection))
				{
					absorption *= itW->second.GetAbsorption();
					if (start != intersection) // case: point on edge (consecutive intersections are identical)
						return true;

					if (itW->second.VertexMatch(intersection))  // case: point on corner (will be triggered twice)
						absorption *= INV_SQRT_6;
					else
						absorption *= 0.5;
					return true;
				}
			}
			return false;
		}

		////////////////////////////////////////

		bool ImageEdge::CheckObstructions(const Vec3& source, const ImageSourceData& imageSource, const std::vector<Vec3>& intersections) const
		{
			int refIdx = static_cast<int>(intersections.size()) - 1;
			bool obstruction = false;
			IDPair edgePlaneIDs = imageSource.GetEdge().GetPlaneIDs();

			if (imageSource.IsReflection(refIdx))
				obstruction = LineRoomObstruction(mListenerPosition, intersections[refIdx], { imageSource.GetID(refIdx) });
			else
				obstruction = LineRoomObstruction(mListenerPosition, intersections[refIdx], { edgePlaneIDs.first, edgePlaneIDs.second });

			if (obstruction)
				return true;

			if (imageSource.IsReflection(0))
				obstruction = LineRoomObstruction(intersections[0], source, { imageSource.GetID(0) });
			else
				obstruction = LineRoomObstruction(intersections[0], source, { edgePlaneIDs.first, edgePlaneIDs.second });

			int p = 0;
			while (!obstruction && p < refIdx)
			{
				if (imageSource.IsReflection(p))
				{
					if (imageSource.IsReflection(p + 1))
						obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], { imageSource.GetID(p), imageSource.GetID(p + 1) });
					else
						obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], { imageSource.GetID(p), edgePlaneIDs.first, edgePlaneIDs.second });
				}
				else
					obstruction = LineRoomObstruction(intersections[p], intersections[p + 1], { edgePlaneIDs.first, edgePlaneIDs.second, imageSource.GetID(p + 1) });
				p++;
			}
			return obstruction;
		}

		////////////////////////////////////////

		bool ImageEdge::LineRoomObstruction(const Vec3& start, const Vec3& end, const std::unordered_set<size_t>& excludedPlaneIds) const
		{
			for (const auto& itP : mPlanes)
			{
				// Skip excluded planes
				if (excludedPlaneIds.find(itP.first) != excludedPlaneIds.end())
					continue;

				if (LinePlaneObstruction(start, end, itP.second))
					return true;
			}
			return false;
		}

		////////////////////////////////////////

		bool ImageEdge::LinePlaneObstruction(const Vec3& start, const Vec3 end, const Plane& plane) const
		{
			if (plane.LinePlaneObstruction(start, end))
			{
				if (LineWallObstruction(start, end, plane.GetWalls()))
					return true;
			}
			return false;
		}

		////////////////////////////////////////

		bool ImageEdge::LineWallObstruction(const Vec3& start, const Vec3& end, const std::vector<size_t>& wallIDs) const
		{
			for (auto& idW : wallIDs)
			{
				auto itW = mWalls.find(idW);
				if (itW == mWalls.end()) // case: wall doesn't exist
					continue;

				if (itW->second.LineWallObstruction(start, end))
					return true;
			}
			return false;
		}

		////////////////////////////////////////

		Absorption ImageEdge::CalculateDirectivity(const SourceData& source, const Vec3& point) const
		{
			Absorption directivity(frequencyBands.Length());
			switch (source.directivity)
			{
			case SourceDirectivity::omni:
			{ directivity = 1.0; break; }
			case SourceDirectivity::cardioid:
			{
				Real angle = acos(Dot(source.forward, UnitVector(point - source.position)));
				Real ret = 0.5 * (1 + cos(angle));
				if (ret < EPS)
					directivity = EPS;
				directivity = ret;
				break;
			}
			case SourceDirectivity::genelec8020c:
			{
				Vec3 direction = UnitVector(point - source.position);
				Vec3 localDirection = source.orientation.RotateVector(direction);

				Real theta = acos(localDirection.z);
				Real phi = atan2(localDirection.y, localDirection.x);

				
				directivity = GENELEC.Response(frequencyBands, theta, phi);
				break;
			}
			default:
				directivity = 1.0;
			}
			return directivity;
		}

		////////////////////////////////////////

		void ImageEdge::ReflectPointInRoom(const SourceData& source, SourceAudioData& direct, ImageSourceDataMap& imageSources)
		{

#ifdef PROFILE_BACKGROUND_THREAD
			BeginIEM();
			BeginDirect();
#endif

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
				direct.directivity = CalculateDirectivity(source, mListenerPosition);
			else
				direct.directivity = 0.0;

#ifdef PROFILE_BACKGROUND_THREAD
			EndDirect();
#endif

			if (mIEMConfig.order < 1 || (!mIEMConfig.reflections && mIEMConfig.diffraction == DiffractionSound::none && mIEMConfig.diffractedReflections == DiffractionSound::none))
			{
				sp.clear();

				direct.feedsFDN = mIEMConfig.lateReverb;
				EraseOldEntries(imageSources);
#ifdef PROFILE_BACKGROUND_THREAD
				EndIEM();
#endif
				return;
			}
			else
				direct.feedsFDN = false;

			if (sp.size() != mIEMConfig.order)
				sp.resize(mIEMConfig.order, std::vector<ImageSourceData>());

			size_t counter = 0;

			if ((mIEMConfig.diffraction != DiffractionSound::none || mIEMConfig.diffractedReflections != DiffractionSound::none) && mEdges.size() > 0)
				counter = FirstOrderDiffraction(source, imageSources);

			if ((mIEMConfig.reflections || mIEMConfig.diffractedReflections != DiffractionSound::none) && mWalls.size() > 0)
			{
				counter = FirstOrderReflections(source, imageSources, counter);

				sp[0].resize(counter, ImageSourceData(frequencyBands.Length()));

				if (mIEMConfig.order < 2)
				{
					EraseOldEntries(imageSources);
#ifdef PROFILE_BACKGROUND_THREAD
					EndIEM();
#endif
					return;
				}

				HigherOrderPaths(source, imageSources);
			}
			else
				sp[0].resize(counter, ImageSourceData(frequencyBands.Length()));

			EraseOldEntries(imageSources);
#ifdef PROFILE_BACKGROUND_THREAD
			EndIEM();
#endif
			return;
		}

		////////////////////////////////////////

		size_t ImageEdge::FirstOrderDiffraction(const SourceData& source, ImageSourceDataMap& imageSources)
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
				if (it.second.GetLength() < mIEMConfig.minEdgeLength)
					continue;

				// Source checks
				EdgeZone zone = it.second.FindEdgeZone(source.position);

				if (zone == EdgeZone::Invalid)
					continue;

				if (mIEMConfig.diffraction == DiffractionSound::shadowZone && zone == EdgeZone::NonShadowed)
					continue;


				ImageSourceData& imageSource = counter < size ? sp[0][counter] : sp[0].emplace_back(frequencyBands.Length());
				
				if (counter < size)
					imageSource.Clear();
				counter++;

				imageSource.Valid();
				imageSource.AddEdgeID(it.first);
				imageSource.UpdateDiffractionPath(source.position, mListenerPosition, it.second);

				if (mIEMConfig.diffraction == DiffractionSound::none)
					continue;

				// Receiver checks
				if (it.second.GetReceiverZone() == EdgeZone::Invalid)
					continue;

				if (mIEMConfig.diffraction == DiffractionSound::shadowZone && it.second.GetReceiverZone() == EdgeZone::NonShadowed)
					continue;

				if (!imageSource.GetDiffractionPath().valid)
					continue;

				if (!imageSource.GetDiffractionPath().inShadow && mIEMConfig.diffraction == DiffractionSound::shadowZone)
					continue;

				if (CheckObstructions(source.position, imageSource, { imageSource.GetApex() }))
					continue;

#ifdef DEBUG_IEM
				Vec3 position;
				position = imageSource.GetTransform().GetPosition();
				Debug::send_path(imageSource.GetKey(), { imageSource.GetApex() }, position);
#endif

				InitImageSource(source, imageSource.GetApex(), imageSource, imageSources, feedsFDN);
			}
#ifdef PROFILE_BACKGROUND_THREAD
			EndFirstOrderDiff();
#endif
			return counter;
		}

		////////////////////////////////////////

		size_t ImageEdge::FirstOrderReflections(const SourceData& source, ImageSourceDataMap& imageSources, size_t counter)
		{
#ifdef PROFILE_BACKGROUND_THREAD
			BeginFirstOrderRef();
#endif
			size_t size = sp[0].size();

			bool feedsFDN = mIEMConfig.order == 1 && mIEMConfig.lateReverb;
			Vec3 position;
			std::vector<Vec3> intersections = std::vector<Vec3>(1, Vec3());
			for (const auto& it : mPlanes)
			{
				if (!it.second.ReflectPointInPlane(position, source.position))
					continue;

				ImageSourceData& imageSource = counter < size ? sp[0][counter] : sp[0].emplace_back(frequencyBands.Length());

				imageSource.SetPreviousPlane(Vec4(it.second.GetD(), it.second.GetNormal()));

				if (counter < size)
					imageSource.Clear();
				counter++;

				imageSource.Valid();
				imageSource.AddPlaneID(it.first);

				imageSource.SetTransform(position);

				if (!mIEMConfig.reflections)
					continue;

				if (!it.second.GetReceiverValid())
					continue;

				if (!FindIntersections(imageSource, intersections))
					continue;

				if (CheckObstructions(source.position, imageSource, intersections))
					continue;

#ifdef DEBUG_IEM
				Vec3 position;
				position = imageSource.GetTransform().GetPosition();
				Debug::send_path(imageSource.GetKey(), intersections, position);
#endif

				InitImageSource(source, intersections[0], imageSource, imageSources, feedsFDN);
			}
#ifdef PROFILE_BACKGROUND_THREAD
			EndFirstOrderRef();
#endif
			return counter;
		}

		////////////////////////////////////////

		void ImageEdge::HigherOrderPaths(const SourceData& source, ImageSourceDataMap& imageSources)
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
					bool rValid = it.second.GetReceiverValid();
					for (const ImageSourceData& vS : sp[prevRefIdx])
					{
						// HOD reflections
						if (!vS.IsDiffraction())
						{
							// Can't reflect in same plane twice
							if (it.first == vS.GetID())
								continue;

							Vec4 previousPlaneData = vS.GetPreviousPlane();
							Vec4 planeData = Vec4(it.second.GetD(), it.second.GetNormal());

							// Can't reflect in parallel plane
							if (planeData.x == previousPlaneData.x && planeData.y == previousPlaneData.y && planeData.z == previousPlaneData.z)
								continue;

							// Can't reflect in coplanar plane
							if (planeData == -previousPlaneData)
								continue;

							if (!it.second.ReflectPointInPlane(position, vS.GetPosition()))
								continue;

							ImageSourceData& imageSource = counter < size ? sp[refIdx][counter] : sp[refIdx].emplace_back(vS);

							imageSource.SetPreviousPlane(planeData);

							if (counter < size)
								sp[refIdx][counter].Update(vS);
							counter++;

							imageSource.Reset();
							imageSource.Valid();
							imageSource.AddPlaneID(it.first);
							imageSource.SetTransform(position);

							if (!mIEMConfig.reflections)
								continue;

							if (!rValid)
								continue;

							if (!FindIntersections(imageSource, intersections))
								continue;

							if (CheckObstructions(source.position, imageSource, intersections))
								continue;

#ifdef DEBUG_IEM
							Vec3 position;
							position = imageSource.GetTransform().GetPosition();
							Debug::send_path(imageSource.GetKey(), intersections, position);
#endif

							InitImageSource(source, intersections[0], imageSource, imageSources, feedsFDN);
						}
						// HOD reflections (post diffraction)
						else if (mIEMConfig.diffractedReflections != DiffractionSound::none)
						{
							const Edge& edge = vS.GetEdge();

							Vec4 planeData = Vec4(it.second.GetD(), it.second.GetNormal());

							if (vS.IsReflection(prevRefIdx))
							{
								// Can't reflect in same plane twice
								if (it.first == vS.GetID())
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

							ImageSourceData& imageSource = counter < size ? sp[refIdx][counter] : sp[refIdx].emplace_back(vS);

							imageSource.SetPreviousPlane(planeData);

							if (counter < size)
								sp[refIdx][counter].Update(vS);
							counter++;

							imageSource.Reset();
							imageSource.Valid();
							imageSource.AddPlaneID(it.first);

							position = imageSource.GetPosition();
							it.second.ReflectPointInPlaneNoCheck(position);
							imageSource.UpdateDiffractionPath(position, mListenerPosition, it.second);

							if (!rValid)
								continue;

							EdgeZone zone = imageSource.GetEdge().FindEdgeZone(mListenerPosition);

							// Receiver checks
							if (zone == EdgeZone::Invalid)
								continue;

							if (mIEMConfig.diffractedReflections == DiffractionSound::shadowZone && zone == EdgeZone::NonShadowed)
								continue;

							if (!imageSource.GetDiffractionPath().valid)
								continue;

							if (!imageSource.GetDiffractionPath().inShadow && mIEMConfig.diffractedReflections == DiffractionSound::shadowZone)
								continue;

							if (!FindIntersections(imageSource, intersections))
								continue;

							if (CheckObstructions(source.position, imageSource, intersections))
								continue;

#ifdef DEBUG_IEM
							Vec3 position;
							position = imageSource.GetTransform().GetPosition();
							Debug::send_path(imageSource.GetKey(), intersections, position);
#endif

							InitImageSource(source, intersections[0], imageSource, imageSources, feedsFDN);
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
					sp[refIdx].resize(counter, ImageSourceData(frequencyBands.Length()));
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
					if (it.second.GetLength() < mIEMConfig.minEdgeLength)
						continue;

					EdgeZone rZone = it.second.GetReceiverZone();
					for (const ImageSourceData& vS : sp[prevRefIdx])
					{
						if (vS.IsDiffraction())
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

						ImageSourceData& imageSource = counter < size ? sp[refIdx][counter] : sp[refIdx].emplace_back(vS);

						if (counter < size)
							sp[refIdx][counter].Update(vS);
						counter++;

						imageSource.Reset();
						imageSource.Valid();
						imageSource.AddEdgeID(it.first);
						imageSource.UpdateDiffractionPath(imageSource.GetPosition(), mListenerPosition, it.second);

						// Receiver checks
						if (rZone == EdgeZone::Invalid)
							continue;

						if (mIEMConfig.diffractedReflections == DiffractionSound::shadowZone && it.second.GetReceiverZone() == EdgeZone::NonShadowed)
							continue;

						if (!imageSource.GetDiffractionPath().valid)
							continue;

						if (!imageSource.GetDiffractionPath().inShadow && mIEMConfig.diffractedReflections == DiffractionSound::shadowZone)
							continue;

						if (!FindIntersections(imageSource, intersections))
							continue;

						if (CheckObstructions(source.position, imageSource, intersections))
							continue;

#ifdef DEBUG_IEM
						Vec3 position;
						position = imageSource.GetTransform().GetPosition();
						Debug::send_path(imageSource.GetKey(), intersections, position);
#endif

						InitImageSource(source, intersections[0], imageSource, imageSources, feedsFDN);
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
				sp[refIdx].resize(counter, ImageSourceData(frequencyBands.Length()));
			}
		}

		////////////////////////////////////////

		void ImageEdge::InitImageSource(const SourceData& source, const Vec3& intersection, ImageSourceData& imageSource, ImageSourceDataMap& imageSources, bool feedsFDN)
		{
			Absorption directivity(frequencyBands.Length());
			directivity = CalculateDirectivity(source, intersection);
			imageSource.AddAbsorption(directivity);

			/*Real directivity = CalculateDirectivity(source, intersection);
			imageSource.SetDirectivity(directivity);*/

			imageSource.SetDistance(mListenerPosition);
			imageSource.Visible(feedsFDN);
			imageSource.UpdateCycle(currentCycle);
			imageSources.insert_or_assign(imageSource.GetKey(), imageSource);
		}

		////////////////////////////////////////

		typedef std::pair<Real, size_t> PlaneDistanceID;
		bool comparator(const PlaneDistanceID& l, const PlaneDistanceID& r) { return l.first < r.first; }

		void ImageEdge::UpdateLateReverbFilters()
		{
			if (!mIEMConfig.lateReverb)
			{
#ifdef DEBUG_IEM
				for (int j = 0; j < reverbDirections.size(); j++)
					Debug::remove_path("rev_" + IntToStr(j));
#endif
				return;
			}
#ifdef PROFILE_BACKGROUND_THREAD
			BeginLateReverb();
#endif
			Real k;
			Vec3 point, intersection;
			for (int j = 0; j < reverbDirections.size(); j++)
			{
#ifdef DEBUG_IEM
				Debug::send_path("rev_" + IntToStr(j), {mListenerPosition}, reverbDirections[j]);
#endif
				std::vector<PlaneDistanceID> ks = std::vector(mPlanes.size(), PlaneDistanceID(0.0, -1));
				point = mListenerPosition + reverbDirections[j];
				int i = 0;
				for (auto& it : mPlanes)
				{
					k = it.second.PointPlanePosition(point);
					if (it.second.GetReceiverValid() && k < 0) // receiver in front of wall and point behind wall
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
							valid = LinePlaneIntersection(mListenerPosition, point, itP->second, reverbAbsorptions[j], intersection);
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

		////////////////////////////////////////

		void ImageEdge::EraseOldEntries(ImageSourceDataMap& imageSources)
		{
			for (auto it = imageSources.begin(); it != imageSources.end();)
			{
				if (it->second.UpdatedThisCycle(currentCycle))
					++it;
				else
				{
#ifdef DEBUG_IEM
					Debug::remove_path(it->first);
#endif
					it = imageSources.erase(it);
				}
			}
		}
	}
}