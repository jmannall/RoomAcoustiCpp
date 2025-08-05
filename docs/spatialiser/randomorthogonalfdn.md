Implements a Feedback Delay Network (FDN) using a random orthogonal feedback matrix for decorrelation and natural-sounding reverberation.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/FDN.h`
- **Source:** `Spatialiser/FDN.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Matrix.h`, `Common/Vec.h`, `Common/Coefficients.h`, `DSP/Buffer.h`, `DSP/GraphicEQ.h`, `Spatialiser/Types.h`

---

## Class Definition

```cpp
class RandomOrthogonalFDN : public FDN
{
public:
    RandomOrthogonalFDN(const Coefficients& T60, const Vec& dimensions, const Config& config);
    ~RandomOrthogonalFDN();

    static Matrix InitMatrix(const size_t numChannels);
};
```

---

## Public Methods

### `#!cpp RandomOrthogonalFDN(const Coefficients& T60, const Vec& dimensions, const Config& config)`
**Constructor.**  
Initializes a RandomOrthogonalFDN with the given decay time, room dimensions, and configuration.
- `T60`: Target decay time.
- `dimensions`: Room dimensions.
- `config`: Spatialiser configuration.

---

### `#!cpp ~RandomOrthogonalFDN()`
**Destructor.**  
Cleans up the RandomOrthogonalFDN.

---

### `#!cpp static Matrix InitMatrix(const size_t numChannels)`
Initializes a random orthogonal matrix for use as the FDN feedback matrix.
- `numChannels`: Number of FDN channels.
- **Returns:** Random orthogonal matrix.

---

## Implementation Notes

- The random orthogonal matrix is generated using Gram-Schmidt orthogonalization.
- Used to provide decorrelation and natural-sounding late reverberation in FDNs.

## Example Usage

```cpp
#include "Spatialiser/FDN.h"
using namespace RAC::Spatialiser;

Coefficients T60 = { 1.2, 1.0, 0.8 };
Vec dimensions = { 5.0, 4.0, 3.0 };
Config config;
config.fs = 48000;
config.numLateReverbChannels = 3;
config.numFrames = 512;

// Create RandomOrthogonalFDN
RandomOrthogonalFDN fdn(T60, dimensions, config);

// Process audio
Matrix input(config.numLateReverbChannels, config.numFrames);
std::vector<Buffer> output(config.numLateReverbChannels, Buffer(config.numFrames));
fdn.ProcessAudio(input, output);
```
