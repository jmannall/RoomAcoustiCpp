/*
* @brief Interface between the API and Spatialiser global context
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
		 * @param config The configuration of the spatialisation mode.
		 */
		void UpdateSpatialisationMode(const SPATConfig config);
		
		/**
		 * Updates the configuration for the Image Source Model (ISM).
		 *
		 * @param config The new configuration for the ISM.
		 */
		void UpdateISMConfig(const ISMConfig& config);

		/**
		 * Updates the model in order to calculate the late reverberation time (T60).
		 *
		 * @param model The model used to calculate the late reverberation time.
		 */
		void UpdateReverbTimeModel(const ReverbTime model);

		/**
		 * Updates the FDN matrix used to process the late reverberation.
		 *
		 * @param model The matrix used within the FDN.
		 */
		void UpdateFDNModel(const FDNMatrix& model);

		/**
		 * Updates the volume and dimensions of the room.
		 *
		 * @param volume The volume of the room.
		 * @param dimensions The dimensions of the room for the delay lines.
		 */
		void UpdateRoom(const Real volume, const vec& dimensions);

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
		void UpdateListener(const vec3& position, const vec4& orientation);

		/**
		 * Initializes a new audio source and returns its ID.
		 *
		 * @return The ID of the new audio source.
		 */
		int InitSource();

		/**
		 * Updates the position and orientation of the audio source with the given ID.
		 *
		 * @param id The ID of the audio source to update.
		 * @param position The new position of the source.
		 * @param orientation The new orientation of the source.
		 */
		void UpdateSource(size_t id, const vec3& position, const vec4& orientation);

		/**
		 * Removes the audio source with the given ID.
		 *
		 * @param id The ID of the audio source to remove.
		 */
		void RemoveSource(size_t id);

		/**
		 * Initializes a new wall with the given parameters and returns its ID.
		 *
		 * @param normal The normal vector of the wall.
		 * @param vData The vertices of the wall.
		 * @param numVertices The number of vertices provided in the vData parameter.
		 * @param absorption The frequency absorption coefficients.
		 * @param reverbWall The reverb wall.
		 * @return The ID of the new wall.
		 */
		int InitWall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption);
		
		/**
		 * Updates the position and orientation of the wall with the given ID.
		 *
		 * @param id The ID of the wall to update.
		 * @param normal The new normal vector of the wall.
		 * @param vData The new vertices of the wall.
		 * @param numVertices The number of vertices provided in the vData parameter.
		 */
		void UpdateWall(size_t id, const vec3& normal, const Real* vData, size_t numVertices);

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