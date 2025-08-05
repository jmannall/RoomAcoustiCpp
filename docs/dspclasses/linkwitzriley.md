Implements a multi-band Linkwitz-Riley crossover filterbank with configurable band gains and cutoff frequencies.  
Uses cascaded IIR filters for band splitting and gain control.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/LinkwitzRileyFilter.h`
- **Source:** `DSP/LinkwitzRileyFilter.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Coefficients.h`, `DSP/IIRFilter.h`, `DSP/Interpolate.h`, `Common/ReleasePool.h`

---

## Class Definition

```cpp
class LinkwitzRiley
{
public:
    LinkwitzRiley(const int sampleRate);
    LinkwitzRiley(const std::array<Real, 4> gains, const int sampleRate);
    LinkwitzRiley(const std::array<Real, 4> gains, const std::array<Real, 3> fc, const int sampleRate);
    ~LinkwitzRiley();

    Real GetOutput(const Real input, const Real lerpFactor);
    inline void SetTargetGains(const std::array<Real, 4>& gains);
    inline void ClearBuffers();

    const Coefficients fm;

private:
    void InitFilters(const int sampleRate, const std::array<Real, 3>& fc);
    inline Coefficients CalculateMidFrequencies(const std::array<Real, 3>& fc);
    void InterpolateGains(const Real lerpFactor);

    std::atomic<std::shared_ptr<Coefficients>> targetGains;
    Coefficients currentGains;
    std::vector<std::unique_ptr<IIRFilter2Param1>> filters;
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
- `sampleRate`: Sample rate for filter coefficients.

---

### `#!cpp LinkwitzRiley(const std::array<Real, 4> gains, const int sampleRate)`
**Constructor.**  
Initializes a Linkwitz-Riley filterbank with specified band gains and default cutoff frequencies.
- `gains`: Filter band gains.
- `sampleRate`: Sample rate for filter coefficients.

---

### `#!cpp LinkwitzRiley(const std::array<Real, 4> gains, const std::array<Real, 3> fc, const int sampleRate)`
**Constructor.**  
Initializes a Linkwitz-Riley filterbank with specified band gains and cutoff frequencies.
- `gains`: Filter band gains.
- `fc`: Cutoff frequencies.
- `sampleRate`: Sample rate for filter coefficients.

---

### `#!cpp ~LinkwitzRiley()`
**Destructor.**  
Cleans up the filterbank.

---

### `#!cpp Real GetOutput(const Real input, const Real lerpFactor)`
Returns the output of the Linkwitz-Riley filterbank for a given input.
- `input`: Input sample.
- `lerpFactor`: Interpolation factor.
- **Returns:** Output sample.

---

### `#!cpp inline void SetTargetGains(const std::array<Real, 4>& gains)`
Sets the target gains for each band.
- `gains`: New target gains.

---

### `#!cpp inline void ClearBuffers()`
Resets the filter buffers.

---

## Private Methods

### `#!cpp void InitFilters(const int sampleRate, const std::array<Real, 3>& fc)`
Initializes the filter sections.
- `sampleRate`: Sample rate.
- `fc`: Cutoff frequencies.

---

### `#!cpp inline Coefficients CalculateMidFrequencies(const std::array<Real, 3>& fc)`
Calculates the pass band center frequencies.
- `fc`: Cutoff frequencies.
- **Returns:** Center frequencies.

---

### `#!cpp void InterpolateGains(const Real lerpFactor)`
Linearly interpolates the current gains with the target gains.
- `lerpFactor`: Interpolation factor.

---

## Internal Data Members

- `#!cpp std::atomic<std::shared_ptr<Coefficients>> targetGains`: Target filter band gains.
- `#!cpp Coefficients currentGains`: Current filter band gains.
- `#!cpp std::vector<std::unique_ptr<IIRFilter2Param1>> filters`: Filter sections.
- `#!cpp std::atomic<bool> initialised`: True if initialized.
- `#!cpp std::atomic<bool> gainsEqual`: True if current and target gains are equal.
- `#!cpp static ReleasePool releasePool`: Memory management for shared pointers.
- `#!cpp const Coefficients fm`: Filter band mid frequencies.

---

## Implementation Notes

- Default cutoff frequencies: 176.0, 775.0, 3408.0 Hz.
- Band mid frequencies are calculated geometrically.
- Uses atomic operations for thread safety.

## Example Usage

```cpp
#include "DSP/LinkwitzRileyFilter.h"
using namespace RAC::DSP;

std::array<Real, 4> gains = { 1.0, 1.0, 1.0, 1.0 };
std::array<Real, 3> fc = { 200.0, 1000.0, 5000.0 };
int sampleRate = 48000;

// Create Linkwitz-Riley filterbank
LinkwitzRiley lr(gains, fc, sampleRate);

// Set new target gains
lr.SetTargetGains({ 1.0, 0.8, 1.2, 1.0 });

// Process audio sample
Real output = lr.GetOutput(1.0, 0.01);
```
