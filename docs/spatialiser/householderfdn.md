Implements a Feedback Delay Network (FDN) using a Householder feedback matrix for maximum energy spreading and decorrelation.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/FDN.h`
- **Source:** `Spatialiser/FDN.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Matrix.h`, `Common/Vec.h`, `Common/Coefficients.h`, `DSP/Buffer.h`, `DSP/GraphicEQ.h`, `Spatialiser/Types.h`

---

## Class Definition

```cpp
class HouseHolderFDN : public FDN
{
public:
    HouseHolderFDN(const Coefficients& T60, const Vec& dimensions, const Config& config);
    ~HouseHolderFDN();

protected:
    inline void ProcessMatrix() override;

private:
    Real houseHolderFactor;
};
```

---

## Public Methods

### `#!cpp HouseHolderFDN(const Coefficients& T60, const Vec& dimensions, const Config& config)`
**Constructor.**  
Initializes a Householder FDN with the given decay time, room dimensions, and configuration.
- `T60`: Target decay time.
- `dimensions`: Room dimensions.
- `config`: Spatialiser configuration.

---

### `#!cpp ~HouseHolderFDN()`
**Destructor.**  
Cleans up the Householder FDN.

---

## Protected Methods

### `#!cpp inline void ProcessMatrix() override`
Processes the Householder feedback matrix for the FDN.
- Updates the input vector `x` using the Householder transformation and previous output `y`.

---

## Internal Data Members

- `#!cpp Real houseHolderFactor`: Precomputed factor for the Householder matrix.

---

## Implementation Notes

- The Householder matrix is a special orthogonal matrix that maximizes energy mixing between channels.
- Used for decorrelation and uniform energy distribution in FDN reverberators.

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

// Create HouseHolderFDN
HouseHolderFDN fdn(T60, dimensions, config);

// Process audio
Matrix input(config.numLateReverbChannels, config.numFrames);
std::vector<Buffer> output(config.numLateReverbChannels, Buffer(config.numFrames));
fdn.ProcessAudio(input, output);
```
