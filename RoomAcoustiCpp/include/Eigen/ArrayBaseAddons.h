
EIGEN_DEVICE_FUNC inline Index Length() const { return rows(); }

EIGEN_DEVICE_FUNC inline Index Rows() const { return rows(); }
EIGEN_DEVICE_FUNC inline Index Cols() const { return cols(); }

EIGEN_DEVICE_FUNC inline void Min(const Scalar& x) { min(x); }
EIGEN_DEVICE_FUNC inline void Max(const Scalar& x) { max(x); }

EIGEN_DEVICE_FUNC inline const Log10ReturnType Log10() const { return log10(); }
EIGEN_DEVICE_FUNC inline const LogReturnType Log() const { return log(); }
EIGEN_DEVICE_FUNC inline const SqrtReturnType Sqrt() const { return sqrt(); }
EIGEN_DEVICE_FUNC inline const AbsReturnType Abs() const { return abs(); }
EIGEN_DEVICE_FUNC inline const SquareReturnType Square() const { return square(); }
EIGEN_DEVICE_FUNC inline const CosReturnType Cos() const { return cos(); }
EIGEN_DEVICE_FUNC inline const SinReturnType Sin() const { return sin(); }

template <typename ScalarExponent>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const UnaryPowReturnType<ScalarExponent> Pow(
    const ScalarExponent& exponent) const {
    return pow(exponent);
}

EIGEN_DEVICE_FUNC inline auto Pow10() const
{
	// Log10: 2.302585092994045684017991454684364207601101488628772976033328
    return (this->derived() * Scalar(2.302585092994045684017991454684364207601101488628772976033328)).exp();
}
