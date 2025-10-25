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
#include "Spatialiser/ContextOptionalArguments.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/SourceManager.h"
#include "Spatialiser/Reverb.h"
#include "Spatialiser/Room.h"
#include "Spatialiser/ImageEdge.h"
#include "Spatialiser/HeadphoneEQ.h"
#include "Spatialiser/TracingThread.h"

// 3DTI Headers
#include "Common/Transform.h"
#include "BinauralSpatializer/3DTI_BinauralSpatializer.h"

#include <chrono>
#include <filesystem>
#include <ctime>

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
			* @param optionalArguments Optional initialization arguments
			*/
			explicit Context(const DSPData& data, const ContextOptionalArguments &optionalArguments = ContextOptionalArguments());

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
			* @brief Initialises the Image Edge Model (IEM) and sets the diffraction model.
			* 
			* @param enabled True to enable early reflection DSP, false to disable.
			* @param data The user defined IEM configuration data.
			* @param model The diffraction model to use.
			*/
			bool InitEarlyReverb(const bool enabled, const EarlyReverbData& data, const DiffractionModel model);

			/**
			* @brief Initialises MoDART late reverberation.
			* 
			* @param data The user defined MoDART configuration data.
			* @return True if the MoDART late reverberation was initialised successfully, false otherwise.
			*/
			bool InitMoDART(const MoDARTData& data);

			/**
			* @brief Initialises SingleFDN late reverberation.
			*
			* @param roomData The user defined room configuration data.
			* @param data The user defined SingleFDN configuration data.
			* @return True if the SingleFDN late reverberation was initialised successfully, false otherwise.
			*/
			bool InitSingleFDN(const RoomData& roomData, const LateReverbData& data);

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
			inline void UpdateSpatialisationMode(const SpatialisationMode mode) { dspConfig->UpdateSpatialisationMode(mode); }

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
			* @brief Enables the early reverberation DSP.
			*
			* @param enable True to enable early reflections, false to disable.
			*/
			inline void EnableEarlyReverb(const bool enable) { dspConfig->EnableEarlyReverb(enable); }

			/**
			* @brief Updates the image edge model (IEM) configuration.
			*
			* @param config The new IEM configuration.
			*/
			inline void UpdateEarlyConfig(const EarlyReverbData& data) { mImageEdgeModel->UpdateIEMConfig(data, dspConfig); }

			/**
			* @brief Enables the late reverberation DSP.
			*
			* @details If reverb is being reenabled, the late reverb buffers are reset.
			* 
			* @param enable True to enable late reflections, false to disable.
			*/
			inline void EnableLateReverb(const bool enable)
			{
				if (enable && !dspConfig->GetLateReverbEnabled())
					ResetLateReverb();
				dspConfig->EnableLateReverb(enable);
			}

			/**
			* @brief Sets the number of rays used in the late reverberation ray tracing.
			*
			* @param numRays The number of rays to use for ray tracing.
			*/
			inline void UpdateLateReverbNumberOfRays(const int numRays)
			{
				if (lateReverbInitialised.load(std::memory_order_acquire))
					mRayTracing->SetNumberOfRays(numRays);
			}

			/**
			* @brief Updates the intial delay for MoDART late reverberation.
			*
			* @param delay The initial delay in seconds.
			*/
			void UpdateMoDARTDelay(const Real delay);

			/**
			* @brief Updates the minimum reverberation time to model. Controls the number of modes in MoDART.
			*
			* @param T60 The minimum reverberation time in seconds.
			*/
			void UpdateMoDARTMinimumReverbTime(const Real T60);

			/**
			* Updates the model in order to calculate the late reverberation time (T60).
			*
			* @param model The model used to calculate the late reverberation time.
			*/
			void UpdateSingleFDNReverbTime(const ReverbFormula model);

			/**
			* Overrides the current late reverberation time (T60).
			*
			* @param T60 The late reverberation time.
			*/
			void UpdateSingleFDNReverbTime(const Coefficients<>& T60);

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
			* @brief Returns a pointer to the ray tracing class.
			*
			* @return A pointer to the ray tracing class.
			*/
			inline std::shared_ptr<TracingThread> GetRayTracing() { return mRayTracing; }

			/**
			* @brief Sets a flag to clear the late reverberation buffers.
			*/
			inline void ResetLateReverb() { dspConfig->FlagClearBuffers(); }

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

			inline size_t InitMaterial(const Coefficients<>& material) { return mRoom->InitMaterial(material); }

			void UpdateMaterial(size_t id, const Coefficients<>& material);

			inline void RemoveMaterial(size_t id) { mRoom->RemoveMaterial(id); }

			/**
			* @brief Initialises a new wall in the spatialsier.
			* 
			* @param vData The vertices of the wall.
			* @param absorption The absorption of the wall.
			* @param polygonId The ID of the polygon the wall belongs to.
			* 
			* @return The ID of the new wall.
			*/
			size_t InitWall(const Vertices& vData, size_t materialID);
			
			/**
			* @brief Updates the position of a wall.
			* 
			* @param id The ID of the wall to update.
			* @param vData The new vertices of the wall.
			*/
			inline void UpdateWall(size_t id, const Vertices& vData) { mRoom->UpdateWall(id, vData); }

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
			* @brief Updates the late reverb gain.
			* 
			* @param gain The new late reverb gain.
			*/
			inline void UpdateLateReverbGain(const Real gain) {  }

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
			void GetOutput(Buffer<>& sendBuffer);

			void RecordImpulseResponse(const Vec3& position, const Vec4& orientation, Buffer<>& outputBuffer);

			/**
			* @brief Sets the spatialiser to impulse response mode if mode is true
			*
			* @params mode True if disable all interpolation, false otherwise.
			*/
			inline void UpdateImpulseResponseMode(const bool mode) { dspConfig->UpdateImpulseResponseMode(mode); }

		private:
			void InitLateReverb(const LateReverbData& data);

			void CreateAudioThreadPool(size_t numDesiredWorkerThreads);

			/**
			* Spatialiser
			*/
			const std::shared_ptr<DSPConfig> dspConfig;				// RAC DSPConfig
			std::atomic<bool> mIsRunning;			// Flag to check if the spatialiser is running
			std::thread IEMThread;			// Background thread to run the image edge model
			std::thread rayTracingThread;	// Background thread to run the ray tracing model

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
			Matrix<> mReverbInput;	// Audio reverb input matrix

			std::atomic<bool> audioFlag{ false };	// Flag to check if the audio thread is processing

			std::atomic<bool> earlyReverbInitialised{ false };	// True if the early reverberation has been initialised, false otherwise
			std::atomic<bool> lateReverbInitialised{ false };	// True if the late reverberation has been initialised, false otherwise
			
			/**
			* Handles
			*/
			std::shared_ptr<Room> mRoom{ nullptr };					// Room class
			std::shared_ptr<Reverb> mReverb{ nullptr };				// Reverb class
			std::shared_ptr<SourceManager> mSources{ nullptr };		// Source manager class
			std::shared_ptr<ImageEdge> mImageEdgeModel{ nullptr };	// Image edge class
			std::shared_ptr<TracingThread> mRayTracing{ nullptr };	// Ray tracing class

			std::string logFile;		// Log file path
#if defined(PROFILE_BACKGROUND_THREAD) || defined(PROFILE_AUDIO_THREAD)
			std::string profileFile;	// Profile file path
#endif
		};
	}
}

#endif