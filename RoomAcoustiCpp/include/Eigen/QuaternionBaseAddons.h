
EIGEN_DEVICE_FUNC inline Scalar SquaredNormal() const { return this->squaredNorm(); }
EIGEN_DEVICE_FUNC inline Scalar Normal() const { return this->norm(); }
EIGEN_DEVICE_FUNC inline void Normalise() { this->normalize(); }
EIGEN_DEVICE_FUNC inline Quaternion<Scalar> Normalised() const { return this->normalized(); }

template <class OtherDerived>
EIGEN_DEVICE_FUNC inline Scalar Dot(const QuaternionBase<OtherDerived>& other) const { return this->dot(other); }

EIGEN_DEVICE_FUNC Quaternion<Scalar> InverseMatrix() const { return this->inverse(); }
EIGEN_DEVICE_FUNC Quaternion<Scalar> Conjugate() const { return this->conjugate(); }