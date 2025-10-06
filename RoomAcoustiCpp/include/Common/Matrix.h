/*
* @class Matrix
*
* @brief Declaration of matrix class
*
*/

#ifndef Common_Matrix_h
#define Common_Matrix_h

// C++ headers
#include <cassert>
#include <vector>

// Common headers
#include "Common/Types.h"
#include "Common/Complex.h"
#if MATRIX_LIBRARY == CUSTOM_FLAG
#include "Common/Matrix_private.h"
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
		using Matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
#elif MATRIX_LIBRARY == CUSTOM_FLAG
		extern template class Matrix<int>;
		extern template class Matrix<Real>;
		extern template class Matrix<Complex>;
#endif
		
	}
}

#endif