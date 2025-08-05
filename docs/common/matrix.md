A general-purpose matrix class supporting basic arithmetic, element-wise operations, inversion, transposition, and randomization.  
Includes operator overloads for arithmetic and comparison.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Matrix.h`
- **Source:** `Common/Matrix.cpp`
- **Dependencies:** `Common/Types.h`

---

## Class Definition

```cpp
class Matrix
{
public:
    Matrix();
    Matrix(const int r, const int c);
    Matrix(const std::vector<std::vector<Real>>& matrix);
    ~Matrix();

    inline void Reset();
    inline void AddColumn(const std::vector<Real>& column, const int c);
    inline void AddRow(const std::vector<Real>& row, const int r);
    inline const std::vector<Real>& GetRow(int r) const;
    inline const std::vector<Real>& GetColumn(int c);
    inline const std::vector<std::vector<Real>>& Data() const;
    inline int Rows() const;
    inline int Cols() const;
    Matrix Transpose();
    void Inverse();
    void Log10();
    void Pow10();
    void Max(const Real min);
    void Min(const Real max);
    void RandomUniformDistribution();
    void RandomUniformDistribution(Real a, Real b);

    inline std::vector<Real>& operator[](const int r);
    inline const std::vector<Real>& operator[](const int r) const;
    inline Matrix operator+=(const Matrix& matrix);
    inline Matrix operator-=(const Matrix& matrix);
    inline Matrix operator*=(const Real a);
    inline Matrix operator/=(const Real a);
    inline Matrix operator+=(const Real a);
    inline Matrix operator-=(const Real a);

protected:
    int rows, cols;
    std::vector<std::vector<Real>> data;
    std::vector<Real> column;

private:
    void Init(const std::vector<std::vector<Real>>& matrix);
    void AllocateSpace();
    inline void DeallocateSpace();
};
```

---

## Public Methods

### `#!cpp Matrix()`
**Default constructor.**  
Initializes an empty matrix.

---

### `#!cpp Matrix(const int r, const int c)`
**Constructor.**  
Initializes a matrix of size `r` x `c` with zeros.
- `r`: Number of rows.
- `c`: Number of columns.

---

### `#!cpp Matrix(const std::vector<std::vector<Real>>& matrix)`
**Constructor.**  
Initializes a matrix with the given data.
- `matrix`: 2D vector of values.

---

### `#!cpp ~Matrix()`
**Destructor.**  
Cleans up the matrix.

---

### `#!cpp inline void Reset()`
Sets all elements to zero.

---

### `#!cpp inline void AddColumn(const std::vector<Real>& column, const int c)`
Sets column `c` to the given values.
- `column`: Values to set.
- `c`: Column index.

---

### `#!cpp inline void AddRow(const std::vector<Real>& row, const int r)`
Sets row `r` to the given values.
- `row`: Values to set.
- `r`: Row index.

---

### `#!cpp inline const std::vector<Real>& GetRow(int r) const`
Returns a const reference to row `r`.

---

### `#!cpp inline const std::vector<Real>& GetColumn(int c)`
Returns a reference to column `c`.

---

### `#!cpp inline const std::vector<std::vector<Real>>& Data() const`
Returns a const reference to the matrix data.

---

### `#!cpp inline int Rows() const`
Returns the number of rows.

---

### `#!cpp inline int Cols() const`
Returns the number of columns.

---

### `#!cpp Matrix Transpose()`
Returns the transpose of the matrix.

---

### `#!cpp void Inverse()`
Inverts the matrix (must be square).

---

### `#!cpp void Log10()`
Applies log10 to each element.

---

### `#!cpp void Pow10()`
Applies 10^x to each element.

---

### `#!cpp void Max(const Real min)`
Sets each element to at least `min`.

---

### `#!cpp void Min(const Real max)`
Sets each element to at most `max`.

---

### `#!cpp void RandomUniformDistribution()`
Fills the matrix with random values in [0, 1].

---

### `#!cpp void RandomUniformDistribution(Real a, Real b)`
Fills the matrix with random values in [a, b].

---

### `#!cpp inline std::vector<Real>& operator[](const int r)`
Returns a reference to row `r`.

---

### `#!cpp inline const std::vector<Real>& operator[](const int r) const`
Returns a const reference to row `r`.

---

### `#!cpp inline Matrix operator+=(const Matrix& matrix)`
Adds another matrix element-wise.

---

### `#!cpp inline Matrix operator-=(const Matrix& matrix)`
Subtracts another matrix element-wise.

---

### `#!cpp inline Matrix operator*=(const Real a)`
Multiplies all elements by `a`.

---

### `#!cpp inline Matrix operator/=(const Real a)`
Divides all elements by `a`.

---

### `#!cpp inline Matrix operator+=(const Real a)`
Adds `a` to all elements.

---

### `#!cpp inline Matrix operator-=(const Real a)`
Subtracts `a` from all elements.

---

## Operators

- `operator==`: Element-wise comparison.
- `operator+`, `operator-`, `operator*`, `operator/`: Matrix arithmetic.
- `Mulitply(out, u, v)`: Matrix multiplication.

---

## Internal Data Members

- `#!cpp int rows, cols`: Matrix dimensions.
- `#!cpp std::vector<std::vector<Real>> data`: Matrix data.
- `#!cpp std::vector<Real> column`: Temporary storage for a column.

---

## Implementation Notes

- Supports element-wise and matrix arithmetic.
- Includes randomization and utility functions.
- Inversion uses Gauss-Jordan elimination.

## Example Usage

```cpp
#include "Common/Matrix.h"
using namespace RAC::Common;

Matrix m(3, 3);
m[0][0] = 1.0;
m[1][1] = 2.0;
m[2][2] = 3.0;
Matrix t = m.Transpose();
```
