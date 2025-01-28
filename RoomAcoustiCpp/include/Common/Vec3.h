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
		/**
		* @class Class that stores 3D vector data
		*/
		class Vec3
		{
		public:

			/**
			* @brief Default constructor that initialises a zero vector
			*/
			Vec3() : x(0.0), y(0.0), z(0.0) {}

			/**
			* @brief Constructor that initialises a vector with specified values
			* 
			* @param x The x component of the vector
			* @param y The y component of the vector
			* @param z The z component of the vector
			*/
			Vec3(const Real x, const Real y, const Real z) : x(x), y(y), z(z) {}
#if DATA_TYPE_DOUBLE

			/**
			* @brief Constructor that initialises a double vector from floats
			*
			* @param x The x component of the vector
			* @param y The y component of the vector
			* @param z The z component of the vector
			*/
			Vec3(const float x, const float y, const float z) : x(static_cast<Real>(x)), y(static_cast<Real>(y)), z(static_cast<Real>(z)) {}
#else

			/**
			* @brief Constructor that initialises a float vector from doubles
			*
			* @param x The x component of the vector
			* @param y The y component of the vector
			* @param z The z component of the vector
			*/
			Vec3(const double x, const double y, const double z) : x(static_cast<Real>(x)), y(static_cast<Real>(y)), z(static_cast<Real>(z)) {}
#endif

			/**
			* @brief Default deconstructor
			*/
			~Vec3() {}

			/**
			* @return The length of the vector
			*/
			inline Real Length() const { return sqrt(x * x + y * y + z * z); }

			/**
			* @brief Normalises the vector
			*/
			inline void Normalise()
			{ 
				if (x == 0.0 && y == 0.0 && z == 0.0)
					return;
				*this /= Length();
			}

			/**
			* @brief Rounds the vector values to decimal places based on ROUND_FACTOR
			*/
			inline void RoundVec()
			{
				x = Round(x);
				y = Round(y);
				z = Round(z);
			}

			/**
			* @brief Adds a vector to the current vector
			*/
			inline Vec3& operator+=(const Vec3& v)
			{
				x += v.x;
				y += v.y;
				z += v.z;
				return *this;
			}

			/**
			* @brief Subtracts a vector from the current vector
			*/
			inline Vec3& operator-=(const Vec3& v)
			{
				x -= v.x;
				y -= v.y;
				z -= v.z;
				return *this;
			}

			/**
			* @brief Multiplies the vector by a given value
			*/
			inline Vec3& operator*=(const Real& a)
			{
				x *= a;
				y *= a;
				z *= a;
				return *this;
			}

			/**
			* @brief Divides the vector by a given value
			*/
			inline Vec3& operator/=(const Real& a)
			{
				*this *= (1.0 / a);
				return *this;
			}

			/**
			* @brief Assigns a class with x, y, z parameters to Vec3
			*/
			template <typename Vector3Type>
			inline Vec3& operator=(const Vector3Type& v)
			{
				x = v.x;
				y = v.y;
				z = v.z;
				return *this;
			}

			Real x;		// X coordinate
			Real y;		// Y coordinate
			Real z;		// Z coordinate

		private:
		};

		//////////////////// Vec3 operator overloads ////////////////////

		inline Vec3 operator+(const Vec3& u, const Vec3& v) { return Vec3(u.x + v.x, u.y + v.y, u.z + v.z); }
		inline Vec3 operator-(const Vec3& v) { return Vec3(-v.x, -v.y, -v.z); }
		inline Vec3 operator-(const Vec3& u, const Vec3& v) { return -v + u; }
		inline Vec3 operator*(const Real a, const Vec3& v) { return Vec3(a * v.x, a * v.y, a * v.z); }
		inline Vec3 operator*(const Vec3& v, const Real a) { return a * v; }
		inline Vec3 operator/(const Vec3& v, const Real a) { return Vec3(v.x / a, v.y / a, v.z / a); }
		inline Vec3 operator/(const Real a, const Vec3& v) { return Vec3(a / v.x, a / v.y, a / v.z); }

		/**
		* @brief prints a Vec3 using std::cout << vec3 << std::endl;
		*/ 
		inline std::ostream& operator<<(std::ostream& os, const Vec3& v)
		{
			os << "[ " << v.x << " , " << v.y << " , " << v.z << " ]";
			return os;
		}

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs are equal, false otherwise
		*/
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

		/**
		* @brief Performs an element-wise comparison
		* @return True if any element pairs are unequal, false otherwise
		*/
		inline bool operator!=(const Vec3& u, const Vec3& v)
		{
			if (u == v)
				return false;
			return true;
		}

		//////////////////// Vec3 Functions ////////////////////

		/**
		* @return The unit vector of a given vector
		*/
		inline Vec3 UnitVector(const Vec3& v)
		{
			if (v.x == 0.0 && v.y == 0.0 && v.z == 0.0)
				return v;
			return v / v.Length();
		}

		/**
		* @return The unit vector of a given vector rounded decimal places based on ROUND_FACTOR
		*/
		inline Vec3 UnitVectorRound(Vec3 v)
		{
			v.RoundVec();
			return UnitVector(v);
		}

		/**
		* @brief Calculates the dot product of two vectors
		*/
		inline Real Dot(const Vec3& u, const Vec3& v)
		{
			return u.x * v.x + u.y * v.y + u.z * v.z;
		}

		/**
		* @brief Calculates the cross product of two vectors
		*/
		inline Vec3 Cross(const Vec3& u, const Vec3& v)
		{
			return Vec3(u.y * v.z - u.z * v.y,
				u.z * v.x - u.x * v.z,
				u.x * v.y - u.y * v.x);
		}
	}
}
#endif