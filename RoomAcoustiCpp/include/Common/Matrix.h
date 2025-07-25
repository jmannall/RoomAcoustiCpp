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
#include "Common/Matrix_private.h"


namespace RAC
{
	namespace Common
	{
		extern template class Matrix<Real>;
		extern template class Matrix<ComplexPair>;
		extern template class Matrix<Complex>;
	}
}

#endif