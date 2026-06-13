Provides the public data types (typedefs and enums) used by the RoomAcoustiC++ API.

These types are used when configuring the spatialiser and when updating listener/source state.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/Types.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Coefficients.h`, `Common/Vec3.h`

---

## Typedefs

### `#!cpp using Vertices = std::array<Vec3, 3>`
Triangle vertices used to define a wall surface.

---

## Enums

### `#!cpp enum class SpatialisationMode`
Controls how direct sound is spatialised.

- `SpatialisationMode::none`: no spatialisation (mono)
- `SpatialisationMode::performance`: performance mode (interaural time and level differences)
- `SpatialisationMode::quality`: quality mode (HRTF-based)

---

### `#!cpp enum class SourceDirectivity`
Selects the directivity pattern used by a source.

- `SourceDirectivity::omni`
- `SourceDirectivity::subcardioid`
- `SourceDirectivity::cardioid`
- `SourceDirectivity::supercardioid`
- `SourceDirectivity::hypercardioid`
- `SourceDirectivity::bidirectional`
- `SourceDirectivity::genelec8020c`
- `SourceDirectivity::genelec8020cDTF`
- `SourceDirectivity::qscK8`

---

### `#!cpp enum class DiffractionModel`
Selects the diffraction model.

- `DiffractionModel::attenuate`: distance attenuation only (shadow zone)
- `DiffractionModel::lowPass`: 1 kHz low-pass filter (shadow zone)
- `DiffractionModel::udfa`: UDFA model
- `DiffractionModel::udfai`: UDFA-I model (shadow zone)
- `DiffractionModel::nnBest`: neural network model (shadow zone)
- `DiffractionModel::nnSmall`: smaller neural network model (shadow zone)
- `DiffractionModel::utd`: UTD-based model (shadow zone)
- `DiffractionModel::btm`: BTM model

---

### `#!cpp enum class ReverbFormula`
Selects the formula used to estimate late reverberation time.

- `ReverbFormula::Sabine`
- `ReverbFormula::Eyring`
- `ReverbFormula::Custom`: use user-provided T60 values

---

### `#!cpp enum class FDNMatrix`
Selects the feedback matrix used by the feedback delay network.

- `FDNMatrix::householder`
- `FDNMatrix::randomOrthogonal`

---

### `#!cpp enum class DirectSound`
Selects how the direct sound path is handled.

- `DirectSound::none`: no direct sound
- `DirectSound::doCheck`: perform visibility check
- `DirectSound::alwaysTrue`: always consider direct sound present

---

### `#!cpp enum class LateReverbModel`
Late reverberation model selection.

In typical usage this is selected automatically based on which late reverberation initialisation function you call.

- `LateReverbModel::none`: no late reverberation
- `LateReverbModel::fdn`: feedback delay network (SingleFDN)
- `LateReverbModel::raves`: MoD-ART late reverberation model

---

### `#!cpp enum class DiffractionSound`
Controls whether diffraction sound is rendered.

- `DiffractionSound::none`: no diffraction sound
- `DiffractionSound::shadowZone`: render diffraction only in the shadow zone (recommended)
- `DiffractionSound::allZones`: render diffraction in all zones
