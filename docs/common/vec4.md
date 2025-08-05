A quaternion (4D vector) class for representing rotations and orientations in 3D space.

- **Namespace:** `RAC::Common`
- **Header:** `Common/vec4.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Vec3.h`

---

## Class Definition

```cpp
class Vec4
{
public:
    Vec4();
    Vec4(const Real w, const Real x, const Real y, const Real z);
    Vec4(const float w, const float x, const float y, const float z);
    Vec4(const double w, const double x, const double y, const double z);
    Vec4(const Real w, const Vec3 vec);
    Vec4(const Vec3 vec);
    ~Vec4();

    inline Vec3 Forward() const;
    inline Real SquareNormal() const;
    Vec4 Inverse() const;
    inline Vec4 Conjugate() const;
    Vec3 RotateVector(const Vec3& v) const;
    inline Vec4 operator*(const Vec4& v) const;
    inline Vec4 operator/(const Real a) const;
    template <typename CQuaternionType>
    inline Vec4& operator=(const CQuaternionType& q);

    Real w, x, y, z;
};
```

---

## Public Methods

### `#!cpp Vec4()`
**Default constructor.**  
Initializes a zero quaternion.

---

### `#!cpp Vec4(const Real w, const Real x, const Real y, const Real z)`
**Constructor.**  
Initializes with specified values.

---

### `#!cpp Vec4(const float w, const float x, const float y, const float z)`
**Constructor.**  
Initializes from floats (if using double).

---

### `#!cpp Vec4(const double w, const double x, const double y, const double z)`
**Constructor.**  
Initializes from doubles (if using float).

---

### `#!cpp Vec4(const Real w, const Vec3 vec)`
**Constructor.**  
Initializes from a scalar and a Vec3.

---

### `#!cpp Vec4(const Vec3 vec)`
**Constructor.**  
Initializes from a Vec3 (w = 0).

---

### `#!cpp ~Vec4()`
**Destructor.**  
Cleans up the quaternion.

---

### `#!cpp inline Vec3 Forward() const`
Returns the forward vector.

---

### `#!cpp inline Real SquareNormal() const`
Returns the squared norm.

---

### `#!cpp Vec4 Inverse() const`
Returns the inverse quaternion.

---

### `#!cpp inline Vec4 Conjugate() const`
Returns the conjugate quaternion.

---

### `#!cpp Vec3 RotateVector(const Vec3& v) const`
Rotates a vector by the quaternion.

---

### `#!cpp inline Vec4 operator*(const Vec4& v) const`
Quaternion multiplication.

---

### `#!cpp inline Vec4 operator/(const Real a) const`
Divides by a scalar.

---

### `#!cpp template <typename CQuaternionType> inline Vec4& operator=(const CQuaternionType& q)`
Assigns from another type with w, x, y, z.

---

## Operators

- `operator==`, `operator!=`, `operator-`: Comparison and negation.

---

## Internal Data Members

- `#!cpp Real w, x, y, z`: Quaternion components.

---

## Implementation Notes

- Used for 3D rotations and orientation.
- Supports quaternion algebra and vector rotation.

## Example Usage

```cpp
#include "Common/vec4.h"
using namespace RAC::Common;

Vec4 q(1.0, 0.0, 0.0, 0.0);
Vec3 v(1.0, 0.0, 0.0);
Vec3 rotated = q.RotateVector(v);
```
