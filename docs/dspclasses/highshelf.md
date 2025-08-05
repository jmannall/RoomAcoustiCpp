The `HighShelf` class implements a first-order high-shelf IIR filter with real-time parameter interpolation.
It derives from `IIRFilter1`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `DSP/Interpolate.h`

---

## Class Definition

```cpp
class HighShelf : public IIRFilter1
{
public:
    HighShelf(const int sampleRate);
    HighShelf(const Real fc, const Real gain, const int sampleRate);
    ~HighShelf();

    inline void SetTargetParameters(const Real fc, const Real gain);
    // ...inherited methods from IIRFilter1...
    
private:
    void InterpolateParameters(const Real lerpFactor) override;
    void UpdateCoefficients(const Real fc, const Real gain);

    std::atomic<Real> targetFc;
    std::atomic<Real> targetGain;
    Real currentFc;
    Real currentGain;
};
```

---

## Public Methods

### `#!cpp HighShelf(const int sampleRate)`
**Constructor.**  
Initializes the high-shelf filter with the given sample rate and a default cutoff frequency of 1kHz and gain of 1.0.  
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp HighShelf(const Real fc, const Real gain, const int sampleRate)`
**Constructor.**  
Initializes the high-shelf filter with the given sample rate, cutoff frequency and gain.  
- `fc`: The cutoff frequency (Hz).  
- `gain`: The gain (linear).  
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ~HighShelf()`
**Destructor.**  
Cleans up the filter.

---

### `#!cpp inline void SetTargetParameters(const Real fc, const Real gain)`
Sets the target cutoff frequency and gain for the filter.  
- `fc`: Target cutoff frequency (Hz).  
- `gain`: Target gain (linear).

---

## Private Methods

### `#!cpp void InterpolateParameters(const Real lerpFactor) override`
Interpolates between the current cutoff frequency and gain and target cutoff frequency and gain using linear interpolation.  
- `lerpFactor`: Interpolation factor (0.0 to 1.0)

---

### `#!cpp void UpdateCoefficients(const Real fc, const Real gain)`
Updates the filter coefficients based on the current frequency and gain.  
- `fc`: The cutoff frequency (Hz).  
- `gain`: The gain (linear).

---

## Internal Data Members

- `#!cpp std::atomic<Real> targetFc`: Target cutoff frequency (Hz)
- `#!cpp std::atomic<Real> targetGain`: Target shelf gain (linear)
- `#!cpp Real currentFc`: Current cutoff frequency (Hz)
- `#!cpp Real currentGain`: Current shelf gain (linear)

---

## Implementation Notes

- TO DO: Filter transfer function.

## Example Usage

```cpp
#include "DSP/IIRFilter.h"
using namespace RAC::DSP;

const int sampleRate = 48e3;
const Real lerpFactor = 2.0 / static_cast<Real>(sampleRate); // Example interpolation factor

const Real fc = 500.0;
const Real gain = 2.0;

// Create HighShelf filter
HighShelf filter(fc, gain, sampleRate);

// Update target parameters
filter.SetTargetParameters(1000.0, 0.5);

// Process audio
Real output = filter.GetOutput(1.0, lerpFactor);
```
