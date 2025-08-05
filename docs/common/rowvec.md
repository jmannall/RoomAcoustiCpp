Implements a row vector (`Rowvec`) class, inheriting from `Matrix`.  
Supports vector arithmetic, statistics, and assignment from matrices.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Vec.h`
- **Source:** `Common/Vec.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Matrix.h`

---

## Class Definition

```cpp
class Rowvec : public Matrix
{
public:
    Rowvec();
    Rowvec(const int& length);
    Rowvec(const std::vector<Real>& vector);
    ~Rowvec();

    Real Sum() const;
    inline Real Mean() const;
    inline Real operator[](const int i) const;
    inline Real& operator[](const int i);
    inline Rowvec operator=(const Matrix& matrix);

private:
    void Init(const std::vector<Real>& vec);
};
```

---

## Public Methods

### `#!cpp Rowvec()`
**Default constructor.**  
Initializes an empty row vector.

---

### `#!cpp Rowvec(const int& length)`
**Constructor.**  
Initializes a row vector of zeros.
- `length`: Vector length.

---

### `#!cpp Rowvec(const std::vector<Real>& vector)`
**Constructor.**  
Initializes a row vector with data.
- `vector`: Data to initialize.

---

### `#!cpp ~Rowvec()`
**Destructor.**  
Cleans up the row vector.

---

### `#!cpp Real Sum() const`
Returns the sum of all elements.

---

### `#!cpp inline Real Mean() const`
Returns the mean value of the row vector.

---

### `#!cpp inline Real operator[](const int i) const`
Returns the value at index `i`.

---

### `#!cpp inline Real& operator[](const int i)`
Returns a reference to the value at index `i`.

---

### `#!cpp inline Rowvec operator=(const Matrix& matrix)`
Assigns a matrix to this row vector (must be a row vector).

---

## Internal Data Members

- Inherited from `Matrix`.

---

## Implementation Notes

- `Rowvec` is specialized for row vectors.
- Supports statistics and assignment from matrices.

## Example Usage

```cpp
#include "Common/Vec.h"
using namespace RAC::Common;

Rowvec r({1.0, 2.0, 3.0});
Real mean = r.Mean();
```
