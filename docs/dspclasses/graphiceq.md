A multi-band graphic equalizer using cascaded IIR filters.  
Implements accurate frequency response control using low shelf, peaking, and high shelf filters.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/GraphicEQ.h`
- **Source:** `DSP/GraphicEQ.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Complex.h`, `Common/Coefficients.h`, `Common/Matrix.h`, `Common/Vec.h`, `DSP/IIRFilter.h`, `DSP/Interpolate.h`

---

## Class Definition

```cpp
template<typename T = Real>
class GraphicEQ
{
public:
    GraphicEQ(const Coefficients<>& fc, const Real Q, const int sampleRate);
    GraphicEQ(const Coefficients<>& gain, const Coefficients<>& fc, const Real Q, const int sampleRate);

    bool SetTargetGains(const Coefficients<>& gains);

    T GetOutput(const T input, const Real lerpFactor);
    void GetOutputBatch(const Buffer<T>& inBuffer, Buffer<T>& outBuffer, const Real lerpFactor);
    void ProcessAudio(const Buffer<T>& inBuffer, Buffer<T>& outBuffer, const Real lerpFactor);

    bool IsValid() const;
    inline void ClearBuffers();

private:
    void InitMatrix(const Coefficients<>& fc, const Real Q, const int fs);
    std::pair<Rowvec<>, Real> CalculateGains(const Coefficients<>& gains) const;
    Coefficients<> CreateFrequencyVector(const Coefficients<>& fc) const;
    void InterpolateGain(const Real lerpFactor);
    void ScaleGain(Buffer<T>& buffer, const Real lerpFactor);

    const int numFilters;
    Coefficients<> previousInput;

    std::unique_ptr<PeakLowShelf<T>> lowShelf;
    std::vector<std::unique_ptr<PeakingFilter<T>>> peakingFilters;
    std::unique_ptr<PeakHighShelf<T>> highShelf;

    Matrix<> filterResponseMatrix;

    std::atomic<Real> targetGain;
    Real currentGain;

    std::atomic<bool> initialised{ false };
    std::atomic<bool> gainsEqual{ false };
};
```

---

## Public Methods

### `#!cpp GraphicEQ(const Coefficients<>& fc, const Real Q, const int sampleRate)`
**Constructor.**  
Initializes the `GraphicEQ` with zero gain and the given frequency bands, Q factor, and sample rate.

`fc`: Filter band center frequencies.  
`Q`: Q factor for the filters.  
`sampleRate`: Sample rate for filter coefficient calculation.

---

### `#!cpp GraphicEQ(const Coefficients<>& gain, const Coefficients<>& fc, const Real Q, const int sampleRate)`
**Constructor.**  
Initializes the `GraphicEQ` with given gains, frequency bands, Q factor, and sample rate.

`gain`: Target gain for each center frequency.  
`fc`: Filter band center frequencies.  
`Q`: Q factor for the filters.  
`sampleRate`: Sample rate for filter coefficient calculation.

---

### `#!cpp bool SetTargetGains(const Coefficients<>& gains)`
Sets new target gains for each center frequency.

`gains`: Target response for the `GraphicEQ`.  
**Returns:** True if the target and current gains are currently zero, false otherwise.

---

### `#!cpp T GetOutput(const T input, const Real lerpFactor)`
Returns the output of the `GraphicEQ` for a single input sample.

`input`: Input sample.  
`lerpFactor`: Linear interpolation factor.  
**Returns:** Output sample.

---

### `#!cpp void GetOutputBatch(const Buffer<T>& inBuffer, Buffer<T>& outBuffer, const Real lerpFactor)`
Returns the output of the `GraphicEQ` for a buffer of input samples.

`inBuffer`: Input buffer.  
`outBuffer`: Output buffer.  
`lerpFactor`: Linear interpolation factor.

---

### `#!cpp void ProcessAudio(const Buffer<T>& inBuffer, Buffer<T>& outBuffer, const Real lerpFactor)`
Processes an input buffer and updates the output buffer.

`inBuffer`: Input buffer.  
`outBuffer`: Output buffer.  
`lerpFactor`: Linear interpolation factor.

---

### `#!cpp bool IsValid() const`
Returns whether the filter was successfully initialised and can be used.

**Returns:** True if valid, false otherwise.

---

### `#!cpp inline void ClearBuffers()`
Resets the internal filter buffers.

---

## Private Methods

### `#!cpp void InitMatrix(const Coefficients<>& fc, const Real Q, const int fs)`
Initialises the filter response matrix for gain calculation.

`fc`: Filter band center frequencies.  
`Q`: Q factor.  
`fs`: Sample rate.

---

### `#!cpp std::pair<Rowvec<>, Real> CalculateGains(const Coefficients<>& gains) const`
Calculates the filter gains based on the target response.

`gains`: Target response.  
**Returns:** Pair of calculated gains and DC gain.

---

### `#!cpp Coefficients<> CreateFrequencyVector(const Coefficients<>& fc) const`
Creates a frequency vector for filter response calculation.

`fc`: Filter band center frequencies.  
**Returns:** Frequency vector.

---

### `#!cpp void InterpolateGain(const Real lerpFactor)`
Linearly interpolates the current gain towards the target gain.

`lerpFactor`: Interpolation factor.

---

### `#!cpp void ScaleGain(Buffer<T>& buffer, const Real lerpFactor)`
Scales a buffer by the current gain.

`buffer`: Buffer to scale.  
`lerpFactor`: Linear interpolation factor.

---

## Internal Data Members

- `#!cpp const int numFilters`: Number of filters.
- `#!cpp Coefficients<> previousInput`: Previous target response (used to detect changes).
- `#!cpp std::unique_ptr<PeakLowShelf<T>> lowShelf`: Low-shelf filter.
- `#!cpp std::vector<std::unique_ptr<PeakingFilter<T>>> peakingFilters`: Peaking filters.
- `#!cpp std::unique_ptr<PeakHighShelf<T>> highShelf`: High-shelf filter.
- `#!cpp Matrix<> filterResponseMatrix`: Matrix used for gain calculation.
- `#!cpp std::atomic<Real> targetGain`: Target DC gain.
- `#!cpp Real currentGain`: Current DC gain.
- `#!cpp std::atomic<bool> initialised`: True if initialised.
- `#!cpp std::atomic<bool> gainsEqual`: True if current and target gains are equal.

---

## Implementation Notes

- Based on Audio EQ Cookbook and research by Oliver & Jot (2015).
- Uses matrix inversion for accurate frequency response control.
- Handles gain interpolation for smooth transitions.

## Example Usage

```cpp
#include "DSP/GraphicEQ.h"
using namespace RAC::DSP;

Coefficients<> fc(std::vector<Real>{ 100.0, 1000.0, 5000.0 });
Real Q = 0.707;
int sampleRate = 48000;

// Create GraphicEQ with 3 bands
GraphicEQ<> eq(fc, Q, sampleRate);

// Set target gains
Coefficients<> gains(std::vector<Real>{ 3.0, 0.0, -2.0 });
eq.SetTargetGains(gains);

// Process audio sample
Real output = eq.GetOutput(1.0, 0.01);
```
