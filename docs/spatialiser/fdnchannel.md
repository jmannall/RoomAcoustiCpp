Implements a single channel of a Feedback Delay Network (FDN), including a delay line, absorption, and reflection filters.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/FDN.h`
- **Source:** `Spatialiser/FDN.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Coefficients.h`, `DSP/Buffer.h`, `DSP/GraphicEQ.h`, `Spatialiser/Types.h`

---

## Class Definition

```cpp
class FDNChannel
{
public:
    FDNChannel(const int delayLength, const Coefficients& T60, const Config& config);
    ~FDNChannel();

    inline void SetTargetT60(const Coefficients& T60);
    inline bool SetTargetReflectionFilter(const Coefficients& gains);
    inline void Reset();
    inline void ProcessOutput(const Buffer& data, Buffer& outputBuffer, const int numFrames, const Real lerpFactor);
    Real GetOutput(const Real input, const Real lerpFactor);

private:
    inline Coefficients CalculateFilterGains(const Coefficients& T60) const;

    const Real mT;
    Buffer mBuffer;
    int idx;
    GraphicEQ mAbsorptionFilter;
    GraphicEQ mReflectionFilter;
    std::atomic<bool> clearBuffers;
};
```

---

## Public Methods

### `#!cpp FDNChannel(const int delayLength, const Coefficients& T60, const Config& config)`
**Constructor.**  
Initializes the channel with a delay line, absorption, and reflection filters.
- `delayLength`: Delay in samples.
- `T60`: Target decay time coefficients.
- `config`: Spatialiser configuration.

---

### `#!cpp ~FDNChannel()`
**Destructor.**  
Cleans up the channel.

---

### `#!cpp inline void SetTargetT60(const Coefficients& T60)`
Sets the target T60 and updates the absorption filter gains.
- `T60`: New target decay time.

---

### `#!cpp inline bool SetTargetReflectionFilter(const Coefficients& gains)`
Sets the target reflection filter gains.
- `gains`: Target reflection filter gains.
- **Returns:** True if all current and target reflection gains are zero, false otherwise.

---

### `#!cpp inline void Reset()`
Resets the internal buffers to zero.

---

### `#!cpp inline void ProcessOutput(const Buffer& data, Buffer& outputBuffer, const int numFrames, const Real lerpFactor)`
Processes the output reflection filter.
- `data`: Input audio data.
- `outputBuffer`: Output buffer.
- `numFrames`: Number of frames.
- `lerpFactor`: Linear interpolation factor.

---

### `#!cpp Real GetOutput(const Real input, const Real lerpFactor)`
Processes a single sample through the delay and absorption filter.
- `input`: Input sample.
- `lerpFactor`: Linear interpolation factor.
- **Returns:** Output sample.

---

## Private Methods

### `#!cpp inline Coefficients CalculateFilterGains(const Coefficients& T60) const`
Calculates the filter gain coefficients required for a given T60.
- `T60`: Target decay time.
- **Returns:** Filter gain coefficients.

---

## Internal Data Members

- `#!cpp const Real mT`: Delay in seconds.
- `#!cpp Buffer mBuffer`: Internal delay line.
- `#!cpp int idx`: Current delay line read index.
- `#!cpp GraphicEQ mAbsorptionFilter`: Absorption filter.
- `#!cpp GraphicEQ mReflectionFilter`: Reflection filter.
- `#!cpp std::atomic<bool> clearBuffers`: Flag to clear buffers.

---

## Implementation Notes

- Used as a building block for FDN reverberators.
- Handles buffer clearing and parameter updates atomically.

## Example Usage

```cpp
#include "Spatialiser/FDN.h"
using namespace RAC::Spatialiser;

Config config;
config.fs = 48000;
Coefficients T60 = { 1.2, 1.0, 0.8 };
FDNChannel channel(2400, T60, config);

channel.SetTargetT60({ 1.0, 0.9, 0.7 });
Real output = channel.GetOutput(1.0, 0.01);
```
