#include "Spatialiser/TracingThread.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		// TODO: Add a helper function `void Dot(const Vec<>& a, const Vec<>& b, Vec<>& out)`

		// TODO: Initialize TracingThread in the constructor of Context.cpp

		// TODO: Handle the tracing thread in a function "void TracingProcessor(Context* context)" in Context.cpp

		TracingThread::TracingThread(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb)
		{
			// TODO: Initialize `int numRays`
			// TODO: Initialize `RayPencil hemispherePencil`

			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->GetReverbSourceDirections(reverbDirections);
			numReverbDirections = reverbDirections.size();
			// TODO: Initialize `std::vector<int> frontClusters, backClusters` by calling `clusterReverbDirections()`

			shared_ptr<Room> sharedRoom = mRoom.lock();
			sharedRoom->GetTriangleMeshSoA(triangles);

			// TODO: Initialize `int numPaths`
			// TODO: Initialize `std::vector<std::vector<int>> pathIndexing`

			// TODO: Allocate `Vec<Real> energyContributions` with size `numPaths`
			// TODO: Allocate `Coefficients<Real> sourceResidues` with size `numFDNs`
			// TODO: Allocate `Coefficients<Real> listenerResidues` with size `numReverbDirections`
		}

		void TracingThread::setNumberOfRays(int numRays) {
			// TODO: Define this.
			//		Don't forget to call `clusterReverbDirections()` afterwards.
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
				hemispherePencil.move_origin(mListenerPosition);
				hemispherePencil.traceAll(triangles);

				// TODO: Compute energy contributions of each reverbDirection to each propagation path (using pathIndexing in combination with hemispherePencil.getIndices)

				// TODO: for (slope_idx = 0; slope_idx < mReverb.get_num_slopes(); ++slope_idx)
					// TODO: eigenvector = mReverb.get_right_eigenvector(slope_idx);
					// TODO: residues = zeros(reverbDirections.size());
					// TODO: for (dir_idx = 0; dir_idx < reverbDirections.size(); ++dir_idx)
						// TODO: residues[dir_idx] = dot(energy_contributions[dir_idx], eigenvector)
					// TODO: Return residues to FDN[slope_idx]
					// TODO: sharedReverb->SetTargetListenerResidues(slope_idx, residues)
			}

			shared_ptr<SourceManager> sharedSource = mSourceManager.lock();
			sharedSource->ResetUnusedSources();
			mSources = sharedSource->GetSourceData();

			for (Source::Data& source : mSources) {
				if (source.hasChanged) {
					hemispherePencil.move_origin(source.position);
					hemispherePencil.traceAll(triangles);

					// TODO: Compute energy contributions to each propagation path (using pathIndexing in combination with hemispherePencil.getIndices)

					// TODO: residues = zeros(mReverb.get_num_slopes());
					// TODO: for (slope_idx = 0; slope_idx < mReverb.get_num_slopes(); ++slope_idx)
						// TODO: eigenvector = mReverb.get_left_eigenvector(slope_idx);
						// TODO: residues[slope_idx] = dot(energy contributions, eigenvector)
					// TODO: sharedSource->SetSourceTargetResidues(source.id, residues)
				}
			}
		}

		void TracingThread::clusterReverbDirections() {
			// TODO: Define this.
		}

		void TracingThread::computeEnergyContributions(int reverbDirectionIdx, Vec<>& contributions) {
			// TODO: Define this.
		}
	}
}
