/*
*
*  \Spatialiser context
*
*/

#ifndef Spatialiser_Context_h
#define Spatialiser_Context_h

// C++ headers
#include <thread>

// Misc headers
#include "AudioManager.h"
#include "Debug.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/HRTFManager.h"
#include "Spatialiser/Reverb.h"

// 3DTI Headers
#include "Common/Transform.h"
#include "BinauralSpatializer/3DTI_BinauralSpatializer.h"
#include "HRTF/HRTFFactory.h"
#include "HRTF/HRTFCereal.h"
#include "ILD/ILDCereal.h"

namespace Spatialiser
{
	//////////////////// Context ////////////////////

	class Context
	{
	public:

		// Load and Destroy
		Context(const Config* config);
		~Context();

		bool FilesLoaded();

		// Image Source Model
		void StopRunning() { mIsRunning = false; }
		bool IsRunning() const { return mIsRunning; }

		void UpdateISMConfig(const ISMConfig& config) { mISMConfig = config; }
		ISMConfig GetISMConfig() const { return mISMConfig; }

		HRTFManager* GetHRTFManager() { return mSources; }
		Room* GetRoom() { return mRoom; }

		//Reverb
		void SetFDNParameters(const float& volume, const vec& dimensions);

		// Listener
		void UpdateListener(const vec3& position, const quaternion& orientation);

		// Source
		size_t InitSource();
		void UpdateSource(size_t id, const vec3& position, const quaternion& orientation);
		void RemoveSource(size_t id);

		// Wall
		size_t InitWall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
		void UpdateWall(size_t id, const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
		void RemoveWall(size_t id, const ReverbWall& reverbWall);

		// Audio
		void SubmitAudio(size_t id, const float* data, size_t numFrames);
		void GetOutput(float** bufferPtr);

	private:

		// Configs
		Config mConfig;
		ISMConfig mISMConfig;

		// 3DTI components
		Binaural::CCore mCore;
		shared_ptr<Binaural::CListener> mListener;

		// Bools TO DO: remove file loaded function
		bool hrtfLoaded;
		bool ildLoaded;

		// Buffers
		Buffer mOutputBuffer;
		matrix mReverbInput;
		Buffer mSendBuffer;

		// Handles
		char* mMem = nullptr;
		HRTFManager* mSources;
		Reverb* mReverb;
		Room* mRoom;

		// Mutexes
		std::mutex audioMutex;
		std::mutex roomMutex;
		std::mutex highPriority;
		std::mutex lowPriority;

		// Image Source Model
		std::thread ISMThread;
		bool mIsRunning;
	};
}

#endif