/*
* @class vec3
*
* @brief Declaration of vec3 class
*
*/

#ifndef Common_Vec3_h
#define Common_Vec3_h

// C++ headers
#include <iostream>
#include <cmath>
#include <algorithm>

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"

namespace RAC
{
	namespace Common
	{

		//////////////////// vec3 class ////////////////////

		class Vec3
		{
		public:

			// Load and Destroy
			Vec3() : x(0.0), y(0.0), z(0.0) {}
			Vec3(const Real x_, const Real y_, const Real z_) : x(x_), y(y_), z(z_) {}
#if DATA_TYPE_DOUBLE
			Vec3(const float x_, const float y_, const float z_) : x(static_cast<Real>(x_)), y(static_cast<Real>(y_)), z(static_cast<Real>(z_)) {}
#else
			Vec3(const double x_, const double y_, const double z_) : x(static_cast<Real>(x_)), y(static_cast<Real>(y_)), z(static_cast<Real>(z_)) {}
#endif
			~Vec3() {}

			Real Length() const { return sqrt(x * x + y * y + z * z); }

			inline void Normalise()
			{ 
				if (x == 0.0 && y == 0.0 && z == 0.0)
					return;
				*this /= Length();
			}

			inline void RoundVec()
			{
				x = Round(x);
				y = Round(y);
				z = Round(z);
			}

			inline Vec3 Min(const Vec3& v)
			{
				this->x = std::min(this->x, v.x);
				this->y = std::min(this->y, v.y);
				this->z = std::min(this->z, v.z);
				return *this;
			}

			inline Vec3 Max(const Vec3& v)
			{
				this->x = std::max(this->x, v.x);
				this->y = std::max(this->y, v.y);
				this->z = std::max(this->z, v.z);
				return *this;
			}

			inline const Real GetElevationRadians() const
			{
				// Error handler:
				float distance = Length();
				if (distance == 0.0f) // Distance from source to listener is zero
					return 0.0f;

				// 0=front; 90=up; -90=down
				//float cosAngle = *upAxis / GetDistance(); // Error check: division by zero
				//float angle = SafeAcos(cosAngle);
				//return (M_PI / 2.0f) - angle;

				// 0=front; 90=up; 270=down (LISTEN)
				Real cosAngle = y / distance;
				Real angle = SafeAcos(cosAngle);
				Real adjustedAngle = (PI_1 * 2.5) - angle;

				// Check limits (always return 0 instead of 2PI)
				if (adjustedAngle >= PI_2)
					adjustedAngle = std::fmod(adjustedAngle, PI_2);

				return adjustedAngle;
			}

			// Get azimuth in radians, according to the selected axis convention. Currently uses LISTEN database convention for azimuth angles: anti-clockwise full circle starting with 0º in front.
			inline const Real GetAzimuthRadians() const
			{
				// Error handler:
				Real rightAxis = x;
				Real forwardAxis = z;
				if ((rightAxis == 0.0f) && (forwardAxis == 0.0f)) // Azimuth cannot be computed for a(0, 0, z) vector. 0.0 is returned
					return 0.0f;

				// front=0; left=-90; right=90
				//return atan2(*rightAxis, *forwardAxis);		

				//front=0; left=90; right=270 (LISTEN)
				Real angle = std::atan2(rightAxis, forwardAxis);
				Real adjustedAngle = std::fmod((PI_2 - angle), PI_2);

				// Check limits (always return 0 instead of 2PI)
				if (adjustedAngle >= PI_2)
					adjustedAngle = std::fmod(adjustedAngle, PI_2);

				return adjustedAngle;
			}

			// Operators
			inline Vec3& operator+=(const Vec3& v)
			{
				this->x += v.x;
				this->y += v.y;
				this->z += v.z;
				return *this;
			}

			inline Vec3& operator-=(const Vec3& v)
			{
				this->x -= v.x;
				this->y -= v.y;
				this->z -= v.z;
				return *this;
			}

			inline Vec3& operator*=(const Real& a)
			{
				this->x *= a;
				this->y *= a;
				this->z *= a;
				return *this;
			}

			inline Vec3& operator/=(const Real& a)
			{
				*this *= (1.0 / a);
				return *this;
			}

			template <typename CVector3Type>
			inline Vec3& operator=(const CVector3Type& v)
			{
				this->x = v.x;
				this->y = v.y;
				this->z = v.z;
				return *this;
			}

			// Member variables
			Real x;
			Real y;
			Real z;

		private:
		};

		//////////////////// Operators ////////////////////

		inline bool operator==(const Vec3& u, const Vec3& v)
		{
			if (u.x == v.x)
			{
				if (u.y == v.y)
				{
					if (u.z == v.z)
						return true;
				}
			}
			return false;
		}

		inline bool operator!=(const Vec3& u, const Vec3& v)
		{
			if (u == v)
				return false;
			return true;
		}

		inline Vec3 operator+(const Vec3& u, const Vec3& v)
		{
			return Vec3(u.x + v.x, u.y + v.y, u.z + v.z);
		}

		inline Vec3 operator-(const Vec3& v)
		{
			return Vec3(-v.x, -v.y, -v.z);
		}

		inline Vec3 operator-(const Vec3& u, const Vec3& v)
		{
			return -v + u;
		}

		inline Vec3 operator*(const Real a, const Vec3& v)
		{
			return Vec3(a * v.x, a * v.y, a * v.z);
		}

		inline Vec3 operator*(const Vec3& v, const Real a)
		{
			return a * v;
		}

		inline Vec3 operator/(const Vec3& v, const Real a)
		{
			// return (1.0 / a) * v;
			return Vec3(v.x / a, v.y / a, v.z / a);
		}

		inline Vec3 operator/(const Real a, const Vec3& v)
		{
			return Vec3(a / v.x, a / v.y, a / v.z);
		}

		// print the vec3 directly using std::cout << vec3 << std::endl;
		inline std::ostream& operator<<(std::ostream& os, const Vec3& v)
		{
			os << "[ " << v.x << " , " << v.y << " , " << v.z << " ]";
			return os;
		}

		//////////////////// Functions ////////////////////

		inline Vec3 UnitVector(const Vec3& v)
		{
			if (v.x == 0.0 && v.y == 0.0 && v.z == 0.0)
				return v;
			return v / v.Length();
		}

		inline Vec3 UnitVectorRound(Vec3 v)
		{
			v.RoundVec();
			return UnitVector(v);
		}

		inline Real Dot(const Vec3& u, const Vec3& v)
		{
			return u.x * v.x + u.y * v.y + u.z * v.z;
		}

		inline Vec3 Cross(const Vec3& u, const Vec3& v)
		{
			return Vec3(u.y * v.z - u.z * v.y,
				u.z * v.x - u.x * v.z,
				u.x * v.y - u.y * v.x);
		}
	}
}
#endif