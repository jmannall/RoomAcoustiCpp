Defines the configuration structs used to initialise and control RoomAcoustiC++.

These types are passed to the API in `Spatialiser/Interface.h` to configure DSP, spatialisation, early reverberation, and late reverberation.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/Configs.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Complex.h`, `Common/Coefficients.h`, `Common/Vec3.h`, `Common/Matrix.h`, `Common/Vec.h`, `Spatialiser/Types.h`

---

## DSP Configuration

### `#!cpp struct DSPData`
Stores DSP configuration data that is typically set once at initialisation.

**Fields:**

- `fs`: sample rate in Hz (default: 48000)
- `numFrames`: number of frames (audio samples) per audio callback (default: 512)
- `numReverbSources`: number of output channels (i.e. directions around listener) for late reverberation spatialization (default: 12)
- `fdnSize`: number of delay lines in each FDN (default: 12)
- `Q`: Q factor for the GraphicEQ (default: 0.98)
- `frequencyBands`: centre frequencies for the banded processing (default: {250, 500, 1000, 2000})
- `numFrequencyBands`: number of frequency bands (derived from `frequencyBands`)

**Methods:**

- `GetLerpFactor()`: interpolation factor used internally for parameter smoothing
- `UpdateLerpFactor(Real lerpFactor)`: updates the lerp factor used for parameter smoothing

---

## Early Reverberation (IEM)

### `#!cpp struct EarlyReverbData`
User configuration for the Image Edge Model (early reflections and low-order diffraction).

**Fields:**

- `direct`: direct sound visibility model (`DirectSound`)
- `reflOrder`: maximum number of reflections in reflection-only paths
- `shadowDiffOrder`: maximum order for shadowed diffraction paths
- `specularDiffOrder`: maximum order for specular diffraction paths
- `minEdgeLength`: minimum edge length for diffraction
- `maxPathLength`: maximum path length for image sources

---

## Late Reverberation

### `#!cpp struct LateReverbData`
User configuration for late reverberation ray tracing and FDN settings.

**Fields:**

- `enabled`: enable/disable late reverberation (default: false)
- `numRays`: number of rays used for ray tracing updates (default: 1000)
- `feedbackMatrix`: feedback matrix type for the FDN (`FDNMatrix`)

---

### `#!cpp struct RoomData`
Room configuration used to estimate and/or override late reverberation times.

**Fields:**

- `formula`: reverberation formula (`ReverbFormula`)
- `volume`: room volume in cubic metres
- `dimensions`: Room dimensions in metres
- `customT60`: per-band late reverberation time (used when `formula` is `ReverbFormula::Custom`)

---

### `#!cpp struct MoDARTData`
Configuration and precomputed data for the MoD-ART late reverberation model.

Other than `minimumT60`, the entries of `MoDARTData` are generated offline and then passed to RoomAcoustiC++ (`InitMoDART`) when the scene is loaded.
See the [notes about preprocessing](https://github.com/IoSR-Surrey/MoD-ART/) for more details about these parameters.

`MoDARTData` extends `LateReverbData`.

**Fields:**

- `delay`: delay before late reverberation starts, in seconds
- `minimumT60`: minimum T60 to process, in seconds
- `indexing`: propagation path indexing matrix
- `frequencyIndexing`: index of each FDN's frequency band
- `t60s`: late reverberation time of each FDN
- `leftEigenvectors`: left eigenvectors for each FDN
- `rightEigenvectors`: right eigenvectors for each FDN
