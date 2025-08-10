/**
*
*  \Defines DLL linakage
* 
*/

// C++ headers
#include <string>

// Common headers
#include "Common/Coefficients.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Interface.h"

// DSP headers
#include "DSP/Buffer.h"

// Unity headers
#include "Unity/Debug.h"

//////////////////// DLL Linkage ////////////////////

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

//////////////////// Namespaces ////////////////////

using namespace RAC::Spatialiser;
using namespace RAC::Common;
using namespace RAC::DSP;

//////////////////// Variables ////////////////////

static Buffer<> buffer(1);	// Return buffer
static int NUM_FREQUENCY_BANDS = 0;	// Store number of frequency bands for any reflection filters
static int NUM_FRAMES = 0;

extern "C"
{

	//////////////////// API ////////////////////

	/**
	* Initializes the spatialiser with the given parameters.
	*
	* @param fs The sample rate for audio processing.
	* @param numFrames The number of frames in an audio buffer.
	* @param numReverbSources The number of reverb sources for late reverberation spatialisation.
	* @param lerpFactor The interpolation factor for audio parameters.
	* @param Q The quality factor for GraphicEQ. (0.98 is a good starting point)
	* @param frequencyBandData The center frequency bands for frequency dependent filtering.
	* @param numFrequencyBands The number of frequency bands provided by the frequencyBandData parameter.
	*
	* @return True if the initialization was successful, false otherwise.
	*/
	EXPORT bool API RACInit(int fs, int numFrames, int numReverbSources, float lerpFactor, float Q, const float* frequencyBandData, int numFrequencyBands)
	{
		buffer.ResizeBuffer(2 * numFrames);
		NUM_FREQUENCY_BANDS = numFrequencyBands;
		NUM_FRAMES = numFrames;
		Coefficients<> frequencyBands = Coefficients<>(numFrequencyBands);
		for (int i = 0; i < numFrequencyBands; i++)
			frequencyBands[i] = static_cast<Real>(frequencyBandData[i]);

		std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numReverbSources, static_cast<Real>(lerpFactor), static_cast<Real>(Q), frequencyBands);
		return Init(config);
	}

	/**
	* Exits and cleans up the spatialiser.
	*
	* This function should be called when the spatialiser is no longer needed.
	* It will free up any resources that the spatialiser is using.
	*/
	EXPORT void API RACExit()
	{
		Exit();
	}

	/**
	* Loads the HRTF, near field and ILD files.
	*
	* @param hrtfResamplingStep The step size for resampling the HRTF. This should be between 5 - 90. Smaller values indicate higher quality.
	* @param paths An array of file paths in the order HRTF, near field and ILD files. The paths are expected to be null-terminated C strings.
	*
	* @return True if the spatialisation mode was successfully set, false otherwise.
	*/
	EXPORT bool API RACLoadSpatialisationFiles(int hrtfResamplingStep, const char** paths)
	{
		std::vector<std::string> filePaths = { std::string(*(paths)), std::string(*(paths + 1)), std::string(*(paths + 2)) };
		return LoadSpatialisationFiles(hrtfResamplingStep, filePaths);
	}

	/**
	* Sets the headphone EQ filters.
	*
	* Should be called if a headphone EQ is desired (implemented as a stereo FIR filter)
	* If not called, the headphone EQ will not be applied.
	* 
	* @param leftIR The impulse response for the left channel.
	* @param rightIR The impulse response for the right channel.
	* @param irLength The length of the impulse responses.
	*/
	EXPORT void API RACSetHeadphoneEQ(const float* leftIR, const float* rightIR, int irLength)
	{
		std::vector<Real> left(irLength);
		std::vector<Real> right(irLength);

		std::transform(leftIR, leftIR + irLength, left.begin(), [](float f) { return static_cast<Real>(f); });
		std::transform(rightIR, rightIR + irLength, right.begin(), [](float f) { return static_cast<Real>(f); });

		SetHeadphoneEQ(left, right);
	}

	/**
	* Sets the spatialisation mode (high quality, high performance or none).
	*
	* Should be called after spatialisation files have been loaded, otherwise defaults to none
	* 
	* The mapping is as follows:
	* 0 -> none (mono)
	* 1 -> high performance (ILD only)
	* 2 -> high quality (HRTF)
	*
	* @param id The ID corresponding to a spatialisation mode.
	*/
	EXPORT void API RACUpdateSpatialisationMode(int id)
	{
		switch (id)
		{
		case(0):
		{ UpdateSpatialisationMode(SpatialisationMode::none); break; }
		case(1):
		{ UpdateSpatialisationMode(SpatialisationMode::performance); break; }
		case(2):
		{ UpdateSpatialisationMode(SpatialisationMode::quality); break; }
		default:
		{ UpdateSpatialisationMode(SpatialisationMode::none); break; }
		}
	}

	/**
	* @brief Convert direct sound mode id to enum
	*/
	DirectSound SelectDirectMode(int directMode)
	{
		switch (directMode)
		{
		case(0):
		{ return DirectSound::none; break; }
		case(1):
		{ return DirectSound::check; break; }
		case(2):
		{ return DirectSound::ignoreCheck; break; }
		default:
		{ return DirectSound::none; break; }
		}
	}

	/**
	* Updates the configuration for the Image Edge Model (IEM).
	* 
	* The direct sound is mapped as follows.
	* 0 -> none
	* 1 -> check
	* 2 -> ignoreCheck (ignores visibility check)
	*
	* @param direct Whether to consider direct sound.
	* @param reflOrder The maximum number of reflections in reflection only paths.
	* @param shadowDiffOrder The maximum number of reflections or diffractions in shadowed diffraction paths.
	* @param specularDiffOrder The maximum number of reflections or diffractions in specular diffraction paths.
	* @param lateReverb Whether to consider late reverberation.
	* @param minEdgeLength The minimum edge length to consider diffraction for.
	*/
	EXPORT void API RACUpdateIEMConfig(int dir, int reflOrder, int shadowDiffOrder, int specularDiffOrder, bool rev, float edgeLen)
	{
		DirectSound direct = SelectDirectMode(dir);
		UpdateIEMConfig(IEMData(direct, reflOrder, shadowDiffOrder, specularDiffOrder, rev, static_cast<Real>(edgeLen)));
	}

	/**
	* Updates the late reverberation time (T60).
	* 
	* Should be called after the late reverberation has been initialised, if a custom T60 is desired.
	*
	* @param T60Data The late reverberation time.
	*/
	EXPORT void API RACUpdateReverbTime(const float* T60Data)
	{
		Coefficients T60 = Coefficients<>(static_cast<size_t>(NUM_FREQUENCY_BANDS));
		for (int i = 0; i < NUM_FREQUENCY_BANDS; i++)
			T60[i] = static_cast<Real>(T60Data[i]);

		UpdateReverbTime(T60);
	}

	/**
	* Updates the model in order to calculate the late reverberation time (T60).
	*
	* Should be called after the late reverberation has been initialised.
	* 
	* The mapping is as follows:
	* 0 -> Sabine
	* 1 -> Eyring
	* 2 -> Custom
	* 
	* @param id The ID corresponding to a reverb time formula.
	*/
	EXPORT void API RACUpdateReverbTimeModel(int id)
	{
		switch (id)
		{
			case(0):
			{ UpdateReverbTime(ReverbFormula::Sabine); break; }
			case(1):
			{ UpdateReverbTime(ReverbFormula::Eyring); break; }
			case(2):
			{ UpdateReverbTime(ReverbFormula::Custom); break; }
			default:
			{ UpdateReverbTime(ReverbFormula::Sabine); break; }
		}
	}

	/**
	* Updates the model used to process diffraction.
	*
	* The mapping is as follows:
	* 0 -> Atteuate (1 / r)
	* 1 -> Low pass filter (1kHz)
	* 2 -> UDFA
	* 3 -> UDFA (shadow zone only)
	* 4 -> NN-IIR (best)
	* 5 -> NN-IIR (small)
	* 6 -> UTD
	* 7 -> BTM
	*
	* @param id The ID corresponding to a diffraction model.
	*/
	EXPORT void API RACUpdateDiffractionModel(int id)
	{
		switch (id)
		{
		case(0):
		{ UpdateDiffractionModel(DiffractionModel::attenuate); break; }
		case(1):
		{ UpdateDiffractionModel(DiffractionModel::lowPass); break; }
		case(2):
		{ UpdateDiffractionModel(DiffractionModel::udfa); break; }
		case(3):
		{ UpdateDiffractionModel(DiffractionModel::udfai); break; }
		case(4):
		{ UpdateDiffractionModel(DiffractionModel::nnBest); break; }
		case(5):
		{ UpdateDiffractionModel(DiffractionModel::nnSmall); break; }
		case(6):
		{ UpdateDiffractionModel(DiffractionModel::utd); break; }
		case(7):
		{ UpdateDiffractionModel(DiffractionModel::btm); break; }
		default:
		{ UpdateDiffractionModel(DiffractionModel::attenuate); break; }
		}
	}

	/**
	* @brief Convert FDN matrix id to enum
	*/
	FDNMatrix SelectFDNMatrix(int fdnMatrix)
	{
		switch (fdnMatrix)
		{
		case(0):
		{ return FDNMatrix::householder; break; }
		case(1):
		{ return FDNMatrix::randomOrthogonal; break; }
		default:
		{ return FDNMatrix::householder; break; }
		}
	}

	/**
	* Updates the volume and dimensions of the room.
	*
	* This function should be called when the volume or dimensions of the room changes and after all walls have been initialised.
	* It will reset the Feedback Delay Network (FDN) so should be considered a new room rather than a dynamic change.
	* 
	* The mapping is as follows:
	* 0 -> Householder
	* 1 -> Random orthogonal
	* 
	* @param volume The volume (m^3) of the room.
	* @param dimensionData The primary dimensions (m) of the room.
	* @param numDimensions The number of dimensions provided in the dimensionData parameter.
	* @param id The ID corresponding to a FDN matrix type.
	* @return True if the late reverb was initialised successfully, false otherwise.
	*/
	EXPORT bool API RACInitLateReverb(float volume, const float* dimensionData, int numDimensions, int id)
	{
		Vec dimensions = Vec(numDimensions);
		for (int i = 0; i < numDimensions; i++)
			dimensions[i] = static_cast<Real>(dimensionData[i]);

		FDNMatrix fdnMatrix = SelectFDNMatrix(id);

		return InitLateReverb(static_cast<Real>(volume), dimensions, fdnMatrix);
	}

	/**
	* Clears the internal FDN buffers.
	*/
	EXPORT void API RACResetFDN()
	{
		ResetFDN();
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
	EXPORT void API RACUpdateListener(float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateListener(Vec3(posX, posY, posZ), Vec4(oriW, oriX, oriY, oriZ));
	}

	/**
	* Initializes a new audio source and returns its ID.
	*
	* This function should be called when a new audio source is created.
	* It will allocate resources for the new source and return an ID that can be used to reference the source in future calls.
	*
	* @return The ID of the new audio source.
	*/
	EXPORT int API RACInitSource()
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
	EXPORT void API RACUpdateSource(int id, float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateSource(static_cast<size_t>(id), Vec3(posX, posY, posZ), Vec4(oriW, oriX, oriY, oriZ));
	}

	/**
	* @brief Convert Directivity id to enum
	*/
	SourceDirectivity SelectDirectivity(int directivity)
	{
		switch (directivity)
		{
		case(0):
		{ return SourceDirectivity::omni; }
		case(1):
		{ return SourceDirectivity::subcardioid; }
		case(2):
		{ return SourceDirectivity::cardioid; }
		case(3):
		{ return SourceDirectivity::supercardioid; }
		case(4):
		{ return SourceDirectivity::hypercardioid; }
		case(5):
		{ return SourceDirectivity::bidirectional; }
		case(6):
		{ return SourceDirectivity::genelec8020c; }
		case(7):
		{ return SourceDirectivity::genelec8020cDTF; }
		case(8):
		{ return SourceDirectivity::qscK8; }
		default:
		{ return SourceDirectivity::omni; }
		}
	}

	/**
	* Updates the directivity of the audio source with the given ID.
	* 
	* The mapping is as follows:
	* 0 -> omni
	* 1 -> subcardioid
	* 2 -> cardioid
	* 3 -> supercardioid
	* 4 -> hypercardioid
	* 5 -> bidirectional
	* 6 -> genelec8020c
	* 7 -> genelec8020c DTF
	* 8 -> qscK8
	* 
	* @param id The ID of the audio source to update.
	* @param directivityID The new directivity of the source.
	*/
	EXPORT void API RACUpdateSourceDirectivity(int id, int directivityID)
	{
		SourceDirectivity directivity = SelectDirectivity(directivityID);
		UpdateSourceDirectivity(static_cast<size_t>(id), directivity);
	}

	/**
	* Removes the audio source with the given ID.
	*
	* This function should be called when an audio source is no longer needed.
	* It will free up any resources that the source was using.
	*
	* @param id The ID of the audio source to remove.
	*/
	EXPORT void API RACRemoveSource(int id)
	{
		RemoveSource(static_cast<size_t>(id));
	}

	/**
	* Initializes a new wall with the given parameters and returns its ID.
	*
	* This function should be called when a new wall is created. A wall must have 3 vertices.
	* It will allocate resources for the new wall and return an ID that can be used to reference the wall in future calls.
	*
	* @param verticesData The vertices of the wall.
	* @param absorptionData The frequency absorption coefficients.
	*
	* @return The ID of the new wall.
	*/
	EXPORT int API RACInitWall(const float* verticesData, const float* absorptionData)
	{
		std::vector<Real> a = std::vector<Real>(NUM_FREQUENCY_BANDS);
		for (int i = 0; i < NUM_FREQUENCY_BANDS; i++)
			a[i] = static_cast<Real>(absorptionData[i]);
		Absorption absorption = Absorption(a);

		Vertices vertices = { Vec3(verticesData[0], verticesData[1], verticesData[2]),
			Vec3(verticesData[3], verticesData[4], verticesData[5]),
			Vec3(verticesData[6], verticesData[7], verticesData[8]) };

		return InitWall(vertices, absorption);
	}

	/**
	* Updates the position and orientation of the wall with the given ID.
	*
	* This function should be called when the position or orientation of a wall changes.
	* It will update the internal representation of the wall to match the new position and orientation.
	*
	* @param id The ID of the wall to update.
	* @param verticesData The vertices of the wall.
	*/
	EXPORT void API RACUpdateWall(int id, const float* verticesData)
	{
		Vertices vertices = { Vec3(verticesData[0], verticesData[1], verticesData[2]),
			Vec3(verticesData[3], verticesData[4], verticesData[5]),
			Vec3(verticesData[6], verticesData[7], verticesData[8]) };

		UpdateWall(static_cast<size_t>(id), vertices);
	}

	/**
	* Updates the absorption of the wall with the given ID.
	*
	* This function should be called when the absorption of a wall changes.
	* It will update the internal representation of the wall to match the new absorption and update the late reverberation time.
	*
	* @param id The ID of the wall to update.
	* @param absorptionData The frequency absorption coefficients.
	*/
	EXPORT void API RACUpdateWallAbsorption(int id, const float* absorptionData)
	{
		std::vector<Real> a = std::vector<Real>(NUM_FREQUENCY_BANDS);
		for (int i = 0; i < NUM_FREQUENCY_BANDS; i++)
			a[i] = static_cast<Real>(absorptionData[i]);
		Absorption absorption = Absorption(a);

		UpdateWallAbsorption(static_cast<size_t>(id), absorption);
	}

	/**
	* Removes the wall with the given ID.
	*
	* This function should be called when a wall is no longer needed.
	* It will free up any resources that the wall was using and remove it from the spatialiser.
	*
	* @param id The ID of the wall to remove.
	*/
	EXPORT void API RACRemoveWall(int id)
	{
		RemoveWall(static_cast<size_t>(id));
	}

	/**
	* Updates the planes and edges of the room.
	*
	* This function should be called after all walls have been updated for a frame.
	* It will update the planes and edges of the room to match the new wall positions and orientations.
	*/
	EXPORT void API RACUpdatePlanesAndEdges()
	{
		UpdatePlanesAndEdges();
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
	EXPORT void API RACSubmitAudio(int id, const float* data)
	{
		Buffer<> buffer = Buffer<>(NUM_FRAMES);
		std::transform(data, data + NUM_FRAMES, buffer.begin(),
			[](float value) { return static_cast<Real>(value); });
		SubmitAudio(static_cast<size_t>(id), buffer);
	}

	/**
	* Processes the output of the spatialiser.
	*
	* This function should be called after all audio sources have been updated for a frame.
	* It will process the late reverberation and prepare the interleaved output buffer.
	*
	* @return True if the processing was successful and the output buffer is ready, false otherwise.
	*/
	EXPORT bool API RACProcessOutput()
	{
		GetOutput(buffer);	
		if (!buffer.Valid())
			return false;
		return true;
	}

	/**
	* Returns a pointer to the output buffer of the spatialiser.
	*
	* This function should be called after RACProcessOutput has returned true.
	* It will return a pointer to the output buffer that contains the processed output buffer.
	*
	* @param buf A pointer to a float pointer. This will be set to point to the interleaved output buffer.
	*/
	EXPORT void API RACGetOutputBuffer(float* sendBuffer)
	{
		for (Real value : buffer)
			*sendBuffer++ = static_cast<float>(value);
	}

	/**
	* Sets the spatialiser to impulse response mode if mode is true
	*
	* This function should be called with true if the output of a stationary source is being recorded.
	* 
	* @param lerpFactor The default interpolation factor.
	* @params mode True if disable all interpolation, false otherwise.
	*/
	EXPORT void API RACUpdateImpulseResponseMode(bool mode)
	{
		UpdateImpulseResponseMode(mode);
	}
}