
EIGEN_DEVICE_FUNC inline RealScalar SquaredNormal() const { return this->squaredNorm(); }
EIGEN_DEVICE_FUNC inline RealScalar Normal() const { return this->norm(); }
EIGEN_DEVICE_FUNC inline const PlainObject Normalised() const { return this->normalized(); }
EIGEN_DEVICE_FUNC inline void Normalise() { this->normalize(); }

EIGEN_DEVICE_FUNC inline void Min(const Scalar& x) { this->array() = this->array().min(x); }
EIGEN_DEVICE_FUNC inline void Max(const Scalar& x) { this->array() = this->array().max(x); }
EIGEN_DEVICE_FUNC inline void Log10() { this->array() = this->array().log10(); }
EIGEN_DEVICE_FUNC inline void Pow10() {
    this->array() = (2.302585092994045684017991454684364207601101488628772976033328 * this->array()).exp();
}

EIGEN_DEVICE_FUNC inline const Inverse<Derived> InverseMatrix() const { return this->inverse(); }

EIGEN_DEVICE_FUNC inline Derived& operator-=(const Scalar& s) {
    this->array() -= s;
    return derived();
}

