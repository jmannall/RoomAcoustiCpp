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

// Eigen headers
#if MATRIX_LIBRARY == EIGEN_FLAG
#include <Eigen/Dense>
#include <Eigen/Geometry>
#endif

namespace RAC
{
	namespace Common
	{

#if MATRIX_LIBRARY == EIGEN_FLAG
		using Vec3 = Eigen::Vector<Real, 3>;
#elif MATRIX_LIBRARY == CUSTOM_FLAG
		/**
		* @class Class that stores 3D vector data
		*/
		class Vec3
		{
		public:

			/**
			* @brief Default constructor that initialises a zero vector
			*/
			Vec3() : mX(0.0), mY(0.0), mZ(0.0) {}

			/**
			* @brief Constructor that initialises a vector with specified values
			* 
			* @param mX The mX component of the vector
			* @param mY The mY component of the vector
			* @param mZ The mZ component of the vector
			*/
			Vec3(const Real mX, const Real mY, const Real mZ) : mX(mX), mY(mY), mZ(mZ) {}
#if DATA_TYPE_DOUBLE

			/**
			* @brief Constructor that initialises a double vector from floats
			*
			* @param mX The mX component of the vector
			* @param mY The mY component of the vector
			* @param mZ The mZ component of the vector
			*/
			Vec3(const float mX, const float mY, const float mZ) : mX(static_cast<Real>(mX)), mY(static_cast<Real>(mY)), mZ(static_cast<Real>(mZ)) {}
#else

			/**
			* @brief Constructor that initialises a float vector from doubles
			*
			* @param mX The mX component of the vector
			* @param mY The mY component of the vector
			* @param mZ The mZ component of the vector
			*/
			Vec3(const double mX, const double mY, const double mZ) : mX(static_cast<Real>(mX)), mY(static_cast<Real>(mY)), mZ(static_cast<Real>(mZ)) {}
#endif

			/**
			* @brief Default deconstructor
			*/
			~Vec3() {}

			inline Real x() const { return mX; }
			inline Real y() const { return mY; }
			inline Real z() const { return mZ; }

			inline Real& x() { return mX; }
			inline Real& y() { return mY; }
			inline Real& z() { return mZ; }

			/**
			* @return The length of the vector
			*/
			inline Real SquareNormal() const { return mX * mX + mY * mY + mZ * mZ; }

			/**
			* @return The length of the vector
			*/
			inline Real Normal() const { return std::sqrt(SquareNormal()); }

			/**
			* @brief Normalises the vector
			*/
			inline void Normalise()
			{ 
				if (mX == 0.0 && mY == 0.0 && mZ == 0.0)
					return;
				*this /= Normal();
			}

			/**
			* @brief Return the normalised vector
			*/
			inline Vec3 Normalised() const
			{
				if (mX == 0.0 && mY == 0.0 && mZ == 0.0)
					return Vec3();
				Real normal = Normal();
				return Vec3(mX / normal, mY / normal, mZ / normal);
			}

			inline Real Sum() { return mX + mY + mZ; }

			/**
			* @brief Rounds the vector values to decimal places based on ROUND_FACTOR
			*/
			/*inline void RoundVec()
			{
				mX = Round(mX);
				mY = Round(mY);
				mZ = Round(mZ);
			}*/

			/**
			* @brief Calculates the dot product of two vectors
			*/
			inline Real dot(const Vec3& v) const
			{
				return mX * v.x() + mY * v.y() + mZ * v.z();
			}

			/**
			* @brief Calculates the cross product of two vectors
			*/
			inline Vec3 cross(const Vec3& v) const
			{
				return Vec3(mY * v.z() - mZ * v.y(),
					mZ * v.x() - mX * v.z(),
					mX * v.y() - mY * v.x());
			}

			/**
			* @brief Adds a vector to the current vector
			*/
			inline Vec3& operator+=(const Vec3& v)
			{
				mX += v.x();
				mY += v.y();
				mZ += v.z();
				return *this;
			}

			/**
			* @brief Subtracts a vector from the current vector
			*/
			inline Vec3& operator-=(const Vec3& v)
			{
				mX -= v.x();
				mY -= v.y();
				mZ -= v.z();
				return *this;
			}

			/**
			* @brief Multiplies the vector by a given value
			*/
			inline Vec3& operator*=(const Real& a)
			{
				mX *= a;
				mY *= a;
				mZ *= a;
				return *this;
			}

			/**
			* @brief Divides the vector by a given value
			*/
			inline Vec3& operator/=(const Real& a)
			{
				*this *= ((Real)1.0 / a);
				return *this;
			}

			/**
			* @brief Assigns a class with mX, mY, mZ parameters to Vec3
			*/
			template <typename Vector3Type>
			inline Vec3& operator=(const Vector3Type& v)
			{
				mX = v.x();
				mY = v.y();
				mZ = v.z();
				return *this;
			}

			/**
			 * @brief For eigen compatibility
			 */
			inline Vec3& noalias() { return *this; }

		private:
			Real mX;		// X coordinate
			Real mY;		// Y coordinate
			Real mZ;		// Z coordinate
		};

		//////////////////// Vec3 operator overloads ////////////////////

		inline Vec3 operator+(const Vec3& u, const Vec3& v) { return Vec3(u.x() + v.x(), u.y() + v.y(), u.z() + v.z()); }
		inline Vec3 operator-(const Vec3& v) { return Vec3(-v.x(), -v.y(), -v.z()); }
		inline Vec3 operator-(const Vec3& u, const Vec3& v) { return -v + u; }
		inline Vec3 operator*(const Real a, const Vec3& v) { return Vec3(a * v.x(), a * v.y(), a * v.z()); }
		inline Vec3 operator*(const Vec3& v, const Real a) { return a * v; }
		inline Vec3 operator/(const Vec3& v, const Real a) { return Vec3(v.x() / a, v.y() / a, v.z() / a); }
		inline Vec3 operator/(const Real a, const Vec3& v) { return Vec3(a / v.x(), a / v.y(), a / v.z()); }

		/**
		* @brief prints a Vec3 using std::cout << vec3 << std::endl;
		*/ 
		inline std::ostream& operator<<(std::ostream& os, const Vec3& v)
		{
			os << "[ " << v.x() << " , " << v.y() << " , " << v.z() << " ]";
			return os;
		}

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs are equal, false otherwise
		*/
		inline bool operator==(const Vec3& u, const Vec3& v)
		{
			if (u.x() == v.x())
			{
				if (u.y() == v.y())
				{
					if (u.z() == v.z())
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
		/*inline Vec3 UnitVector(const Vec3& v)
		{
			if (v.x() == 0.0 && v.y() == 0.0 && v.z() == 0.0)
				return v;
			return v / v.Normal();
		}*/

		/**
		* @return The unit vector of a given vector rounded decimal places based on ROUND_FACTOR
		*/
		/*inline Vec3 UnitVectorRound(Vec3 v)
		{
			v.RoundVec();
			return UnitVector(v);
		}*/

		///**
		//* @brief Calculates the dot product of two vectors
		//*/
		//inline Real Dot(const Vec3& u, const Vec3& v)
		//{
		//	return u.x() * v.x() + u.y() * v.y() + u.z() * v.z();
		//}

		///**
		//* @brief Calculates the cross product of two vectors
		//*/
		//inline Vec3 Cross(const Vec3& u, const Vec3& v)
		//{
		//	return Vec3(u.y() * v.z() - u.z() * v.y(),
		//		u.z() * v.x() - u.x() * v.z(),
		//		u.x() * v.y() - u.y() * v.x());
		//}
#endif
		/**
		* @brief Rounds v to decimal places based on ROUND_FACTOR
		*/
		inline Vec3 Round(const Vec3& v)
		{
			return Vec3(Round(v.x()), Round(v.y()), Round(v.z()));
		}
	}
}
#endif // Common_Vec3_h