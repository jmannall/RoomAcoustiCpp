/*
*
*  \Complex variable
*
*/

#ifndef Spatialiser_Complex_h
#define Spatialiser_Complex_h

#include <complex>

#include "Spatialiser/Types.h"

namespace Spatialiser
{
	// Complex Type
	typedef std::complex<Real> Complex;

	static const Complex imUnit(0.0f, 1.0);

	inline Complex operator*(const Complex& a, const Real& b)
	{
		return Complex(a.real() * b, a.imag() * b);
	}

	inline Complex operator*(const Real& b, const Complex& a)
	{
		return a * b;
	}

	inline Complex operator+(const Complex& a, const Real& b)
	{
		return Complex(a.real() + b, a.imag());
	}

	inline Complex operator+(const Real& b, const Complex& a)
	{
		return a + b;
	}

	inline Complex operator/(const Complex& a, const Real& b)
	{
		return Complex(a.real() / b, a.imag() / b);
	}
}

#endif