Implements a column vector (`Vec`) class, inheriting from `Matrix`.  
Supports vector arithmetic, normalization, randomization, and statistics.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Vec.h`
- **Source:** `Common/Vec.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Matrix.h`

---

## Class Definition

```cpp
class Vec : public Matrix
{
public:
    Vec();
    Vec(const int& length);
    Vec(const std::vector<Real>& vector);
    ~Vec();

    void RandomNormalDistribution();
    void RandomUniformDistribution();
    void RandomUniformDistribution(Real a, Real b);
    void Normalise();
    Real CalculateNormal() const;
    void Max(const Real min);
    void Min(const Real max);
    Real Sum() const;
    inline Real Mean() const;
    inline Real operator[](const int i) const;
    inline Real& operator[](const int i);
    inline Vec operator=(const Matrix& matrix);

private:
    void Init(const std::vector<Real>& vector);
};
```

---

## Public Methods

### `#!cpp Vec()`
**Default constructor.**  
Initializes an empty column vector.

---

### `#!cpp Vec(const int& length)`
**Constructor.**  
Initializes a column vector of zeros.
- `length`: Vector length.

---

### `#!cpp Vec(const std::vector<Real>& vector)`
**Constructor.**  
Initializes a column vector with data.
- `vector`: Data to initialize.

---

### `#!cpp ~Vec()`
**Destructor.**  
Cleans up the vector.

---

### `#!cpp void RandomNormalDistribution()`
Fills the vector with random values from a normal distribution.

---

### `#!cpp void RandomUniformDistribution()`
Fills the vector with random values in [0, 1].

---

### `#!cpp void RandomUniformDistribution(Real a, Real b)`
Fills the vector with random values in [a, b].
- `a`: Lower bound.
- `b`: Upper bound.

---

### `#!cpp void Normalise()`
Normalizes the vector to unit length.

---

### `#!cpp Real CalculateNormal() const`
Returns the Euclidean norm (magnitude) of the vector.

---

### `#!cpp void Max(const Real min)`
Sets each element to at least `min`.

---

### `#!cpp void Min(const Real max)`
Sets each element to at most `max`.

---

### `#!cpp Real Sum() const`
Returns the sum of all elements.

---

### `#!cpp inline Real Mean() const`
Returns the mean value of the vector.

---

### `#!cpp inline Real operator[](const int i) const`
Returns the value at index `i`.

---

### `#!cpp inline Real& operator[](const int i)`
Returns a reference to the value at index `i`.

---

### `#!cpp inline Vec operator=(const Matrix& matrix)`
Assigns a matrix to this vector (must be a column vector).

---

## Internal Data Members

- Inherited from `Matrix`.

---

## Implementation Notes

- `Vec` is specialized for column vectors.
- Supports randomization, normalization, and statistics.

## Example Usage

```cpp
#include "Common/Vec.h"
using namespace RAC::Common;

Vec v(3);
v.RandomUniformDistribution();
v.Normalise();
Real sum = v.Sum();
```
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

- `Vec` and `Rowvec` are specialized for column and row vectors, respectively.
- Support randomization, normalization, and statistics.

## Example Usage

```cpp
#include "Common/Vec.h"
using namespace RAC::Common;

Vec v(3);
v.RandomUniformDistribution();
v.Normalise();
Real sum = v.Sum();
```
