A multi-band graphic equalizer using cascaded IIR filters.  
Implements accurate frequency response control using low shelf, peaking, and high shelf filters.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/GraphicEQ.h`
- **Source:** `DSP/GraphicEQ.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Coefficients.h`, `Common/Matrix.h`, `Common/Vec.h`, `DSP/IIRFilter.h`, `DSP/Interpolate.h`

---

## Class Definition

```cpp
class GraphicEQ
{
public:
    GraphicEQ(const Coefficients& fc, const Real Q, const int sampleRate);
    GraphicEQ(const Coefficients& gain, const Coefficients& fc, const Real Q, const int sampleRate);
    ~GraphicEQ();

    bool SetTargetGains(const Coefficients& gains);
    Real GetOutput(const Real input, const Real lerpFactor);
    void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor);
    inline void ClearBuffers();

private:
    void InitMatrix(const Coefficients& fc, const Real Q, const Real fs);
    std::pair<Rowvec, Real> CalculateGains(const Coefficients& gains) const;
    Coefficients CreateFrequencyVector(const Coefficients& fc) const;
    void InterpolateGain(const Real lerpFactor);

    const int numFilters;
    Coefficients previousInput;
    std::unique_ptr<PeakLowShelf> lowShelf;
    std::vector<std::unique_ptr<PeakingFilter>> peakingFilters;
    std::unique_ptr<PeakHighShelf> highShelf;
    Matrix filterResponseMatrix;
    std::atomic<Real> targetGain;
    Real currentGain;
    std::atomic<bool> initialised;
    std::atomic<bool> gainsEqual;
};
```

---

## Public Methods

### `#!cpp GraphicEQ(const Coefficients& fc, const Real Q, const int sampleRate)`
**Constructor.**  
Initializes the GraphicEQ with zero gain and the given frequency bands, Q factor, and sample rate.
- `fc`: Filter band center frequencies.
- `Q`: Q factor for the filters.
- `sampleRate`: Sample rate for filter coefficients.

---

### `#!cpp GraphicEQ(const Coefficients& gain, const Coefficients& fc, const Real Q, const int sampleRate)`
**Constructor.**  
Initializes the GraphicEQ with given gains, frequency bands, Q factor, and sample rate.
- `gain`: Target gain for each center frequency.
- `fc`: Filter band center frequencies.
- `Q`: Q factor for the filters.
- `sampleRate`: Sample rate for filter coefficients.

---

### `#!cpp ~GraphicEQ()`
**Destructor.**  
Cleans up the GraphicEQ.

---

### `#!cpp bool SetTargetGains(const Coefficients& gains)`
Sets new target gains for each center frequency.
- `gains`: Target response for the GraphicEQ.
- **Returns:** True if the target and current gains are currently zero, false otherwise.

---

### `#!cpp Real GetOutput(const Real input, const Real lerpFactor)`
Returns the output of the GraphicEQ given an input.
- `input`: Input sample.
- `lerpFactor`: Linear interpolation factor.
- **Returns:** Output sample.

---

### `#!cpp void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor)`
Processes an input buffer and updates the output buffer.
- `inBuffer`: Input buffer.
- `outBuffer`: Output buffer.
- `numFrames`: Number of frames.
- `lerpFactor`: Linear interpolation factor.

---

### `#!cpp inline void ClearBuffers()`
Resets the filter buffers.

---

## Private Methods

### `#!cpp void InitMatrix(const Coefficients& fc, const Real Q, const Real fs)`
Initializes the filter response matrix for gain calculation.
- `fc`: Filter band center frequencies.
- `Q`: Q factor.
- `fs`: Sample rate.

---

### `#!cpp std::pair<Rowvec, Real> CalculateGains(const Coefficients& gains) const`
Calculates the filter gains based on the target response.
- `gains`: Target response.
- **Returns:** Pair of calculated gains and DC gain.

---

### `#!cpp Coefficients CreateFrequencyVector(const Coefficients& fc) const`
Creates a frequency vector for filter response calculation.
- `fc`: Filter band center frequencies.
- **Returns:** Frequency vector.

---

### `#!cpp void InterpolateGain(const Real lerpFactor)`
Linearly interpolates the current gain with the target gain.
- `lerpFactor`: Interpolation factor.

---

## Internal Data Members

- `#!cpp const int numFilters`: Number of filters.
- `#!cpp Coefficients previousInput`: Previous target response.
- `#!cpp std::unique_ptr<PeakLowShelf> lowShelf`: Low-shelf filter.
- `#!cpp std::vector<std::unique_ptr<PeakingFilter>> peakingFilters`: Peaking filters.
- `#!cpp std::unique_ptr<PeakHighShelf> highShelf`: High-shelf filter.
- `#!cpp Matrix filterResponseMatrix`: Matrix for gain calculation.
- `#!cpp std::atomic<Real> targetGain`: Target DC gain.
- `#!cpp Real currentGain`: Current DC gain.
- `#!cpp std::atomic<bool> initialised`: True if initialized.
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

Coefficients fc = { 100.0, 1000.0, 5000.0 };
Real Q = 0.707;
int sampleRate = 48000;

// Create GraphicEQ with 3 bands
GraphicEQ eq(fc, Q, sampleRate);

// Set target gains
Coefficients gains = { 3.0, 0.0, -2.0 };
eq.SetTargetGains(gains);

// Process audio sample
Real output = eq.GetOutput(1.0, 0.01);
```
