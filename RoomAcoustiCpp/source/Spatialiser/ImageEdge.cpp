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

// Common headers
#include "Common/RACProfiler.h"

// Spatialiser headers
#include "Spatialiser/ImageEdge.h"
#include "Spatialiser/Directivity.h"

// Unity headers
#include "Unity/Debug.h"

namespace RAC
{
	using namespace Unity;
	using namespace Common;
	namespace Spatialiser
	{
		//////////////////// ImageEdge Class ////////////////////

		////////////////////////////////////////

		ImageEdge::ImageEdge(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const std::shared_ptr<Config>& config) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb), frequencyBands(config->frequencyBands), currentCycle(false), configChanged(true), reverbRunning(false),
			mIEMConfig(config->GetDiffractionModel(), config->GetLateReverbModel()), mIEMConfigStore(config->GetDiffractionModel(), config->GetLateReverbModel())
		{
			sp = std::vector<std::vector<ImageSourceData>>();
			sp.push_back(std::vector<ImageSourceData>());

			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->GetReverbSourceDirections(reverbDirections);
			reverbAbsorptions = std::vector<Absorption<>>(static_cast<int>(reverbDirections.size()), Absorption(frequencyBands.Length()));
		}

		////////////////////////////////////////

		void ImageEdge::RunIEM()
		{
			PROFILE_BackgroundThread
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
			sharedSource->ResetUnusedSources();
			mSources = sharedSource->GetSourceData();

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

			if (imageSources.size() != mSources.size())
			{
				imageSources.resize(mSources.size());
				mSourceAudioDatas.resize(mSources.size(), Source::AudioData(frequencyBands.Length(), mIEMConfig.GetLateReverbModel()));
				mCurrentCycles.resize(mSources.size());
				// doIEM = true;
			}

			int i = 0;
			for (Source::Data& source : mSources)
			{
				if (doIEM || source.hasChanged)
				{
					mCurrentCycles[i] = !mCurrentCycles[i];
					currentCycle = mCurrentCycles[i];
					ReflectPointInRoom(source, mSourceAudioDatas[i], imageSources[i]);
				}

				sharedSource->UpdateSourceData(source.id, mSourceAudioDatas[i], imageSources[i]);

#ifdef IEM_FLAG
				Debug::IEMFlag(source.id);
#endif
				++i;
			}	

			if (reverbRunning || doIEM)
				reverbRunning = UpdateLateReverbFilters(doIEM);

#ifdef IEM_FLAG
			Debug::IEMFlag(-1);
#endif
		}

		////////////////////////////////////////

		void ImageEdge::UpdateRValid()
		{
			// Determine if receiver is in front or behind plane face
			for (auto& [planeID, plane] : mPlanes)
				plane.SetReceiverValid(mListenerPosition);

			// Determine where receiver lies around an edge
			for (auto& [edgeID, edge] : mEdges)
				edge.SetReceiverZone(mListenerPosition);
		}

		////////////////////////////////////////

		bool ImageEdge::FindIntersections(ImageSourceData& imageSource, std::vector<Vec3>& intersections) const
		{
			int bounceIdx = static_cast<int>(intersections.size()) - 1;
			imageSource.ResetAbsorption();
			Absorption<>& absorption = imageSource.GetAbsorption();
			bool valid = false;
			if (imageSource.IsReflection(bounceIdx))
			{
				auto it = mPlanes.find(imageSource.GetID(bounceIdx));
				if (it != mPlanes.end()) // case: plane exists
					valid = LinePlaneIntersection(mListenerPosition, imageSource.GetPosition(bounceIdx), it->second, absorption, intersections[bounceIdx]);
				else
					return false;
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

		bool ImageEdge::LinePlaneIntersection(const Vec3& start, const Vec3& end, const Plane& plane, Absorption<>& absorption, Vec3& intersection) const
		{
			if (plane.LinePlaneIntersection(start, end))
			{
				if (LineWallIntersection(start, end, plane.GetWalls(), absorption, intersection))
					return true;
			}
			return false;
		}

		////////////////////////////////////////

		bool ImageEdge::LineWallIntersection(const Vec3& start, const Vec3& end, const std::vector<size_t>& wallIDs, Absorption<>& absorption, Vec3& intersection) const
		{
			for (const size_t wallID : wallIDs)
			{
				auto itW = mWalls.find(wallID);
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
			for (const auto& [planeID, plane] : mPlanes)
			{
				// Skip excluded planes
				if (excludedPlaneIds.find(planeID) != excludedPlaneIds.end())
					continue;

				if (LinePlaneObstruction(start, end, plane))
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
			for (const size_t wallID : wallIDs)
			{
				auto itW = mWalls.find(wallID);
				if (itW == mWalls.end()) // case: wall doesn't exist
					continue;

				if (itW->second.LineWallObstruction(start, end))
					return true;
			}
			return false;
		}

		////////////////////////////////////////

		Absorption<> ImageEdge::CalculateDirectivity(const Source::Data& source, const Vec3& point) const
		{
			Absorption directivity(frequencyBands.Length());
			Real ret = 0.0;
			switch (source.directivity)
			{
			case SourceDirectivity::omni:
			{ directivity = 1.0; break; }
			case SourceDirectivity::subcardioid:
			{
				Real angle = acos(Dot(source.forward, UnitVector(point - source.position)));
				ret = 0.7 + 0.3 * cos(angle);
				break;
			}
			case SourceDirectivity::cardioid:
			{
				Real angle = std::acos(Dot(source.forward, UnitVector(point - source.position)));
				ret = 0.5 * (1 + std::cos(angle));
				break;
			}
			case SourceDirectivity::supercardioid:
			{
				Real angle = std::acos(Dot(source.forward, UnitVector(point - source.position)));
				ret = 0.37 + 0.63 * std::cos(angle);
				break;
			}
			case SourceDirectivity::hypercardioid:
			{
				Real angle = std::acos(Dot(source.forward, UnitVector(point - source.position)));
				ret = 0.25 + 0.75 * std::cos(angle);
				break;
			}
			case SourceDirectivity::bidirectional:
			{
				Real angle = std::acos(Dot(source.forward, UnitVector(point - source.position)));
				ret = std::abs(std::cos(angle));
				break;
			}
			case SourceDirectivity::genelec8020c:
			{
				Vec3 direction = UnitVector(point - source.position);
				Vec3 localDirection = source.orientation.RotateVector(direction);

				Real theta = std::acos(localDirection.z);
				Real phi = -std::atan2(localDirection.x, localDirection.y); // -phi converts from left-handed coordinate system to right-handed coordinate system
				// Debug::Log("Theta: " + RealToStr(Rad2Deg(theta)) + ", Phi: " + RealToStr(Rad2Deg(phi < 0.0 ? phi + PI_2: phi)), Colour::Yellow);

				directivity = GENELEC.Response(frequencyBands, theta, phi);
				break;
			}
			case SourceDirectivity::genelec8020cDTF:
			{
				Vec3 direction = UnitVector(point - source.position);
				Vec3 localDirection = source.orientation.RotateVector(direction);

				Real theta = std::acos(localDirection.z);
				Real phi = -std::atan2(localDirection.x, localDirection.y); // -phi converts from left-handed coordinate system to right-handed coordinate system

				directivity = GENELEC_DTF.Response(frequencyBands, theta, phi);
				break;
			}
			case SourceDirectivity::qscK8:
			{
				Vec3 direction = UnitVector(point - source.position);
				Vec3 localDirection = source.orientation.RotateVector(direction);

				Real theta = std::acos(localDirection.z);
				Real phi = -std::atan2(localDirection.x, localDirection.y); // -phi converts from left-handed coordinate system to right-handed coordinate system

				directivity = QSC_K8.Response(frequencyBands, theta, phi);
				break;
			}
			default:
				directivity = 1.0;
			}

			if (ret != 0.0)
				directivity = ret < EPS ? EPS : ret;
			return directivity;
		}

		////////////////////////////////////////

		Absorption<> ImageEdge::Direct(const Source::Data& source)
		{
			PROFILE_Direct
			bool lineOfSight = false;
			// Direct sound
			switch (mIEMConfig.data.direct)
			{
			case DirectSound::check:
				lineOfSight = !LineRoomObstruction(mListenerPosition, source.position);
				break;
			case DirectSound::ignoreCheck:
				lineOfSight = true;
				break;
			default:
				lineOfSight = false;
			}

			if (lineOfSight)
			{
#ifdef DEBUG_IEM
				Debug::send_path(IntToStr(source.id) + "s", { source.position }, mListenerPosition);
#endif
				return CalculateDirectivity(source, mListenerPosition);
			}
			else
			{
#ifdef DEBUG_IEM
				Debug::remove_path(IntToStr(source.id) + "s");
#endif
				return 0.0;
			}
		}

		void ImageEdge::ReflectPointInRoom(const Source::Data& source, Source::AudioData& direct, ImageSourceDataMap& imageSources)
		{
			PROFILE_ImageEdgeModel
			direct.directivity = Direct(source);
			direct.reverbSend = mIEMConfig.GetLateReverbModel();

			if (mIEMConfig.MaxOrder() < 1)
			{
				sp.clear();
				EraseOldEntries(imageSources);
				return;
			}
			
			if (direct.reverbSend == LateReverbModel::fdn && (mIEMConfig.data.reflOrder > 0 || mIEMConfig.data.shadowDiffOrder > 1))
				direct.reverbSend = LateReverbModel::none;

			if (sp.size() != mIEMConfig.MaxOrder())
				sp.resize(mIEMConfig.MaxOrder(), std::vector<ImageSourceData>());

			size_t counter = 0;

			if ((mIEMConfig.data.shadowDiffOrder > 0 || mIEMConfig.data.specularDiffOrder > 0) && mEdges.size() > 0)
				counter = FirstOrderDiffraction(source, imageSources);

			if (mWalls.size() > 0)
			{
				counter = FirstOrderReflections(source, imageSources, counter);

				sp[0].resize(counter, ImageSourceData(frequencyBands.Length()));

				if (mIEMConfig.MaxOrder() < 2)
				{
					EraseOldEntries(imageSources);
					return;
				}

				HigherOrderPaths(source, imageSources);
			}
			else
				sp[0].resize(counter, ImageSourceData(frequencyBands.Length()));

			EraseOldEntries(imageSources);
			return;
		}

		////////////////////////////////////////

		size_t ImageEdge::FirstOrderDiffraction(const Source::Data& source, ImageSourceDataMap& imageSources)
		{
			PROFILE_FirstOrderDiffraction
			size_t size = sp[0].size();
			size_t counter = 0;
			int order;
			bool feedsFDN = mIEMConfig.FeedsFDN(1);

			for (const auto& [edgeID, edge] : mEdges)
			{
				if (edge.GetLength() < mIEMConfig.data.minEdgeLength)
					continue;

				// Source checks
				EdgeZone zone = edge.FindEdgeZone(source.position);

				if (zone == EdgeZone::Invalid)
					continue;

				if (mIEMConfig.data.specularDiffOrder < 1 && zone == EdgeZone::NonShadowed)
					continue;

				ImageSourceData& imageSource = counter < size ? sp[0][counter] : sp[0].emplace_back(frequencyBands.Length());
				
				if (counter < size)
					imageSource.Clear();
				counter++;

				imageSource.Valid();
				imageSource.AddEdgeID(edgeID);
				imageSource.UpdateDiffractionPath(source.position, mListenerPosition, edge);

				/*if (mIEMConfig.diffraction == DiffractionSound::none)
					continue;*/

				// Receiver checks
				if (edge.GetReceiverZone() == EdgeZone::Invalid)
					continue;

				if (mIEMConfig.data.specularDiffOrder < 1 && edge.GetReceiverZone() == EdgeZone::NonShadowed)
					continue;

				if (!imageSource.GetDiffractionPath().valid)
					continue;

				if (imageSource.GetDiffractionPath().inShadowZone)
					order = mIEMConfig.data.shadowDiffOrder;
				else
					order = mIEMConfig.data.specularDiffOrder;

				if (order < 1)
					continue;

				if (CheckObstructions(source.position, imageSource, { imageSource.GetApex() }))
					continue;

				InitImageSource(source, imageSource.GetApex(), imageSource, imageSources, feedsFDN);
#ifdef DEBUG_IEM
				Vec3 position;
				position = imageSource.GetTransform().GetPosition();
				Debug::send_path(imageSource.GetKey(), { imageSource.GetApex() }, position);
#endif
			}
			return counter;
		}

		////////////////////////////////////////

		size_t ImageEdge::FirstOrderReflections(const Source::Data& source, ImageSourceDataMap& imageSources, size_t counter)
		{
			PROFILE_FirstOrderReflections
			size_t size = sp[0].size();

			bool feedsFDN = mIEMConfig.FeedsFDN(1);
			Vec3 position;
			std::vector<Vec3> intersections = std::vector<Vec3>(1, Vec3());
			for (const auto& [planeID, plane] : mPlanes)
			{
				if (!plane.ReflectPointInPlane(position, source.position))
					continue;

				ImageSourceData& imageSource = counter < size ? sp[0][counter] : sp[0].emplace_back(frequencyBands.Length());

				imageSource.SetPreviousPlane(Vec4(plane.GetD(), plane.GetNormal()));
				if (counter < size)
					imageSource.Clear();
				counter++;

				imageSource.Valid();
				imageSource.AddPlaneID(planeID);
				imageSource.SetTransform(position);

				if (mIEMConfig.data.reflOrder < 1)
					continue;

				if (!plane.GetReceiverValid())
					continue;

				if (!FindIntersections(imageSource, intersections))
					continue;

				if (CheckObstructions(source.position, imageSource, intersections))
					continue;

				InitImageSource(source, intersections[0], imageSource, imageSources, feedsFDN);
#ifdef DEBUG_IEM
				Vec3 position;
				position = imageSource.GetTransform().GetPosition();
				Debug::send_path(imageSource.GetKey(), intersections, position);
#endif
			}
			return counter;
		}

		////////////////////////////////////////

		void ImageEdge::HigherOrderPaths(const Source::Data& source, ImageSourceDataMap& imageSources)
		{
			// Check for first order reflections in sp
			if (sp[0].size() == 0)
				return;

			int order;
			Vec3 position;
			std::vector<Vec3> intersections = { Vec3() };
			for (int refIdx = 1; refIdx < mIEMConfig.MaxOrder(); refIdx++)
			{
				int refOrder = refIdx + 1;
				int prevRefIdx = refIdx - 1;
				const bool feedsFDN = mIEMConfig.FeedsFDN(refOrder);

				intersections.emplace_back(Vec3());
				// Check m is not null
				if (sp[prevRefIdx].size() == 0)
					return;

				size_t size = sp[refIdx].size();
				size_t counter = 0;
				{
#ifdef PROFILE_BACKGROUND_THREAD_DETAILED

					ProfilerCategories category;
					switch (refOrder)
					{
					case 2:
						category = ProfilerCategories::SecondOrderReflections;
						break;
					case 3:
						category = ProfilerCategories::ThirdOrderReflections;
						break;
					default:
						category = ProfilerCategories::HigherOrderReflection;
					}
					ProfileSection section(category);
#endif
					bool skipReflections = refOrder == mIEMConfig.MaxOrder() && mIEMConfig.data.reflOrder < refOrder;
					for (const auto& [planeID, plane] : mPlanes)
					{
						bool rValid = plane.GetReceiverValid();
						for (const ImageSourceData& vS : sp[prevRefIdx])
						{
							// HOD reflections
							if (!vS.IsDiffraction())
							{
								if (skipReflections)
									continue;

								// Can't reflect in same plane twice
								if (planeID == vS.GetID())
									continue;

								Vec4 previousPlaneData = vS.GetPreviousPlane();
								Vec4 planeData = Vec4(plane.GetD(), plane.GetNormal());

								// Can't reflect in parallel plane
								if (planeData.x == previousPlaneData.x && planeData.y == previousPlaneData.y && planeData.z == previousPlaneData.z)
									continue;

								// Can't reflect in coplanar plane
								if (planeData == -previousPlaneData)
									continue;

								if (!plane.ReflectPointInPlane(position, vS.GetPosition()))
									continue;

								ImageSourceData& imageSource = counter < size ? sp[refIdx][counter] : sp[refIdx].emplace_back(vS);

								imageSource.SetPreviousPlane(planeData);
								if (counter < size)
									imageSource.Update(vS);
								else
									imageSource.IncreaseImageSourceOrder();
								counter++;

								imageSource.Reset();
								imageSource.Valid();
								imageSource.AddPlaneID(planeID);
								imageSource.SetTransform(position);

								if (mIEMConfig.data.reflOrder < refOrder)
									continue;

								if (!rValid)
									continue;

								if (!FindIntersections(imageSource, intersections))
									continue;

								if (CheckObstructions(source.position, imageSource, intersections))
									continue;

								InitImageSource(source, intersections[0], imageSource, imageSources, feedsFDN);
#ifdef DEBUG_IEM
								Vec3 position;
								position = imageSource.GetTransform().GetPosition();
								Debug::send_path(imageSource.GetKey(), intersections, position);
#endif
							}
							// HOD reflections (post diffraction)
							else if (refIdx < mIEMConfig.data.shadowDiffOrder || refIdx < mIEMConfig.data.specularDiffOrder)
							{
								const Edge& edge = vS.GetEdge();

								Vec4 planeData = Vec4(plane.GetD(), plane.GetNormal());

								if (vS.IsReflection(prevRefIdx))
								{
									// Can't reflect in same plane twice
									if (planeID == vS.GetID())
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
									if (edge.IncludesPlane(planeID))
										continue;
								}

								// Check edge in front of plane
								if (!plane.EdgePlanePosition(edge))
									continue;

								ImageSourceData& imageSource = counter < size ? sp[refIdx][counter] : sp[refIdx].emplace_back(vS);

								imageSource.SetPreviousPlane(planeData);
								if (counter < size)
									imageSource.Update(vS);
								else
									imageSource.IncreaseImageSourceOrder();
								counter++;

								imageSource.Reset();
								imageSource.Valid();
								imageSource.AddPlaneID(planeID);

								// position = imageSource.GetPosition(prevRefIdx);
								position = imageSource.GetDiffractionPath().sData.point;
								plane.ReflectPointInPlaneNoCheck(position);
								imageSource.UpdateDiffractionPath(position, mListenerPosition, plane);

								if (!rValid)
									continue;

								EdgeZone zone = imageSource.GetEdge().FindEdgeZone(mListenerPosition);

								// Receiver checks
								if (zone == EdgeZone::Invalid)
									continue;

								if (mIEMConfig.data.specularDiffOrder < refOrder && zone == EdgeZone::NonShadowed)
									continue;

								if (!imageSource.GetDiffractionPath().valid)
									continue;

								order = imageSource.GetDiffractionPath().inShadowZone ? mIEMConfig.data.shadowDiffOrder : mIEMConfig.data.specularDiffOrder;

								if (order < refOrder)
									continue;

								if (!FindIntersections(imageSource, intersections))
									continue;

								if (CheckObstructions(source.position, imageSource, intersections))
									continue;

								InitImageSource(source, intersections[0], imageSource, imageSources, feedsFDN);
#ifdef DEBUG_IEM
								Vec3 position;
								position = imageSource.GetTransform().GetPosition();
								Debug::send_path(imageSource.GetKey(), intersections, position);
#endif
							}
						}
					}
				}

				if (mIEMConfig.data.specularDiffOrder < refOrder && mIEMConfig.data.shadowDiffOrder < refOrder)
				{
					sp[refIdx].resize(counter, ImageSourceData(frequencyBands.Length()));
					continue;
				}
#ifdef PROFILE_BACKGROUND_THREAD_DETAILED
				ProfilerCategories category;
				switch (refOrder)
				{
				case 2:
					category = ProfilerCategories::SecondOrderDiffraction;
					break;
				case 3:
					category = ProfilerCategories::ThirdOrderDiffraction;
					break;
				default:
					category = ProfilerCategories::HigherOrderDiffraction;
				}
				ProfileSection section = ProfileSection(category);
#endif
				for (const auto& [edgeID, edge] : mEdges)
				{
					if (edge.GetLength() < mIEMConfig.data.minEdgeLength)
						continue;

					EdgeZone rZone = edge.GetReceiverZone();
					for (const ImageSourceData& vS : sp[prevRefIdx])
					{
						if (vS.IsDiffraction())
							continue;

						// Can't diffract in edge attached to plane
						if (edge.IncludesPlane(vS.GetID()))
							continue;

						// Source checks
						EdgeZone zone = edge.FindEdgeZone(vS.GetPosition());

						if (zone == EdgeZone::Invalid)
							continue;

						if (mIEMConfig.data.specularDiffOrder < refOrder && zone == EdgeZone::NonShadowed)
							continue;

						ImageSourceData& imageSource = counter < size ? sp[refIdx][counter] : sp[refIdx].emplace_back(vS);

						if (counter < size)
							imageSource.Update(vS);
						else
							imageSource.IncreaseImageSourceOrder();
						counter++;

						imageSource.Reset();
						imageSource.Valid();
						imageSource.AddEdgeID(edgeID);
						imageSource.UpdateDiffractionPath(imageSource.GetPosition(prevRefIdx), mListenerPosition, edge);

						// Receiver checks
						if (rZone == EdgeZone::Invalid)
							continue;

						if (mIEMConfig.data.specularDiffOrder < refOrder && edge.GetReceiverZone() == EdgeZone::NonShadowed)
							continue;

						if (!imageSource.GetDiffractionPath().valid)
							continue;

						order = imageSource.GetDiffractionPath().inShadowZone ? mIEMConfig.data.shadowDiffOrder : mIEMConfig.data.specularDiffOrder;

						if (order < refOrder)
							continue;

						if (!FindIntersections(imageSource, intersections))
							continue;

						if (CheckObstructions(source.position, imageSource, intersections))
							continue;

						InitImageSource(source, intersections[0], imageSource, imageSources, feedsFDN);
#ifdef DEBUG_IEM
						Vec3 position;
						position = imageSource.GetTransform().GetPosition();
						Debug::send_path(imageSource.GetKey(), intersections, position);
#endif
					}
				}
				sp[refIdx].resize(counter, ImageSourceData(frequencyBands.Length()));
			}
		}

		////////////////////////////////////////

		void ImageEdge::InitImageSource(const Source::Data& source, const Vec3& intersection, ImageSourceData& imageSource, ImageSourceDataMap& imageSources, bool feedsFDN)
		{
			Absorption directivity(frequencyBands.Length());
			directivity = CalculateDirectivity(source, intersection);
			imageSource.AddAbsorption(directivity);

			imageSource.SetDistance(mListenerPosition);
			imageSource.Visible(feedsFDN);
			imageSource.UpdateCycle(currentCycle);
			imageSource.CreateKey(source.id);
			imageSources.insert_or_assign(imageSource.GetKey(), std::pair<int, ImageSourceData>(-1, imageSource));
		}

		////////////////////////////////////////

		typedef std::pair<Real, size_t> PlaneDistanceID;
		bool comparator(const PlaneDistanceID& l, const PlaneDistanceID& r) { return l.first < r.first; }

		bool ImageEdge::UpdateLateReverbFilters(bool updateFilters)
		{
			PROFILE_ReverbRayTracing
			if (mIEMConfig.GetLateReverbModel(false) != LateReverbModel::fdn)
				return reverbRunning;

			if (!mIEMConfig.data.lateReverb)
			{
				for (int j = 0; j < reverbDirections.size(); j++)
					reverbAbsorptions[j] = 0.0;
				std::shared_ptr<Reverb> sharedReverb = mReverb.lock();
				sharedReverb->UpdateReflectionFilters(reverbAbsorptions);
#ifdef DEBUG_IEM
				if (!mIEMConfig.data.lateReverb)
				{
					for (int j = 0; j < reverbDirections.size(); j++)
						Debug::remove_path(IntToStr(j) + "l");
				}
#endif
				return reverbRunning;
			}

			Real k;
			Vec3 point, intersection;
			for (int j = 0; j < reverbDirections.size(); j++)
			{
#ifdef DEBUG_IEM
				Debug::send_path(IntToStr(j) + "l", { mListenerPosition }, reverbDirections[j]);
#endif
				std::vector<PlaneDistanceID> ks = std::vector(mPlanes.size(), PlaneDistanceID(0.0, -1));
				point = mListenerPosition + reverbDirections[j];
				int i = 0;
				for (const auto& [planeID, plane] : mPlanes)
				{
					k = plane.PointPlanePosition(point);
					if (plane.GetReceiverValid() && k < 0) // receiver in front of wall and point behind wall
						ks[i] = PlaneDistanceID(k, planeID); // A more negative k means the plane is closer to the receiver
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
			std::shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->UpdateReflectionFilters(reverbAbsorptions);

			return true;
		}

		////////////////////////////////////////

		void ImageEdge::EraseOldEntries(ImageSourceDataMap& imageSources)
		{
			for (auto it = imageSources.begin(); it != imageSources.end();)
			{
				if (it->second.second.UpdatedThisCycle(currentCycle))
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