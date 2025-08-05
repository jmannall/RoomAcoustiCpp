Stores absorption coefficients for materials, derived from `Coefficients`.  
Handles reflectance-to-absorption conversion and area tracking.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Coefficients.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Definitions.h`

---

## Class Definition

```cpp
template <typename T = std::vector<Real>>
class Absorption : public Coefficients<T>
{
public:
    Absorption(int len);
    Absorption(const T& R);
    ~Absorption();

    void Reset();
    inline Absorption& operator=(Real x);
    inline Absorption& operator-();
    inline Absorption& operator+=(const Absorption& v);
    inline Absorption& operator-=(const Absorption& v);
    inline Absorption& operator*=(const Absorption& v);
    inline Absorption& operator/=(const Absorption& v);
    inline Absorption& operator+=(const Real a);
    inline Absorption& operator*=(const Real a);

    Real mArea;
};
```

---

## Public Methods

### `#!cpp Absorption(int len)`
Initializes with ones.
- `len`: Number of coefficients.

---

### `#!cpp Absorption(const T& R)`
Initializes from reflectance values (converts to absorption).
- `R`: Reflectance values.

---

### `#!cpp ~Absorption()`
Destructor.

---

### `#!cpp void Reset()`
Sets all coefficients to one.

---

### `#!cpp inline Absorption& operator=(Real x)`
Sets all coefficients to `x`.

---

### `#!cpp inline Absorption& operator-()`
Negates all coefficients.

---

### `#!cpp inline Absorption& operator+=(const Absorption& v)`
Adds another absorption (areas are added).

---

### `#!cpp inline Absorption& operator-=(const Absorption& v)`
Subtracts another absorption (areas are subtracted).

---

### `#!cpp inline Absorption& operator*=(const Absorption& v)`
Element-wise multiply.

---

### `#!cpp inline Absorption& operator/=(const Absorption& v)`
Element-wise divide.

---

### `#!cpp inline Absorption& operator+=(const Real a)`
Adds a value to all coefficients.

---

### `#!cpp inline Absorption& operator*=(const Real a)`
Multiplies all coefficients by a value.

---

## Internal Data Members

- `#!cpp Real mArea`: Area covered by the absorption coefficients.

---

## Implementation Notes

- Converts reflectance to absorption using sqrt(1 - R).
- Supports area tracking and arithmetic.

## Example Usage

```cpp
#include "Common/Coefficients.h"
using namespace RAC::Common;

Absorption<> a(4);
a[2] = 0.5;
a.mArea = 10.0;
```
