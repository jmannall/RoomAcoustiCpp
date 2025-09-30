/*
* @brief Interface between the API and RoomAcoustiCpp global context
* 
*/

#ifndef RoomAcoustiCpp_Interface_h
#define RoomAcoustiCpp_Interface_h

// Common headers
#include "Common/Vec.h"
#include "Common/Vec3.h"
#include "Common/Vec4.h"
#include "Common/Types.h"
#include "DSP/Buffer.h"

// Spatialiser headers
#include "Spatialiser/Types.h"

namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{
		/**
		* @brief Initializes the spatialiser with the given configuration and file paths.
		*
		* @param config The configuration of the spatialiser.
		* @return True if the initialization was successful, false otherwise.
		*/
		bool Init(const DSPData& data);

		/**
		* @brief Exits and cleans up the spatialiser.
		*/
		void Exit();

		/**
		* @brief Sets the spatialisation mode for the HRTF processing.
		*
		* @param hrtfResamplingStep The step size for resampling the HRTF.
		* @param filePaths The file paths for HRTF files.
		*/
		bool LoadSpatialisationFiles(const int hrtfResamplingStep, const std::vector<std::string>& filePaths);

		/**
		* @brief Sets the headphone EQ filters.
		*
		* @param leftIR The impulse response for the left channel.
		* @param rightIR The impulse response for the right channel.
		*/
		void SetHeadphoneEQ(const Buffer<>& leftIR, const Buffer<>& rightIR);

		/**
		* @brief Sets the spatialisation mode for the HRTF processing.
		*
		* @param mode The new spatialisation mode.
		*/
		void UpdateSpatialisationMode(const SpatialisationMode mode);
		
		/**
		* @brief Enables the early reverberation DSP.
		* 
		* @param enable True to enable early reflections, false to disable.
		*/
		void EnableEarlyReverb(const bool enable);

		/**
		* @brief Updates the configuration for the Image Edge Model (IEM).
		*
		* @param config The new configuration for the IEM.
		*/
		void UpdateEarlyConfig(const EarlyReverbData& data);

		/**
		* @brief Enables the late reverberation DSP.
		*
		* @param enable True to enable late reflections, false to disable.
		*/
		void EnableLateReverb(const bool enable);

		/**
		* @brief Sets the number of rays used in the late reverberation ray tracing.
		* 
		* @param numRays The number of rays to use for ray tracing.
		*/
		void UpdateLateReverbNumberOfRays(const int numRays);

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
		* @brief Updates the model in order to calculate the late reverberation time (T60).
		*
		* @param model The model used to calculate the late reverberation time.
		*/
		void UpdateSingleFDNReverbTime(const ReverbFormula model);

		/**
		* @brief Overrides the current late reverberation time (T60).
		*
		* @param T60 The late reverberation time.
		*/
		void UpdateSingleFDNReverbTime(const Coefficients<>& T60);

		/**
		* @brief Updates the model used to process diffraction.
		*
		* @param model The diffraction model.
		*/
		void UpdateDiffractionModel(const DiffractionModel model);

		/**
		* @brief Initialises the Image Edge Model (IEM) and sets the diffraction model.
		*
		* @param enabled True to enable early reflection DSP, false to disable.
		* @param data The user defined IEM configuration data.
		* @param model The diffraction model to use.
		*/
		bool InitEarlyReverb(const bool enabled, const EarlyReverbData& data, const DiffractionModel model);

		/**
		* @brief Initialises SingleFDN late reverberation.
		*
		* @param roomData The user defined room configuration data.
		* @param data The user defined SingleFDN configuration data.
		* @return True if the SingleFDN late reverberation was initialised successfully, false otherwise.
		*/
		bool InitSingleFDN(const RoomData& roomData, const LateReverbData& data);

		/**
		* @brief Initialises MoDART late reverberation.
		*
		* @param data The user defined MoDART configuration data.
		* @return True if the MoDART late reverberation was initialised successfully, false otherwise.
		*/
		bool InitMoDART(const MoDARTData& data);

		/**
		* @brief Clears the internal FDN buffers.
		*/
		void ResetLateReverb();

		/**
		* @brief Updates the listener's position and orientation.
		*
		* @param position The new position of the listener.
		* @param orientation The new orientation of the listener.
		*/
		void UpdateListener(const Vec3& position, const Vec4& orientation);

		/**
		* @brief Initializes a new audio source and returns its ID.
		*
		* @return The ID of the new audio source.
		*/
		int InitSource();

		/**
		* @brief Updates the position and orientation of the audio source with the given ID.
		*
		* @param id The ID of the audio source to update.
		* @param position The new position of the source.
		* @param orientation The new orientation of the source.
		*/
		void UpdateSource(const size_t id, const Vec3& position, const Vec4& orientation);

		/**
		* @brief Updates the directivity of the audio source with the given ID.
		* 
		* @param id The ID of the audio source to update.
		* @param directivity The new directivity of the source.
		*/
		void UpdateSourceDirectivity(const size_t id, const SourceDirectivity directivity);

		/**
		* @brief Removes the audio source with the given ID.
		*
		* @param id The ID of the audio source to remove.
		*/
		void RemoveSource(const size_t id);

		int InitMaterial(const Absorption<>& material);

		void UpdateMaterial(size_t id, const Absorption<>& material);

		void RemoveMaterial(size_t id);

		/**
		* @brief Initializes a new wall with the given parameters and returns its ID.
		*
		* @param vData The vertices of the wall.
		* @param absorption The frequency absorption coefficients.
		* @return The ID of the new wall.
		*/
		int InitWall(const Vertices& vData, const size_t materialId);
		
		/**
		* @brief Updates the position and orientation of the wall with the given ID.
		*
		* @param id The ID of the wall to update.
		* @param vData The new vertices of the wall.
		*/
		void UpdateWall(size_t id, const Vertices& vData);

		/**
		* @brief Removes the wall with the given ID.
		*
		* @param id The ID of the wall to remove.
		* @param reverbWall The reverb wall.
		*/
		void RemoveWall(size_t id);

		/**
		* @brief Updates the planes and edges of the room.
		*/
		void UpdatePlanesAndEdges();

		/**
		* @brief Updates the late reverberation gain.
		* 
		* @param gain The new late reverberation gain.
		*/
		void UpdateLateReverbGain(const Real gain);

		/**
		* @brief Submits an audio buffer to the audio source with the given ID.
		*
		* @param id The ID of the audio source to update.
		* @param data The new audio data for the source.
		*/
		void SubmitAudio(size_t id, const Buffer<>& data);

		/**
		* @brief Processes the audio for the current audio callback and updates the output buffer.
		*
		* @details If outputBuffer.Length() != 2 * numFrames, it will be resized.
		* 
		* @param outputBuffer Buffer to write the audio output to.
		*/
		void GetOutput(Buffer<>& outputBuffer);

		/**
		* @brief Sets the spatialiser to impulse response mode if mode is true
		*
		* @params mode True if disable all interpolation, false otherwise.
		*/
		void UpdateImpulseResponseMode(const bool mode);
	}
}
#endif