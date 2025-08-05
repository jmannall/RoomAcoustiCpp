# PeakLowShelf

The `PeakLowShelf` class implements a second-order peak low-shelf IIR filter.
It derives from `IIRFilter2Param1`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `DSP/Interpolate.h`

---
## Class Definition

```cpp
class PeakLowShelf : public IIRFilter2Param1
{
public:
    PeakLowShelf(const Real fc, const Real Q, const int sampleRate);
    PeakLowShelf(const Real fc, const Real gain, const Real Q, const int sampleRate);
    ~PeakLowShelf();

    inline void SetTargetGain(const Real gain);
    // ...inherited methods from IIRFilter2Param1...

private:
    void UpdateCoefficients(const Real gain) override;

    const Real cosOmega;
    const Real alpha;
};
```

---
## Public Methods

### `#!cpp PeakLowShelf(const Real fc, const Real Q, const int sampleRate)`
**Constructor.**  
Initializes the filter with the given cutoff frequency, Q, and sample rate. Gain is set to 1.0.
- `fc`: The cutoff frequency (Hz).
- `Q`: The quality factor.
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp PeakLowShelf(const Real fc, const Real gain, const Real Q, const int sampleRate)`
**Constructor.**  
Initializes the filter with the given cutoff frequency, gain, Q, and sample rate.
- `fc`: The cutoff frequency (Hz).
- `gain`: The shelf gain (linear).
- `Q`: The quality factor.
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ~PeakLowShelf()`
**Destructor.**  
Cleans up the filter.

---

### `#!cpp inline void SetTargetGain(const Real gain)`
Sets the target gain for the filter.
- `gain`: The shelf gain (linear).

---
## Private Methods

### `#!cpp void UpdateCoefficients(const Real gain) override`
Updates the filter coefficients based on the current gain.
- `gain`: The shelf gain (linear).

---
## Internal Data Members

- `#!cpp const Real cosOmega`: Cosine of the cutoff frequency (precomputed).
- `#!cpp const Real alpha`: Alpha value for the filter (precomputed).

---
## Implementation Notes

- TO DO: Add filter transfer function

## Example Usage

```cpp
#include "DSP/IIRFilter.h"
using namespace RAC::DSP;

const int sampleRate = 48e3;
const Real fc = 200.0;
const Real Q = 0.707;
const Real gain = 2.0;

// Create PeakLowShelf filter
PeakLowShelf filter(fc, gain, Q, sampleRate);

// Update target gain
filter.SetTargetGain(3.0);

// Process audio sample
Real output = filter.GetOutput(1.0, 0.01); // Example input signal
```
