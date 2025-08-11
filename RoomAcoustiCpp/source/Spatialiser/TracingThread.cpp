#include "Spatialiser/TracingThread.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		// TODO: Make all other required modifications of Context.h/.cpp

		TracingThread::TracingThread(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const std::shared_ptr<Config>& config) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb),
			numReverbDirections(config->numReverbSources), numFDNs(config->numRavesFDNs),
			numRays(config->numRays), hemispherePencil(config->numRays, true),
			sourceResidues(config->numRavesFDNs), listenerResidues(config->numReverbSources)
		{
			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->GetReverbSourceDirections(reverbDirections);
			clusterReverbDirections();
		}

		void TracingThread::setNumberOfRays(int newNumRays) {
			// TODO: Prevent data races.
			numRays = newNumRays;
			RayPencil hemispherePencil(numRays, true);
			clusterReverbDirections();
		}

		void TracingThread::InitRoom(const std::vector<std::vector<int>>& indexing, const Vec<>& decayRates) {
			shared_ptr<Room> sharedRoom = mRoom.lock();
			sharedRoom->CreateTriangleMeshSoA();

			assert(decayRates.Rows() == numFDNs);
			pathIndexing = indexing;
			decayPerSecond = decayRates;
		}

		void TracingThread::RunTracing() {
			// TODO: PROFILE_TracingThread

			shared_ptr<Room> sharedRoom = mRoom.lock();
			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			shared_ptr<SourceManager> sharedSource = mSourceManager.lock();

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

				for (int slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
					for (int dir_idx = 0; dir_idx < numReverbDirections; ++dir_idx) {
						computeEnergyContributions(dir_idx);
						// TODO: Double-check theory: is any different normalization required (e.g. normalize by path etendue)? If constant, bake it into the eigenvectors; if not, implement it here.
						listenerResidues[dir_idx] = Dot(energyContributions, sharedReverb->GetRightEigenvector(slope_idx));
					}
					sharedReverb->SetTargetListenerResidues(slope_idx, listenerResidues);
				}
			}

			mSources = sharedSource->GetSourceData();

			for (Source::Data& source : mSources) {
				if (source.hasChanged) {
					hemispherePencil.moveOrigin(source.position);
					hemispherePencil.traceAll(sharedRoom->GetTriangleMeshSoA());

					computeEnergyContributions();
					// TODO: Double-check theory: is any different normalization required (e.g. normalize by path etendue)? If constant, bake it into the eigenvectors; if not, implement it here.

					for (int slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
						sourceResidues[slope_idx] = Dot(energyContributions, sharedReverb->GetLeftEigenvector(slope_idx));
					}
					sharedSource->SetSourceTargetResidues(source.id, sourceResidues);
				}
			}
		}

		void TracingThread::clusterReverbDirections() {
			// Re-allocate these in case the number of rays has changed.
			std::vector<int> frontClusters(numRays, -1);
			std::vector<int> backClusters(numRays, -1);

			hemispherePencil.clusterDirections(reverbDirections, frontClusters, backClusters);
		}

		void TracingThread::computeEnergyContributions(int reverbDirectionIdx) {
			int pathIdx;
			std::vector<int> frontIndices(numRays, -1);
			std::vector<int> backIndices(numRays, -1);

			hemispherePencil.getIndices(frontIndices, backIndices);

			// Reset contributions to 0
			for (int i = 0; i < numPaths; ++i) {
				// TODO: It would be nice to overload the = operator for element-wise Vec definition.
				energyContributions[i] = 0;
			}

			for (int i = 0; i < numRays; ++i) {
				if ((reverbDirectionIdx == -1) || (reverbDirectionIdx == frontClusters[i])) {
					// Contribution of front ray
					pathIdx = pathIndexing[frontIndices[i]][backIndices[i]];
					energyContributions[pathIdx] += 1;
				}

				if ((reverbDirectionIdx == -1) || (reverbDirectionIdx == backClusters[i])) {
					// Contribution of back ray
					pathIdx = pathIndexing[backIndices[i]][frontIndices[i]];
					energyContributions[pathIdx] += 1;
				}
			}

			// Normalize by number of rays
			for (int i = 0; i < numPaths; ++i) {
				// TODO: It would be nice to overload the / operator for element-wise Vec division, as already done for Vec3.
				energyContributions[i] /= numRays;
			}
		}
	}
}
