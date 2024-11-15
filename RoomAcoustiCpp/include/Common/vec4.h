/*
* @class vec4
*
* @brief Declaration of vec4 class
*
*/

#ifndef Common_vec4_h
#define Common_vec4_h

#include "Common/Types.h"
#include "Common/vec3.h"

#include "3dti_Toolkit/Common/Transform.h"

namespace RAC
{
	namespace Common
	{

		//////////////////// vec4 class ////////////////////

		class Vec4
		{
		public:
			
			// Load and Destroy
			Vec4() : w(0.0), x(0.0), y(0.0), z(0.0) {}
			Vec4(const Real w_, const Real x_, const Real y_, const Real z_) : w(w_), x(x_), y(y_), z(z_) {}
#if DATA_TYPE_DOUBLE
			Vec4(const float w_, const float x_, const float y_, const float z_) : w(static_cast<Real>(w_)), x(static_cast<Real>(x_)), y(static_cast<Real>(y_)), z(static_cast<Real>(z_)) {}
#else
			Vec4(const double w_, const double x_, const double y_, const double z_) : w(static_cast<Real>(w_)), x(static_cast<Real>(x_)), y(static_cast<Real>(y_)), z(static_cast<Real>(z_)) {}
#endif
			Vec4(const Real w_, const Vec3 vec) : w(w_), x(vec.x), y(vec.y), z(vec.z) {}

			Vec4(const Vec3 vec) : w(0.0), x(vec.x), y(vec.y), z(vec.z) {}
			~Vec4() {}

			inline Real SquareNormal() const { return w * w + x * x + y * y + z * z; }

			template <typename CQuaternionType>
			inline Vec4& operator=(const CQuaternionType& q)
			{
				this->w = q.w;
				this->x = q.x;
				this->y = q.y;
				this->z = q.z;
				return *this;
			}

			// Member variables
			Real w;
			Real x;
			Real y;
			Real z;

		private:
		};

		//////////////////// Operators ////////////////////

		inline bool operator==(const Vec4& u, const Vec4& v)
		{
			if (u.w == v.w)
			{
				if (u.x == v.x)
				{
					if (u.y == v.y)
					{
						if (u.z == v.z)
							return true;
					}
				}
			}
			return false;
		}

		inline bool operator!=(const Vec4& u, const Vec4& v)
		{
			if (u == v)
				return false;
			return true;
		}

		inline Vec4 operator-(const Vec4& v)
		{
			return Vec4(-v.w, -v.x, -v.y, -v.z);
		}
	}
}

#endif