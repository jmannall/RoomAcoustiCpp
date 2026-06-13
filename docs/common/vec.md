Implements the dynamic-length column vector type (`Vec`) used throughout the library.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

`Vec<>` is a 1D container of `Real` values with a small set of convenience methods (e.g., length queries, zeroing) and standard linear-algebra operations.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Vec.h`
- **Source:** `Common/Vec.cpp`
- **Dependencies:** `Common/Types.h`

---

## Type Definition

`Vec` is a template:

```cpp
template <typename T = Real>
using Vec = /* dynamic-length column vector */;
Vec<> v; // Defaults to Real
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

## Utility Functions

### `#!cpp Real ThreeWayDot(const Vec<>& u, const Vec<>& v, const Vec<>& w)`
Returns `u · (v ⊙ w)` (dot product of `u` with the element-wise product of `v` and `w`).

---

## Example Usage

```cpp
#include "Common/Vec.h"
using namespace RAC::Common;

Vec<> v(std::vector<Real>({1.0, 2.0, 3.0}));
Real s = v.Sum();

Vec<> u(std::vector<Real>({1.0, 0.0, 0.0}));
Vec<> w(std::vector<Real>({0.5, 0.5, 0.5}));
Real t = ThreeWayDot(u, v, w);
```
