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

			Vec3 Forward() const
			{
				Vec3 forward;
				forward.x = 2.0 * (x * z + w * y);
				forward.y = 2.0 * (y * z - w * x);
				forward.z = 1.0 - 2.0 * (x * x + y * y);
				forward.Normalise();
				return forward;
			}

			inline Real SqrNorm() const
			{
				return w * w + x * x + y * y + z * z;
			}

			inline const Vec4 Inverse() const
			{
				// Error handler:
				Real norm = SqrNorm();		// Not completely sure that we can use SqrNorm instead of Norm...
				if (norm == 0.0f)	// Computing inverse of quaternion with zero norm (returns ZERO quaternion)
					return Vec4(0.0, 0.0, 0.0, 0.0);

				//else		
				//	SET_RESULT(RESULT_OK, "Inverse of quaternion was computed succesfully");

				Real invNorm = 1.0f / norm;

				Real newW = w * invNorm;
				Real newX = -x * invNorm;
				Real newY = -y * invNorm;
				Real newZ = -z * invNorm;

				return Vec4(newW, newX, newY, newZ);
			}

			inline const Vec3 RotateVector(Vec3 vector) const
			{
				// Error handler:
				// Trust in Inverse for setting result

				// Convert vector into quaternion, forcing quaternion axis convention
				Vec4 vectorQuaternion = Vec4(vector);

				// Left product
				Vec4 leftProduct = *this * vectorQuaternion;

				// Right product
				Vec4 rightProduct = leftProduct * Inverse();

				// Convert result quaternion into vector
				Vec3 result = Vec3(rightProduct.x, rightProduct.y, rightProduct.z);

				return result;
			}

			// Quaternion product (not commutative). Use this for rotating!
			inline const Vec4 operator* (const Vec4 _rightHand) const
			{
				Real newW = w * _rightHand.w - x * _rightHand.x - y * _rightHand.y - z * _rightHand.z;
				Real newX = w * _rightHand.x + x * _rightHand.w + y * _rightHand.z - z * _rightHand.y;
				Real newY = w * _rightHand.y + y * _rightHand.w + z * _rightHand.x - x * _rightHand.z;
				Real newZ = w * _rightHand.z + z * _rightHand.w + x * _rightHand.y - y * _rightHand.x;
				return Vec4(newW, newX, newY, newZ);
			}

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