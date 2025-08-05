# HighPass

The `HighPass` class implements a second-order high-pass IIR filter.
It derives from `IIRFilter2Param1`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `DSP/Interpolate.h`

---
## Class Definition

```cpp
class HighPass : public IIRFilter2Param1
{
public:
    HighPass(const int sampleRate);
    HighPass(const Real fc, const int sampleRate);
    ~HighPass();

    inline void SetTargetFc(const Real fc);
    // ...inherited methods from IIRFilter2Param1...

private:
    void UpdateCoefficients(const Real fc) override;
};
```

---
## Public Methods

### `#!cpp HighPass(const int sampleRate)`
**Constructor.**  
Initializes the filter with the given sample rate and a default cutoff frequency of 1000Hz.
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp HighPass(const Real fc, const int sampleRate)`
**Constructor.**  
Initializes the filter with the given sample rate and cutoff frequency.
- `fc`: The cutoff frequency (Hz).
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ~HighPass()`
**Destructor.**  
Cleans up the filter.

---

### `#!cpp inline void SetTargetFc(const Real fc)`
Sets the target cutoff frequency for the filter.
- `fc`: The cutoff frequency (Hz).

---
## Private Methods

### `#!cpp void UpdateCoefficients(const Real fc) override`
Updates the filter coefficients based on the current frequency.
- `fc`: The cutoff frequency (Hz).

---

## Implementation Notes

- TO DO: Add filter transfer function

## Example Usage

```cpp
#include "DSP/IIRFilter.h"
using namespace RAC::DSP;

const int sampleRate = 48e3;
const Real lerpFactor = 2.0 / static_cast<Real>(sampleRate); // Example interpolation factor

const Real fc = 2000.0;

// Create HighPass filter
HighPass filter(fc, sampleRate);

// Update target cutoff frequency
filter.SetTargetFc(4000.0);

// Process audio sample
Real output = filter.GetOutput(1.0, lerpFactor); // Example input signal
```
