/*
* @class Context
* 
* @brief Declaration of the Context class
*
* @details Defines the RoomAcoustiCpp global context
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
#include "Spatialiser/HeadphoneEQ.h"

// 3DTI Headers
#include "Common/Transform.h"
#include "BinauralSpatializer/3DTI_BinauralSpatializer.h"


#include <chrono>
#include <filesystem>
#include <ctime>

inline std::string GetTimestamp() {
	// Get current time
	auto now = std::chrono::system_clock::now();
	std::time_t time_now = std::chrono::system_clock::to_time_t(now);

	std::tm local_time;
	localtime_s(&local_time, &time_now);

	// Format time into string: YYYY-MM-DD_HH-MM-SS
	std::stringstream ss;
	ss << std::put_time(&local_time, "%Y-%m-%d_%H-%M-%S");
	std::string timestamp = ss.str();

	// Full log file path
	return timestamp;
}

inline std::string GetLogPath(std::string timestamp)
{
	return timestamp + "_RoomAcoustiCpp_log.txt";
}

inline std::string GetProfilePath(std::string timestamp)
{
	return timestamp + "_RoomAcoustiCpp_profile.txt";
}

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		/**
		* @brief Global context for the spatialiser.
		* 
		* @details This class is the main interface for the spatialiser
		*/
		class Context
		{
		public:

			/**
			* @brief Constructor that initialises the spatialiser with the given configuration.
			* 
			* @param config The configuration for the spatialiser.
			*/
			Context(const std::shared_ptr<Config> config);

			/**
			* @brief Default deconstructor.
			*/
			~Context();

			/**
			* @brief Loads the HRTF, near field and ILD files from the given file paths.
			*
			* @param hrtfResamplingStep The resampling step for the HRTF files.
			* @param filePaths The file paths for the HRTF, near field and ILD files.
			* 
			* @return True if the files were loaded successfully, false otherwise.
			*/
			bool LoadSpatialisationFiles(const int hrtfResamplingStep, const std::vector<std::string>& filePaths);
				
			/**
			* @brief Sets the headphone EQ filters.
			*
			* @param leftIR The impulse response for the left channel.
			* @param rightIR The impulse response for the right channel.
			*/
			inline void SetHeadphoneEQ(const Buffer<>& leftIR, const Buffer<>& rightIR) { headphoneEQ.SetFilters(leftIR, rightIR); applyHeadphoneEQ = true; }

			/**
			* @brief Updates the spatialisation mode for each component of the spatialiser.
			*
			* @param mode The new spatialisation mode.
			*/
			void UpdateSpatialisationMode(const SpatialisationMode mode);

			/**
			* @brief Stop the spatialiser running.
			*/
			void StopRunning() { mIsRunning.store(false, std::memory_order_release); }

			/**
			* @brief Check if the spatialiser is running.
			* 
			* @return True if the spatialiser is running, false otherwise.
			*/
			bool IsRunning() const { return mIsRunning.load(std::memory_order_acquire); }

			/**
			* @brief Updates the image edge model (IEM) configuration.
			*
			* @param config The new IEM configuration.
			*/
			inline void UpdateIEMConfig(const IEMConfig& config) { mImageEdgeModel->UpdateIEMConfig(config); }

			/**
			* Updates the model in order to calculate the late reverberation time (T60).
			*
			* @param model The model used to calculate the late reverberation time.
			*/
			void UpdateReverbTime(const ReverbFormula model);

			/**
			* Overrides the current late reverberation time (T60).
			*
			* @param T60 The late reverberation time.
			*/
			void UpdateReverbTime(const Coefficients<>& T60);

			/**
			* @brief Updates the diffraction model.
			*
			* @param model The new diffraction model.
			*/
			void UpdateDiffractionModel(const DiffractionModel model);

			/**
			* @brief Returns a pointer to the room class.
			* 
			* @return A pointer to the room class.
			*/
			inline std::shared_ptr<Room> GetRoom() { return mRoom; }

			/**
			* @brief Returns a pointer to the image edge class.
			* 
			* @return A pointer to the image edge class.
			*/
			inline std::shared_ptr<ImageEdge> GetImageEdgeModel() { return mImageEdgeModel; }

			/**
			* @brief Sets the room volume and dimensions.
			* 
			* @param volume The volume of the room used to predict the reverberation time.
			* @param dimensions The dimensions of the room used to set the FDN delay lines.
			* @return True if the late reverb was initialised successfully, false otherwise.
			*/
			bool InitLateReverb(const Real volume, const Vec& dimensions, const FDNMatrix matrix);

			/**
			* @brief Clears the internal FDN buffers.
			*/
			inline void ResetFDN() { mReverb->Reset(); }

			/**
			* @brief Update the listener position and orientation.
			* 
			* @param position The new position of the listener.
			* @param orientation The new orientation of the listener.
			*/
			void UpdateListener(const Vec3& position, const Vec4& orientation);

			/**
			* @brief Initialises a new source in the spatialsier.
			* 
			* @return The ID of the new source.
			*/
			int InitSource();

			/**
			* @brief Updates the position and orientation of a source.
			* 
			* @param id The ID of the source to update.
			* @param position The new position of the source.
			* @param orientation The new orientation of the source.
			*/
			void UpdateSource(size_t id, const Vec3& position, const Vec4& orientation);

			/**
			* @brief Updates the directivity of a source.
			* 
			* @param id The ID of the source to update.
			* @param directivity The new directivity of the source.
			*/
			inline void UpdateSourceDirectivity(size_t id, const SourceDirectivity& directivity) { mSources->UpdateSourceDirectivity(id, directivity); }

			/**
			* @brief Removes a source from the spatialiser.
			* 
			* @param id The ID of the source to remove.
			*/
			void RemoveSource(size_t id);

			/**
			* @brief Initialises a new wall in the spatialsier.
			* 
			* @param vertices The vertices of the wall.
			* @param absorption The absorption of the wall.
			* @return The ID of the wall.
			*/
			int InitWall(const Vertices& vertices, const Absorption<>& absorption);
			
			/**
			* @brief Updates the position of a wall.
			* 
			* @param id The ID of the wall to update.
			* @param vData The new vertices of the wall.
			*/
			void UpdateWall(size_t id, const Vertices& vData);

			/**
			* @brief Updates the absorption of a wall.
			*
			* @param id The ID of the wall to update.
			* @param absorption The new absorption of the wall.
			*/
			void UpdateWallAbsorption(size_t id, const Absorption<>& absorption);

			/**
			* @brief Removes a wall from the spatialiser.
			* 
			* @param id The ID of the wall to remove.
			*/
			void RemoveWall(size_t id);

			/**
			* @brief Updates the planes and edges of the room.
			*/
			void UpdatePlanesAndEdges();

			/**
			* @brief Sends an audio buffer to a source and adds the output to mOutputBuffer.
			* 
			* @param id The ID of the source to send the audio to.
			* @param data The audio buffer.
			*/
			inline void SubmitAudio(size_t id, const Buffer<>& data) { PROFILE_SubmitAudio mSources->SetInputBuffer(id, data); }

			/**
			* @brief Accesses the output of the spatialiser.
			* @details Processes the reverberation and adds the output to mOutputBuffer.
			* This is copied to mSendBuffer and mOutuputBuffer and mReverbBuffer are reset.
			* 
			* @param bufferPtr Pointer to a float pointer that is pointed towards mSendBuffer.
			*/
			void GetOutput(float** bufferPtr);

			/**
			* @brief Sets the spatialiser to impulse response mode if mode is true
			*
			* @params mode True if disable all interpolation, false otherwise.
			*/
			void UpdateImpulseResponseMode(const bool mode);

		private:
			/**
			* Spatialiser
			*/
			const std::shared_ptr<Config> mConfig;				// RAC Config
			std::atomic<bool> mIsRunning;			// Flag to check if the spatialiser is running
			std::thread IEMThread;		// Background thread to run the image edge model
			Vec3 listenerPosition;		// Stored listener position
			Real headRadius;			// Stored head radius from 3DTI
			bool applyHeadphoneEQ;		// Flag to apply headphone EQ
			HeadphoneEQ headphoneEQ;	// Headphone EQ

			/**
			* 3DTI components
			*/
			Binaural::CCore mCore;							// 3DTI core
			std::shared_ptr<Binaural::CListener> mListener;	// 3DTI listener

			/**
			* Audio buffers
			*/
			Buffer<> mInputBuffer;	// Audio input buffer
			Buffer<> mOutputBuffer;	// Audio output buffer
			Matrix mReverbInput;	// Audio reverb input matrix
			std::vector<float> mSendBuffer;	// Audio send buffer (float)

			/**
			* Handles
			*/
			std::shared_ptr<Room> mRoom;				// Room class
			std::shared_ptr<Reverb> mReverb;			// Reverb class
			std::shared_ptr<SourceManager> mSources;	// Source manager class
			std::shared_ptr<ImageEdge> mImageEdgeModel;	// Image edge class

			std::mutex audioMutex;		// Mutex for audio processing

			std::string logFile;		// Log file path
#ifdef PROFILE_BACKGROUND_THREAD || PROFILE_AUDIO_THREAD
			std::string profileFile;	// Profile file path
#endif
		};
	}
}

#endif