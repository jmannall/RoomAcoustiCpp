
EIGEN_DEVICE_FUNC inline Index Length() const
{
	static_assert(ColsAtCompileTime == 1 || RowsAtCompileTime == 1,
		"The Length() function is only for 1D vectors (<N,1> or <1,N>).");
	return this->size();
}
EIGEN_DEVICE_FUNC inline Index Rows() const { return this->rows(); }
EIGEN_DEVICE_FUNC inline Index Cols() const { return this->cols(); }

EIGEN_DEVICE_FUNC inline void Resize(Index newSize)
{
	Index oldSize = this->size();
	this->conservativeResize(newSize);
	if (newSize > oldSize)
		this->segment(oldSize, newSize - oldSize).setZero();
}