Provides the main API for interacting with the RoomAcoustiC++ library, including initialisation, listener control, source and geometry creation and manipulation, and audio submission and retrieval.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/Interface.h`
- **Source:** `Spatialiser/Interface.cpp`
- **Dependencies:** `Common/Vec.h`, `Common/Vec3.h`, `Common/Vec4.h`, `Common/Types.h`, `DSP/Buffer.h`, `Spatialiser/Types.h`, `Spatialiser/Context.h`

---

## Configuration

### `#!cpp bool Init(const std::shared_ptr<Config> config)`
Initializes the spatialiser with the given configuration.

`config`: Shared pointer to configuration object.  
**Returns:** True if initialization was successful.

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

### `#!cpp void UpdateIEMConfig(const IEMConfig& config)`
Updates the Image Edge Model configuration.

`config`: IEM configuration.

---

### `#!cpp void UpdateReverbTime(const ReverbFormula model)`
Sets the late reverberation time calculation model (Sabine or Eyring's formula)

`model`: Reverberation formula.

---

### `#!cpp void UpdateReverbTime(const Coefficients<>& T60)`
Overrides the late reverberation time.

`T60`: Reverberation time coefficients.

---

### `#!cpp void UpdateDiffractionModel(const DiffractionModel model)`
Sets the diffraction model (Attenuate, LPF, UDFA, UDFA-I, NNBest, NNSmall, UTD, BTM).

`model`: Diffraction model.

---

### `#!cpp void InitLateReverb(const Real volume, const Vec& dimensions, FDNMatrix matrix)`
Initializes late reverberation with room volume, dimensions, and FDN matrix.

`volume`: Room volume.
`dimensions`: Room dimensions.
`matrix`: FDN feedback matrix.

---

### `#!cpp void ResetFDN()`
Clears internal FDN buffers.

---

## Listener

### `#!cpp void UpdateListener(const Vec3& position, const Vec4& orientation)`
Sets the listener's position and orientation.

- `position`: Listener position.
- `orientation`: Listener orientation (quaternion).

---

## Sources

### `#!cpp size_t InitSource()`
Creates a new audio source.

- **Returns:** Source ID.

---

### `#!cpp void UpdateSource(const size_t id, const Vec3& position, const Vec4& orientation)`
Updates the position and orientation of a source.

- `id`: Source ID.
- `position`: New position.
- `orientation`: New orientation.

---

### `#!cpp void UpdateSourceDirectivity(const size_t id, const SourceDirectivity directivity)`
Sets the directivity pattern for a source.

- `id`: Source ID.
- `directivity`: Directivity pattern.

---

### `#!cpp void RemoveSource(const size_t id)`
Removes a source.

- `id`: Source ID.

---

## Geometry

### `#!cpp size_t InitWall(const Vertices& vData, const Absorption<>& absorption)`
Creates a new wall with vertices and absorption.

- `vData`: Wall vertices.
- `absorption`: Absorption coefficients.
- **Returns:** Wall ID.

---

### `#!cpp void UpdateWall(size_t id, const Vertices& vData)`
Updates the geometry of a wall.

- `id`: Wall ID.
- `vData`: New vertices.

---

### `#!cpp void UpdateWallAbsorption(size_t id, const Absorption<>& absorption)`
Updates the absorption of a wall.

- `id`: Wall ID.
- `absorption`: New absorption coefficients.

---

### `#!cpp void RemoveWall(size_t id)`
Removes a wall.

- `id`: Wall ID.

---

### `#!cpp void UpdatePlanesAndEdges()`
Updates room planes and edges. Should be called after any updates to walls.

---

## Audio

### `#!cpp void SubmitAudio(size_t id, const Buffer<>& data)`
Submits an audio buffer for a source.

- `id`: Source ID.
- `data`: Audio buffer.

---

### `#!cpp void GetOutput(float** bufferPtr)`
Retrieves the processed output buffer.

- `bufferPtr`: Pointer to output buffer.

---

### `#!cpp void UpdateImpulseResponseMode(const bool mode)`
Sets impulse response mode.

- `mode`: If true disables all interpolation.

---

## Example Usage

```cpp
#include "Spatialiser/Interface.h"
using namespace RAC::Spatialiser;

std::shared_ptr<Config> config = std::make_shared<Config>();
Init(config);
bool success = RACLoadSpatialisationFiles(5, { "path/to/hrtf", "path/to/ildNearField", "path/to/ild" });
UpdateSpatialisationMode(SpatialisationMode::Quality);

IEMConfig iemConfig(DirectSound::doCheck, 2, 2, 1, true, 0.0);
UpdateIEMConfig(iemConfig);

UpdateListener(Vec3(0, 2, 0), Vec4(1, 0, 0, 0));

// Create shoebox
Vec3 pos(7, 3, 4);
double volume = pos.x * pos.y * pos.z;
Absorption<> absorption(0.03, 0.04, 0.06, 0.1, 0.12);
std::vector<size_t> wallIDs(12);
wallIDs[0] = InitWall({ Vec3(0.0, pos.y, 0.0),
        Vec3(pos.x, pos.y, 0.0),
        Vec3(pos.x, pos.y, pos.z) }, absorption);
wallIDs[1] = InitWall({ Vec3(0.0, pos.y, 0.0),
        Vec3(pos.x, pos.y, pos.z),
        Vec3(0.0, pos.y, pos.z) }, absorption);
wallIDs[2] = InitWall({ Vec3(pos.x, 0.0, 0.0),
        Vec3(0.0, 0.0, 0.0),
        Vec3(0.0, 0.0, pos.z) }, absorption);
wallIDs[3] = InitWall({ Vec3(pos.x, 0.0, 0.0),
        Vec3(0.0, 0.0, pos.z),
        Vec3(pos.x, 0.0, pos.z) }, absorption);
wallIDs[4] = InitWall({ Vec3(pos.x, 0.0, pos.z),
        Vec3(pos.x, pos.y, pos.z),
        Vec3(pos.x, pos.y, 0.0) }, absorption);
wallIDs[5] = InitWall({ Vec3(pos.x, 0.0, pos.z),
        Vec3(pos.x, pos.y, 0.0),
        Vec3(pos.x, 0.0, 0.0) }, absorption);
wallIDs[6] = InitWall({ Vec3(0.0, 0.0, 0.0),
        Vec3(0.0, pos.y, 0.0),
        Vec3(0.0, pos.y, pos.z) }, absorption);
wallIDs[7] = InitWall({ Vec3(0.0, 0.0, 0.0),
        Vec3(0.0, pos.y, pos.z),
        Vec3(0.0, 0.0, pos.z) }, absorption);
wallIDs[8] = InitWall({ Vec3(0.0, 0.0, 0.0),
        Vec3(pos.x, 0.0, 0.0),
        Vec3(pos.x, pos.y, 0.0) }, absorption);
wallIDs[9] = InitWall({ Vec3(0.0, 0.0, 0.0),
        Vec3(pos.x, pos.y, 0.0),
        Vec3(0.0, pos.y, 0.0) }, absorption);
wallIDs[10] = InitWall({ Vec3(0.0, pos.y, pos.z),
        Vec3(pos.x, pos.y, pos.z),
        Vec3(pos.x, 0.0, pos.z) }, absorption);
wallIDs[11] = InitWall({ Vec3(0.0, pos.y, pos.z),
        Vec3(pos.x, 0.0, pos.z),
        Vec3(0.0, 0.0, pos.z) }, absorption);
UpdatePlanesAndEdges();

InitLateReverb(volume, Vec(pos.z, pos.y, pos.z), FDNMatrix::RandomOrthogonal);

size_t id = InitSource();
UpdateDirectivity(id, SourceDirectivity::Genelec8020c);
UpdateSource(id, Vec3(1, 2, 3), Vec4(1, 0, 0, 0));

SubmitAudio(id, Buffer<>(config->numFrames));

Buffer<float> output(2 * config->numFrames);
float* outputPtr = &output[0];
GetOutput(&outputPtr);

RemoveSource(id);
for (size_t wallID : wallIDs)
    RemoveWall(wallID);
Exit();

```