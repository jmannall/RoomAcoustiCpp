/*
* @class Vec, Rowvec
*
* @brief Declaration of Vec and Rowvec classes
*
*/

#ifndef Common_Vec_h
#define Common_Vec_h

// Common headers
#include "Common/Types.h"
#include "Common/Complex.h"
#include "Common/Vec_private.h"

namespace RAC
{
	namespace Common
	{
		extern template class Vec<Real>;
		extern template class Vec<ComplexPair>;
		extern template class Vec<Complex>;

		extern template class Rowvec<Real>;
		extern template class Rowvec<ComplexPair>;
		extern template class Rowvec<Complex>;
	}
}

#endif