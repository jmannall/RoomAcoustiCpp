/*
*
*  \vec4 class
*
*/

#ifndef Common_vec4_h
#define Common_vec4_h

#include "Common/Types.h"

namespace UIE
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