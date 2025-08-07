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

			// TODO: Initialize TriangleSoA& triangles; pointing towards the (existing) attribute of mRoom
			
			// TODO: Somehow initialize std::vector<std::vector<int>> pathIndexing (passed by Context as an argument to this constructor?)
			
			// TODO: Initialize RayPencil hemispherePencil; Is the number of rays passed by Context as an argument to this constructor? A global constant (or variable)? A compiler flag?
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

				// TODO: Compute energy contributions of each reverbDirection to each propagation path (using pathIndexing in combination with hemispherePencil.get_indices)
				
				// TODO: for (slope_idx = 0; slope_idx < mReverb.get_num_slopes(); ++slope_idx)
					// TODO: eigenvector = mReverb.get_right_eigenvector(slope_idx);
					// TODO: residues = zeros(reverbDirections.size());
					// TODO: for (dir_idx = 0; dir_idx < reverbDirections.size(); ++dir_idx)
						// TODO: residues[dir_idx] = dot(energy_contributions[dir_idx], eigenvector)
					// TODO: Return residues to FDN[slope_idx]
					// TODO: mReverb->UpdateListenerResidues(slope_idx, residues);
			}

			shared_ptr<SourceManager> sharedSource = mSourceManager.lock();
			sharedSource->ResetUnusedSources();
			mSources = sharedSource->GetSourceData();

			for (Source::Data& source : mSources) {
				if (source.hasChanged) {
					hemispherePencil.move_origin(source.position);
					hemispherePencil.trace_all(triangles);

					// TODO: Compute energy contributions to each propagation path (using pathIndexing in combination with hemispherePencil.get_indices)

					// TODO: residues = zeros(mReverb.get_num_slopes());
					// TODO: for (slope_idx = 0; slope_idx < mReverb.get_num_slopes(); ++slope_idx)
						// TODO: eigenvector = mReverb.get_left_eigenvector(slope_idx);
						// TODO: residues[slope_idx] = dot(energy contributions, eigenvector)
					// TODO: sharedSource->UpdateSourceResidues(source.id, residues);
				}
			}
		}
	}
}
