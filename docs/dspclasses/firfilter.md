The `FIRFilter` class implements a lock free Finite Impulse Response (FIR) filter for digital signal processing. It allows filtering of input signals using a configurable impulse response, supporting dynamic resizing and interpolation of the filter coefficients.

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
        FIRFilter(const Buffer& ir, const int maxSize);
        ~FIRFilter();

        Real GetOutput(const Real input, const Real lerpFactor);
        bool SetTargetIR(const Buffer& ir);
        void Reset();

    private:
        void InterpolateIR(const Real lerpFactor);

        const int maxFilterLength;
        std::atomic<std::shared_ptr<const Buffer>> targetIR;
        Buffer currentIR;
        Buffer inputLine;
        size_t irLength;
        size_t oldIrLength;
        int count;
        std::atomic<bool> clearInputLine;
        std::atomic<bool> irsEqual;
        std::atomic<bool> initialised;
        static ReleasePool releasePool;
};
```

---

## Public Methods

### `#!cpp FIRFilter(const Buffer& ir, const int maxSize)`
**Constructor.**  
Initializes the FIR filter with the given impulse response and maximum filter size.  
Pads the impulse response to a multiple of 8 for efficient processing.  
Sets `initialised` to true if the impulse response is less than or equal to the specified maximum size.

- `ir`: The impulse response buffer.
- `maxSize`: The maximum size of the FIRFilter.

---

### `#!cpp ~FIRFilter()`
**Destructor.**  
Cleans up the filter.

---

### `#!cpp Real GetOutput(const Real input, const Real lerpFactor)`
Processes a single input sample and returns the filtered output.  
If the `initialised` flag is false, returns 0.0.  
If the `irsEqual` flag is false, calls `InterpolateIR`.  
If the `clearInputLine` flag is true, zeros the input buffer before processing.

- `input`: The input sample.
- `lerpFactor`: The lerp factor for interpolation between the current and target impulse response.
- **Returns:** The filtered output sample.

---

### `#!cpp bool SetTargetIR(const Buffer& ir)`
Updates the target impulse response for the filter.  
The new impulse response is padded to a multiple of 8 and must not exceed the maximum filter length.  
Sets `irsEqual` to false.

- `ir`: The new target impulse response.
- **Returns:** True if the target impulse response was set successfully, false otherwise.

---

### `#!cpp void Reset()`
Sets `clearInputLine` to true, which will clear the input line to zeros the next time `GetOutput` is called.

---

## Private Methods

### `#!cpp void InterpolateIR(const Real lerpFactor)`
Interpolates between the current impulse response and the target impulse response using linear interpolation.  
If the current and target impulse responses become equal, sets `irsEqual` to true.

- `lerpFactor`: The lerp factor for interpolation.

---

## Internal Data Members

- `#!cpp const int maxFilterLength`: The maximum filter length (always a multiple of 8).
- `#!cpp std::atomic<std::shared_ptr<const Buffer>> targetIR`: The target impulse response (thread-safe).
- `#!cpp Buffer currentIR`: The current impulse response buffer (audio thread only).
- `#!cpp Buffer inputLine`: The input line buffer (audio thread only).
- `#!cpp size_t irLength`: The current impulse response length.
- `#!cpp size_t oldIrLength`: The previous impulse response length (used when interpolating).
- `#!cpp int count`: Index for the next sample entry to the input line.
- `#!cpp std::atomic<bool> clearInputLine`: Flag to clear the input line.
- `#!cpp std::atomic<bool> irsEqual`: Flag indicating if the current and target impulse responses are equal.
- `#!cpp std::atomic<bool> initialised`: Flag indicating if the filter is initialized.
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

const int fs = 48e3;
const Real lerpFactor = 2.0 / static_cast<Real>(fs);

const Buffer irInit = { 1.0, 0.5, 0.25, 0.0 };
const Buffer irUpdate = { 3.1, 0.9, -0.8, 0.3, 0.4, -0.2, 0.0, 0.87, -0.1 };

// Create FIR filter of max length 16 samples
FIRFilter filter(irInit, 16);

// Update target impulse response
filter.SetTargetIR(irUpdate);

// Process audio
Real output = filter.GetOutput(1.0, lerpFactor);
```