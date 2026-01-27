Implements the dynamic-length row vector type (`Rowvec`) used throughout the library.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

`Rowvec<>` is a 1D container of `Real` values laid out as a row vector.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Vec.h`
- **Source:** `Common/Vec.cpp`
- **Dependencies:** `Common/Types.h`

---

## Type Definition

`Rowvec` is a template:

```cpp
template <typename T = Real>
using Rowvec = /* dynamic-length row vector */;
Rowvec<> r; // Defaults to Real
```

---

## Common Operations

### `#!cpp int Length() const`
Returns the number of elements.

---

### `#!cpp void Reset()`
Sets all entries to 0.

---

### `#!cpp Real Sum() const`
Returns the sum of all entries.

---

### `#!cpp Real Mean() const`
Returns the mean value.

---

### Indexing
Access elements using the container's indexing operator.

---

## Example Usage

```cpp
#include "Common/Vec.h"
using namespace RAC::Common;

Rowvec<> r(std::vector<Real>({1.0, 2.0, 3.0}));
Real mean = r.Mean();
```
