#pragma once

#include <complex>

using complexF = std::complex<float>;

const complexF imUnit(0.0f, 1.0f);

inline complexF operator*(const complexF& a, const float& b)
{
	return complexF(a.real() * b, a.imag() * b);
}

inline complexF operator*(const float& b, const complexF& a)
{
	return a * b;
}

inline complexF operator+(const complexF& a, const float& b)
{
	return complexF(a.real() + b, a.imag());
}

inline complexF operator+(const float& b, const complexF& a)
{
	return a + b;
}

inline complexF operator/(const complexF& a, const float& b)
{
	return complexF(a.real() / b, a.imag() / b);
}