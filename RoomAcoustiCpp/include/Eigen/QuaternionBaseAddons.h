

EIGEN_DEVICE_FUNC inline Scalar SquaredNormal() const { return squaredNorm(); }
EIGEN_DEVICE_FUNC inline Scalar Normal() const { return norm(); }
EIGEN_DEVICE_FUNC inline void Normalise() { normalize(); }
EIGEN_DEVICE_FUNC inline Quaternion<Scalar> Normalised() const { return normalized(); }

template <class OtherDerived>
EIGEN_DEVICE_FUNC inline Scalar Dot(const QuaternionBase<OtherDerived>& other) const { return dot(other); }

EIGEN_DEVICE_FUNC Quaternion<Scalar> Inverse() const { return inverse(); }
EIGEN_DEVICE_FUNC Quaternion<Scalar> Conjugate() const { return conjugate(); }