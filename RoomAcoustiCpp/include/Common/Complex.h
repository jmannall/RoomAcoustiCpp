/*
* @type Complex
*
* @brief Declaration of Complex type
*
*/

#ifndef Common_Complex_h
#define Common_Complex_h

// C++ headers
#include <complex>

// Common headers
#include <format>

#include "Common/Types.h"

namespace RAC
{
	namespace Common
	{
		typedef std::complex<Real> Complex;

#define RAC_ALIGN_COMPLEX		RAC_ALIGN(16)

#if DATA_TYPE_DOUBLE
		static const Complex imUnit(0.0, 1.0); // Imaginary unit
#else
		static const Complex imUnit(0.0f, 1.0f); // Imaginary Unit
#endif

		/**
		* @brief Multiplies a complex number by a real number
		*/
		inline Complex operator*(Complex a, Real b) { return a *= b; }
		inline Complex operator*(Real b, Complex a) { return a *= b; }
		inline Complex operator*(Complex a, Complex b) { return a *= b; }

		/**
		* @brief Adds a real number to a complex number
		*/
		inline Complex operator+(Complex a, Real b) { return a += b; }
		inline Complex operator+(Real b, Complex a) { return a += b; }
		inline Complex operator+(Complex a, Complex b) { return a += b; }

		/*inline Complex operator-(Complex a)
		{
			a.real = -a.real;
			a.imag = -a.imag;
			return a;
		}*/
		inline Complex operator-(Complex a, Real b) { return a += -b; }
		inline Complex operator-(Real b, Complex a) { return -a + b; }
		inline Complex operator-(Complex a, Complex b) { return a -= b; }

		/**
		* @brief Divides a complex number by a real number
		*/
		inline Complex operator/(Complex a, Real b) { return a *= ((Real)1.0 / b); }
		//inline Complex operator/(Complex a, Complex b)
		//{
		//	Real denominator = b.real * b.real + b.imag * b.imag;
		//	if (denominator == 0.0)
		//		return Complex(0.0, 0.0); // Avoid division by zero
		//	Real real = a.real * b.real + a.imag * b.imag;
		//	Real imag = a.imag * b.real - a.real * b.imag;
		//	return Complex(real, imag) / denominator;
		//}
	}
}

#endif