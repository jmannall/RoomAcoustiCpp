/*
* @class Vec, Rowvec
*
* @brief Declaration of Vec and Rowvec classes
*
*/

#ifndef Common_Vec_h
#define Common_Vec_h

// C++ headers
#include <assert.h>

// Common headers
#include "Common/Types.h"
#include "Common/Complex.h"
#include "Common/Definitions.h"
#if MATRIX_LIBRARY == CUSTOM_FLAG
#include "Common/Vec_private.h"
#endif

// Eigen headers
#if MATRIX_LIBRARY == EIGEN_FLAG
#include <Eigen/Dense>
#endif

namespace RAC
{
	namespace Common
	{

#if MATRIX_LIBRARY == EIGEN_FLAG
		template<typename T = Real>
		using Vec = Eigen::Vector<T, Eigen::Dynamic>;

		template<typename T = Real>
		using Rowvec = Eigen::RowVector<T, Eigen::Dynamic>;

		inline Real ThreeWayDot(const Vec<>& u, const Vec<>& v, const Vec<>& w)
		{
			assert(u.Rows() == v.Rows());
			assert(u.Rows() == w.Rows());

			return u.dot(v.cwiseProduct(w));
		}

#elif MATRIX_LIBRARY == CUSTOM_FLAG
		extern template class Vec<int>;
		extern template class Vec<Real>;
		extern template class Vec<Complex>;

		extern template class Rowvec<int>;
		extern template class Rowvec<Real>;
		extern template class Rowvec<Complex>;
#endif

		/**
		 * @brief Get a set of prime numbers (returns -1 if the requested range exceeds the known primes)
		 *
		 * @param minIndex Index (within the list of primes) of the first prime to include
		 * @param length The number of primes to return
		 * @param stride Step between subsequent primes in the resulting set (1 means adjacent primes)
		 * @return Vector of primes
		 */
		inline Vec<int> GetSetOfPrimes(int minIndex, int length, int stride)
		{
			Vec<int> result = Vec<int>::Constant(length, -1);

			// TODO: Throw an exception in either case.
			if (stride == 0)
				return result;
			if (minIndex < 0 || minIndex >= sizeof(PRIMES))
				return result;

			int i = minIndex;
			for (int j = 0; j < length; ++j)
			{
				result(j) = PRIMES[i];
				i += stride;
				if (i < 0 || i >= sizeof(PRIMES))
					break;
			}
			return result;
		}
	}
}

#endif