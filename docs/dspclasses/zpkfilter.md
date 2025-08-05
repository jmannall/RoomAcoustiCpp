# ZPKFilter

The `ZPKFilter` class implements a second-order IIR filter using zero-pole-gain (ZPK) representation.
It derives from `IIRFilter2`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `DSP/Interpolate.h`, `Common/ReleasePool.h`

---
## Class Definition

```cpp
class ZPKFilter : public IIRFilter2
{
public:
    ZPKFilter(const int sampleRate);
    ZPKFilter(const Coefficients& zpk, const int sampleRate);
    ~ZPKFilter();

    void SetTargetParameters(const Coefficients& zpk);
    // ...inherited methods from IIRFilter2...

private:
    void InterpolateParameters(const Real lerpFactor) override;
    void UpdateCoefficients(const Coefficients& zpk);

    std::atomic<std::shared_ptr<Coefficients>> targetZPK;
    Coefficients currentZPK;
    static ReleasePool releasePool;
};
```

---
## Public Methods

### `#!cpp ZPKFilter(const int sampleRate)`
**Constructor.**  
Initializes the filter with the given sample rate and default ZPK coefficients.  
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ZPKFilter(const Coefficients& zpk, const int sampleRate)`
**Constructor.**  
Initializes the filter with the given sample rate and ZPK coefficients.  
- `zpk`: The zero-pole-gain coefficients.  
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ~ZPKFilter()`
**Destructor.**  
Cleans up the filter.

---

### `#!cpp void SetTargetParameters(const Coefficients& zpk)`
Sets the target ZPK coefficients for the filter.  
- `zpk`: The zero-pole-gain coefficients.

---
## Private Methods

### `#!cpp void InterpolateParameters(const Real lerpFactor) override`
Interpolates between the current ZPK parameters and target ZPK parameters using linear interpolation.  
- `lerpFactor`: Interpolation factor (0.0 to 1.0)

---

### `#!cpp void UpdateCoefficients(const Coefficients& zpk)`
Updates the filter coefficients based on the current ZPK values.  
- `zpk`: The zero-pole-gain coefficients.

---
## Internal Data Members

- `#!cpp std::atomic<std::shared_ptr<Coefficients>> targetZPK`: The target ZPK coefficients for the filter.
- `#!cpp Coefficients currentZPK`: The current ZPK coefficients for the filter.
- `#!cpp static ReleasePool releasePool`: Garbage collector for shared pointers after atomic replacement.

---
## Implementation Notes

- TO DO: Add filter transfer function

## Example Usage

```cpp
#include "DSP/IIRFilter.h"
using namespace RAC::DSP;

const int sampleRate = 48e3;
const Real lerpFactor = 2.0 / static_cast<Real>(sampleRate); // Example interpolation factor

// Zeros, poles and gain
Coefficients zpk = { -0.9, -0.5, -0.6, 0.3, 1.0 };  // { z1, z2, p1, p2, k }
Coefficients newZPK = { -0.8, -0.2, -0.7, 0.2, 0.9 };

// Create ZPKFilter filter
ZPKFilter filter(zpk, sampleRate);

// Update target ZPK coefficients
filter.SetTargetParameters(newZPK);

// Process audio sample
Real output = filter.GetOutput(1.0, lerpFactor);
```
