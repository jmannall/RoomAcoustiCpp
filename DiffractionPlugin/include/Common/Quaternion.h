/*
*
*  \quaternion class
*
*/

#ifndef Common_Quaternion_h
#define Common_Quaternion_h

#include "Common/Types.h"

namespace UIE
{
	namespace Common
	{

		//////////////////// quaternion class ////////////////////

		class quaternion
		{
		public:

			// Load and Destroy
			quaternion() : w(0.0), x(0.0), y(0.0), z(0.0) {}
			quaternion(const Real w_, const Real x_, const Real y_, const Real z_) : w(w_), x(x_), y(y_), z(z_) {}
			~quaternion() {}

			// Member variable
			Real w;
			Real x;
			Real y;
			Real z;

		private:
		};
	}
}

#endif