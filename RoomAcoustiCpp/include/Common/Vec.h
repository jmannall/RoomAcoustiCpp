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
			assert(u.Length() == v.Length());
			assert(u.Length() == w.Length());

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
	}
}

#endif