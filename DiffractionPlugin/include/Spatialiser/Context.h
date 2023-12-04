#pragma once

#ifndef SPATIALISER_CONTEXT_H_
#define SPATIALISER_CONTEXT_H_

#include <thread>

#include "AudioManager.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/HRTFManager.h"
#include "Spatialiser/Reverb.h"

#include "Common/Transform.h"
#include "Debug.h"

namespace Spatialiser
{
	class Context
	{
	public:
		Context(const Config* config);
		~Context();

		bool IsRunning() const { return mIsRunning; }
		ISMConfig GetISMConfig() const{ return mISMConfig; }
		void UpdateISMConfig(const ISMConfig& config) { mISMConfig = config; }

		void StopRunning() { mIsRunning = false; }
		bool FilesLoaded();

		void UpdateListener(const vec3& position, const quaternion& orientation);

		size_t InitSource();
		void UpdateSource(size_t id, const vec3& position, const quaternion& orientation);
		void RemoveSource(size_t id);

		// int InitRoom(const Room& room);
		size_t InitWall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
		void UpdateWall(size_t id, const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
		void RemoveWall(size_t id, const ReverbWall& reverbWall);

		void SetFDNParameters(const float& volume , const vec& dimensions);

		void SubmitAudio(size_t id, const float* data, size_t numFrames);
		void GetOutput(float** bufferPtr);

		//void UpdateISM();

		// Getters
		HRTFManager* GetHRTFManager() { return mSources; }
		Room* GetRoom() { return mRoom; }
		//WedgeManager* GetWedgeManager() { return mWedges; }
		//SourceManager* GetSourceManager() { return mSources; }
		//PathManager* GetPathManager() { return mPaths; }
		//EmissionManager* GetEmissionManager() { return mEmissions; }

	private:
		Config mConfig;
		Binaural::CCore mCore;
		shared_ptr<Binaural::CListener> mListener;

		bool hrtfLoaded;
		bool ildLoaded;

		int mNumChannels;
		Buffer mInputBuffer;
		Buffer mOutputBuffer;
		matrix mReverbInput;
		Buffer mSendBuffer;

		char* mMem = nullptr;
		HRTFManager* mSources;
		Reverb* mReverb;

		std::mutex audioMutex;
		std::mutex roomMutex;
		std::mutex highPriority;
		std::mutex lowPriority;

		std::thread mBackgroundProcessor;	// background thread handle
		bool mIsRunning;
		ISMConfig mISMConfig;
		int count;

		Room* mRoom;
		//void SetOutputBuffer(float* buffer);
		//DSPConfig mConfig;				// copy of the user configuration
		//Model mModel;

		//int mNumFrames = 0;				// number of frames of audio data sent in this audio callback
		//int mNumChannels = 2;

		//// memory for all buffers allocated at once
		//	// stored in m_mem
		//char* mMem = nullptr;
		//unsigned mBufferSize;					// size in bytes of each buffer

		//Buffer mInputBuffer;
		//Buffer mOutputBuffer;
		//Buffer mAttenuateBuffer;
		//Buffer mOffBuffer;
		//Buffer mLpfBuffer;
		//Buffer mUdfaBuffer;
		//Buffer mUdfaiBuffer;
		//Buffer mNNBestBuffer;
		//Buffer mNNSmallBuffer;
		//Buffer mUtdBuffer;
		//Buffer mBtmBuffer;
		//Buffer mSendBuffer;

		//bool mIsRunning;
		//Receiver mListener;

		//std::thread mBackgroundProcessor;	// background thread handle
		//WedgeManager* mWedges;				// wedge handle
		//SourceManager* mSources;			// source handle
		//PathManager* mPaths;				// path handle
		//EmissionManager* mEmissions;		// emissions handle
	};
}

#endif  SPATIALISER_CONTEXT_H_