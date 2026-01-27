A general-purpose dynamic matrix type used across the library.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

`Matrix<>` supports common matrix operations (transpose, inversion, element-wise transforms) and basic convenience methods.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Matrix.h`
- **Source:** `Common/Matrix.cpp`
- **Dependencies:** `Common/Types.h`

---

## Type Definition

`Matrix` is a template:

```cpp
template <typename T = Real>
using Matrix = /* dynamic 2D matrix */;
Matrix<> A; // Defaults to Real
```

---

## Common Operations

### `#!cpp int Rows() const`
Returns the number of rows.

---

### `#!cpp int Cols() const`
Returns the number of columns.

---

### `#!cpp void Reset()`
Sets all entries to 0.

---

### `#!cpp void Log10()`
Applies `log10` element-wise.

---

### `#!cpp void Pow10()`
Applies `10^x` element-wise.

---

### `#!cpp void Min(Real value)`
Clamps entries to be at most `value`.

---

### `#!cpp void Max(Real value)`
Clamps entries to be at least `value`.

---

### `#!cpp Matrix Transposed() const`
Returns the transpose.

---

### `#!cpp Matrix InverseMatrix() const`
Returns the inverse of a square matrix.

---


## Example Usage

```cpp
#include "Common/Matrix.h"
using namespace RAC::Common;

Matrix<> A(3, 3);
A.Reset();
A(0, 0) = 1.0;
A(1, 1) = 1.0;
A(2, 2) = 1.0;

Matrix<> inv = A.InverseMatrix();
```
