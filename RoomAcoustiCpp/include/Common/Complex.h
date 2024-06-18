/*
* @class Complex
*
* @brief Declaration of Complex class
*
*/

#ifndef Common_Complex_h
#define Common_Complex_h

#include <complex>

#include "Common/Types.h"

namespace RAC
{
	namespace Common
	{
		//////////////////// Data Type ////////////////////

		typedef std::complex<Real> Complex;

#if DATA_TYPE_DOUBLE
		static const Complex imUnit(0.0, 1.0);
#else
		static const Complex imUnit(0.0f, 1.0f);
#endif

		//////////////////// Functions ////////////////////

		inline Complex operator*(const Complex a, const Real b)
		{
			return Complex(a.real() * b, a.imag() * b);
		}

		inline Complex operator*(const Real b, const Complex a)
		{
			return a * b;
		}

		inline Complex operator+(const Complex a, const Real b)
		{
			return Complex(a.real() + b, a.imag());
		}

		inline Complex operator+(const Real b, const Complex a)
		{
			return a + b;
		}

		inline Complex operator/(const Complex a, const Real b)
		{
			return Complex(a.real() / b, a.imag() / b);
		}
	}
}

#endif