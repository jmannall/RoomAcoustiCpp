/*
*
*  \vec3 class
*
*/

#ifndef Common_Vec3_h
#define Common_Vec3_h

#include <iostream>
#include <cmath>;

#include "Common/Types.h"

namespace UIE
{
	namespace Common
	{

		//////////////////// vec3 class ////////////////////

		class vec3
		{
		public:

			// Load and Destroy
			vec3() : x(0.0), y(0.0), z(0.0) {}
			vec3(const Real x_, const Real y_, const Real z_) : x(x_), y(y_), z(z_) {}
			~vec3() {}

			Real Length() { return sqrt(x * x + y * y + z * z); }

			// Operators
			inline vec3 operator+=(const vec3& v)
			{
				this->x += v.x;
				this->y += v.y;
				this->z += v.z;
				return *this;
			}

			// Member variables
			Real x;
			Real y;
			Real z;

		private:
		};

		//////////////////// Operators ////////////////////

		inline bool operator==(const vec3& u, const vec3& v)
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

		inline bool operator!=(const vec3& u, const vec3& v)
		{
			if (u == v)
				return false;
			return true;
		}

		inline vec3 operator+(const vec3& u, const vec3& v)
		{
			return vec3(u.x + v.x, u.y + v.y, u.z + v.z);
		}

		inline vec3 operator-(const vec3& v)
		{
			return vec3(-v.x, -v.y, -v.z);
		}

		inline vec3 operator-(const vec3& u, const vec3& v)
		{
			return -v + u;
		}

		inline vec3 operator*(const Real a, const vec3& v)
		{
			return vec3(a * v.x, a * v.y, a * v.z);
		}

		inline vec3 operator*(const vec3& v, const Real a)
		{
			return a * v;
		}

		inline vec3 operator/(const vec3& v, const Real a)
		{
			return (1.0 / a) * v;
		}

		inline vec3 operator/(const Real a, const vec3& v)
		{
			return vec3(a / v.x, a / v.y, a / v.z);
		}

		// print the vec3 directly using std::cout << vec3 << std::endl;
		inline std::ostream& operator<<(std::ostream& os, const vec3& v)
		{
			os << "[ " << v.x << " , " << v.y << " , " << v.z << " ]";
			return os;
		}

		//////////////////// Functions ////////////////////

		inline vec3 UnitVector(vec3 v)
		{
			Real len = v.Length();
			if (len == 0.0)
			{
				return vec3(0.0, 0.0, 0.0);
			}
			return v / len;
		}

		inline Real Dot(vec3 v, vec3 u)
		{
			return v.x * u.x + v.y * u.y + v.z * u.z;
		}

		inline vec3 Cross(vec3 v, vec3 u)
		{
			return vec3(v.y * u.z - v.z * u.y,
				v.z * u.x - v.x * u.z,
				v.x * u.y - v.y * u.x);
		}
	}
}
#endif