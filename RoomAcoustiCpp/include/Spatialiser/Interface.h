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

// Spatialiser headers
#include "Spatialiser/Types.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		/**
		* Initializes the spatialiser with the given configuration and file paths.
		*
		* @param config The configuration of the spatialiser.
		* @return True if the initialization was successful, false otherwise.
		*/
		bool Init(const Config& config);

		/**
		* Exits and cleans up the spatialiser.
		*/
		void Exit();

		/**
		* Sets the spatialisation mode for the HRTF processing.
		*
		* @param hrtfResamplingStep The step size for resampling the HRTF.
		* @param filePaths The file paths for HRTF files.
		*/
		bool LoadSpatialisationFiles(const int hrtfResamplingStep, const std::vector<std::string>& filePaths);

		/**
		* Sets the spatialisation mode for the HRTF processing.
		*
		* @param mode The new spatialisation mode.
		*/
		void UpdateSpatialisationMode(const SpatialisationMode mode);
		
		/**
		* Updates the configuration for the Image Edge Model (IEM).
		*
		* @param config The new configuration for the IEM.
		*/
		void UpdateIEMConfig(const IEMConfig& config);

		/**
		* Overrides the current late reverberation time (T60).
		*
		* @param T60 The late reverberation time.
		*/
		void UpdateReverbTime(const Coefficients& T60);

		/**
		* Updates the model in order to calculate the late reverberation time (T60).
		*
		* @param model The model used to calculate the late reverberation time.
		*/
		void UpdateReverbTimeFormula(const ReverbFormula model);

		/**
		* Updates the FDN matrix used to process the late reverberation.
		*
		* @param model The matrix used within the FDN.
		*/
		void InitFDNMatrix(const FDNMatrix matrixType);

		/**
		* Updates the model used to process diffraction.
		*
		* @param model The diffraction model.
		*/
		void UpdateDiffractionModel(const DiffractionModel model);

		/**
		* Updates the volume and dimensions of the room.
		*
		* @param volume The volume of the room.
		* @param dimensions The dimensions of the room for the delay lines.
		*/
		void UpdateRoom(const Real volume, const Vec& dimensions);

		/**
		* Clears the internal FDN buffers.
		*/
		void ResetFDN();

		/**
		* Updates the listener's position and orientation.
		*
		* @param position The new position of the listener.
		* @param orientation The new orientation of the listener.
		*/
		void UpdateListener(const Vec3& position, const Vec4& orientation);

		/**
		* Initializes a new audio source and returns its ID.
		*
		* @return The ID of the new audio source.
		*/
		size_t InitSource();

		/**
		* Updates the position and orientation of the audio source with the given ID.
		*
		* @param id The ID of the audio source to update.
		* @param position The new position of the source.
		* @param orientation The new orientation of the source.
		*/
		void UpdateSource(const size_t id, const Vec3& position, const Vec4& orientation);

		/**
		* Updates the directivity of the audio source with the given ID.
		* 
		* @param id The ID of the audio source to update.
		* @param directivity The new directivity of the source.
		*/
		void UpdateSourceDirectivity(const size_t id, const SourceDirectivity directivity);

		/**
		* Removes the audio source with the given ID.
		*
		* @param id The ID of the audio source to remove.
		*/
		void RemoveSource(const size_t id);

		/**
		* Initializes a new wall with the given parameters and returns its ID.
		*
		* @param vData The vertices of the wall.
		* @param absorption The frequency absorption coefficients.
		* @return The ID of the new wall.
		*/
		size_t InitWall(const Vertices& vData, const Absorption& absorption);
		
		/**
		* Updates the position and orientation of the wall with the given ID.
		*
		* @param id The ID of the wall to update.
		* @param vData The new vertices of the wall.
		*/
		void UpdateWall(size_t id, const Vertices& vData);

		/**
		* Updates the absorption of the wall with the given ID.
		*
		* @param id The ID of the wall to update.
		* @param absorption The new absortion of the wall.
		*/
		void UpdateWallAbsorption(size_t id, const Absorption& absorption);

		/**
		* Removes the wall with the given ID.
		*
		* @param id The ID of the wall to remove.
		* @param reverbWall The reverb wall.
		*/
		void RemoveWall(size_t id);

		/**
		* Updates the planes and edges of the room.
		*/
		void UpdatePlanesAndEdges();

		/**
		* Submits an audio buffer to the audio source with the given ID.
		*
		* @param id The ID of the audio source to update.
		* @param data The new audio data for the source.
		*/
		void SubmitAudio(size_t id, const float* data);

		/**
		* Returns a pointer to the output buffer of the spatialiser.
		*
		* @param bufferPtr A pointer to a float pointer. This will be set to point to the output buffer.
		*/
		void GetOutput(float** bufferPtr);
	}
}
#endif