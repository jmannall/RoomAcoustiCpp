/*
* @class ComplexFDN
*
* @brief Declaration of ComplexFDN class
*
*/

#ifndef RoomAcoustiCpp_ComplexFDN_h
#define RoomAcoustiCpp_ComplexFDN_h

// Common headers
#include "Common/Complex.h"
#include "Common/RACProfiler.h"

//DSP headers
#include "DSP/Interpolate.h"

// Spatialiser headers
#include "Spatialiser/FDN.h"

namespace RAC
{
	namespace Spatialiser
	{
		class MODART
		{
		public:
			MODART(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<Config> config) : fdn(T60, dimensions, config)
			{}

			void ProcessAudio(const Matrix<Complex>& data, std::vector<Buffer<Complex>>& outputBuffers, const Real lerpFactor)
			{
				fdn.ProcessAudio(data, outputBuffers, lerpFactor);
			}

		private:

			FDN<Complex> fdn;
		};
	}
}


#endif  // RoomAcoustiCpp_ComplexFDN_h