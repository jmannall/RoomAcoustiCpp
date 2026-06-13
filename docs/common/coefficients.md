Stores and manipulates coefficient arrays, most commonly used for **frequency-dependent parameters** (e.g., absorption, T60, band gains).

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

`Coefficients<>` behaves like a 1D array of `Real` values, with convenience methods for sizing, initialisation and common element-wise transforms.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Coefficients.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Definitions.h`

---

## Type Definition

`Coefficients` is a template:

```cpp
template <typename T = Real, /*dynamic or fixed size*/>
using Coefficients = /* 1D coefficient array */;
Coefficients<> c; // Defaults to Real
```

---

## Common Operations

### `#!cpp int Length() const`
Returns the number of coefficients.

---

### `#!cpp void Reset()`
Sets all coefficients to 0.

---

### `#!cpp Coefficients Log() const`
Returns element-wise natural logarithm.

---

### `#!cpp Coefficients Pow10() const`
Returns element-wise `10^x`.

---

### `#!cpp Coefficients Pow(Real exponent) const`
Returns element-wise `x^exponent`.

---

### `#!cpp Coefficients Sqrt() const`
Returns element-wise square root.

---

### `#!cpp Coefficients Square() const`
Returns element-wise square.

---

### `#!cpp Coefficients Abs() const`
Returns element-wise absolute value.

---

### `#!cpp Coefficients Sin() const`
Returns element-wise sine.

---

### `#!cpp Coefficients Cos() const`
Returns element-wise cosine.

---

### Indexing
Access coefficients using the container's indexing operator.

---

## Example Usage

```cpp
#include "Common/Coefficients.h"
using namespace RAC::Common;

// 4-band coefficient vector
Coefficients<> c(std::vector<Real>({0.2, 0.3, 0.4, 0.5}));

int n = c.Length();

// Convert from dB-like values using 10^x
Coefficients<> linear = c.Pow10();
```
