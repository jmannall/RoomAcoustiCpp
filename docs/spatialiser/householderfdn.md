Implements a Feedback Delay Network (FDN) using a Householder feedback matrix for good decorrelation.

Most users will configure late reverberation through the main API in `Spatialiser/Interface.h` (e.g., `InitSingleFDN`). `HouseHolderFDN` is exposed for advanced usage.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/FDN.h`
- **Source:** `Spatialiser/FDN.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Matrix.h`, `Common/Vec.h`, `Common/Coefficients.h`, `DSP/Buffer.h`, `Spatialiser/Types.h`, `Spatialiser/Configs.h`

---

## Class Definition

```cpp
template <typename T = Real>
class HouseHolderFDN : public FDN<T>
{
public:
    HouseHolderFDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig>& dspConfig);
    HouseHolderFDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig>& dspConfig);
    ~HouseHolderFDN();


    inline void ProcessMatrix() override;

private:
    Real houseHolderFactor;
};
```

---

## Public Methods

### `#!cpp HouseHolderFDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig>& dspConfig)`
**Constructor (Real).**  
Initialises a Householder FDN with the given target decay time, room dimensions, and DSP configuration.

`T60`: Target decay time per frequency band.  
`dimensions`: Room dimensions.  
`dspConfig`: DSP configuration.

---

### `#!cpp HouseHolderFDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig>& dspConfig)`
**Constructor (Complex).**  
Initialises a complex-valued Householder FDN using explicit delay lengths.

`T60`: Target decay time in seconds.  
`delayLengths`: Delay line lengths (in samples).  
`dspConfig`: DSP configuration.

---

### `#!cpp ~HouseHolderFDN()`
**Destructor.**  
Cleans up the Householder FDN.

---

### `#!cpp inline void ProcessMatrix() override`
Processes the Householder feedback matrix.

---

## Internal Data Members

- `#!cpp Real houseHolderFactor`: Precomputed factor for the Householder matrix.

---

## Implementation Notes

- The Householder matrix is a special orthogonal matrix, which is particularly efficient to apply.

## Example Usage

```cpp
#include "Spatialiser/Configs.h"
#include "Spatialiser/FDN.h"

using namespace RAC::Spatialiser;

DSPData dsp;
std::shared_ptr<DSPConfig> dspConfig = std::make_shared<DSPConfig>(dsp);

Coefficients<> T60 = Coefficients<>::Constant(dsp.frequencyBands.Length(), 1.0);

Vec<> dims(std::vector<Real>({ 5.0, 4.0, 3.0 }));

HouseHolderFDN<Real> fdn(T60, dims, dspConfig);
AudioData audioData(dspConfig);

Matrix<> input(dspConfig->GetData().fdnSize, dspConfig->GetData().numFrames);
std::vector<Buffer<>> output(dspConfig->GetData().numReverbSources, Buffer<>(dspConfig->GetData().numFrames));

fdn.ProcessAudio(input, output, audioData);
```
