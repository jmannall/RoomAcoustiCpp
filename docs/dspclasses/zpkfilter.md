# ZPKFilter

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

The `ZPKFilter` class implements a second-order IIR filter using a zero-pole-gain (ZPK) representation.
It derives from `IIRFilter2<>`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Coefficients.h`, `DSP/Interpolate.h`, `Common/ReleasePool.h`

---
## Class Definition

```cpp
class ZPKFilter : public IIRFilter2<>
{
public:
    // ZPK parameters: { z1, z2, p1, p2, k }

    ZPKFilter(const int sampleRate);
    ZPKFilter(const Coefficients<Real, 5>& zpk, const int& sampleRate);

    void SetTargetParameters(const Coefficients<Real, 5>& zpk);
    void SetTargetGain(const Real k);

private:
    void InterpolateParameters(const Real lerpFactor) override;
    void UpdateCoefficients(const Coefficients<Real, 5>& zpk);

#ifdef __ANDROID__
    std::shared_ptr<Coefficients<Real, 5>> targetZPK;
#else
    std::atomic<std::shared_ptr<Coefficients<Real, 5>>> targetZPK;
#endif
    Coefficients<Real, 5> currentZPK;

    static ReleasePool releasePool;
};
```

---
## Public Methods

### `#!cpp ZPKFilter(const int sampleRate)`
**Constructor.**  
Initialises the filter with the given sample rate and default ZPK parameters.

`sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ZPKFilter(const Coefficients<Real, 5>& zpk, const int& sampleRate)`
**Constructor.**  
Initialises the filter with the given sample rate and ZPK parameters.

`zpk`: The zero-pole-gain parameters `{ z1, z2, p1, p2, k }`.  
`sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp void SetTargetParameters(const Coefficients<Real, 5>& zpk)`
Sets the target ZPK parameters for the filter.

`zpk`: The zero-pole-gain parameters `{ z1, z2, p1, p2, k }`.

---

### `#!cpp void SetTargetGain(const Real k)`
Sets the target gain parameter `k` (keeping the current zeros/poles).

`k`: The target gain.

---
## Private Methods

### `#!cpp void InterpolateParameters(const Real lerpFactor) override`
Interpolates between the current and target ZPK parameters.

`lerpFactor`: Interpolation factor (0.0 to 1.0).

---

### `#!cpp void UpdateCoefficients(const Coefficients<Real, 5>& zpk)`
Updates the filter coefficients based on the given ZPK parameters.

`zpk`: The zero-pole-gain parameters.

---
## Internal Data Members

- `#!cpp targetZPK`: Target ZPK parameters (stored atomically on most platforms).
- `#!cpp Coefficients<Real, 5> currentZPK`: Current ZPK parameters (audio thread).
- `#!cpp static ReleasePool releasePool`: Manages memory for shared pointers after atomic replacement.

---
## Implementation Notes

- Intended for use with NN models that output poles/zeros/gain in ZPK form.

## Example Usage

```cpp
#include "DSP/IIRFilter.h"
using namespace RAC::DSP;

const int sampleRate = 48000;
const Real lerpFactor = 2.0 / static_cast<Real>(sampleRate); // Example interpolation factor

Coefficients<Real, 5> zpk(std::array<Real, 5>{ -0.9, -0.5, -0.6, 0.3, 1.0 });
Coefficients<Real, 5> newZPK(std::array<Real, 5>{ -0.8, -0.2, -0.7, 0.2, 0.9 });

ZPKFilter filter(zpk, sampleRate);

filter.SetTargetParameters(newZPK);

Real output = filter.GetOutput(1.0, lerpFactor);
```
