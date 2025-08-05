# LowPass1

The `LowPass1` class implements a first-order low-pass IIR filter with real-time parameter interpolation.
It derives from `IIRFilter1`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `DSP/Interpolate.h`

---

## Class Definition

```cpp
class LowPass1 : public IIRFilter1
{
public:
    LowPass1(const int sampleRate);
    LowPass1(const Real fc, const int sampleRate);
    ~LowPass1();

    inline void SetTargetFc(const Real fc);
    // ...inherited methods from IIRFilter1...

private:
    void InterpolateParameters(const Real lerpFactor) override;
    void UpdateCoefficients(const Real fc);

    std::atomic<Real> targetFc;
    Real currentFc;
};
```

---

## Public Methods

### `#!cpp LowPass1(const int sampleRate)`
**Constructor.**  
Initializes the low-pass filter with the given sample rate and a default cutoff frequency of 1kHz.
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp LowPass1(const Real fc, const int sampleRate)`
**Constructor.**  
Initializes the low-pass filter with the given sample rate and cutoff frequency.
- `fc`: The cutoff frequency (Hz).
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ~LowPass1()`
**Destructor.**  
Cleans up the filter.

---

### `#!cpp inline void SetTargetFc(const Real fc)`
Sets the target cutoff frequency for the filter.
- `fc`: The cutoff frequency (Hz).

---

## Private Methods

### `#!cpp void InterpolateParameters(const Real lerpFactor) override`
Interpolates between the current cutoff frequency and target cutoff frequency using linear interpolation.
- `lerpFactor`: Interpolation factor (0.0 to 1.0)

---

### `#!cpp void UpdateCoefficients(const Real fc)`
Updates the filter coefficients based on the current frequency.
- `fc`: The cutoff frequency (Hz).

---

## Internal Data Members

- `#!cpp std::atomic<Real> targetFc`: Target cutoff frequency (Hz)
- `#!cpp Real currentFc`: Current cutoff frequency (Hz)

---

## Implementation Notes

- TO DO: Add filter transfer function

## Example Usage

```cpp
#include "DSP/IIRFilter.h"
using namespace RAC::DSP;

const int sampleRate = 48e3;
const Real lerpFactor = 2.0 / static_cast<Real>(sampleRate); // Example interpolation factor

const Real fc = 1000.0;

// Create LowPass1 filter
LowPass1 filter(fc, sampleRate);

// Update target cutoff frequency
filter.SetTargetFc(2000.0);

// Process audio sample
Real output = filter.GetOutput(1.0, lerpFactor);
```
