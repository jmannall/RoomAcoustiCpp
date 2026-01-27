Implements a Feedback Delay Network (FDN) using a random orthogonal feedback matrix for high decorrelation and natural-sounding reverberation.

Most users will configure late reverberation through the main API in `Spatialiser/Interface.h` (e.g., `InitSingleFDN`). `RandomOrthogonalFDN` is exposed for advanced usage.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/FDN.h`
- **Source:** `Spatialiser/FDN.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Matrix.h`, `Common/Vec.h`, `Common/Coefficients.h`, `DSP/Buffer.h`, `Spatialiser/Types.h`, `Spatialiser/Configs.h`

---

## Class Definition

```cpp
template <typename T = Real>
class RandomOrthogonalFDN : public FDN<T>
{
public:
    RandomOrthogonalFDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig>& dspConfig);
    RandomOrthogonalFDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig>& dspConfig);
    ~RandomOrthogonalFDN();

    static Matrix<> InitMatrix(const size_t numChannels);
};
```

---

## Public Methods

### `#!cpp RandomOrthogonalFDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig>& dspConfig)`
**Constructor (Real).**  
Initialises a random-orthogonal FDN with the given target decay time, room dimensions, and DSP configuration.

`T60`: Target decay time per frequency band.  
`dimensions`: Room dimensions.  
`dspConfig`: DSP configuration.

---

### `#!cpp RandomOrthogonalFDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig>& dspConfig)`
**Constructor (Complex).**  
Initialises a complex-valued random-orthogonal FDN using explicit delay lengths.

`T60`: Target decay time in seconds.  
`delayLengths`: Delay line lengths (in samples).  
`dspConfig`: DSP configuration.

---

### `#!cpp ~RandomOrthogonalFDN()`
**Destructor.**  
Cleans up the random-orthogonal FDN.

---

### `#!cpp static Matrix<> InitMatrix(const size_t numChannels)`
Initialises a random orthogonal feedback matrix.

`numChannels`: The number of FDN channels.  
**Returns:** A random orthogonal matrix.

---

## Implementation Notes

- Random orthogonal feedback matrices are typically less prone to coloration than simpler matrices such as Householder, at the cost of additional computation.

## Example Usage

```cpp
#include "Spatialiser/Configs.h"
#include "Spatialiser/FDN.h"

using namespace RAC::Spatialiser;

DSPData dsp;
std::shared_ptr<DSPConfig> dspConfig = std::make_shared<DSPConfig>(dsp);

Coefficients<> T60 = Coefficients<>::Constant(dsp.frequencyBands.Length(), 1.0);

Vec<> dims(std::vector<Real>({ 5.0, 4.0, 3.0 }));

RandomOrthogonalFDN<Real> fdn(T60, dims, dspConfig);
AudioData audioData(dspConfig);

Matrix<> input(dspConfig->GetData().fdnSize, dspConfig->GetData().numFrames);
std::vector<Buffer<>> output(dspConfig->GetData().numReverbSources, Buffer<>(dspConfig->GetData().numFrames));

fdn.ProcessAudio(input, output, audioData);
```
