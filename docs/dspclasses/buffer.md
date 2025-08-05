A resizeable buffer class for storing and manipulating audio or impulse response data.  
Provides element access, resizing, arithmetic operations, and validity checks.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/Buffer.h`
- **Source:** `DSP/Buffer.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Definitions.h`

---

## Class Definition

```cpp
class Buffer
{
public:
    Buffer();
    Buffer(const int length);
    Buffer(const std::vector<Real>& vector);
    ~Buffer();

    inline void Reset();
    inline size_t Length() const;
    void ResizeBuffer(const size_t numSamples);
    bool Valid();
    inline Real& operator[](const int i);
    inline Real operator[](const int i) const;
    inline Buffer operator*=(const Real a);
    inline Buffer operator+=(const Real a);
    inline Buffer operator+=(const Buffer& x);

    inline auto begin();
    inline auto end();
    inline const auto begin() const;
    inline const auto end() const;

private:
    std::vector<Real> mBuffer;
};
```

---

## Public Methods

### `#!cpp Buffer()`
**Default constructor.**  
Initializes buffer with 1 sample.

---

### `#!cpp Buffer(const int length)`
**Constructor.**  
Initializes the buffer with a specified number of samples.
- `length`: Number of samples.

---

### `#!cpp Buffer(const std::vector<Real>& vector)`
**Constructor.**  
Initializes the buffer with a vector of Real values.
- `vector`: Vector of values.

---

### `#!cpp ~Buffer()`
**Destructor.**  
Cleans up the buffer.

---

### `#!cpp inline void Reset()`
Sets all samples in the buffer to 0.

---

### `#!cpp inline size_t Length() const`
Returns the length of the buffer.
- **Returns:** Number of samples.

---

### `#!cpp void ResizeBuffer(const size_t numSamples)`
Resizes the buffer to a specified number of samples.
- `numSamples`: New buffer size.

---

### `#!cpp bool Valid()`
Checks if the buffer contains only valid (non-NaN) values.
- **Returns:** False if any value is NaN, true otherwise.

---

### `#!cpp inline Real& operator[](const int i)`
Access the buffer at the specified index.
- `i`: Index.
- **Returns:** Reference to value at index.

---

### `#!cpp inline Real operator[](const int i) const`
Access the buffer at the specified index (const).
- `i`: Index.
- **Returns:** Value at index.

---

### `#!cpp inline Buffer operator*=(const Real a)`
Multiplies each sample in the buffer by a scalar value.
- `a`: Scalar.

---

### `#!cpp inline Buffer operator+=(const Real a)`
Adds a scalar value to each sample in the buffer.
- `a`: Scalar.

---

### `#!cpp inline Buffer operator+=(const Buffer& x)`
Adds another buffer to this buffer (element-wise).
- `x`: Buffer to add.

---

### `#!cpp inline auto begin()`
Returns iterator to beginning of buffer.

---

### `#!cpp inline auto end()`
Returns iterator to end of buffer.

---

### `#!cpp inline const auto begin() const`
Returns const iterator to beginning of buffer.

---

### `#!cpp inline const auto end() const`
Returns const iterator to end of buffer.

---

## Internal Data Members

- `#!cpp std::vector<Real> mBuffer`: Buffer data.

---

## Implementation Notes

- Used for audio and impulse response storage.
- Supports arithmetic and element-wise operations.
- Provides validity checking for NaN values.

## Example Usage

```cpp
#include "DSP/Buffer.h"
using namespace RAC::DSP;

Buffer buf(8);
buf[0] = 1.0;
buf[1] = 2.0;
buf += 0.5;
buf *= 2.0;
if (buf.Valid()) {
    // Use buffer
}
```
