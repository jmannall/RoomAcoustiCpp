A resizable 1D buffer used for storing and manipulating audio samples and impulse responses.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

`Buffer<>` is used in the API for audio submission (`SubmitAudio`), output retrieval (`GetOutput`), and for providing impulse responses (e.g., headphone EQ).

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/Buffer.h`
- **Source:** `DSP/Buffer.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Definitions.h`

---

## Type Definition

`Buffer` is a template:

```cpp
template <typename T = Real>
using Buffer = /* 1D sample buffer */;
Buffer<> b; // Defaults to Real
```

---

## Common Operations

### `#!cpp Buffer()`
Creates an empty buffer.

---

### `#!cpp Buffer(int length)`
Creates a buffer of the given length, initialised to 0.

`length`: Number of samples.

---

### `#!cpp Buffer(const std::vector<T>& values)`
Creates a buffer from an existing vector of values.

`values`: Initial sample data.

---

### `#!cpp static Buffer Zero(int length)`
Creates a zero-initialised buffer.

---

### `#!cpp size_t Length() const`
Returns the number of samples.

---

### `#!cpp void Resize(size_t newLength)`
Resizes the buffer.

If the buffer grows, new samples are initialised to 0.

---

### `#!cpp void Reset()`
Sets all samples to 0.

---

### `#!cpp bool Valid()`
Returns true if all values are finite (i.e., no NaNs/Infs).

---

### Indexing
Access samples using the buffer's indexing operator (e.g., `buf[i]`), or via `data()` if you need raw contiguous access.

---

## Example Usage

```cpp
#include "DSP/Buffer.h"
using namespace RAC::DSP;

Buffer<> buf(8);
buf.data()[0] = 1.0;
buf.data()[1] = 2.0;

buf += 0.5;
buf *= 2.0;

if (buf.Valid())
{
    // Use buffer
}
```
