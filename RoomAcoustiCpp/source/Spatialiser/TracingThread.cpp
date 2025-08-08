#include "Spatialiser/TracingThread.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		// TODO: Add a helper function `void Dot(const Vec<>& a, const Vec<>& b, Vec<>& out)`

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
			// TODO: How are Coefficients initialized????
			Coefficients<Real> sourceResidues(numFDNs, 0.0);
			Coefficients<Real> listenerResidues(numReverbDirections, 0.0);
			//sourceResidues = Coefficients<Real>(numFDNs, 0.0);
			//listenerResidues = Coefficients<Real>(numReverbDirections, 0.0);
			//sourceResidues = Coefficients(std::vector<Real>(numFDNs, 0.0));
			//listenerResidues = Coefficients(std::vector<Real>(numReverbDirections, 0.0));
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

				for (slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
					// TODO: eigenvector = mReverb.get_right_eigenvector(slope_idx);
					// TODO: residues = zeros(reverbDirections.size());
					for (dir_idx = 0; dir_idx < numReverbDirections; ++dir_idx) {
						computeEnergyContributions(dir_idx);
						// TODO: Check if any different normalization is required (e.g. normalize by path etendue); if constant, bake it into the eigenvectors; if not, implement it here.
						// TODO: residues[dir_idx] = dot(energy_contributions[dir_idx], eigenvector)
					}
				// TODO: Return residues to FDN[slope_idx]
				// TODO: sharedReverb->SetTargetListenerResidues(slope_idx, residues)
				}
			}

			shared_ptr<SourceManager> sharedSource = mSourceManager.lock();
			sharedSource->ResetUnusedSources();
			mSources = sharedSource->GetSourceData();

			for (Source::Data& source : mSources) {
				if (source.hasChanged) {
					hemispherePencil.moveOrigin(source.position);
					hemispherePencil.traceAll(triangles);

					computeEnergyContributions();
					// TODO: Check if any different normalization is required (e.g. normalize by path etendue); if constant, bake it into the eigenvectors; if not, implement it here.

					// TODO: residues = zeros(mReverb.get_num_slopes());
					for (slope_idx = 0; slope_idx < numFDNs; ++slope_idx) {
						// TODO: eigenvector = mReverb.get_left_eigenvector(slope_idx);
						// TODO: residues[slope_idx] = dot(energy contributions, eigenvector)
					}
					// TODO: sharedSource->SetSourceTargetResidues(source.id, residues)
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
