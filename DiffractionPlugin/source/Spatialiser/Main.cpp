/**
 *
 *  \Defines DLL linakage
 * 
 */

// C++ headers
#include <string>

// Common headers
#include "Common/AudioManager.h"
#include "Common/Coefficients.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Interface.h"

// DSP headers
#include "DSP/Buffer.h"

// DLL Linkage
#ifdef _ANDROID
#define API
#define EXPORT __attribute__ ((visibility ("default")))
#elif _WINDOWS
#define API __stdcall
#define EXPORT __declspec(dllexport)
#else
#define API
#define EXPORT
#endif

using namespace UIE::Spatialiser;
using namespace UIE::Common;
using namespace UIE::DSP;

// Pointer to return buffer
static float* buffer = nullptr;

// Store number of bands
static int numAbsorptionBands = 0;

extern "C"
{

	//////////////////// API ////////////////////

	/**
	 * Initializes the spatialiser with the given parameters.
	 *
	 * @param fs The sample rate for audio processing.
	 * @param numFrames The number of frames in an audio buffer.
	 * @param numFDNChannels The number of feedback delay network channels.
	 * @param lerpFactor The interpolation factor for audio parameters.
	 * @param hrtfMode The mode for HRTF processing. 0 for quality, 1 for performance, 2 for none.
	 * @param fBands The center frequency bands for reflection filters.
	 * @param numBands The number of frequency bands provided in the fBands parameter.
	 * @param paths The file paths for HRTF files.
	 *
	 * @return True if the initialization was successful, false otherwise.
	 */
	EXPORT bool API SPATInit(int fs, int numFrames, int numFDNChannels, float lerpFactor, const float* fBands, int numBands)
	{
		numAbsorptionBands = numBands;
		Coefficients frequencyBands = Coefficients(static_cast<size_t>(numBands));
		for (int i = 0; i < numBands; i++)
			frequencyBands[i] = static_cast<Real>(fBands[i]);

		Config config = Config(fs, numFrames, numFDNChannels, static_cast<Real>(lerpFactor), frequencyBands);
		return Init(&config);
	}

	/**
	 * Exits and cleans up the spatialiser.
	 *
	 * This function should be called when the spatialiser is no longer needed.
	 * It will free up any resources that the spatialiser is using.
	 */
	EXPORT void API SPATExit()
	{
		Exit();
	}

	/**
	 * Sets the spatialisation mode for the HRTF processing.
	 *
	 * @param qualityDepth The depth up to which to use high quality HRTF processing. -2 for none, -1 for all (including late reverberation) and 0 for direct sound and 1, 2, 3 etc for the corresponding reflection/diffraction order.
	 * @param performanceDepth The depth up to which to use high performance ILD processing. -2 for none, -1 for all (including late reverberation) and 0 for direct sound and 1, 2, 3 etc for the corresponding reflection/diffraction order.
	 * @param hrtfResamplingStep The step size for resampling the HRTF. This should be between 5 - 90. Smaller values indicate higher quality.
	 * @param paths An array of file paths in the order HRTF, near field and ILD files. The paths are expected to be null-terminated C strings.
	 *
	 * @return True if the spatialisation mode was successfully set, false otherwise.
	 */
	EXPORT bool API SPATSetSpatialisationMode(int qualityDepth, int performanceDepth, int hrtfResamplingStep, const char** paths)
	{
		std::vector<std::string> filePaths = { std::string(*(paths)), std::string(*(paths + 1)), std::string(*(paths + 2)) };

		SPATConfig config = SPATConfig(qualityDepth, performanceDepth);
		return SetSpatialisationMode(config, hrtfResamplingStep, filePaths);
	}

	/**
	 * Updates the configuration for the Image Source Model (ISM).
	 *
	 * @param order The maximum number of reflections or diffractions to consider in the ISM.
	 * @param dir Whether to consider direct sound.
	 * @param ref Whether to consider reflected sound.
	 * @param diff Whether to consider diffracted sound.
	 * @param refDiff Whether to consider combined reflected diffraction sound.
	 * @param rev Whether to consider late reverberation.
	 * @param spDiff Whether to consider diffraction outside of the shadow zone.
	 */
	EXPORT void API SPATUpdateISMConfig(int order, bool dir, bool ref, bool diff, bool refDiff, bool rev, bool spDiff)
	{
		UpdateISMConfig(ISMConfig(order, dir, ref, diff, refDiff, rev, spDiff));
	}

	/**
	 * Sets the parameters for the Feedback Delay Network (FDN) reverb.
	 *
	 * @param volume The volume of the room.
	 * @param dim The dimensions of the room for the delay lines.
	 * @param numDimensions The number of dimensions provided in the dim parameter.
	 */
	EXPORT void API SPATSetFDNParameters(float volume, const float* dim, int numDimensions)
	{
		Buffer in = Buffer(numDimensions);
		vec dimensions = vec(numDimensions);
		for (int i = 0; i < numDimensions; i++)
			dimensions.AddEntry(static_cast<Real>(dim[i]), i);

		SetFDNParameters(static_cast<Real>(volume), dimensions);
	}

	/**
	 * Updates the listener's position and orientation.
	 *
	 * @param posX The x-coordinate of the listener's position.
	 * @param posY The y-coordinate of the listener's position.
	 * @param posZ The z-coordinate of the listener's position.
	 * @param oriW The w-component of the listener's orientation quaternion.
	 * @param oriX The x-component of the listener's orientation quaternion.
	 * @param oriY The y-component of the listener's orientation quaternion.
	 * @param oriZ The z-component of the listener's orientation quaternion.
	 */
	EXPORT void API SPATUpdateListener(float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateListener(vec3(posX, posY, posZ), vec4(oriW, oriX, oriY, oriZ));
	}

	/**
	 * Initializes a new audio source and returns its ID.
	 *
	 * This function should be called when a new audio source is created.
	 * It will allocate resources for the new source and return an ID that can be used to reference the source in future calls.
	 *
	 * @return The ID of the new audio source.
	 */
	EXPORT int API SPATInitSource()
	{
		return InitSource();
	}

	/**
	 * Updates the position and orientation of the audio source with the given ID.
	 *
	 * @param id The ID of the audio source to update.
	 * @param posX The x-coordinate of the source's position.
	 * @param posY The y-coordinate of the source's position.
	 * @param posZ The z-coordinate of the source's position.
	 * @param oriW The w-component of the source's orientation quaternion.
	 * @param oriX The x-component of the source's orientation quaternion.
	 * @param oriY The y-component of the source's orientation quaternion.
	 * @param oriZ The z-component of the source's orientation quaternion.
	 */
	EXPORT void API SPATUpdateSource(int id, float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateSource(static_cast<size_t>(id), vec3(posX, posY, posZ), vec4(oriW, oriX, oriY, oriZ));
	}

	/**
	 * Removes the audio source with the given ID.
	 *
	 * This function should be called when an audio source is no longer needed.
	 * It will free up any resources that the source was using.
	 *
	 * @param id The ID of the audio source to remove.
	 */
	EXPORT void API SPATRemoveSource(int id)
	{
		RemoveSource(static_cast<size_t>(id));
	}

	/**
	 * Returns the ReverbWall enum value corresponding to the given ID.
	 *
	 * This function is used to map integer IDs to ReverbWall enum values.
	 * The mapping is as follows:
	 * 0 -> posZ
	 * 1 -> negZ
	 * 2 -> posX
	 * 3 -> negX
	 * 4 -> posY
	 * 5 -> negY
	 * Any other value -> none
	 *
	 * @param id The ID to map to a ReverbWall enum value.
	 * @return The corresponding ReverbWall enum value.
	 */
	ReverbWall ReturnReverbWall(int id)
	{
		switch (id)
		{
		case 0:
		{ return ReverbWall::posZ; break; }
		case 1:
		{ return ReverbWall::negZ; break; }
		case 2:
		{ return ReverbWall::posX; break; }
		case 3:
		{ return ReverbWall::negX; break; }
		case 4:
		{ return ReverbWall::posY; break; }
		case 5:
		{ return ReverbWall::negY; break; }
		default:
		{ return ReverbWall::none; break; }
		}
	}

	/**
	 * Initializes a new wall with the given parameters and returns its ID.
	 *
	 * This function should be called when a new wall is created.
	 * It will allocate resources for the new wall and return an ID that can be used to reference the wall in future calls.
	 *
	 * @param nX The x-coordinate of the wall's normal vector.
	 * @param nY The y-coordinate of the wall's normal vector.
	 * @param nZ The z-coordinate of the wall's normal vector.
	 * @param vData The vertices of the wall.
	 * @param numVertices The number of vertices provided in the vData parameter.
	 * @param absorption The frequency absorption coefficients.
	 * @param reverbWallId The ID of the reverb wall.
	 *
	 * @return The ID of the new wall.
	 */
	EXPORT int API SPATInitWall(float nX, float nY, float nZ, const float* vData, int numVertices, float* absorption, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);
		std::vector<Real> a = std::vector<Real>(numAbsorptionBands);
		for (int i = 0; i < numAbsorptionBands; i++)
			a[i] = static_cast<Real>(absorption[i]);
		Absorption abs = Absorption(a);

		int numCoords = 3 * numVertices;
		Buffer in = Buffer(numCoords);
		for (int i = 0; i < numCoords; i++)
			in[i] = static_cast<Real>(vData[i]);

		return InitWall(vec3(nX, nY, nZ), &in[0], static_cast<size_t>(numVertices), abs, reverbWall);
	}

	/**
	 * Updates the position and orientation of the wall with the given ID.
	 *
	 * This function should be called when the position or orientation of a wall changes.
	 * It will update the internal representation of the wall to match the new position and orientation.
	 *
	 * @param id The ID of the wall to update.
	 * @param nX The x-coordinate of the wall's normal vector.
	 * @param nY The y-coordinate of the wall's normal vector.
	 * @param nZ The z-coordinate of the wall's normal vector.
	 * @param vData The vertices of the wall.
	 * @param numVertices The number of vertices provided in the vData parameter.
	 */
	EXPORT void API SPATUpdateWall(int id, float nX, float nY, float nZ, const float* vData, int numVertices)
	{
		int numCoords = 3 * numVertices;
		Buffer in = Buffer(numCoords);
		for (int i = 0; i < numCoords; i++)
			in[i] = static_cast<Real>(vData[i]);

		UpdateWall(static_cast<size_t>(id), vec3(nX, nY, nZ), &in[0], static_cast<size_t>(numVertices));
	}

	/**
	 * Frees up the ID of the wall with the given ID.
	 *
	 * This function should be called after a wall is removed.
	 * It will free up the ID of the wall so that it can be reused for new walls.
	 *
	 * @param id The ID of the wall to free.
	 */
	EXPORT void API SPATFreeWallId(int id)
	{
		FreeWallId(static_cast<size_t>(id));
	}

	/**
	 * Removes the wall with the given ID.
	 *
	 * This function should be called when a wall is no longer needed.
	 * It will free up any resources that the wall was using and remove it from the spatialiser.
	 *
	 * @param id The ID of the wall to remove.
	 * @param reverbWallId The ID of the reverb wall.
	 */
	EXPORT void API SPATRemoveWall(int id, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);

		RemoveWall(static_cast<size_t>(id), reverbWall);
	}

	/**
	 * Submits an audio buffer to the audio source with the given ID.
	 *
	 * This function should be called when there is a new audio buffer for a source.
	 * It will process the audio buffer and add it to the output buffer.
	 *
	 * @param id The ID of the audio source to update.
	 * @param data The new audio buffer for the source.
	 */
	EXPORT void API SPATSubmitAudio(int id, const float* data)
	{
		SubmitAudio(static_cast<size_t>(id), data);
	}

	/**
	 * Processes the output of the spatialiser.
	 *
	 * This function should be called after all audio sources have been updated for a frame.
	 * It will process the later reverberation and prepare the interleaved output buffer.
	 *
	 * @return True if the processing was successful and the output buffer is ready, false otherwise.
	 */
	EXPORT bool API SPATProcessOutput()
	{
		GetOutput(&buffer);
		if (!buffer)
		{
#ifdef DEBUG_AUDIO_THREAD
	Debug::Log("Process Output Failed", Colour::Orange);
#endif
			return false;
		}
		else if (std::isnan(*buffer))
		{
#ifdef DEBUG_AUDIO_THREAD
	Debug::Log("Process Output is NaN", Colour::Orange);
#endif
			return false;
		}
#ifdef DEBUG_AUDIO_THREAD
	Debug::Log("Process Output Success", Colour::Orange);
#endif
		return true;
	}

	/**
	 * Returns a pointer to the output buffer of the spatialiser.
	 *
	 * This function should be called after SPATProcessOutput has returned true.
	 * It will return a pointer to the output buffer that contains the processed output buffer.
	 *
	 * @param buf A pointer to a float pointer. This will be set to point to the interleaved output buffer.
	 */
	EXPORT void API SPATGetOutputBuffer(float** buf)
	{
		*buf = buffer;
	}
}