

EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE Matrix(const std::vector<Scalar>& values)
    : Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime>(
        (RowsAtCompileTime == Eigen::Dynamic ? static_cast<Index>(values.size()) : 1),
        (ColsAtCompileTime == Eigen::Dynamic ? static_cast<Index>(values.size()) : 1))
{
    static_assert(ColsAtCompileTime == 1 || RowsAtCompileTime == 1,
        "This constructor is only for 1D vectors (Matrix<N,1> or Matrix<1,N>).");

    std::copy(values.begin(), values.end(), this->data());
}


    