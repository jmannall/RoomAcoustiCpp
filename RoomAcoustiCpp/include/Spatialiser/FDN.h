/*
* @class FDN, FDNChannel
*
* @brief Declaration of FDN and FDNChannel classes
*
*/

#ifndef RoomAcoustiCpp_FDN_h
#define RoomAcoustiCpp_FDN_h

// C++ headers
#include <vector>
#include <mutex>
#include <cassert>

// Spatialiser headers
#include "Spatialiser/FDN_private.h"

// Common headers
#include "Common/Types.h"

namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{
		extern template class FDNChannel<Real>;
		extern template class FDNChannel<ComplexPair>;
		extern template class FDNChannel<Complex>;

		extern template class FDN<Real>;
		extern template class FDN<ComplexPair>;
		extern template class FDN<Complex>;

		extern template class RandomOrthogonalFDN<Real>;
		extern template class RandomOrthogonalFDN<Complex>;
		extern template class RandomOrthogonalFDN<ComplexPair>;
	}
}

#endif