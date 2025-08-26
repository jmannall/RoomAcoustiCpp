#include "Spatialiser/TracingThread.h"

// Common headers
#include "Common/RACProfiler.h"

// Unity headers
#include "Unity/Debug.h"

namespace RAC
{
	using namespace Unity;
	using namespace Common;
	namespace Spatialiser
	{
		TracingThread::TracingThread(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const std::shared_ptr<Config>& config) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb),
			numReverbDirections(config->numReverbSources), numFDNs(config->GetNumRavesFDNs()),
			decayPerSecond(config->GetNumRavesFDNs()),
			sourceResidues(config->GetNumRavesFDNs()),
			listenerResidues(config->GetNumRavesFDNs(), Coefficients<>(config->numReverbSources))
		{
			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->GetReverbSourceDirections(reverbDirections);
			clusterReverbDirections();

			numPaths = 0;

			// Note: the number of rays is forced to be even, because `hemispherePencil` actually uses half as many rays under the hood.
			if (config->numRays % 2 == 0)
				numRays = config->numRays;
			else
				numRays = config->numRays + 1;

			hemispherePencil = RayPencil(numRays / 2, true);

			rayDistances = Vec<Real>(numRays);
			rayCosines = Vec<Real>(numRays);
			frontIndices = std::vector<int>(numRays, -1);
			backIndices = std::vector<int>(numRays, -1);
			rayClusters = std::vector<int>(numRays, -1);
		}

		void TracingThread::setNumberOfRays(int newNumRays) {
			lock_guard<std::mutex> lock(rayPencilMutex);

			// Note: the number of rays is forced to be even, because `hemispherePencil` actually uses half as many rays under the hood.
			if (newNumRays % 2 == 0)
				numRays = newNumRays;
			else
				numRays = newNumRays + 1;

			RayPencil hemispherePencil(numRays / 2, true);

			clusterReverbDirections();

			rayDistances = Vec<Real>(numRays);
			rayCosines = Vec<Real>(numRays);
			frontIndices = std::vector<int>(numRays, -1);
			backIndices = std::vector<int>(numRays, -1);
			rayClusters = std::vector<int>(numRays, -1);
		}

		// TODO: Make ray tracing frequency dependent
		void TracingThread::InitRoom(int paths, const std::vector<std::vector<int>>& indexing, const Vec<>& decayRates) {
			lock_guard<std::mutex> lock(rayPencilMutex);
			shared_ptr<Room> sharedRoom = mRoom.lock();

			sharedRoom->CreateTriangleMeshSoA();

			numPaths = paths;
			numFDNs = decayRates.Rows();

			assert(decayRates.Rows() == numFDNs);
			pathIndexing = indexing;
			decayPerSecond = decayRates;
			sourceResidues = Coefficients<>(numFDNs);
			listenerResidues.resize(numFDNs, Coefficients<>(numReverbDirections));

			energyContributions = Vec<Real>(numPaths);
			contributionDelays = Vec<Real>(numPaths);
			contributionDelayScaling = Vec<Real>(numPaths);
		}

		void TracingThread::RunTracing() {
			PROFILE_ReverbRayTracing
#ifdef RTM_FLAG
			Debug::RTMStartFlag();
#endif

			lock_guard<std::mutex> lock(rayPencilMutex);
			shared_ptr<Room> sharedRoom = mRoom.lock();
			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			shared_ptr<SourceManager> sharedSource = mSourceManager.lock();

			if (numPaths == 0)
				return;

			bool listenerMoved = false;
			{
				lock_guard<std::mutex> lock(dataStoreMutex);
				if (mListenerPosition != mListenerPositionStore)
				{
					mListenerPosition = mListenerPositionStore;
					listenerMoved = true;
				}
			}
			if (listenerMoved) {
				hemispherePencil.moveOrigin(mListenerPosition);
				hemispherePencil.traceAll(sharedRoom->GetTriangleMeshSoA());

				for (int dir_idx = 0; dir_idx < numReverbDirections; ++dir_idx) {
					computeEnergyContributions(dir_idx);
					for (int slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
						for (int path_idx = 0; path_idx < numPaths; ++path_idx) {
							contributionDelayScaling[path_idx] = std::pow(decayPerSecond[slope_idx], -contributionDelays[path_idx]);
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
							contributionDelayScaling[path_idx] = std::pow(decayPerSecond[slope_idx], -contributionDelays[path_idx]);
						}

						sourceResidues[slope_idx] = 
							ThreeWayDot(
								energyContributions,
								contributionDelayScaling,
								sharedReverb->GetLeftEigenvector(slope_idx));

						// Compensate gain based on preceding delay.
						sourceResidues[slope_idx] *= std::pow(decayPerSecond[slope_idx], sharedReverb->GetPrecedingDelay());

#ifdef DEBUG_RTM
						Debug::send_residue(sourceResidues[slope_idx], true, source.id, slope_idx);
#endif
					}
					sharedSource->SetSourceTargetResidues(source.id, sourceResidues);
				}
			}
#ifdef RTM_FLAG
			Debug::RTMEndFlag();
#endif
		}

		void TracingThread::clusterReverbDirections() {
			hemispherePencil.clusterDirections(reverbDirections, rayClusters);
		}

		void TracingThread::computeEnergyContributions(int reverbDirectionIdx) {
			int pathIdx;
			Real distance;

			hemispherePencil.getDistances(rayDistances);
			hemispherePencil.getIndices(frontIndices, backIndices);
			// If self-shadowing is enabled, get the incidence cosines as well.
			if (SELF_SHADOWING_RADIUS > 0.0)
				hemispherePencil.getCosines(rayCosines);

			// Reset contributions to 0
			for (int i = 0; i < numPaths; ++i) {
				energyContributions[i] = 0;
				contributionDelays[i] = 0;
			}

			for (int i = 0; i < numRays; ++i) {
				// Did the ray hit valid triangles on both sides?
				if ((frontIndices[i] == -1) || (backIndices[i] == -1))
					continue;

				// Does the ray fall within the bundle specified by `reverbDirectionIdx`?
				if ((reverbDirectionIdx == -1) || (reverbDirectionIdx == rayClusters[i])) {
					// Is the ray self-shadowed?
					if (SELF_SHADOWING_RADIUS > 0.0)
						if (rayCosines[i] < SELF_SHADOWING_RADIUS / (2 * rayDistances[i]))
							continue;

					// Add energy contribution of the ray
					pathIdx = pathIndexing[frontIndices[i]][backIndices[i]];
					distance = rayDistances[i];
					energyContributions[pathIdx] += 1;
					contributionDelays[pathIdx] += distance;
				}
			}

			// Normalize by number of rays
			for (int i = 0; i < numPaths; ++i) {
				if (energyContributions[i] == 0)
					continue;

				// Turn the propagation distance (meters) into a propagation delay (seconds), and also take its average within each path.
				// N.B.: The distances are averaged using the number of hits, NOT the total number of rays.
				contributionDelays[i] /= energyContributions[i] * SPEED_OF_SOUND;
				// Normalize the portion of rays in the path, AFTER having used it to average their distances.
				energyContributions[i] /= numRays;
			}
		}
	}
}
