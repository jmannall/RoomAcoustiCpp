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
using namespace RAC::Unity;

//////////////////// Variables ////////////////////

static float* buffer = nullptr;	// Pointer to return buffer
static int numAbsorptionBands = 0;	// Store number of frequency bands for any reflection filters
static int numAudioFrames = 0;

extern "C"
{

	//////////////////// API ////////////////////

	/**
	* Initializes the spatialiser with the given parameters.
	*
	* @param fs The sample rate for audio processing.
	* @param numFrames The number of frames in an audio buffer.
	* @param maxFDNChannels The number of feedback delay network channels.
	* @param lerpFactor The interpolation factor for audio parameters.
	* @param Q The quality factor for reflection filters. (0.77 is a good starting point)
	* @param fBands The center frequency bands for reflection filters.
	* @param numBands The number of frequency bands provided in the fBands parameter.
	*
	* @return True if the initialization was successful, false otherwise.
	*/
	EXPORT bool API RACInit(int fs, int numFrames, int maxFDNChannels, float lerpFactor, float Q, const float* fBands, int numBands)
	{

		int numFDNChannels = 0;
		if (maxFDNChannels < 1)
		{ numFDNChannels = 0; }
		else if (maxFDNChannels < 2)
		{ numFDNChannels = 1; }
		else if (maxFDNChannels < 4)
		{ numFDNChannels = 2; }
		else if (maxFDNChannels < 6)
		{ numFDNChannels = 4; }
		else if (maxFDNChannels < 8)
		{ numFDNChannels = 6; }
		else if (maxFDNChannels < 12)
		{ numFDNChannels = 8; }
		else if (maxFDNChannels < 16)
		{ numFDNChannels = 12; }
		else if (maxFDNChannels < 20)
		{ numFDNChannels = 16; }
		else if (maxFDNChannels < 24)
		{ numFDNChannels = 20; }
		else if (maxFDNChannels < 32)
		{ numFDNChannels = 24; }
		else { numFDNChannels = 32; }

		numAbsorptionBands = numBands;
		numAudioFrames = numFrames;
		Coefficients frequencyBands = Coefficients(numBands);
		for (int i = 0; i < numBands; i++)
			frequencyBands[i] = static_cast<Real>(fBands[i]);

		std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numFDNChannels, static_cast<Real>(lerpFactor), static_cast<Real>(Q), frequencyBands);
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
	* @brief Sets the headphone EQ filters.
	*
	* @param leftIR The impulse response for the left channel.
	* @param rightIR The impulse response for the right channel.
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
	* The mapping is as follows:
	* 0 -> none
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


	DirectSound SelectDirectMode(int dir)
	{
		switch (dir)
		{
		case(0):
		{ return DirectSound::none; break; }
		case(1):
		{ return DirectSound::doCheck; break; }
		case(2):
		{ return DirectSound::alwaysTrue; break; }
		default:
		{ return DirectSound::none; break; }
		}
	}

	DiffractionSound SelectDiffractionMode(int diff)
	{
		switch (diff)
		{
		case(0):
		{ return DiffractionSound::none; break; }
		case(1):
		{ return DiffractionSound::shadowZone; break; }
		case(2):
		{ return DiffractionSound::allZones; break; }
		default:
		{ return DiffractionSound::none; break; }
		}
	}

	/**
	* Updates the configuration for the Image Edge Model (IEM).
	* 
	* The direct sound is mapped as follows.
	* 0 -> none
	* 1 -> doCheck
	* 2 -> alwaysTrue
	*
	* @param dir Whether to consider direct sound.
	* @param reflOrder The maximum number of reflections in reflection only paths.
	* @param shadowDiffOrder The maximum number of reflections or diffractions in shadowed diffraction paths.
	* @param specularDiffOrder The maximum number of reflections or diffractions in specular diffraction paths.
	* @param rev Whether to consider late reverberation.
	*/
	EXPORT void API RACUpdateIEMConfig(int dir, int reflOrder, int shadowDiffOrder, int specularDiffOrder, bool rev, float edgeLen)
	{
		DirectSound direct = SelectDirectMode(dir);
		UpdateIEMConfig(IEMConfig(direct, reflOrder, shadowDiffOrder, specularDiffOrder, rev, static_cast<Real>(edgeLen)));
	}

	/**
	* Updates the late reverberation time (T60).
	*
	* @param t60 The late reverberation time.s
	*/
	EXPORT void API RACUpdateReverbTime(const float* t60)
	{
		Coefficients T60 = Coefficients<>(static_cast<size_t>(numAbsorptionBands));
		for (int i = 0; i < numAbsorptionBands; i++)
			T60[i] = static_cast<Real>(t60[i]);

		UpdateReverbTime(T60);
	}

	/**
	* Updates the model in order to calculate the late reverberation time (T60).
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
	* 3 -> UDFA (interpolated)
	* 4 -> NN-IIR (best)
	* 5 -> NN-IIR (small)
	* 6 -> UTD
	* 7 -> Btm
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
	* @param dim The dimensions (m) of the room for the delay lines.
	* @param numDimensions The number of dimensions provided in the dim parameter. This must be a factor of numFDNChannels set in RACInit. 
	* @param id The ID corresponding to a FDN matrix type.
	*/
	EXPORT void API RACUpdateRoom(float volume, const float* dim, int numDimensions, int id)
	{
		Vec dimensions = Vec(numDimensions);
		for (int i = 0; i < numDimensions; i++)
			dimensions[i] = static_cast<Real>(dim[i]);

		FDNMatrix fdnMatrix = FDNMatrix::householder; // Default to householder matrix
		switch (id)
		{
		case(0):
		{ fdnMatrix = FDNMatrix::householder; break; }
		case(1):
		{ fdnMatrix = FDNMatrix::randomOrthogonal; break; }
		}

		InitLateReverb(static_cast<Real>(volume), dimensions, fdnMatrix);
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
		return static_cast<int>(InitSource());
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
	* @param directivity The new directivity of the source.
	*/
	EXPORT void API RACUpdateSourceDirectivity(int id, int directivity)
	{
		switch (directivity)
		{
		case(0):
		{ UpdateSourceDirectivity(id, SourceDirectivity::omni); break; }
		case(1):
		{ UpdateSourceDirectivity(id, SourceDirectivity::subcardioid); break; }
		case(2):
		{ UpdateSourceDirectivity(id, SourceDirectivity::cardioid); break; }
		case(3):
		{ UpdateSourceDirectivity(id, SourceDirectivity::supercardioid); break; }
		case(4):
		{ UpdateSourceDirectivity(id, SourceDirectivity::hypercardioid); break; }
		case(5):
		{ UpdateSourceDirectivity(id, SourceDirectivity::bidirectional); break; }
		case(6):
		{ UpdateSourceDirectivity(id, SourceDirectivity::genelec8020c); break; }
		case(7):
		{ UpdateSourceDirectivity(id, SourceDirectivity::genelec8020cDTF); break; }
		case(8):
		{ UpdateSourceDirectivity(id, SourceDirectivity::qscK8); break; }
		default:
		{ UpdateSourceDirectivity(id, SourceDirectivity::omni); break; }
		}
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
	* @param nX The x-coordinate of the wall's normal vector.
	* @param nY The y-coordinate of the wall's normal vector.
	* @param nZ The z-coordinate of the wall's normal vector.
	* @param vData The vertices of the wall.
	* @param absorption The frequency absorption coefficients.
	*
	* @return The ID of the new wall.
	*/
	EXPORT int API RACInitWall(const float* vData, const float* absorption)
	{
		std::vector<Real> a = std::vector<Real>(numAbsorptionBands);
		for (int i = 0; i < numAbsorptionBands; i++)
			a[i] = static_cast<Real>(absorption[i]);
		Absorption abs = Absorption(a);

		Vertices vertices = { Vec3(vData[0], vData[1], vData[2]),
			Vec3(vData[3], vData[4], vData[5]),
			Vec3(vData[6], vData[7], vData[8]) };

		return static_cast<int>(InitWall(vertices, abs));
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
	*/
	EXPORT void API RACUpdateWall(int id, const float* vData)
	{
		Vertices vertices = { Vec3(vData[0], vData[1], vData[2]),
			Vec3(vData[3], vData[4], vData[5]),
			Vec3(vData[6], vData[7], vData[8]) };

		UpdateWall(static_cast<size_t>(id), vertices);
	}

	/**
	* Updates the absorption of the wall with the given ID.
	*
	* This function should be called when the absorption of a wall changes.
	* It will update the internal representation of the wall to match the new absorption and update the late reverberation time.
	*
	* @param id The ID of the wall to update.
	* @param absorption The frequency absorption coefficients.
	*/
	EXPORT void API RACUpdateWallAbsorption(int id, const float* absorption)
	{
		std::vector<Real> a = std::vector<Real>(numAbsorptionBands);
		for (int i = 0; i < numAbsorptionBands; i++)
			a[i] = static_cast<Real>(absorption[i]);
		Absorption abs = Absorption(a);

		UpdateWallAbsorption(static_cast<size_t>(id), abs);
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
	* Updates the late reverberation gain.
	* 
	* @param gain The new late reverberation gain.
	*/
	EXPORT void API RACUpdateLateReverbGain(float gain)
	{
		UpdateLateReverbGain(static_cast<Real>(gain));
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
		Buffer<> buffer = Buffer<>(numAudioFrames);
		std::transform(data, data + numAudioFrames, buffer.begin(),
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
		GetOutput(&buffer);	
		if (!buffer)
			return false;
		if (std::isnan(*buffer))
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
	EXPORT void API RACGetOutputBuffer(float** buf)
	{
		*buf = buffer;
	}

	/**
	* Sets the spatialiser to impulse response mode if mode is true
	*
	* This function should be called if the output of a stationary source is being recorded.
	* Call the function again with the original lerp factor to restore normal operation.
	* 
	* @param lerpFactor The default interpolation factor.
	* @params mode True if disable 3DTI Interpolation, false otherwise.
	*/
	EXPORT void API RACUpdateImpulseResponseMode(float lerpFactor, bool mode)
	{
		UpdateImpulseResponseMode(static_cast<Real>(lerpFactor), mode);
	}
}