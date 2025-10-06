

EIGEN_DEVICE_FUNC inline Scalar Sum() const { return sum(); }
EIGEN_DEVICE_FUNC Scalar Mean() const { return mean(); }
EIGEN_DEVICE_FUNC Scalar Trace() const { return trace(); }
EIGEN_DEVICE_FUNC Scalar Product() const { return prod(); }

template <int NaNPropagation>
EIGEN_DEVICE_FUNC typename internal::traits<Derived>::Scalar Min() const { return minCoeff(); }
template <int NaNPropagation>
EIGEN_DEVICE_FUNC typename internal::traits<Derived>::Scalar Max() const { return maxCoeff(); }

typedef Transpose<Derived> TransposeReturnType;
EIGEN_DEVICE_FUNC TransposeReturnType Transposed() { return transpose(); }
typedef Transpose<const Derived> ConstTransposeReturnType;
EIGEN_DEVICE_FUNC const ConstTransposeReturnType Transposed() const { return transpose(); }
EIGEN_DEVICE_FUNC void Transpose() { transposeInPlace(); }

EIGEN_DEVICE_FUNC Derived& SetConstant(const Scalar& value) { return setConstant(value); }
EIGEN_DEVICE_FUNC Derived& Reset() { return setZero(); }
EIGEN_DEVICE_FUNC Derived& SetOnes() { return setOnes(); }
EIGEN_DEVICE_FUNC Derived& RandomUniformDistribution() { return setRandom(); }

EIGEN_DEVICE_FUNC inline RowXpr Row(Index i) { return row(i); }
EIGEN_DEVICE_FUNC inline ConstRowXpr Row(Index i) const { return row(i); }
EIGEN_DEVICE_FUNC inline ColXpr Col(Index i) { return col(i); }
EIGEN_DEVICE_FUNC inline ConstColXpr Col(Index i) const { return col(i); }