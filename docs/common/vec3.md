A 3D vector class with arithmetic, normalization, and geometric operations.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Vec3.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Definitions.h`

---

## Class Definition

```cpp
class Vec3
{
public:
    Vec3();
    Vec3(const Real x, const Real y, const Real z);
    Vec3(const float x, const float y, const float z);
    Vec3(const double x, const double y, const double z);
    ~Vec3();

    inline Real Length() const;
    inline void Normalise();
    inline void RoundVec();
    inline Vec3& operator+=(const Vec3& v);
    inline Vec3& operator-=(const Vec3& v);
    inline Vec3& operator*=(const Real& a);
    inline Vec3& operator/=(const Real& a);
    template <typename Vector3Type>
    inline Vec3& operator=(const Vector3Type& v);

    Real x, y, z;
};
```

---

## Public Methods

### `#!cpp Vec3()`
**Default constructor.**  
Initializes a zero vector.

---

### `#!cpp Vec3(const Real x, const Real y, const Real z)`
**Constructor.**  
Initializes with specified values.

---

### `#!cpp Vec3(const float x, const float y, const float z)`
**Constructor.**  
Initializes from floats (if using double).

---

### `#!cpp Vec3(const double x, const double y, const double z)`
**Constructor.**  
Initializes from doubles (if using float).

---

### `#!cpp ~Vec3()`
**Destructor.**  
Cleans up the vector.

---

### `#!cpp inline Real Length() const`
Returns the vector length.

---

### `#!cpp inline void Normalise()`
Normalizes the vector.

---

### `#!cpp inline void RoundVec()`
Rounds the vector components.

---

### `#!cpp inline Vec3& operator+=(const Vec3& v)`
Adds another vector.

---

### `#!cpp inline Vec3& operator-=(const Vec3& v)`
Subtracts another vector.

---

### `#!cpp inline Vec3& operator*=(const Real& a)`
Multiplies by a scalar.

---

### `#!cpp inline Vec3& operator/=(const Real& a)`
Divides by a scalar.

---

### `#!cpp template <typename Vector3Type> inline Vec3& operator=(const Vector3Type& v)`
Assigns from another type with x, y, z.

---

## Operators

- `operator+`, `operator-`, `operator*`, `operator/`: Arithmetic.
- `operator==`, `operator!=`: Comparison.
- `operator<<`: Stream output.

---

## Functions

- `UnitVector`, `UnitVectorRound`, `Dot`, `Cross`: Geometric operations.

---

## Internal Data Members

- `#!cpp Real x, y, z`: Vector components.

---

## Implementation Notes

- Used for 3D geometry and spatial calculations.

## Example Usage

```cpp
#include "Common/Vec3.h"
using namespace RAC::Common;

Vec3 a(1.0, 2.0, 3.0);
Vec3 b(4.0, 5.0, 6.0);
Vec3 c = a + b;
c.Normalise();
```
