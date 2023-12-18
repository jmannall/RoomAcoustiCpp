
#include "Spatialiser/SourceManager.h"

using namespace Spatialiser;

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

/*vec3 SourceManager::GetSourcePosition(shared_ptr<Binaural::CSingleSourceDSP> source)
{
	Common::CTransform transform = source->GetCurrentSourceTransform();
	Common::CVector3 position = transform.GetPosition();
	return vec3(position.x, position.y, position.z);
}*/

void SourceManager::ProcessAudio(const size_t& id, const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer, const float lerpFactor)
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
