Defines mathematical and physical constants (e.g., speed of sound, pi, sqrt(2)), and provides utility functions for conversions, rounding, factorials, and Legendre polynomials.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Definitions.h`
- **Source:** *(inline header only)*
- **Dependencies:** `Common/Types.h`

---

## Constants

- `T_CELCIUS`: Reference temperature in Celsius.
- `SPEED_OF_SOUND`: Speed of sound in air at reference temperature.
- `INV_SPEED_OF_SOUND`: Inverse speed of sound.
- `ROUND_FACTOR`: Rounding factor for floating-point operations.
- `EPS`: Tolerance for floating-point comparisons.
- `MIN_VALUE`: Minimum value for trimming filters.
- `PI_1`, `PI_2`, `PI_4`, `PI_8`: Pi and multiples.
- `SQRT_2`, `SQRT_3`, `SQRT_6`: Square roots.
- `INV_SQRT_2`, `INV_SQRT_3`, `INV_SQRT_6`: Inverse square roots.
- `LOG_10`, `LOG2_10`, `INV_LOG2_10`: Logarithmic constants.

---

## Functions

### `#!cpp inline Real Deg2Rad(Real x)`
Converts degrees to radians.
- `x`: Angle in degrees.
- **Returns:** Angle in radians.

---

### `#!cpp inline Real Rad2Deg(Real x)`
Converts radians to degrees.
- `x`: Angle in radians.
- **Returns:** Angle in degrees.

---

### `#!cpp inline Real Pow10(Real x)`
Calculates 10 raised to the power of `x`.
- `x`: Exponent.
- **Returns:** 10^x.

---

### `#!cpp inline Real Log10(Real x)`
Calculates the base-10 logarithm of `x`.
- `x`: Input value.
- **Returns:** log10(x).

---

### `#!cpp inline Real cot(const Real x)`
Calculates the cotangent of `x`.
- `x`: Angle in radians.
- **Returns:** cot(x).

---

### `#!cpp inline Real SafeAcos(Real x)`
Returns the arc-cosine of `x`, clamped to [-1, 1].
- `x`: Input value.
- **Returns:** acos(x) in radians.

---

### `#!cpp inline Real Sign(const Real x)`
Returns the sign of `x`.
- `x`: Input value.
- **Returns:** 1 if x > 0, -1 if x < 0, 0 if x == 0.

---

### `#!cpp inline Real Round(Real x)`
Rounds `x` to decimal places based on `ROUND_FACTOR`.
- `x`: Value to round.
- **Returns:** Rounded value.

---

### `#!cpp inline Real Round(Real x, size_t dp)`
Rounds `x` to a given number of decimal places.
- `x`: Value to round.
- `dp`: Number of decimal places.
- **Returns:** Rounded value.

---

### `#!cpp inline Real Factorial(const int n)`
Returns the factorial of `n` (n!).
- `n`: Integer.
- **Returns:** n!

---

### `#!cpp inline Real DoubleFactorial(const int n)`
Returns the double factorial of `n` (n!!).
- `n`: Integer.
- **Returns:** n!!

---

### `#!cpp inline Real LegendrePolynomial(const int l, const int m, const Real x)`
Computes the associated Legendre polynomial \( P_l^m(x) \).
- `l`: Degree.
- `m`: Order.
- `x`: Argument (typically cos(theta)).
- **Returns:** \( P_l^m(x) \).

---

### `#!cpp inline Real NormalisedSHLegendrePlm(const int l, const int m, const Real x)`
Computes the normalised spherical harmonic associated Legendre function.
- `l`: Degree.
- `m`: Order.
- `x`: Argument (typically cos(theta)).
- **Returns:** Normalised \( P_l^m(x) \).

---

## Implementation Notes

- Constants are defined for both float and double precision.
- Mathematical functions are implemented as inline functions for performance.

## Example Usage

```cpp
#include "Common/Definitions.h"
using namespace RAC::Common;

Real angle_deg = 90.0;
Real angle_rad = Deg2Rad(angle_deg);
Real y = Pow10(2.0); // 100.0
```
