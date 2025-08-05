Provides linear interpolation and approximate equality functions for real values, buffers, and coefficient classes.  
Also includes platform-specific functions for flushing denormals to zero for performance.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/Interpolate.h`
- **Source:** `DSP/Interpolate.cpp`
- **Dependencies:** `Common/Coefficients.h`, `DSP/Buffer.h`, `DSP/IIRFilter.h`, `Spatialiser/Types.h`

---

## Functions

### `#!cpp inline void FlushDenormals()`
Forces the CPU to flush denormal floating-point numbers to zero.  
Improves performance for recursive filter structures on supported platforms.

---

### `#!cpp inline void NoFlushDenormals()`
Resets the CPU to use denormals (disables flush-to-zero mode).

---

### `#!cpp inline Real Lerp(Real start, const Real end, const Real factor)`
Performs a linear interpolation between two real values.
- `start`: The current value.
- `end`: The target value.
- `factor`: The interpolation factor (must be between 0 and 1).
- **Returns:** The interpolated value.

---

### `#!cpp inline void Lerp(Buffer& start, const Buffer& end, const Real factor)`
Performs a linear interpolation between two buffers.  
If `start` is longer than `end`, the remaining samples are interpolated to zero.
- `start`: The current buffer to be interpolated.
- `end`: The target buffer.
- `factor`: The interpolation factor (must be between 0 and 1).

---

### `#!cpp inline void Lerp(Coefficients& start, const Coefficients& end, const Real factor)`
Performs a linear interpolation between two coefficient vectors.
- `start`: The current coefficients to be interpolated.
- `end`: The target coefficients.
- `factor`: The interpolation factor (must be between 0 and 1).

---

### `#!cpp inline bool Equals(const Real a, const Real b)`
Checks for approximate equality between two real values.
- `a`: Value 1.
- `b`: Value 2.
- **Returns:** True if equal within the threshold `EPS`, false otherwise.

---

### `#!cpp inline bool Equals(const Coefficients& u, const Coefficients& v)`
Checks for approximate equality between two coefficient vectors.
- `u`: Coefficients 1.
- `v`: Coefficients 2.
- **Returns:** True if equal within the threshold `EPS`, false otherwise.

---

### `#!cpp inline bool Equals(const Buffer& u, const Buffer& v, const int length)`
Checks for approximate equality between two buffers up to a specified length.
- `u`: Buffer 1.
- `v`: Buffer 2.
- `length`: Number of elements to compare.
- **Returns:** True if equal within the threshold `EPS`, false otherwise.

---

## Implementation Notes

- Uses platform-specific intrinsics for denormal handling (Windows, Android).
- Linear interpolation is used for smooth parameter transitions in real-time audio.
- Equality checks use a small epsilon threshold for floating-point comparison.

## Example Usage

```cpp
#include "DSP/Interpolate.h"
using namespace RAC::DSP;

Real a = 1.0, b = 2.0;
Real x = Lerp(a, b, 0.5); // x == 1.5

Buffer buf1(4), buf2(4);
buf1[0] = 1.0; buf2[0] = 2.0;
Lerp(buf1, buf2, 0.25);

bool eq = Equals(1.0, 1.0 + 1e-8); // true if within EPS
```
