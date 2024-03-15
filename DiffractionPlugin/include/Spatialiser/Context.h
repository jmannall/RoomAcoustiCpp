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
#include "Spatialiser/ImageEdge.h"

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
			Context(const Config& config);
			~Context();

			bool LoadSpatialisationFiles(const int& hrtfResamplingStep, const std::vector<std::string>& filePaths);
			void UpdateSpatialisationMode(const SPATConfig& config);

			// Image Source Model
			void StopRunning() { mIsRunning = false; }
			bool IsRunning() const { return mIsRunning; }

			inline void UpdateISMConfig(const ISMConfig& config) { mImageEdgeModel->UpdateISMConfig(config); }
			void UpdateReverbTimeModel(const ReverbTime& model);
			void UpdateFDNModel(const FDNMatrix& model);

			std::shared_ptr<Room> GetRoom() { return mRoom; }
			std::shared_ptr<ImageEdge> GetImageEdgeModel() { return mImageEdgeModel; }
			//Reverb
			void UpdateRoom(const Real& volume, const vec& dimensions);

			// Listener
			void UpdateListener(const vec3& position, const vec4& orientation);

			// Source
			size_t InitSource();
			void UpdateSource(size_t id, const vec3& position, const vec4& orientation);
			void RemoveSource(size_t id);

			// Wall
			size_t InitWall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption);
			void UpdateWall(size_t id, const vec3& normal, const Real* vData, size_t numVertices);
			void FreeWallId(size_t id) { mRoom->FreeWallId(id); }
			void RemoveWall(size_t id);
			void UpdatePlanesAndEdges();

			// Audio
			void SubmitAudio(size_t id, const float* data);
			void GetOutput(float** bufferPtr);

			inline void GetWallVertices(int id, float** wallVertices)
			{
				mRoom->GetWallVertices(id, wallVertices);
			}

		private:

			// Configs
			Config mConfig;

			// 3DTI components
			Binaural::CCore mCore;
			std::shared_ptr<Binaural::CListener> mListener;

			// Buffers
			Buffer mInputBuffer;
			Buffer mOutputBuffer;
			matrix mReverbInput;
			BufferF mSendBuffer;

			// Handles
			std::shared_ptr<Room> mRoom;
			std::shared_ptr<Reverb> mReverb;
			std::shared_ptr<SourceManager> mSources;
			std::shared_ptr<ImageEdge> mImageEdgeModel;

			// Image Source Model
			std::thread ISMThread;
			bool mIsRunning;
		};
	}
}

#endif