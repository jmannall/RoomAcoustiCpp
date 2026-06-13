The `FIRFilter` class implements a lock-free Finite Impulse Response (FIR) filter for digital signal processing. It allows filtering of input signals using a configurable impulse response, supporting dynamic resizing and interpolation of the filter coefficients.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/FIRFilter.h`
- **Source:** `DSP/FIRFilter.cpp`
- **Dependencies:** `DSP/Buffer.h`, `DSP/Interpolate.h`, `Common/Types.h`, `Common/ReleasePool.h`

---

## Class Definition

``` cpp title="" linenums="1"
class FIRFilter
{
    public:
        FIRFilter(const Buffer<>& ir, const int maxSize);

        bool IsValid() const;

        virtual Real GetOutput(const Real input, const Real lerpFactor);
        bool SetTargetIR(const Buffer<>& ir);
        inline void ClearBuffers();

    private:
        void InterpolateIR(const Real lerpFactor);

        const int maxFilterLength;
        size_t irLength;

        std::atomic<bool> initialised;

        Buffer<> currentIR;
        Buffer<> inputLine;

        int count;

        std::atomic<std::shared_ptr<const Buffer<>>> targetIR;
        size_t oldIrLength;

        std::atomic<bool> irsEqual;
        static ReleasePool releasePool;
};
```

---

## Public Methods

### `#!cpp FIRFilter(const Buffer<>& ir, const int maxSize)`
**Constructor.**  
Initializes the FIR filter with the given impulse response and maximum filter size.  
Pads the maximum size to a multiple of 8 for efficient processing.  
Sets the filter to valid if the impulse response can be set successfully.

`ir`: The impulse response buffer.  
`maxSize`: The maximum size of the FIRFilter.

---

### `#!cpp bool IsValid() const`
Returns whether the filter was successfully initialised and can be used.

**Returns:** True if valid, false otherwise.

---

### `#!cpp Real GetOutput(const Real input, const Real lerpFactor)`
Processes a single input sample and returns the filtered output.  
If the filter is not valid, returns 0.

`input`: The input sample.  
`lerpFactor`: The lerp factor for interpolation between the current and target impulse response.  
**Returns:** The filtered output sample.

---

### `#!cpp bool SetTargetIR(const Buffer<>& ir)`
Updates the target impulse response for the filter.  
The new impulse response is padded/handled for efficient processing and must not exceed the maximum filter length.

`ir`: The new target impulse response.  
**Returns:** True if the target impulse response was set successfully, false otherwise.

---

### `#!cpp inline void ClearBuffers()`
Clears the internal input line (sets it to zero).

---

## Private Methods

### `#!cpp void InterpolateIR(const Real lerpFactor)`
Interpolates between the current impulse response and the target impulse response using linear interpolation.

`lerpFactor`: The lerp factor for interpolation.

---

## Internal Data Members

- `#!cpp const int maxFilterLength`: The maximum filter length (always a multiple of 8).
- `#!cpp size_t irLength`: Length of the current impulse response.
- `#!cpp std::atomic<bool> initialised`: True if the filter is initialised.
- `#!cpp Buffer<> currentIR`: The current impulse response buffer.
- `#!cpp Buffer<> inputLine`: The input line buffer.
- `#!cpp int count`: Index for the next sample entry to the input line.
- `#!cpp std::atomic<std::shared_ptr<const Buffer<>>> targetIR`: The target impulse response (thread-safe).
- `#!cpp size_t oldIrLength`: Previous impulse response length (used when interpolating).
- `#!cpp std::atomic<bool> irsEqual`: True if the current and target impulse responses are known to be equal.
- `#!cpp static ReleasePool releasePool`: Garbage collector for shared pointers after atomic replacement.

---

## Implementation Notes

- The maximum filter size is fixed at construction. Any attempt to set a target impulse response larger than this maximum will fail.
- The impulse response is always padded to a multiple of eight for efficient vectorized processing.
- The input line buffer is double the filter length to simplify circular buffer logic and avoid bounds checks in the processing loop.
- The filter supports dynamic resizing and smooth transitions between impulse responses using linear interpolation.
- The filter is lock-free for real-time audio processing, using atomics for thread safety.

---

## Example Usage

```cpp
#include "DSP/FIRFilter.h"
using namespace RAC::DSP;

const int fs = 48000;
const Real lerpFactor = 2.0 / static_cast<Real>(fs);

Buffer<> irInit(std::vector<Real>{ 1.0, 0.5, 0.25, 0.0 });
Buffer<> irUpdate(std::vector<Real>{ 3.1, 0.9, -0.8, 0.3, 0.4, -0.2, 0.0, 0.87, -0.1 });

// Create FIR filter of max length 16 samples
FIRFilter filter(irInit, 16);

// Update target impulse response
filter.SetTargetIR(irUpdate);

// Process audio
Real output = filter.GetOutput(1.0, lerpFactor);
```
