/*
* @brief Defines the RoomAcoustiCpp global context
*
*/

#ifndef RoomAcoustiCpp_Context_h
#define RoomAcoustiCpp_Context_h

// C++ headers
#include <thread>

// Common headers
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

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		/**
		* Global context for the spatialiser.
		* 
		* @details This class is the main interface for the spatialiser
		*/
		class Context
		{
		public:

			/**
			* Constructor that initialises the spatialiser with the given configuration.
			* 
			* @param config The configuration for the spatialiser.
			*/
			Context(const Config& config);

			/**
			* Default deconstructor.
			*/
			~Context();

			/**
			* Loads the HRTF, near field and ILD files from the given file paths.
			*
			* @param hrtfResamplingStep The resampling step for the HRTF files.
			* @param filePaths The file paths for the HRTF, near field and ILD files.
			* 
			* @return True if the files were loaded successfully, false otherwise.
			*/
			bool LoadSpatialisationFiles(const int hrtfResamplingStep, const std::vector<std::string>& filePaths);
				
			/**
			* Updates the spatialisation mode for each component of the spatialiser.
			*
			* @param mode The new spatialisation mode.
			*/
			void UpdateSpatialisationMode(const SpatMode mode);

			/**
			* Stop the spatialiser running.
			*/
			void StopRunning() { mIsRunning = false; }

			/**
			* Check if the spatialiser is running.
			* 
			* @return True if the spatialiser is running, false otherwise.
			*/
			bool IsRunning() const { return mIsRunning; }

			/**
			* Updates the image edge model (IEM) configuration.
			*
			* @param config The new IEM configuration.
			*/
			inline void UpdateIEMConfig(const IEMConfig& config) { mImageEdgeModel->UpdateIEMConfig(config); }

			/**
			* Updates the reverb time.
			*
			* @param T60 The new reverb time.
			*/
			inline void UpdateReverbTime(const Coefficients& T60) { mReverb->UpdateReverbTime(T60); }

			/**
			* Updates the reverb time model.
			*
			* @param model The new reverb time model.
			*/
			void UpdateReverbTimeModel(const ReverbTime model);

			/**
			* Updates the feedback delay network (FDN) model.
			*
			* @param model The new FDN model.
			*/
			inline void UpdateFDNModel(const FDNMatrix model) { mReverb->UpdateFDNModel(model); }

			/**
			* Updates the diffraction model.
			*
			* @param model The new diffraction model.
			*/
			void UpdateDiffractionModel(const DiffractionModel model);

			/**
			* Returns a pointer to the room class.
			* 
			* @return A pointer to the room class.
			*/
			inline std::shared_ptr<Room> GetRoom() { return mRoom; }

			/**
			* Returns a pointer to the image edge class.
			* 
			* @return A pointer to the image edge class.
			*/
			inline std::shared_ptr<ImageEdge> GetImageEdgeModel() { return mImageEdgeModel; }

			/**
			* Sets the room volume and dimensions.
			* 
			* @param volume The volume of the room used to predict the reverberation time.
			* @param dimensions The dimensions of the room used to set the FDN delay lines.
			*/
			void UpdateRoom(const Real volume, const Vec& dimensions);

			inline void ResetFDN() { mReverb->ResetFDN(); }

			/**
			* Update the listener position and orientation.
			* 
			* @param position The new position of the listener.
			* @param orientation The new orientation of the listener.
			*/
			void UpdateListener(const Vec3& position, const Vec4& orientation);

			/**
			* Initialises a new source in the spatialsier.
			* 
			* @return The ID of the new source.
			*/
			size_t InitSource();

			/**
			* Updates the position and orientation of a source.
			* 
			* @param id The ID of the source to update.
			* @param position The new position of the source.
			* @param orientation The new orientation of the source.
			*/
			void UpdateSource(size_t id, const Vec3& position, const Vec4& orientation);

			/**
			* Removes a source from the spatialiser.
			* 
			* @param id The ID of the source to remove.
			*/
			void RemoveSource(size_t id);

			/**
			* Initialises a new wall in the spatialsier.
			* 
			* @param normal The normal of the wall.
			* @param vData The vertices of the wall.
			* @param numVertices The number of vertices in the wall.
			* @param absorption The absorption of the wall.
			* 
			* @return The ID of the new wall.
			*/
			size_t InitWall(const Vec3& normal, const Real* vData, const Absorption& absorption);
			
			/**
			* Updates the position of a wall.
			* 
			* @param id The ID of the wall to update.
			* @param normal The new normal of the wall.
			* @param vData The new vertices of the wall.
			* @param numVertices The number of vertices in the wall.
			*/
			void UpdateWall(size_t id, const Vec3& normal, const Real* vData);

			/**
			* Updates the absorption of a wall.
			*
			* @param id The ID of the wall to update.
			* @param absorption The new absorption of the wall.
			*/
			void UpdateWallAbsorption(size_t id, const Absorption& absorption);

			/**
			* Removes a wall from the spatialiser.
			* 
			* @param id The ID of the wall to remove.
			*/
			void RemoveWall(size_t id);

			/**
			* Updates the planes and edges of the room.
			*/
			void UpdatePlanesAndEdges();

			/**
			* Sends an audio buffer to a source and adds the output to mOutputBuffer.
			* 
			* @param id The ID of the source to send the audio to.
			* @param data The audio buffer.
			*/
			void SubmitAudio(size_t id, const float* data);

			/**
			* Accesses the output of the spatialiser.
			*
			* Processes the reverberation and adds the output to mOutputBuffer.
			* This is copied to mSendBuffer and mOutuputBuffer and mReverbBuffer are reset.
			* 
			* @param bufferPtr Pointer to a float pointer that is pointed towards mSendBuffer.
			*/
			void GetOutput(float** bufferPtr);

		private:

			//////////////////// Member Variables ////////////////////

			/**
			* Spatialiser
			*/
			Config mConfig;			// RAC Config
			bool mIsRunning;		// Flag to check if the spatialiser is running
			std::thread IEMThread;	// Background thread to run the image edge model
			Vec3 listenerPosition;	// Stored listener position
			Real headRadius;		// Stored head radius from 3DTI

			/**
			* 3DTI components
			*/
			Binaural::CCore mCore;							// 3DTI core
			std::shared_ptr<Binaural::CListener> mListener;	// 3DTI listener

			/**
			* Audio buffers
			*/
			Buffer mInputBuffer;	// Audio input buffer
			Buffer mOutputBuffer;	// Audio output buffer
			Matrix mReverbInput;	// Audio reverb input matrix
			BufferF mSendBuffer;	// Audio send buffer (float)

			/**
			* Handles
			*/
			std::shared_ptr<Room> mRoom;				// Room class
			std::shared_ptr<Reverb> mReverb;			// Reverb class
			std::shared_ptr<SourceManager> mSources;	// Source manager class
			std::shared_ptr<ImageEdge> mImageEdgeModel;	// Image edge class

			std::mutex audioMutex; // Mutex for audio processing
		};
	}
}

#endif