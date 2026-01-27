Defines a complex number type alias and arithmetic operators for real/complex combinations.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

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

Overloads are also provided for `Real` on the left-hand side (e.g., `Real * Complex`) and for complex-to-complex arithmetic.

### `#!cpp Complex operator*(Complex a, Real b)`
Multiplies a complex number by a real number.

---

### `#!cpp Complex operator+(Complex a, Real b)`
Adds a real number to a complex number.

---

### `#!cpp Complex operator/(Complex a, Real b)`
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
