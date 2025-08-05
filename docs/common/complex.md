Defines a complex number type alias and arithmetic operators for real/complex combinations.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Complex.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `<complex>`

---

## Type Definition

```cpp
typedef std::complex<Real> Complex;
```

---

## Constants

- `imUnit`: The imaginary unit (0, 1) as a `Complex`.

---

## Operators

### `#!cpp Complex operator*(const Complex a, const Real b)`
Multiplies a complex number by a real number.

---

### `#!cpp Complex operator+(const Complex a, const Real b)`
Adds a real number to a complex number.

---

### `#!cpp Complex operator/(const Complex a, const Real b)`
Divides a complex number by a real number.

---

## Implementation Notes

- Provides convenience operators for mixing real and complex arithmetic.
- `imUnit` is defined for both float and double precision.

## Example Usage

```cpp
#include "Common/Complex.h"
using namespace RAC::Common;

Complex z = Complex(1.0, 2.0);
Complex w = z * 2.0 + 1.0;
```
