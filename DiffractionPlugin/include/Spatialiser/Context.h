/*
*
*  \Spatialiser context
*
*/

#ifndef Spatialiser_Context_h
#define Spatialiser_Context_h

// C++ headers
#include <thread>

// Common headers
#include "Common/AudioManager.h"
#include "Common/Matrix.h"
#include "Common/Vec.h"
#include "Common/Vec3.h"
#include "Common/Vec4.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/SourceManager.h"
#include "Spatialiser/Reverb.h"
#include "Spatialiser/Room.h"

// 3DTI Headers
#include "Common/Transform.h"
#include "BinauralSpatializer/3DTI_BinauralSpatializer.h"
#include "HRTF/HRTFFactory.h"
#include "HRTF/HRTFCereal.h"
#include "ILD/ILDCereal.h"

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Context ////////////////////

		class Context
		{
		public:

			// Load and Destroy
			Context(const Config* config);
			~Context();

			bool SetSpatialisationMode(const SPATConfig& config, const int& hrtfResamplingStep, const std::vector<std::string>& filePaths);

			// Image Source Model
			void StopRunning() { mIsRunning = false; }
			bool IsRunning() const { return mIsRunning; }

			void UpdateISMConfig(const ISMConfig& config) { mISMConfig = config; }
			ISMConfig GetISMConfig() const { return mISMConfig; }

			SourceManager* GetHRTFManager() { return mSources; }
			Room* GetRoom() { return mRoom; }

			//Reverb
			void UpdateRoom(const Real& volume, const vec& dimensions);

			// Listener
			void UpdateListener(const vec3& position, const vec4& orientation);

			// Source
			size_t InitSource();
			void UpdateSource(size_t id, const vec3& position, const vec4& orientation);
			void RemoveSource(size_t id);

			// Wall
			size_t InitWall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
			void UpdateWall(size_t id, const vec3& normal, const Real* vData, size_t numVertices);
			void FreeWallId(size_t id) { mRoom->FreeWallId(id); }
			void RemoveWall(size_t id);

			// Audio
			void SubmitAudio(size_t id, const float* data);
			void GetOutput(float** bufferPtr);

		private:

			// Configs
			Config mConfig;
			ISMConfig mISMConfig;

			// 3DTI components
			Binaural::CCore mCore;
			shared_ptr<Binaural::CListener> mListener;

			// Buffers
			Buffer mInputBuffer;
			Buffer mOutputBuffer;
			matrix mReverbInput;
			BufferF mSendBuffer;

			// Handles
			char* mMem = nullptr;
			SourceManager* mSources;
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
}

#endif