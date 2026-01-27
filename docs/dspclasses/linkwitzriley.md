Implements a multi-band Linkwitz-Riley crossover filterbank with configurable band gains and cutoff frequencies.  
Uses cascaded IIR filters for band splitting and gain control.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/LinkwitzRileyFilter.h`
- **Source:** `DSP/LinkwitzRileyFilter.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Coefficients.h`, `DSP/IIRFilter.h`, `DSP/Interpolate.h`, `Common/ReleasePool.h`

---

## Class Definition

```cpp
class LinkwitzRiley
{
    using Parameters = Coefficients<Real, 4>;
public:
    LinkwitzRiley(const int sampleRate);
    LinkwitzRiley(const Parameters& gains, const int sampleRate);
    LinkwitzRiley(const Parameters& gains, const std::array<Real, 3> fc, const int sampleRate);
    ~LinkwitzRiley() {};

    Real GetOutput(const Real input, const Real lerpFactor);
    inline void SetTargetGains(const Parameters& gains);
    inline void ClearBuffers();

    static inline Parameters DefaultFM();
    const Parameters fm;

private:
    void InitFilters(const int sampleRate, const std::array<Real, 3>& fc);
    static inline Parameters CalculateMidFrequencies(const std::array<Real, 3>& fc);
    void InterpolateGains(const Real lerpFactor);

#ifdef __ANDROID__
    std::shared_ptr<Parameters> targetGains;
#else
    std::atomic<std::shared_ptr<Parameters>> targetGains;
#endif
    Parameters currentGains;

    std::array<std::optional<LowPass>, 10> lowPassFilters;
    std::array<std::optional<HighPass>, 10> highPassFilters;

    std::atomic<bool> initialised;
    std::atomic<bool> gainsEqual;

    static ReleasePool releasePool;
};
```

---

## Public Methods

### `#!cpp LinkwitzRiley(const int sampleRate)`
**Constructor.**  
Initializes a default Linkwitz-Riley filterbank with default gains and cutoff frequencies.

`sampleRate`: Sample rate for filter coefficients.

---

### `#!cpp LinkwitzRiley(const Parameters& gains, const int sampleRate)`
**Constructor.**  
Initializes a Linkwitz-Riley filterbank with specified band gains and default cutoff frequencies.

`gains`: Filter band gains.  
`sampleRate`: Sample rate for filter coefficients.

---

### `#!cpp LinkwitzRiley(const Parameters& gains, const std::array<Real, 3> fc, const int sampleRate)`
**Constructor.**  
Initializes a Linkwitz-Riley filterbank with specified band gains and cutoff frequencies.

`gains`: Filter band gains.  
`fc`: Cutoff frequencies.  
`sampleRate`: Sample rate for filter coefficients.

---

### `#!cpp ~LinkwitzRiley()`
**Destructor.**  
Default destructor.

---

### `#!cpp Real GetOutput(const Real input, const Real lerpFactor)`
Returns the output of the Linkwitz-Riley filterbank for a given input.

`input`: Input sample.  
`lerpFactor`: Interpolation factor.  
**Returns:** Output sample.

---

### `#!cpp inline void SetTargetGains(const Parameters& gains)`
Sets the target gains for each band.

`gains`: New target gains.

---

### `#!cpp inline void ClearBuffers()`
Resets the internal filter buffers.

---

### `#!cpp static inline Parameters DefaultFM()`
Returns the default mid-band frequencies (`fm`) for the default cutoff frequencies.

---

## Internal Data Members

- `#!cpp targetGains`: Target filter band gains (stored atomically on most platforms).
- `#!cpp Parameters currentGains`: Current filter band gains (audio thread).
- `#!cpp lowPassFilters`: Low-pass filter sections.
- `#!cpp highPassFilters`: High-pass filter sections.
- `#!cpp std::atomic<bool> initialised`: True if initialized.
- `#!cpp std::atomic<bool> gainsEqual`: True if current and target gains are equal.
- `#!cpp static ReleasePool releasePool`: Memory management for shared pointers.
- `#!cpp const Parameters fm`: Filter band mid frequencies.

---

## Implementation Notes

- Default cutoff frequencies: 176.0, 775.0, 3408.0 Hz.
- Band mid frequencies are calculated geometrically.

## Example Usage

```cpp
#include "DSP/LinkwitzRileyFilter.h"
using namespace RAC::DSP;

int sampleRate = 48000;

Coefficients<Real, 4> gains(std::array<Real, 4>{ 1.0, 1.0, 1.0, 1.0 });
std::array<Real, 3> fc = { 200.0, 1000.0, 5000.0 };

LinkwitzRiley lr(gains, fc, sampleRate);

lr.SetTargetGains(Coefficients<Real, 4>(std::array<Real, 4>{ 1.0, 0.8, 1.2, 1.0 }));

Real output = lr.GetOutput(1.0, 0.01);
```
