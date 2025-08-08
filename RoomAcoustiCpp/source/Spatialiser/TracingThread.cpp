#include "Spatialiser/TracingThread.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		// TODO: Prepare Python data from existing 3-room environment for testing

		// TODO: Initialize TracingThread in the constructor of Context.cpp

		// TODO: Handle the tracing thread in a function "void TracingProcessor(Context* context)" in Context.cpp

		TracingThread::TracingThread(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const std::shared_ptr<Config>& config) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb), numReverbDirections(config->numReverbSources), numFDNs(config->numRavesFDNs)
		{
			// TODO: Initialize `numRays` taking the value from somewhere. Passed as argument? new Config class? Global constant DEFAULT_NUM_RAYS?
			numRays = 1000;
			RayPencil hemispherePencil(*new Vec3, numRays, true);

			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->GetReverbSourceDirections(reverbDirections);
			clusterReverbDirections();

			shared_ptr<Room> sharedRoom = mRoom.lock();
			sharedRoom->GetTriangleMeshSoA(triangles);

			// TODO: Initialize `numPaths` and `pathIndexing` taking the values from somewhere. Passed as arguments?
			numPaths = 1;
			std::vector<std::vector<int>> pathIndexing(1, std::vector<int>(1, 0));

			Vec<Real> energyContributions(numPaths, 0.0);
			// TODO: How are Coefficients initialized?
			Coefficients<Real> sourceResidues(numFDNs, 0.0);
			Coefficients<Real> listenerResidues(numReverbDirections, 0.0);
		}

		void TracingThread::setNumberOfRays(int newNumRays) {
			numRays = newNumRays;
			RayPencil hemispherePencil(*new Vec3, numRays, true);
			clusterReverbDirections();
		}

		void TracingThread::RunTracing() {
			// TODO: PROFILE_TracingThread

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
				hemispherePencil.traceAll(triangles);

				for (int slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
					for (int dir_idx = 0; dir_idx < numReverbDirections; ++dir_idx) {
						computeEnergyContributions(dir_idx);
						// TODO: Double-check theory: is any different normalization required (e.g. normalize by path etendue)? If constant, bake it into the eigenvectors; if not, implement it here.
						listenerResidues[dir_idx] = Dot(energyContributions, mReverb.getRightEigenvector(slope_idx));
					}
					shared_ptr<Reverb> sharedReverb = mReverb.lock();
					sharedReverb->SetTargetListenerResidues(slope_idx, listenerResidues);
					// TODO: is sharedReverb unlocked automatically?
				}
			}

			shared_ptr<SourceManager> sharedSource = mSourceManager.lock();
			sharedSource->ResetUnusedSources();
			mSources = sharedSource->GetSourceData();
			// TODO: is sharedSource unlocked automatically?

			for (Source::Data& source : mSources) {
				if (source.hasChanged) {
					hemispherePencil.moveOrigin(source.position);
					hemispherePencil.traceAll(triangles);

					computeEnergyContributions();
					// TODO: Double-check theory: is any different normalization required (e.g. normalize by path etendue)? If constant, bake it into the eigenvectors; if not, implement it here.

					for (int slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
						sourceResidues[slope_idx] = Dot(energyContributions, mReverb.getLeftEigenvector(slope_idx));
					}
					shared_ptr<SourceManager> sharedSource = mSourceManager.lock();
					sharedSource->SetSourceTargetResidues(source.id, sourceResidues);
					// TODO: is sharedSource unlocked automatically?
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
				energyContributions[i] /= numRays;
			}
		}
	}
}
