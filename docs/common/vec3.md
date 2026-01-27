A 3D vector type used for positions, directions, and geometry.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

`Vec3` provides common vector operations (normalisation, dot/cross products, approximate comparisons) and is used in the API for listener/source positions and room geometry.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Vec3.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Definitions.h`

---

## Type Definition

```cpp
Vec3 p;                 // default (0, 0, 0)
Vec3 q(1.0, 2.0, 3.0);  // (x, y, z)
```

---

## Common Operations

### Component accessors
Returns the vector components.

```cpp
Real x() const; Real& x();
Real y() const; Real& y();
Real z() const; Real& z();
```

---

### `#!cpp Real SquareNormal() const`
Returns the squared magnitude.

---

### `#!cpp Real Normal() const`
Returns the magnitude.

---

### `#!cpp void Normalise()`
Normalises the vector in place.

---

### `#!cpp Vec3 Normalised() const`
Returns a normalised copy.

---

### `#!cpp Real dot(const Vec3& other) const`
Returns the dot product.

---

### `#!cpp Vec3 cross(const Vec3& other) const`
Returns the cross product.

---

### `#!cpp bool IsApprox(const Vec3& other, Real eps = EPS_GENERAL) const`
Checks approximate equality.

---

## Helper Functions

### `#!cpp Vec3 Round(const Vec3& v)`
Rounds the components using the library's rounding factor.

---

## Example Usage

```cpp
#include "Common/Vec3.h"
using namespace RAC::Common;

Vec3 a(1.0, 2.0, 3.0);
Vec3 b(4.0, 5.0, 6.0);

Real d = a.dot(b);
Vec3 c = a.cross(b);

a.Normalise();
```
