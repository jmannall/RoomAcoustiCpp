Provides the main API for interacting with the RoomAcoustiC++ library, including initialisation, listener control, source and geometry creation and manipulation, and audio submission and retrieval.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/Interface.h`
- **Source:** `Spatialiser/Interface.cpp`
- **Dependencies:** `Common/Vec.h`, `Common/Vec3.h`, `Common/Vec4.h`, `Common/Types.h`, `DSP/Buffer.h`, `Spatialiser/Types.h`, `Spatialiser/Configs.h`, `Spatialiser/ContextOptionalArguments.h`

---

## Configuration

### `#!cpp bool Init(const DSPData& data, const ContextOptionalArguments &optionalArguments)`
Initializes the spatialiser with the given configuration.

`data`: The configuration of the spatialiser.  
`optionalArguments`: Optional arguments.  
**Returns:** True if initialization was successful, false otherwise.

---

### `#!cpp void Exit()`
Cleans up and exits the spatialiser.

---

### `#!cpp bool LoadSpatialisationFiles(const int hrtfResamplingStep, const std::vector<std::string>& filePaths)`
Loads HRTF and related files for spatialisation.

`hrtfResamplingStep`: Step size for HRTF resampling.  
`filePaths`: List of file paths.  
**Returns:** True if files loaded successfully.

---

### `#!cpp void SetHeadphoneEQ(const Buffer<>& leftIR, const Buffer<>& rightIR)`
Sets headphone EQ filters.

`leftIR`: Left channel impulse response.  
`rightIR`: Right channel impulse response.

---

### `#!cpp void UpdateSpatialisationMode(const SpatialisationMode mode)`
Sets the spatialisation mode (none: mono, performance: interaural time and level differences or quality: HRTF).

`mode`: New spatialisation mode.

---

## Early Reverberation (IEM)

### `#!cpp void EnableEarlyReverb(const bool enable)`
Toggles the Image Edge Model on or off.

`enable`: True to enable early reflections, false to disable.

---

### `#!cpp void UpdateEarlyConfig(const EarlyReverbData& data)`
Updates the Image Edge Model configuration.

`data`: The new configuration for the IEM.

---

### `#!cpp void UpdateDiffractionModel(const DiffractionModel model)`
Updates the model used to process diffraction.

`model`: The diffraction model.

---

### `#!cpp bool InitEarlyReverb(const bool enabled, const EarlyReverbData& data, const DiffractionModel model)`
Initialises the Image Edge Model (IEM) and sets the diffraction model.

`enabled`: True to enable early reflection DSP, false to disable.  
`data`: The user defined IEM configuration data.  
`model`: The diffraction model to use.  
**Returns:** True if the early reverberation DSP was initialised successfully, false otherwise.

---

## Late Reverberation (MoD-ART or single FDN)

### `#!cpp void EnableLateReverb(const bool enable)`
Toggles the late reverberation DSP on or off.

`enable`: True to enable late reflections, false to disable.

---

### `#!cpp void UpdateLateReverbNumberOfRays(const int numRays)`
Sets the number of rays used in the late reverberation ray tracing.

`numRays`: The number of rays to use for ray tracing.

---

### `#!cpp void UpdateLateReverbDistanceThresholds(const Real sourceThresh, const Real listenerThresh)`
Sets the distance thresholds (in meters) from the latest updated position which triggers an update of late reverberation tracing.

`sourceThresh`: The distance threshold for all sources.  
`listenerThresh`: The distance threshold for the listener.

---

### `#!cpp void UpdateSelfShadowingRadius(const Real radius)`
Sets the sphere radius (in meters) used to determine self-shadowing during late reverberation tracing.

`radius`: The radius of the listener head used for self-shadowing.

---

### `#!cpp void UpdateMoDARTDelay(const Real delay)`
Updates the initial delay for MoDART late reverberation.

`delay`: The initial delay in seconds.

---

### `#!cpp void UpdateMoDARTMinimumReverbTime(const Real T60)`
Updates the minimum reverberation time to model. This controls the number of modes in MoDART.

`T60`: The minimum reverberation time in seconds.

---

### `#!cpp void UpdateSingleFDNReverbTime(const ReverbFormula model)`
Updates the model used to calculate the late reverberation time (T60).

`model`: The model used to calculate the late reverberation time.

---

### `#!cpp void UpdateSingleFDNReverbTime(const Coefficients<>& T60)`
Overrides the current late reverberation time (T60).

`T60`: The late reverberation time (frequency dependent).

---

### `#!cpp bool InitSingleFDN(const RoomData& roomData, const LateReverbData& data)`
Initialises SingleFDN late reverberation.

`roomData`: The user defined room configuration data.  
`data`: The user defined SingleFDN configuration data.  
**Returns:** True if the SingleFDN late reverberation was initialised successfully, false otherwise.

---

### `#!cpp bool InitMoDART(const MoDARTData& data)`
Initialises MoDART late reverberation.

`data`: The user defined MoDART configuration data.  
**Returns:** True if the MoDART late reverberation was initialised successfully, false otherwise.

---

### `#!cpp void ResetLateReverb()`
Clears the internal late reverberation buffers.

---

### `#!cpp void UpdateLateReverbGain(const Real gain)`
Updates the late reverberation gain.

`gain`: The new late reverberation gain.

---

## Listener

### `#!cpp void UpdateListener(const Vec3& position, const Vec4& orientation)`
Updates the listener's position and orientation.

`position`: The new position of the listener.  
`orientation`: The new orientation of the listener (quaternion).

---

## Sources

### `#!cpp int InitSource()`
Initializes a new audio source.

**Returns:** The ID of the new audio source.

---

### `#!cpp void UpdateSource(const size_t id, const Vec3& position, const Vec4& orientation)`
Updates the position and orientation of the audio source with the given ID.

`id`: The ID of the audio source to update.  
`position`: The new position of the source.  
`orientation`: The new orientation of the source (quaternion).

---

### `#!cpp void UpdateSourceDirectivity(const size_t id, const SourceDirectivity directivity)`
Updates the directivity of the audio source with the given ID.

`id`: The ID of the audio source to update.  
`directivity`: The new directivity of the source.

---

### `#!cpp void RemoveSource(const size_t id)`
Removes the audio source with the given ID.

`id`: The ID of the audio source to remove.

---

## Geometry

### `#!cpp int InitMaterial(const Coefficients<>& material)`
Initialises a new material with the given absorption parameters.

`material`: The frequency absorption coefficients.  
**Returns:** The ID of the new material.

---

### `#!cpp void UpdateMaterial(size_t id, const Coefficients<>& material)`
Updates the material with the given ID.

`id`: The ID of the material to update.  
`material`: The frequency absorption coefficients.

---

### `#!cpp void RemoveMaterial(size_t id)`
Removes the material with the given ID.

`id`: The ID of the material to remove.

---

### `#!cpp int InitWall(const Vertices& vData, const size_t materialId)`
Initializes a new wall with the given parameters.

`vData`: The vertices of the wall.  
`materialId`: The ID of the material to use for this wall.  
**Returns:** The ID of the new wall.

---

### `#!cpp void UpdateWall(size_t id, const Vertices& vData)`
Updates the vertices of the wall with the given ID.

`id`: The ID of the wall to update.  
`vData`: The new vertices of the wall.

---

### `#!cpp void RemoveWall(size_t id)`
Removes the wall with the given ID.

`id`: The ID of the wall to remove.

---

### `#!cpp void UpdatePlanesAndEdges()`
Rebuilds internal room planes and edges after geometry changes (e.g., after adding, updating, or removing walls).

---

## Audio I/O

### `#!cpp void SubmitAudio(size_t id, const Buffer<>& data)`
Submits an audio buffer to the audio source with the given ID.

`id`: The ID of the audio source to submit audio to.  
`data`: The audio data for the source.

---

### `#!cpp void GetOutput(Buffer<>& outputBuffer)`
Processes audio for the current audio callback and writes the output.

If `outputBuffer.Length()` does not match the required number of samples (interleaved stereo), it will be resized.

`outputBuffer`: Buffer to write the audio output to.

---

### `#!cpp void RecordImpulseResponse(const Vec3& position, const Vec4& orientation, Buffer<>& outputBuffer)`
Records an impulse response using the current listener position.

Assumes the listener position does not change during recording.

`position`: The source position.  
`orientation`: The source orientation (quaternion).  
`outputBuffer`: Buffer to write the recorded impulse response to.

---

## Example Usage

```cpp
#include "Spatialiser/Interface.h"
using namespace RAC::Spatialiser;

DSPData config;
Init(config);

bool success = LoadSpatialisationFiles(5, { "path/to/hrtf", "path/to/ildNearField", "path/to/ild" });
UpdateSpatialisationMode(SpatialisationMode::quality);

EarlyReverbData iemConfig(DirectSound::doCheck, 2, 2, 1, 0.0, 1e10);
InitEarlyReverb(true, iemConfig, DiffractionModel::nnSmall);

UpdateListener(Vec3(0, 2, 0), Vec4(1, 0, 0, 0));

// Create shoebox
Vec3 pos(7, 3, 4);
double volume = pos.x() * pos.y() * pos.z();

Coefficients<> absorption(std::vector<Real>({ 0.03, 0.04, 0.06, 0.1, 0.12 }));
size_t materialId = (size_t)InitMaterial(absorption);

std::vector<size_t> wallIDs(12);
wallIDs[0] = InitWall({ Vec3(0.0, pos.y(), 0.0),
        Vec3(pos.x(), pos.y(), 0.0),
        Vec3(pos.x(), pos.y(), pos.z()) }, materialId);
wallIDs[1] = InitWall({ Vec3(0.0, pos.y(), 0.0),
        Vec3(pos.x(), pos.y(), pos.z()),
        Vec3(0.0, pos.y(), pos.z()) }, materialId);
wallIDs[2] = InitWall({ Vec3(pos.x(), 0.0, 0.0),
        Vec3(0.0, 0.0, 0.0),
        Vec3(0.0, 0.0, pos.z()) }, materialId);
wallIDs[3] = InitWall({ Vec3(pos.x(), 0.0, 0.0),
        Vec3(0.0, 0.0, pos.z()),
        Vec3(pos.x(), 0.0, pos.z()) }, materialId);
wallIDs[4] = InitWall({ Vec3(pos.x(), 0.0, pos.z()),
        Vec3(pos.x(), pos.y(), pos.z()),
        Vec3(pos.x(), pos.y(), 0.0) }, materialId);
wallIDs[5] = InitWall({ Vec3(pos.x(), 0.0, pos.z()),
        Vec3(pos.x(), pos.y(), 0.0),
        Vec3(pos.x(), 0.0, 0.0) }, materialId);
wallIDs[6] = InitWall({ Vec3(0.0, 0.0, 0.0),
        Vec3(0.0, pos.y(), 0.0),
        Vec3(0.0, pos.y(), pos.z()) }, materialId);
wallIDs[7] = InitWall({ Vec3(0.0, 0.0, 0.0),
        Vec3(0.0, pos.y(), pos.z()),
        Vec3(0.0, 0.0, pos.z()) }, materialId);
wallIDs[8] = InitWall({ Vec3(0.0, 0.0, 0.0),
        Vec3(pos.x(), 0.0, 0.0),
        Vec3(pos.x(), pos.y(), 0.0) }, materialId);
wallIDs[9] = InitWall({ Vec3(0.0, 0.0, 0.0),
        Vec3(pos.x(), pos.y(), 0.0),
        Vec3(0.0, pos.y(), 0.0) }, materialId);
wallIDs[10] = InitWall({ Vec3(0.0, pos.y(), pos.z()),
        Vec3(pos.x(), pos.y(), pos.z()),
        Vec3(pos.x(), 0.0, pos.z()) }, materialId);
wallIDs[11] = InitWall({ Vec3(0.0, pos.y(), pos.z()),
        Vec3(pos.x(), 0.0, pos.z()),
        Vec3(0.0, 0.0, pos.z()) }, materialId);
UpdatePlanesAndEdges();

Vec<> dimensions(3);
dimensions[0] = (Real)pos.z();
dimensions[1] = (Real)pos.y();
dimensions[2] = (Real)pos.z();

Coefficients<> T60 = Coefficients<>::Constant(config.frequencyBands.Length(), (Real)1.0);
RoomData roomData((Real)volume, T60, ReverbFormula::Sabine, dimensions);

LateReverbData lateData(true, 1000, FDNMatrix::randomOrthogonal);
InitSingleFDN(roomData, lateData);

size_t id = (size_t)InitSource();
UpdateSourceDirectivity(id, SourceDirectivity::genelec8020c);
UpdateSource(id, Vec3(1, 2, 3), Vec4(1, 0, 0, 0));

SubmitAudio(id, Buffer<>(config.numFrames));

Buffer<> output(2 * config.numFrames);
GetOutput(output);

RemoveSource(id);
for (size_t wallID : wallIDs)
    RemoveWall(wallID);

RemoveMaterial(materialId);
Exit();

```