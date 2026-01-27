Implements a first-order IIR filter for simulating air absorption effects.  
Based on the method from Grimm et al. (2014), the distance can be dynamically changed in real-time using linear interpolation to update filter coefficients.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](interface.md). This page documents lower-level details for advanced usage.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/AirAbsorption.h`
- **Source:** `Spatialiser/AirAbsorption.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Debug.h`, `DSP/Buffer.h`, `DSP/IIRFilter.h`

---

## Class Definition

```cpp title="" linenums="1"
class AirAbsorption : public IIRFilter1
{
    public:
        AirAbsorption(const Real distance, const int sampleRate);
        ~AirAbsorption();

        inline void SetTargetDistance(const Real distance);
        void ProcessAudio(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor);
        // ...inherited methods from IIRFilter1...

    private:
        inline void UpdateCoefficients(Real distance);
        void InterpolateParameters(const Real lerpFactor) override;

        const Real constant;
        std::atomic<Real> targetDistance;
        Real currentDistance;
};
```

---

## Public Methods

### `#!cpp AirAbsorption(const Real distance, const int sampleRate)`
**Constructor.**  
Initializes the air absorption filter with a specified distance and sample rate.
- `distance`: Initial distance for filter calculation.
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ~AirAbsorption()`
**Destructor.**  
Cleans up the air absorption filter.

---

### `#!cpp inline void SetTargetDistance(const Real distance)`
Sets the target distance for the filter, triggering parameter interpolation.
- `distance`: The new target distance (must be > 0).

---

### `#!cpp void ProcessAudio(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor)`
Processes an input buffer and writes the filtered output to the output buffer.
- `inBuffer`: Input audio buffer.
- `outBuffer`: Output audio buffer.
- `lerpFactor`: Interpolation factor for parameter smoothing.

---

## Private Methods

### `#!cpp inline void UpdateCoefficients(Real distance)`
Updates the filter coefficients based on the specified distance.
- `distance`: Distance for coefficient calculation.

---

### `#!cpp void InterpolateParameters(const Real lerpFactor) override`
Interpolates between the current and target distance using linear interpolation.
- `lerpFactor`: Interpolation factor (0.0 to 1.0).

---

## Internal Data Members

- `#!cpp const Real constant`: Precomputed constant for coefficient calculation.
- `#!cpp std::atomic<Real> targetDistance`: Target distance for interpolation.
- `#!cpp Real currentDistance`: Current distance.

---

## Implementation Notes

- Based on the method from Grimm et al. (2014), with a correction to the filter equation as noted in the code comments.
- Uses a first-order IIR filter.
- Distance changes are smoothed using linear interpolation for artifact-free transitions.

## Example Usage

```cpp
#include "Spatialiser/AirAbsorption.h"
using namespace RAC::Spatialiser;
using namespace RAC::DSP;

const int sampleRate = 48e3;
const Real lerpFactor = 0.01;

Buffer<> inputBuffer(512);
inputBuffer.data()[0] = 1.0;
Buffer<> outputBuffer(512);

const Real distance = 5.0;
const Real newDistance = 10.0;

// Create AirAbsorption filter
AirAbsorption airAbs(distance, sampleRate);

// Set a new target distance
airAbs.SetTargetDistance(newDistance);

// Process audio buffer
airAbs.ProcessAudio(inputBuffer, outputBuffer, lerpFactor);
```
