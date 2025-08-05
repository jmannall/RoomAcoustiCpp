Stores and manipulates arbitrary coefficient vectors or arrays, with arithmetic and utility operations.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Coefficients.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Definitions.h`

---

## Class Definition

```cpp
template <class T = std::vector<Real>>
class Coefficients
{
public:
    Coefficients(const Real in);
    Coefficients(const int len);
    Coefficients(const int len, const Real in);
    Coefficients(const T& coefficients);
    ~Coefficients();

    inline void Update(const T& coefficients);
    inline int Length() const;
    inline Coefficients Log();
    inline Coefficients Pow10();
    inline Coefficients Sqrt();
    inline Real& operator[](const size_t i);
    inline Real operator[](const size_t i) const;
    inline Coefficients& operator=(Real x);
    inline Coefficients& operator-();
    inline Coefficients& operator+=(const Coefficients& v);
    inline Coefficients& operator-=(const Coefficients& v);
    inline Coefficients& operator*=(const Coefficients& v);
    inline Coefficients& operator/=(const Coefficients& v);
    inline Coefficients& operator+=(const Real a);
    inline Coefficients& operator*=(const Real a);
    inline Coefficients& operator/=(const Real a);
    inline bool operator<(const Real a) const;
    inline bool operator>(const Real a) const;
    inline bool operator<=(const Real a) const;
    inline bool operator>=(const Real a) const;
    inline auto begin();
    inline auto end();
    inline const auto begin() const;
    inline const auto end() const;

protected:
    T mCoefficients;
};
```

---

## Public Methods

### `#!cpp Coefficients(const Real in)`
Initializes all coefficients to a value.
- `in`: Value for all coefficients.

---

### `#!cpp Coefficients(const int len)`
Initializes with zeros.
- `len`: Number of coefficients.

---

### `#!cpp Coefficients(const int len, const Real in)`
Initializes with a value.
- `len`: Number of coefficients.
- `in`: Value for all coefficients.

---

### `#!cpp Coefficients(const T& coefficients)`
Initializes from a vector or array.
- `coefficients`: Data.

---

### `#!cpp ~Coefficients()`
Destructor.

---

### `#!cpp inline void Update(const T& coefficients)`
Updates the coefficients.
- `coefficients`: New data.

---

### `#!cpp inline int Length() const`
Returns the number of coefficients.

---

### `#!cpp inline Coefficients Log()`
Applies log to each coefficient.

---

### `#!cpp inline Coefficients Pow10()`
Applies 10^x to each coefficient.

---

### `#!cpp inline Coefficients Sqrt()`
Applies sqrt to each coefficient.

---

### `#!cpp inline Real& operator[](const size_t i)`
Access by index.

---

### `#!cpp inline Real operator[](const size_t i) const`
Access by index (const).

---

### `#!cpp inline Coefficients& operator=(Real x)`
Sets all coefficients to `x`.

---

### `#!cpp inline Coefficients& operator-()`
Negates all coefficients.

---

### `#!cpp inline Coefficients& operator+=(const Coefficients& v)`
Adds another set of coefficients.

---

### `#!cpp inline Coefficients& operator-=(const Coefficients& v)`
Subtracts another set of coefficients.

---

### `#!cpp inline Coefficients& operator*=(const Coefficients& v)`
Element-wise multiply.

---

### `#!cpp inline Coefficients& operator/=(const Coefficients& v)`
Element-wise divide.

---

### `#!cpp inline Coefficients& operator+=(const Real a)`
Adds a value to all coefficients.

---

### `#!cpp inline Coefficients& operator*=(const Real a)`
Multiplies all coefficients by a value.

---

### `#!cpp inline Coefficients& operator/=(const Real a)`
Divides all coefficients by a value.

---

### `#!cpp inline bool operator<(const Real a) const`
True if all coefficients < a.

---

### `#!cpp inline bool operator>(const Real a) const`
True if all coefficients > a.

---

### `#!cpp inline bool operator<=(const Real a) const`
True if all coefficients <= a.

---

### `#!cpp inline bool operator>=(const Real a) const`
True if all coefficients >= a.

---

## Internal Data Members

- `#!cpp T mCoefficients`: Array or vector of coefficients.

---

## Implementation Notes

- Supports element-wise and scalar arithmetic.
- Operator overloads for arithmetic and comparison.
- Includes utility functions: `Sin`, `Cos`, `Abs`, `Sum`, `Pow`.

## Example Usage

```cpp
#include "Common/Coefficients.h"
using namespace RAC::Common;

Coefficients<> c(4, 1.0);
c[2] = 2.0;
c += 1.0;
auto s = Sum(c);
```
