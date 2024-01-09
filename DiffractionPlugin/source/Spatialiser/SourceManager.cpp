/*
*
*  \Source manager class
*
*/

// C++ headers
#include <unordered_map>
#include <mutex>

// Common headers
#include "Common/Types.h" 

// Spatialiser headers
#include "Spatialiser/Source.h"
#include "Spatialiser/SourceManager.h"

namespace UIE
{
	namespace Spatialiser
	{

		//////////////////// SourceManager class ////////////////////

		size_t SourceManager::Init()
		{
			lock_guard <mutex> lock(updateMutex);
			Source source = Source(mCore, mNumFDNChannels, mHRTFMode, sampleRate);
			if (!mEmptySlots.empty()) // Assign source to an existing ID
			{
				size_t next = mEmptySlots.back();
				mEmptySlots.pop_back();
				mSources.insert_or_assign(next, source);
				source.Deactivate();
				return next;
			}
			else // Create a new ID
			{
				size_t next = mSources.size();
				mSources.insert_or_assign(next, source);
				source.Deactivate();
				return next;
			}
		}

		void SourceManager::ProcessAudio(const size_t& id, const Real* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer, const Real lerpFactor)
		{
			lock_guard<mutex> lock(processAudioMutex);
			auto it = mSources.find(id);
			if (it == mSources.end())
			{
				// Source doesn't exist
			}
			else
			{
				it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer, lerpFactor);
			}
		}
	}
}