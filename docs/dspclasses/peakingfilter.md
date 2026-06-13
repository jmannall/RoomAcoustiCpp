# PeakingFilter

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

The `PeakingFilter` class implements a second-order peaking IIR filter.
It derives from `IIRFilter2Param1<In>`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `DSP/Interpolate.h`

---
## Class Definition

```cpp
template<typename In = Real>
class PeakingFilter : public IIRFilter2Param1<In>
{
public:
    PeakingFilter(const Real fc, const Real Q, const int sampleRate);
    PeakingFilter(const Real fc, const Real gain, const Real Q, const int sampleRate);
    ~PeakingFilter() {};

    inline void SetTargetGain(const Real gain);

private:
    void UpdateCoefficients(const Real gain) override;

    const Real cosOmega;
    const Real alpha;
};
```

---
## Public Methods

### `#!cpp PeakingFilter(const Real fc, const Real Q, const int sampleRate)`
**Constructor.**  
Initializes the filter with the given center frequency, Q, and sample rate. Gain is set to 1.0.

`fc`: The center frequency (Hz).  
`Q`: The quality factor.  
`sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp PeakingFilter(const Real fc, const Real gain, const Real Q, const int sampleRate)`
**Constructor.**  
Initializes the filter with the given center frequency, gain, Q, and sample rate.

`fc`: The center frequency (Hz).  
`gain`: The gain (linear).  
`Q`: The quality factor.  
`sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ~PeakingFilter()`
**Destructor.**  
Default destructor.

---

### `#!cpp inline void SetTargetGain(const Real gain)`
Sets the target gain for the filter.

`gain`: The gain (linear).

---

## Private Methods

### `#!cpp void UpdateCoefficients(const Real gain) override`
Updates the filter coefficients based on the current gain.

`gain`: The gain (linear).

---

## Internal Data Members

- `#!cpp const Real cosOmega`: Cosine of the center frequency (precomputed).
- `#!cpp const Real alpha`: Alpha value for the filter (precomputed).

---

## Example Usage

```cpp
#include "DSP/IIRFilter.h"
using namespace RAC::DSP;

const int sampleRate = 48000;
const Real fc = 1000.0;
const Real Q = 0.707;
const Real gain = 1.5;

PeakingFilter<> filter(fc, gain, Q, sampleRate);
filter.SetTargetGain(0.9);

Real output = filter.GetOutput(1.0, 0.01);
```
