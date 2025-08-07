#include "Spatialiser/TracingThread.h"
namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		TracingThread::TracingThread(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb) :
			mRoom(room), mSourceManager(sourceManager), mReverb(reverb) // TODO: Am I doing this right?
		{
			shared_ptr<Reverb> sharedReverb = mReverb.lock();
			sharedReverb->GetReverbSourceDirections(reverbDirections);

			// TODO: Set the pointer TriangleSoA& triangles
			// TODO: Somehow initialize std::vector<std::vector<int>> pathIndexing (passed as argument to this constructor?)
			// TODO: Initialize RayPencil hemispherePencil (Is the number of rays passed by Context? A global constant (or variable)? A compiler flag?
		}

		void TracingThread::RunTracing()
		{
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
				hemispherePencil.trace_all(triangles);

				// TODO: Compute energy contributions per propagation path, per reverbDirection
				
				// TODO: For each slope in mReverb,
					// TODO: Get reference to right eigenvector
					// TODO: For each reverbDirection,
						// TODO: residue = dot(energy contributions, eigenvector)
					// TODO: Return all directions' residues to the FDN
			}

			shared_ptr<SourceManager> sharedSource = mSourceManager.lock();
			sharedSource->ResetUnusedSources();
			mSources = sharedSource->GetSourceData();

			for (Source::Data& source : mSources) {
				if (source.hasChanged) {
					hemispherePencil.move_origin(source.position);
					hemispherePencil.trace_all(triangles);

					// TODO: Compute energy contributions per propagation path

					// TODO: For each slope in mReverb,
						// TODO: Get reference to left eigenvector
						// TODO: residue = dot(energy contributions, eigenvector)
					// TODO: sharedSource->UpdateSourceResidues(source.id, residues);
				}
			}
		}
	}
}
