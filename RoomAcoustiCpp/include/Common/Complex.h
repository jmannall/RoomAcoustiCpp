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
#include "Common/Types.h"

namespace RAC
{
	namespace Common
	{
		/**
		* @brief Complex number type
		*/
		typedef std::complex<Real> Complex;

#if DATA_TYPE_DOUBLE
		static const Complex imUnit(0.0, 1.0); // Imaginary unit
#else
		static const Complex imUnit(0.0f, 1.0f); // Imaginary Unit
#endif

		/**
		* @brief Multiplies a complex number by a real number
		*/
		inline Complex operator*(const Complex a, const Real b) { return Complex(a.real() * b, a.imag() * b); }
		inline Complex operator*(const Real b, const Complex a) { return a * b; }

		/**
		* @brief Adds a real number to a complex number
		*/
		inline Complex operator+(const Complex a, const Real b) { return Complex(a.real() + b, a.imag()); }
		inline Complex operator+(const Real b, const Complex a) { return a + b; }

		/**
		* @brief Divides a complex number by a real number
		*/
		inline Complex operator/(const Complex a, const Real b) { return Complex(a.real() / b, a.imag() / b); }
	}
}

#endif