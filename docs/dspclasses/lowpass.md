<!-- filepath: c:\GitHub\jmannall\RoomAcoustiCpp\docs\dspclasses\lowpass.md -->
# LowPass

The `LowPass` class implements a second-order low-pass IIR filter.
It derives from `IIRFilter2Param1`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `DSP/Interpolate.h`

---
## Class Definition

```cpp
class LowPass : public IIRFilter2Param1
{
public:
    LowPass(const int sampleRate);
    LowPass(const Real fc, const int sampleRate);
    ~LowPass();

    inline void SetTargetFc(const Real fc);
    // ...inherited methods from IIRFilter2Param1...

private:
    void UpdateCoefficients(const Real fc) override;
};
```

---
## Public Methods

### `#!cpp LowPass(const int sampleRate)`
**Constructor.**  
Initializes the filter with the given sample rate and a default cutoff frequency of 1000Hz.  
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp LowPass(const Real fc, const int sampleRate)`
**Constructor.**  
Initializes the filter with the given sample rate and cutoff frequency.  
- `fc`: The cutoff frequency (Hz).  
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ~LowPass()`
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

const Real fc = 500.0;

// Create LowPass filter
LowPass filter(fc, sampleRate);

// Update target cutoff frequency
filter.SetTargetFc(2000.0);

// Process audio sample
Real output = filter.GetOutput(1.0, lerpFactor); // Example input signal
```
