#include "Spatialiser/TracingThread.h"

// Common headers
#include "Common/RACProfiler.h"

// Unity headers
#include "Unity/Debug.h"

// helpers
#include <cassert>

namespace RAC
{
	using namespace Unity;
	using namespace Common;
	namespace Spatialiser
	{
		// TODO: Save MoDARTData instance, or copy its attributes?
		TracingThread::TracingThread(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const LateReverbData& data, const std::shared_ptr<DSPConfig>& dspConfig) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb),
			numReverbDirections(dspConfig->GetData().numReverbSources),
			clustersSizes(dspConfig->GetData().numReverbSources)
		{
			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->GetReverbSourceDirections(reverbDirections);

			// This initializes the pencil, clusters the reverb directions, and allocates buffers of size numRays.
			SetNumberOfRays(data.numRays);
		}

		void TracingThread::SetNumberOfRays(int newNumRays) {
			lock_guard<std::mutex> lock(rayPencilMutex);

			// Note: the number of rays is forced to be even, because `hemispherePencil` actually uses half as many rays under the hood.
			if (newNumRays % 2 == 0)
				numRays = newNumRays;
			else
				numRays = newNumRays + 1;

			hemispherePencil = RayPencil(numRays / 2, true);

			rayDistances = Vec<>::Zero(numRays);
			rayCosines = Vec<>::Zero(numRays);
			frontIndices = Vec<int>::Constant(numRays, -1);
			backIndices = Vec<int>::Constant(numRays, -1);
			rayClusters = Vec<int>::Constant(numRays, -1);

			hemispherePencil.clusterDirections(reverbDirections, rayClusters);
			for (int dir_idx = 0; dir_idx < numReverbDirections; ++dir_idx)
			{
				clustersSizes(dir_idx) = 0;
				for (int ray_idx = 0; ray_idx < numRays; ++ray_idx)
				{
					if (rayClusters(ray_idx) == dir_idx)
						++clustersSizes(dir_idx);
				}
			}
		}

		void MoDARTTracing::InitRoom(const Matrix<int>& indexing, const Vec<>& decayRates) {
			lock_guard<std::mutex> lock(rayPencilMutex);

			assert(decayRates.Length() == numFDNs);
			pathIndexing = indexing;
			decayPerSecond = decayRates;
			sourceResidues = Coefficients<>(numFDNs);
			listenerResidues.resize(numFDNs, Coefficients<>(numReverbDirections));

			energyContributions = Vec<>(numPaths);
			contributionDelays = Vec<>(numPaths);
			contributionDelayScaling = Vec<>(numPaths);
		}

		void MoDARTTracing::RunTracing() {
			PROFILE_ReverbRayTracing
#ifdef RTM_FLAG
			Debug::RTMStartFlag();
#endif
			tracingStartFlag.store(true, std::memory_order_release);

			// TODO: Should we only update residues relevant to currently active FDNs (i.e T60 > minimumT60)?
			lock_guard<std::mutex> lock(rayPencilMutex);
			shared_ptr<Room> sharedRoom = mRoom.lock();
			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			shared_ptr<SourceManager> sharedSource = mSourceManager.lock();

			if (numPaths == 0)
				return;

			bool listenerMoved = false;
			{
				lock_guard<std::mutex> lock(dataStoreMutex);
				if (mListenerPosition != mListenerPositionIncoming)
				{
					mListenerPosition = mListenerPositionIncoming;
					listenerMoved = true;
				}
			}
			if (listenerMoved) {
				hemispherePencil.moveOrigin(mListenerPosition);
				hemispherePencil.traceAll(sharedRoom->GetTriangleMeshSoA());

				for (int dir_idx = 0; dir_idx < numReverbDirections; ++dir_idx) {
					computeEnergyContributions(dir_idx);
#ifdef DEBUG_RTM
				Debug::send_path(IntToStr(dir_idx) + "l", { mListenerPosition }, reverbDirections[dir_idx]);
#endif

					for (int slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
						for (int path_idx = 0; path_idx < numPaths; ++path_idx) {
							contributionDelayScaling(path_idx) = std::pow(decayPerSecond(slope_idx), -contributionDelays(path_idx));
						}

						listenerResidues[slope_idx][dir_idx] =
							ThreeWayDot(
								energyContributions,
								contributionDelayScaling,
								sharedReverb->GetRightEigenvector(slope_idx));

#ifdef DEBUG_RTM
						Debug::send_residue(listenerResidues[slope_idx][dir_idx], false, dir_idx, slope_idx);
#endif
					}
				}

				for (int slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
					sharedReverb->SetTargetListenerResidues(slope_idx, listenerResidues[slope_idx]);
				}
			}

			mSources = sharedSource->GetSourceData(ThreadID::rayTracing);

			for (Source::Data& source : mSources) {
				if (source.needsUpdate) {
					hemispherePencil.moveOrigin(source.position);
					hemispherePencil.traceAll(sharedRoom->GetTriangleMeshSoA());

					computeEnergyContributions();

					for (int slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
						for (int path_idx = 0; path_idx < numPaths; ++path_idx) {
							contributionDelayScaling(path_idx) = std::pow(decayPerSecond(slope_idx), -contributionDelays(path_idx));
						}

						sourceResidues[slope_idx] =
							ThreeWayDot(
								energyContributions,
								contributionDelayScaling,
								sharedReverb->GetLeftEigenvector(slope_idx));

#ifdef DEBUG_RTM
						Debug::send_residue(sourceResidues[slope_idx], true, source.id, slope_idx);
#endif

						// Compensate gain based on preceding delay.
						sourceResidues[slope_idx] *= std::pow(decayPerSecond(slope_idx), sharedReverb->GetPrecedingDelay());
					}
					sharedSource->SetSourceTargetResidues(source.id, sourceResidues);
				}
			}
#ifdef RTM_FLAG
			Debug::RTMEndFlag();
#endif
			tracingEndFlag.store(true, std::memory_order_release);
			tracingStartFlag.store(false, std::memory_order_release);
		}

		void MoDARTTracing::computeEnergyContributions(int reverbDirectionIdx) {
			int pathIdx;
			Real distance;

			hemispherePencil.getDistances(rayDistances);
			hemispherePencil.getIndices(frontIndices, backIndices);
			// If self-shadowing is enabled, get the incidence cosines as well.
			if (SELF_SHADOWING_RADIUS > 0.0)
				hemispherePencil.getCosines(rayCosines);

			// Reset contributions to 0
			energyContributions.Reset();
			contributionDelays.Reset();

			for (int ray_idx = 0; ray_idx < numRays; ++ray_idx) {
				// Did the ray hit valid triangles on both sides?
				if ((frontIndices(ray_idx) == -1) || (backIndices(ray_idx) == -1))
					continue;

				// Does the ray fall within the bundle specified by `reverbDirectionIdx`?
				if ((reverbDirectionIdx == -1) || (reverbDirectionIdx == rayClusters(ray_idx))) {
					// Is the ray self-shadowed?
					if (SELF_SHADOWING_RADIUS > 0.0)
						if (rayCosines(ray_idx) < SELF_SHADOWING_RADIUS / (2 * rayDistances(ray_idx)))
							continue;

					// Add energy contribution of the ray (pathIndexing is from A to B; back to front)
					pathIdx = pathIndexing(backIndices(ray_idx), frontIndices(ray_idx));
					if (pathIdx >= 0)
					{
						distance = rayDistances(ray_idx);
						energyContributions(pathIdx) += 1;
						contributionDelays(pathIdx) += distance;
					}
				}
			}

			// Normalize by number of rays
			for (int path_idx = 0; path_idx < numPaths; ++path_idx) {
				if (energyContributions(path_idx) == 0)
					continue;

				// Turn the propagation distance (meters) into a propagation delay (seconds), and also take its average within each path.
				// N.B.: The distances are averaged using the number of hits, NOT the total number of rays.
				contributionDelays(path_idx) /= energyContributions(path_idx) * SPEED_OF_SOUND;
				// Normalize the portion of rays in the path, AFTER having used it to average their distances.
				if (reverbDirectionIdx == -1)
					energyContributions(path_idx) /= numRays;
				else
					energyContributions(path_idx) /= (numReverbDirections * clustersSizes(reverbDirectionIdx));
			}
		}

		void SingleFDNTracing::RunTracing() {
			PROFILE_ReverbRayTracing
#ifdef RTM_FLAG
				Debug::RTMStartFlag();
#endif
			tracingStartFlag.store(true, std::memory_order_release);

			// TODO: Should we only update residues relevant to currently active FDNs (i.e T60 > minimumT60)?
			lock_guard<std::mutex> lock(rayPencilMutex);
			shared_ptr<Room> sharedRoom = mRoom.lock();
			shared_ptr<Reverb> sharedReverb = mReverb.lock();

			bool listenerMoved = false;
			{
				lock_guard<std::mutex> lock(dataStoreMutex);
				if (mListenerPosition != mListenerPositionIncoming)
				{
					mListenerPosition = mListenerPositionIncoming;
					listenerMoved = true;
				}
			}
			if (listenerMoved)
			{
				hemispherePencil.moveOrigin(mListenerPosition);
				hemispherePencil.traceAll(sharedRoom->GetTriangleMeshSoA());

				MaterialMap materials = sharedRoom->GetMaterials();
				for (int dir_idx = 0; dir_idx < numReverbDirections; ++dir_idx)
				{
					ComputeEnergyContributions(materials, dir_idx);
#ifdef DEBUG_RTM
					Debug::send_path(IntToStr(dir_idx) + "l", { mListenerPosition }, reverbDirections[dir_idx]);
#endif
				}
				sharedReverb->SetTargetOutputFilters(reflectionGains);
			}
#ifdef RTM_FLAG
			Debug::RTMEndFlag();
#endif
			tracingEndFlag.store(true, std::memory_order_release);
			tracingStartFlag.store(false, std::memory_order_release);
		}

		void SingleFDNTracing::ComputeEnergyContributions(const MaterialMap& materials, int reverbDirectionIdx) {
			// Reset contributions to 0
			for (int dir_idx = 0; dir_idx < numReverbDirections; ++dir_idx)
				reflectionGains[reverbDirectionIdx].Reset();

			hemispherePencil.getDistances(rayDistances);
			hemispherePencil.getIndices(frontIndices, backIndices);
			// If self-shadowing is enabled, get the incidence cosines as well.
			if (SELF_SHADOWING_RADIUS > 0.0)
				hemispherePencil.getCosines(rayCosines);

			for (int ray_idx = 0; ray_idx < numRays; ++ray_idx) {
				// Did the ray hit valid triangles on both sides?
				if ((frontIndices(ray_idx) == -1))
					continue;

				// Does the ray fall within the bundle specified by `reverbDirectionIdx`?
				if ((reverbDirectionIdx == -1) || (reverbDirectionIdx == rayClusters(ray_idx))) {
					// Is the ray self-shadowed?
					if (SELF_SHADOWING_RADIUS > 0.0)
						if (rayCosines(ray_idx) < SELF_SHADOWING_RADIUS / (2 * rayDistances(ray_idx)))
							continue;

					// Add energy contribution of the ray
					auto it = materials.find(frontIndices(ray_idx));
					if (it == materials.end())
						continue;
					reflectionGains[reverbDirectionIdx] += it->second;
				}
			}

			// Normalize by number of rays
			if (reverbDirectionIdx == -1)
				reflectionGains[reverbDirectionIdx] /= static_cast<Real>(numRays);
			else
				reflectionGains[reverbDirectionIdx] /= static_cast<Real>(clustersSizes(reverbDirectionIdx));
		}
	}
}
