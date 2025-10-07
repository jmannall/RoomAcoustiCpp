
// std::initializer_list<T> values
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE Array(const std::vector<Scalar>& values)
    : Eigen::Array<Scalar, RowsAtCompileTime, ColsAtCompileTime>(
        (RowsAtCompileTime == Eigen::Dynamic ? static_cast<Index>(values.size()) : RowsAtCompileTime),
        (ColsAtCompileTime == Eigen::Dynamic ? static_cast<Index>(values.size()) : ColsAtCompileTime))
{
    static_assert(ColsAtCompileTime == 1 || RowsAtCompileTime == 1,
        "This constructor is only for 1D vectors (Array<N,1> or Array<1,N>).");

    std::copy(values.begin(), values.end(), this->data());
}

EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE Array(const std::array<Scalar, RowsAtCompileTime == 1 ? ColsAtCompileTime : RowsAtCompileTime>& values)
    requires (RowsAtCompileTime != Eigen::Dynamic &&
            ColsAtCompileTime != Eigen::Dynamic &&
            (RowsAtCompileTime == 1 || ColsAtCompileTime == 1))
    : Eigen::Array<Scalar, RowsAtCompileTime, ColsAtCompileTime>(RowsAtCompileTime, ColsAtCompileTime)
{
    static_assert(ColsAtCompileTime != Eigen::Dynamic && RowsAtCompileTime != Eigen::Dynamic,
        "This constructor is only for fixed sized arrays.");

    static_assert(ColsAtCompileTime == 1 || RowsAtCompileTime == 1,
        "This constructor is only for 1D vectors (Array<N,1> or Array<1,N>).");

    std::copy(values.begin(), values.end(), this->data());
}

EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE Array(const Scalar& value)
    : Eigen::Array<Scalar, RowsAtCompileTime, ColsAtCompileTime>(RowsAtCompileTime, ColsAtCompileTime)
{
    static_assert(ColsAtCompileTime != Eigen::Dynamic && RowsAtCompileTime != Eigen::Dynamic,
        "This constructor is only for fixed sized arrays.");

	this->setConstant(value);
}