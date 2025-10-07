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

static Buffer<> buffer(1);	// Return buffer
static int NUM_FREQUENCY_BANDS = 0;	// Store number of frequency bands for any reflection filters
static int NUM_FRAMES = 0;

extern "C"
{
	FDNMatrix SelectFDNMatrix(int mat)
	{
		switch (mat)
		{
		default:
		case(0):
		{ return FDNMatrix::householder; }
		case(1):
		{ return FDNMatrix::randomOrthogonal; }
		}
	}

	ReverbFormula SelectReverbFormula(int formula)
	{
		switch (formula)
		{
		default:
		case(0):
		{ return ReverbFormula::Sabine; }
		case(1):
		{ return ReverbFormula::Eyring; }
		case(2):
		{ return ReverbFormula::Custom; }
		}
	}

	DirectSound SelectDirectMode(int dir)
	{
		switch (dir)
		{
		default:
		case(0):
		{ return DirectSound::none; }
		case(1):
		{ return DirectSound::doCheck; }
		case(2):
		{ return DirectSound::alwaysTrue; }
		}
	}

	DiffractionSound SelectDiffractionMode(int diff)
	{
		switch (diff)
		{
		default:
		case(0):
		{ return DiffractionSound::none; }
		case(1):
		{ return DiffractionSound::shadowZone; }
		case(2):
		{ return DiffractionSound::allZones; }
		}
	}

	DiffractionModel SelectDiffractionModel(int model)
	{
		switch (model)
		{
		default:
		case(0):
		{ return DiffractionModel::attenuate; }
		case(1):
		{ return DiffractionModel::lowPass; }
		case(2):
		{ return DiffractionModel::udfa; }
		case(3):
		{ return DiffractionModel::udfai; }
		case(4):
		{ return DiffractionModel::nnBest; }
		case(5):
		{ return DiffractionModel::nnSmall; }
		case(6):
		{ return DiffractionModel::utd; }
		case(7):
		{ return DiffractionModel::btm; }
		}
	}

	Vec<> CreateVec(const float* data, int length)
	{
		Vec<> vec = Vec<>(length);
		for (int i = 0; i < length; i++)
			vec(i) = static_cast<Real>(data[i]);
		return vec;
	}

	Vec<int> CreateIntVec(const int* data, int length)
	{
		Vec<int> vec = Vec<int>(length);
		for (int i = 0; i < length; i++)
			vec(i) = data[i];
		return vec;
	}

	Coefficients<> CreateCoefficients(const float* data, int length)
	{
		Coefficients<> coeff = Coefficients<>(length);
		for (int i = 0; i < length; i++)
			coeff[i] = static_cast<Real>(data[i]);
		return coeff;
	}

	Coefficients<> CreateAbsorptions(const float* data)
	{
		return CreateCoefficients(data, NUM_FREQUENCY_BANDS);
	}

	//////////////////// API ////////////////////

	/**
	* @brief Initializes the spatialiser with the given parameters.
	*
	* @param fs The sample rate for audio processing.
	* @param numFrames The number of frames in an audio buffer.
	* @param numReverbSources The number of reverb sources.
	* @param fdnSize The number of channels in each feedback delay network
	* @param lerpFactor The interpolation factor for audio parameters.
	* @param Q The quality factor for reflection filters. (0.77 is a good starting point)
	* @param frequencyData The center frequency bands for reflection filters.
	* @param numFrequencyBands The number of frequency bands provided in the fBands parameter.
	*
	* @return True if the initialization was successful, false otherwise.
	*/
	EXPORT bool API RACInit(int fs, int numFrames, int numReverbSources, int fdnSize, float lerpFactor, float Q, const float* frequencyBandsData, int numFrequencyBands)
	{
		buffer.ResizeBuffer(2 * numFrames);
		NUM_FREQUENCY_BANDS = numFrequencyBands;
		NUM_FRAMES = numFrames;

		Coefficients<> frequencyBands = CreateCoefficients(frequencyBandsData, numFrequencyBands);

		return Init(DSPData(fs, numFrames, numReverbSources, fdnSize, static_cast<Real>(lerpFactor), static_cast<Real>(Q), frequencyBands));
	}

	/**
	* @brief Exits and cleans up the spatialiser.
	*
	* This function should be called when the spatialiser is no longer needed.
	* It will free up any resources that the spatialiser is using.
	*/
	EXPORT void API RACExit()
	{
		Exit();
	}

	/**
	* @brief Loads the HRTF, near field and ILD files.
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
	* @brief Initialises the Image Edge Model (IEM) and sets the diffraction model.
	* 
	* The direct sound is mapped as follows.
	* 0 -> none
	* 1 -> doCheck
	* 2 -> alwaysTrue
	* 
	* @param enabled True to enable early reflection DSP, false to disable.
	* @param direct Whether to consider direct sound.
	* @param reflOrder The maximum number of reflections in reflection only paths.
	* @param shadowDiffOrder The maximum number of reflections or diffractions in shadowed diffraction paths.
	* @param specularDiffOrder The maximum number of reflections or diffractions in specular diffraction paths.
	* @param rev Whether to consider late reverberation.
	*/
	EXPORT bool API RACInitEarlyReverb(bool enabled, int direct, int reflOrder, int shadowDiffOrder, int specularDiffOrder, float minEdgeLength, float maxPathLen, int diffractionId)
	{
		DiffractionModel model = SelectDiffractionModel(diffractionId);
		EarlyReverbData data(SelectDirectMode(direct), reflOrder, shadowDiffOrder, specularDiffOrder, static_cast<Real>(minEdgeLength), static_cast<Real>(maxPathLen));
		return InitEarlyReverb(enabled, data, model);
	}

	/**
	* @brief Initialises SingleFDN late reverberation.
	*
	* @param enabled True to enable early reflection DSP, false to disable.
	* @params volume The room volume in cubic meters.
	* @params t60Data The late reverberation time in seconds for each frequency band.
	* @params reverbFormulaId The ID corresponding to a reverberation formula.
	* @parmas dimensionData The primary room dimensions in meters.
	* @params numDimensions The number of dimensions provided in the dimensionData parameter.
	* @params numRays The number of rays to use for ray tracing.
	* @param matrixId The ID corresponding to a FDN matrix type.
	*/
	EXPORT bool API RACInitSingleFDN(bool enabled, float volume, const float* t60Data, int reverbFormulaId, const float* dimensionData, int numDimensions, int numRays, int matrixId)
	{
		Coefficients<> t60 = CreateCoefficients(t60Data, NUM_FREQUENCY_BANDS);
		Vec<> dimensions = CreateVec(dimensionData, numDimensions);

		RoomData roomData(static_cast<Real>(volume), t60, SelectReverbFormula(reverbFormulaId), dimensions);

		LateReverbData data(enabled, 0.0, SelectFDNMatrix(matrixId));
		return InitSingleFDN(roomData, data);
	}

	/**
	* @brief Initialises MoDART late reverberation.
	*
	* @param enabled True to enable early reflection DSP, false to disable.
	* @params numRays The number of rays to use for ray tracing.
	* @param matrixId The ID corresponding to a FDN matrix type.
	* @params delay The delay in seconds before late reverberation starts.
	* @params indexingData The indexing matrix for MoDART.
	* @oarams frequencyData The center frequency bands for each FDN.
	* @params t60sData The late reverberation time in seconds for each FDN.
	* @params leftEigenvectorsData The left eigenvectors for each FDN.
	* @params rightEigenvectorsData The right eigenvectors for each FDN.
	* @params numFDNs The number of FDNs.
	* @params numNodes The number of nodes in the indexing matrix.
	* @params numPaths The number of propagation paths in MoDART.
	*/
	EXPORT bool API RACInitMoDART(bool enabled, int numRays, int matrixId, float delay, float minimumT60, const int* indexingData, const int* frequencyIndexingData, const float* t60sData, const float* leftEigenvectorsData, const float* rightEigenvectorsData, int numFDNs, int numNodes, int numPaths)
	{
		Vec<int> frequencyIndexing = CreateIntVec(frequencyIndexingData, numFDNs);
		Vec<> t60s = CreateVec(t60sData, numFDNs);

		Matrix<int> indexing(numNodes, numNodes);
		for (int i = 0; i < numNodes; i++)
			for (int j = 0; j < numNodes; j++)
				indexing(i, j) = indexingData[i * numNodes + j];

		std::vector<Vec<>> leftEigenvectors, rightEigenvectors;
		for (int i = 0; i < numFDNs; i++)
		{
			leftEigenvectors.push_back(CreateVec(leftEigenvectorsData + i * numPaths, numPaths));
			rightEigenvectors.push_back(CreateVec(rightEigenvectorsData + i * numPaths, numPaths));
		}

		MoDARTData data(enabled, numRays, SelectFDNMatrix(matrixId), static_cast<Real>(delay), static_cast<Real>(minimumT60), indexing, frequencyIndexing, t60s, leftEigenvectors, rightEigenvectors);
		return InitMoDART(data);
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
	* @brief Sets the spatialisation mode (high quality, high performance or none).
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
		default:
		case(0):
		{ UpdateSpatialisationMode(SpatialisationMode::none); break; }
		case(1):
		{ UpdateSpatialisationMode(SpatialisationMode::performance); break; }
		case(2):
		{ UpdateSpatialisationMode(SpatialisationMode::quality); break; }
		}
	}

	/**
	* @brief Enables the late reverberation DSP.
	*
	* @param enable True to enable late reflections, false to disable.
	*/
	EXPORT void API RACEnableEarlyReverb(bool enable)
	{
		EnableEarlyReverb(enable);
	}

	/**
	* @brief Updates the configuration for the Image Edge Model (IEM).
	* 
	* The direct sound is mapped as follows.
	* 0 -> none
	* 1 -> doCheck
	* 2 -> alwaysTrue
	*
	* @param direct Whether to consider direct sound.
	* @param reflOrder The maximum number of reflections in reflection only paths.
	* @param shadowDiffOrder The maximum number of reflections or diffractions in shadowed diffraction paths.
	* @param specularDiffOrder The maximum number of reflections or diffractions in specular diffraction paths.
	* @param rev Whether to consider late reverberation.
	*/
	EXPORT void API RACUpdateEarlyConfig(int direct, int reflOrder, int shadowDiffOrder, int specularDiffOrder, float minEdgeLength, float maxPathLen)
	{
		EarlyReverbData data(SelectDirectMode(direct), reflOrder, shadowDiffOrder, specularDiffOrder, static_cast<Real>(minEdgeLength), static_cast<Real>(maxPathLen));
		UpdateEarlyConfig(data);
	}

	/**
	* @brief Updates the model used to process diffraction.
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
	EXPORT void API RACUpdateDiffractionModel(int diffractionId)
	{
		DiffractionModel model = SelectDiffractionModel(diffractionId);
		UpdateDiffractionModel(model);
	}

	/**
	* @brief Enables the late reverberation DSP.
	*
	* @param enable True to enable late reflections, false to disable.
	*/
	EXPORT void API RACEnableLateReverb(bool enable)
	{
		EnableLateReverb(enable);
	}

	/**
	* @brief Sets the number of rays used in the late reverberation ray tracing.
	*
	* @param numRays The number of rays to use for ray tracing.
	*/
	EXPORT void API RACUpdateLateReverbNumberOfRays(int numRays)
	{
		UpdateLateReverbNumberOfRays(numRays);
	}

	/**
	* @brief Updates the intial delay for MoDART late reverberation.
	*
	* @param delay The initial delay in seconds.
	*/
	EXPORT void API RACUpdateMoDARTDelay(float delay)
	{
		UpdateMoDARTDelay(static_cast<Real>(delay));
	}

	/**
	* @brief Updates the minimum reverberation time to model. Controls the number of modes in MoDART.
	*
	* @param T60 The minimum reverberation time in seconds.
	*/
	EXPORT void API RACUpdateMoDARTMinimumReverbTime(float T60)
	{
		UpdateMoDARTMinimumReverbTime(static_cast<Real>(T60));
	}

	/**
	* @brief Updates the late reverberation time (T60).
	*
	* @param t60 The late reverberation time.s
	*/
	EXPORT void API RACUpdateSingleFDNReverbTime(const float* t60Data)
	{
		
		Coefficients<> t60 = CreateCoefficients(t60Data, NUM_FREQUENCY_BANDS);
		UpdateSingleFDNReverbTime(t60);
	}

	/**
	* @brief Updates the model in order to calculate the late reverberation time (T60).
	*
	* The mapping is as follows:
	* 0 -> Sabine
	* 1 -> Eyring
	* 2 -> Custom
	* 
	* @param id The ID corresponding to a reverb time formula.
	*/
	EXPORT void API RACUpdateSingleFDNReverbTimeModel(int formulaId)
	{
		ReverbFormula formula = SelectReverbFormula(formulaId);
		UpdateSingleFDNReverbTime(formula);
	}

	/**
	* @brief Clears the internal FDN buffers.
	*/
	EXPORT void API RACResetLateReverb()
	{
		ResetLateReverb();
	}

	/**
	* @brief Updates the listener's position and orientation.
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
	* @brief Initializes a new audio source and returns its ID.
	*
	* @details This function should be called when a new audio source is created.
	* It will allocate resources for the new source and return an ID that can be used to reference the source in future calls.
	*
	* @return The ID of the new audio source.
	*/
	EXPORT int API RACInitSource()
	{
		return InitSource();
	}

	/**
	* @brief Updates the position and orientation of the audio source with the given ID.
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
	* @brief Updates the directivity of the audio source with the given ID.
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
	EXPORT void API RACUpdateSourceDirectivity(int id, int directivityId)
	{
		SourceDirectivity directivity = SelectDirectivity(directivityId);
		UpdateSourceDirectivity(static_cast<size_t>(id), directivity);
	}

	/**
	* @brief Removes the audio source with the given ID.
	*
	* @details This function should be called when an audio source is no longer needed.
	* It will free up any resources that the source was using.
	*
	* @param id The ID of the audio source to remove.
	*/
	EXPORT void API RACRemoveSource(int id)
	{
		RemoveSource(static_cast<size_t>(id));
	}

	EXPORT int API RACInitMaterial(const float* absorptionData)
	{
		Coefficients<> absorption = CreateAbsorptions(absorptionData);
		return InitMaterial(absorption);
	}

	/**
	* @brief Updates the absorption of the wall with the given ID.
	*
	* @details This function should be called when the absorption of a wall changes.
	* It will update the internal representation of the wall to match the new absorption and update the late reverberation time.
	*
	* @param id The ID of the wall to update.
	* @param absorption The frequency absorption coefficients.
	*/
	EXPORT void API RACUpdateMaterial(int id, const float* absorptionData)
	{
		Coefficients<> absorption = CreateAbsorptions(absorptionData);
		UpdateMaterial(static_cast<size_t>(id), absorption);
	}

	EXPORT void API RACRemoveMaterial(int id)
	{
		RemoveMaterial(static_cast<size_t>(id));
	}

	/**
	* @brief Initializes a new wall with the given parameters and returns its ID.
	*
	* @details This function should be called when a new wall is created. A wall must have 3 vertices.
	* It will allocate resources for the new wall and return an ID that can be used to reference the wall in future calls.
	*
	* @param verticesData The vertices of the wall.
	* @param absorptionData The frequency absorption coefficients.
	*
	* @return The ID of the new wall.
	*/
	EXPORT int API RACInitWall(const float* verticesData, int materialId)
	{
		Vertices vertices = { Vec3(verticesData[0], verticesData[1], verticesData[2]),
			Vec3(verticesData[3], verticesData[4], verticesData[5]),
			Vec3(verticesData[6], verticesData[7], verticesData[8]) };

		return InitWall(vertices, materialId);
	}

	/**
	* @brief Updates the position and orientation of the wall with the given ID.
	*
	* @details This function should be called when the position or orientation of a wall changes.
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
	* @brief Removes the wall with the given ID.
	*
	* @details This function should be called when a wall is no longer needed.
	* It will free up any resources that the wall was using and remove it from the spatialiser.
	*
	* @param id The ID of the wall to remove.
	*/
	EXPORT void API RACRemoveWall(int id)
	{
		RemoveWall(static_cast<size_t>(id));
	}

	/**
	* @brief Updates the planes and edges of the room.
	*
	* @details This function should be called after all walls have been updated for a frame.
	* It will update the planes and edges of the room to match the new wall positions and orientations.
	*/
	EXPORT void API RACUpdatePlanesAndEdges()
	{
		UpdatePlanesAndEdges();
	}

	/**
	* @brief Updates the late reverberation gain.
	* 
	* @param gain The new late reverberation gain.
	*/
	EXPORT void API RACUpdateLateReverbGain(float gain)
	{
		UpdateLateReverbGain(static_cast<Real>(gain));
	}

	/**
	* @brief Submits an audio buffer to the audio source with the given ID.
	*
	* @details This function should be called when there is a new audio buffer for a source.
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
	* @brief Processes the output of the spatialiser.
	*
	* @details This function should be called after all audio sources have been updated for a frame.
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
	* @brief Returns a pointer to the output buffer of the spatialiser.
	*
	* @details This function should be called after RACProcessOutput has returned true.
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
	* @brief Record an impulse response using the current listener position
	*
	* Assumes listener position does not change during recording
	*
	* @param posX The x-coordinate of the source's position.
	* @param posY The y-coordinate of the source's position.
	* @param posZ The z-coordinate of the source's position.
	* @param oriW The w-component of the source's orientation quaternion.
	* @param oriX The x-component of the source's orientation quaternion.
	* @param oriY The y-component of the source's orientation quaternion.
	* @param oriZ The z-component of the source's orientation quaternion.
	* @params outputBuffer Buffer to write to.
	*/
	EXPORT void API RACRecordImpulseResponse(float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ, float* sendBuffer, int numSamples)
	{
		Buffer<> outputBuffer(numSamples);
		RecordImpulseResponse(Vec3(posX, posY, posZ), Vec4(oriW, oriX, oriY, oriZ), outputBuffer);
		for (Real value : outputBuffer)
			*sendBuffer++ = static_cast<float>(value);
	}

	/**
	* @brief Sets the spatialiser to impulse response mode if mode is true
	*
	* @details This function should be called with true if the output of a stationary source is being recorded.
	* 
	* @param lerpFactor The default interpolation factor.
	* @params mode True if disable all interpolation, false otherwise.
	*/
	EXPORT void API RACUpdateImpulseResponseMode(bool mode)
	{
		UpdateImpulseResponseMode(mode);
	}
}