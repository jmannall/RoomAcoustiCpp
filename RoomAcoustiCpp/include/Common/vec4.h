/*
* @class vec4
*
* @brief Declaration of vec4 class
*
*/

#ifndef Common_vec4_h
#define Common_vec4_h

// Common headers
#include "Common/Types.h"
#include "Common/vec3.h"

namespace RAC
{
	namespace Common
	{
		/**
		* @class Class that stores a quaternion data
		*/
		class Vec4
		{
		public:
			
			/**
			* @brief Default constructor that initialises a zero quaternion
			*/
			Vec4() : w(0.0), x(0.0), y(0.0), z(0.0) {}

			/**
			* @brief Constructor that initialises a quaternion with specified values
			*
			* @param w The w component of the quaternion
			* @param x The x component of the quaternion
			* @param y The y component of the quaternion
			* @param z The z component of the quaternion
			*/
			Vec4(const Real w, const Real x, const Real y, const Real z) : w(w), x(x), y(y), z(z) {}
#if DATA_TYPE_DOUBLE

			/**
			* @brief Constructor that initialises a double quaternion from floats
			*
			* @param w The w component of the quaternion
			* @param x The x component of the quaternion
			* @param y The y component of the quaternion
			* @param z The z component of the quaternion
			*/
			Vec4(const float w, const float x, const float y, const float z) : w(static_cast<Real>(w)), x(static_cast<Real>(x)), y(static_cast<Real>(y)), z(static_cast<Real>(z)) {}
#else

			/**
			* @brief Constructor that initialises a float quaternion from doubles
			*
			* @param w The w component of the quaternion
			* @param x The x component of the quaternion
			* @param y The y component of the quaternion
			* @param z The z component of the quaternion
			*/
			Vec4(const double w, const double x, const double y, const double z) : w(static_cast<Real>(w)), x(static_cast<Real>(x)), y(static_cast<Real>(y)), z(static_cast<Real>(z)) {}
#endif

			/**
			* @brief Constructor that initialises a quaternion from a Real and Vec3
			*
			* @param w The w component of the quaternion
			* @param vec The x, y, z components of the quaternion
			*/
			Vec4(const Real w, const Vec3 vec) : w(w), x(vec.x), y(vec.y), z(vec.z) {}

			/**
			* @brief Constructor that initialises a quaternion from a Vec3 with a zero w component
			*
			* @param vec The x, y, z components of the quaternion
			*/
			Vec4(const Vec3 vec) : w(0.0), x(vec.x), y(vec.y), z(vec.z) {}

			/**
			* @brief Default deconstructor
			*/
			~Vec4() {}

			/**
			* @return The forward vector of the quaternion
			*/
			inline Vec3 Forward() const
			{
				Vec3 forward;
				forward.x = 2.0 * (x * z + w * y);
				forward.y = 2.0 * (y * z - w * x);
				forward.z = 1.0 - 2.0 * (x * x + y * y);
				forward.Normalise();
				return forward;
			}

			/**
			* @return The square of the normal of the quaternion
			*/
			inline Real SquareNormal() const { return w * w + x * x + y * y + z * z; }

			/**
			* @return The inverse of the quaternion
			*/
			Vec4 Inverse() const
			{
				Real normSquared = SquareNormal();
				if (normSquared == 0.0f)
					return Vec4();

                return Conjugate() / normSquared;
			}

			/**
			* @return The conjugate of the quaternion
			*/
			inline Vec4 Conjugate() const { return Vec4(w, -x, -y, -z); }

			/**
			* @brief Rotates a vector by the quaternion
			* 
			* @param v The vector to rotate
			* 
			* @return The rotated vector
			*/
			Vec3 RotateVector(const Vec3& v) const
			{
				Vec4 rotatedVector = (*this) * Vec4(v) * Inverse();
				return Vec3(rotatedVector.x, rotatedVector.y, rotatedVector.z);
			}

			/**
			* @brief Performs a quaternion multiplication
			*/
			inline Vec4 operator*(const Vec4& v) const
			{
				return Vec4(w * v.w - x * v.x - y * v.y - z * v.z,
					w * v.x + x * v.w - y * v.z + z * v.y,
					w * v.y + x * v.z + y * v.w - z * v.x,
					w * v.z - x * v.y + y * v.x + z * v.w);
			}

			/**
			* @brief Divides the quaternioin by a given value 
			*/
			inline Vec4 operator/(const Real a) const
			{ 
				return Vec4(w / a, x / a, y / a, z / a);
			}

			/**
			* @brief Assigns a class with w, x, y, z parameters to Vec4
			*/
			template <typename CQuaternionType>
			inline Vec4& operator=(const CQuaternionType& q)
			{
				w = q.w;
				x = q.x;
				y = q.y;
				z = q.z;
				return *this;
			}

			Real w;		// W component of the quaternion
			Real x;		// X component of the quaternion
			Real y;		// Y component of the quaternion
			Real z;		// Z component of the quaternion

		private:
		};

		//////////////////// Vec4 operator overloads ////////////////////

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs are equal, false otherwise
		*/
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

		/**
		* @brief Performs an element-wise comparison
		* @return True if any element pairs are unequal, false otherwise
		*/
		inline bool operator!=(const Vec4& u, const Vec4& v)
		{
			if (u == v)
				return false;
			return true;
		}

		/**
		* @return The inverted w, x, y, z of a quaternion
		*/
		inline Vec4 operator-(const Vec4& v)
		{
			return Vec4(-v.w, -v.x, -v.y, -v.z);
		}
	}
}

#endif