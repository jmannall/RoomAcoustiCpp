#include "Spatialiser/TracingThread.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		TracingThread::TracingThread(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const std::shared_ptr<Config>& config) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb),
			numReverbDirections(config->numReverbSources), numFDNs(config->GetNumRavesFDNs()),
			numRays(config->numRays), hemispherePencil(config->numRays, true),
			decayPerSecond(config->GetNumRavesFDNs()),
			sourceResidues(config->GetNumRavesFDNs()), listenerResidues(config->GetNumRavesFDNs(), Coefficients<>(config->numReverbSources))
		{
			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->GetReverbSourceDirections(reverbDirections);
			clusterReverbDirections();

			numPaths = 0;

			frontDistances = Vec<Real>(numRays, 0.0);
			backDistances = Vec<Real>(numRays, 0.0);
			frontCosines = Vec<Real>(numRays, 0.0);
			backCosines = Vec<Real>(numRays, 0.0);
			frontIndices = std::vector<int>(numRays, -1);
			backIndices = std::vector<int>(numRays, -1);
		}

		void TracingThread::setNumberOfRays(int newNumRays) {
			lock_guard<std::mutex> lock(rayPencilMutex);
			numRays = newNumRays;
			RayPencil hemispherePencil(numRays, true);
			clusterReverbDirections();

			frontDistances = Vec<Real>(numRays, 0.0);
			backDistances = Vec<Real>(numRays, 0.0);
			frontCosines = Vec<Real>(numRays, 0.0);
			backCosines = Vec<Real>(numRays, 0.0);
			frontIndices = std::vector<int>(numRays, -1);
			backIndices = std::vector<int>(numRays, -1);
		}

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

			energyContributions = Vec<Real>(numPaths, 0.0);
			contributionDelays = Vec<Real>(numPaths, 0.0);
			contributionDelayScaling = Vec<Real>(numPaths, 0.0);
		}

		void TracingThread::RunTracing() {
			// TODO: PROFILE_TracingThread

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
					}
				}

				for (int slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
					sharedReverb->SetTargetListenerResidues(slope_idx, listenerResidues[slope_idx]);
				}
			}

			mSources = sharedSource->GetSourceData();

			for (Source::Data& source : mSources) {
				if (source.hasChanged) {
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
					}
					sharedSource->SetSourceTargetResidues(source.id, sourceResidues);
				}
			}
		}

		void TracingThread::clusterReverbDirections() {
			// Re-allocate these in case the number of rays has changed.
			frontClusters = std::vector<int>(numRays, -1);
			backClusters = std::vector<int>(numRays, -1);

			hemispherePencil.clusterDirections(reverbDirections, frontClusters, backClusters);
		}

		void TracingThread::computeEnergyContributions(int reverbDirectionIdx) {
			int pathIdx;
			Real distance;

			hemispherePencil.getDistances(frontDistances, backDistances);
			hemispherePencil.getIndices(frontIndices, backIndices);
			// If self-shadowing is enabled, get the incidence cosines as well.
			if (SELF_SHADOWING_RADIUS > 0.0)
				hemispherePencil.getCosines(frontCosines, backCosines);

			// Reset contributions to 0
			for (int i = 0; i < numPaths; ++i) {
				energyContributions[i] = 0;
				contributionDelays[i] = 0;
			}

			for (int i = 0; i < numRays; ++i) {
				// Did the ray hit valid itriangles on both sides?
				if ((frontIndices[i] == -1) || (backIndices[i] == -1))
					continue;

				if ((reverbDirectionIdx == -1) || (reverbDirectionIdx == frontClusters[i])) {
					// Is the front ray self-shadowed?
					if (SELF_SHADOWING_RADIUS > 0.0)
						if (frontCosines[i] < SELF_SHADOWING_RADIUS / (2 * frontDistances[i]))
							continue;

					// Contribution of front ray
					pathIdx = pathIndexing[frontIndices[i]][backIndices[i]];
					distance = frontDistances[i];
					energyContributions[pathIdx] += 1;
					contributionDelays[pathIdx] += distance;
				}

				if ((reverbDirectionIdx == -1) || (reverbDirectionIdx == backClusters[i])) {
					// Is the back ray self-shadowed?
					if (SELF_SHADOWING_RADIUS > 0.0)
						if (backCosines[i] < SELF_SHADOWING_RADIUS / (2 * backDistances[i]))
							continue;

					// Contribution of back ray
					pathIdx = pathIndexing[backIndices[i]][frontIndices[i]];
					distance = backDistances[i];
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
