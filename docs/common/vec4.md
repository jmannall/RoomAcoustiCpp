A quaternion type for representing 3D rotations and orientations.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

`Vec4` is used in the API to represent listener and source orientations (as quaternions).

- **Namespace:** `RAC::Common`
- **Header:** `Common/vec4.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Vec3.h`

---

## Type Definition

```cpp
Vec4 q;                      // default (0, 0, 0, 0)
Vec4 q2(1.0, 0.0, 0.0, 0.0); // (w, x, y, z)
```

---

## Common Operations

### Component accessors
Returns the quaternion components.

```cpp
Real w() const; Real& w();
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
Normalises the quaternion in place.

---

### `#!cpp Vec4 Normalised() const`
Returns a normalised copy.

---

### `#!cpp Vec4 Conjugate() const`
Returns the conjugate quaternion.

---

### `#!cpp Vec4 InverseMatrix() const`
Returns the inverse quaternion.

---

### `#!cpp Vec4 operator*(const Vec4& other) const`
Quaternion multiplication.

---

## Helper Functions

### `#!cpp Vec3 RotateVector(const Vec3& v, const Vec4& orientation)`
Rotates a vector by the quaternion (assumes a normalised quaternion).

---

## Example Usage

```cpp
#include "Common/vec4.h"
#include "Common/Vec3.h"

using namespace RAC::Common;

Vec4 q(1.0, 0.0, 0.0, 0.0);
Vec3 v(1.0, 0.0, 0.0);

Vec3 rotated = RotateVector(v, q);
```
