#pragma once

#include "AudioManager.h"
#include "WedgeManager.h"
#include "SourceManager.h"
#include "EmissionManager.h"
#include <thread>

class Geometry
{
public:
	Geometry(const DSPConfig* config);
	~Geometry();

	bool IsRunning() const { return mIsRunning; }
	void StopRunning() { mIsRunning = false; }

	void SetListenerPosition(const vec3& listenerPosition) { mListener.position = listenerPosition; }
	vec3 GetListenerPosition() { return mListener.position; }
	void SetModel(const Model& model) { mModel = model; }

	void SubmitAudio(size_t sID, size_t wID, const float* data, unsigned numFrames);
	void GetOutput(float** bufferPtr);

	size_t InitSource(const vec3& position) { return mSources->Init(position); };
	void RemoveSource(size_t id) { mSources->Remove(id); }
	void UpdateSourceData(size_t id, const vec3& position);
	//Source& GetSourceData(size_t id) { Source& source = mSources->GetData(id); }

	size_t InitWedge(const Wedge& wedge) { return mWedges->Init(wedge); }
	void RemoveWedge(size_t id) { mWedges->Remove(id); }
	void UpdateWedgeData(size_t id, const Wedge& newWedge);
	//Wedge& GetWedgeData(size_t id) { Wedge& wedge = mWedges->GetData(id); }
	float GetZ(size_t sID, size_t wID);

	float GetSd(size_t sID, size_t wID);
	float GetRd(size_t sID, size_t wID);

	void UpdatePaths();

	// Getters
	WedgeManager* GetWedgeManager() { return mWedges; }
	SourceManager* GetSourceManager() { return mSources; }
	PathManager* GetPathManager() { return mPaths; }
	EmissionManager* GetEmissionManager() { return mEmissions; }

private:
	void SetOutputBuffer(float* buffer);
	DSPConfig mConfig;				// copy of the user configuration
	Model mModel;

	int mNumFrames = 0;				// number of frames of audio data sent in this audio callback
	int mNumChannels = 2;

	// memory for all buffers allocated at once
		// stored in m_mem
	char* mMem = nullptr;
	unsigned mBufferSize;					// size in bytes of each buffer

	Buffer mInputBuffer;
	Buffer mOutputBuffer;
	Buffer mAttenuateBuffer;
	Buffer mOffBuffer;
	Buffer mLpfBuffer;
	Buffer mUdfaBuffer;
	Buffer mUdfaiBuffer;
	Buffer mNNBestBuffer;
	Buffer mNNSmallBuffer;
	Buffer mUtdBuffer;
	Buffer mBtmBuffer;
	Buffer mSendBuffer;
	   
	bool mIsRunning;
	Receiver mListener;

	std::thread mBackgroundProcessor;	// background thread handle
	WedgeManager* mWedges;				// wedge handle
	SourceManager* mSources;			// source handle
	PathManager* mPaths;				// path handle
	EmissionManager* mEmissions;		// emissions handle
};