/*
* @class vec4
*
* @brief Declaration of vec4 class
*
*/

#ifndef Common_vec4_h
#define Common_vec4_h

#include "Common/Types.h"

namespace RAC
{
	namespace Common
	{

		//////////////////// vec4 class ////////////////////

		class vec4
		{
		public:
			
			// Load and Destroy
			vec4() : w(0.0), x(0.0), y(0.0), z(0.0) {}
			vec4(const Real w_, const Real x_, const Real y_, const Real z_) : w(w_), x(x_), y(y_), z(z_) {}
#if DATA_TYPE_DOUBLE
			vec4(const float w_, const float x_, const float y_, const float z_) : w(static_cast<Real>(w_)), x(static_cast<Real>(x_)), y(static_cast<Real>(y_)), z(static_cast<Real>(z_)) {}
#else
			vec4(const double w_, const double x_, const double y_, const double z_) : w(static_cast<Real>(w_)), x(static_cast<Real>(x_)), y(static_cast<Real>(y_)), z(static_cast<Real>(z_)) {}
#endif
			~vec4() {}

			// Member variables
			Real w;
			Real x;
			Real y;
			Real z;

		private:
		};
	}
}

#endif