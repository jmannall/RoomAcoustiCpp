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

// Eigen's Vec3 are slower than our native implementation for common operations, so use
// our custom one always
#define ALWAYS_USE_CUSTOM_VEC3				( 1 )

namespace RAC
{
	namespace Common
	{
#if MATRIX_LIBRARY == EIGEN_FLAG
		using EigenVec3 = Eigen::Vector<Real, 3>;
#endif

		/**
		* @class Class that stores 3D vector data
		*/
		class CustomVec3
		{
		public:

			/**
			* @brief Default constructor that initialises a zero vector
			*/
			CustomVec3() : mX(0.0), mY(0.0), mZ(0.0) {}

			/**
			* @brief Constructor that initialises a vector with specified values
			* 
			* @param mX The mX component of the vector
			* @param mY The mY component of the vector
			* @param mZ The mZ component of the vector
			*/
			CustomVec3(const Real mX, const Real mY, const Real mZ) : mX(mX), mY(mY), mZ(mZ) {}
#if DATA_TYPE_DOUBLE

			/**
			* @brief Constructor that initialises a double vector from floats
			*
			* @param mX The mX component of the vector
			* @param mY The mY component of the vector
			* @param mZ The mZ component of the vector
			*/
			CustomVec3(const float mX, const float mY, const float mZ) : mX(static_cast<Real>(mX)), mY(static_cast<Real>(mY)), mZ(static_cast<Real>(mZ)) {}
#else

			/**
			* @brief Constructor that initialises a float vector from doubles
			*
			* @param mX The mX component of the vector
			* @param mY The mY component of the vector
			* @param mZ The mZ component of the vector
			*/
			CustomVec3(const double mX, const double mY, const double mZ) : mX(static_cast<Real>(mX)), mY(static_cast<Real>(mY)), mZ(static_cast<Real>(mZ)) {}
#endif

#if MATRIX_LIBRARY == EIGEN_FLAG
			/**
			 * Converts an Eigen vector to this native vector
			 *
			 * @param eigenVector The eigen vector
			 */
			CustomVec3(const EigenVec3& eigenVector) : mX(eigenVector.x()), mY(eigenVector.y()), mZ(eigenVector.z()) {}
#endif

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
			inline CustomVec3 Normalised() const
			{
				if (mX == 0.0 && mY == 0.0 && mZ == 0.0)
					return CustomVec3();
				Real normal = Normal();
				return CustomVec3(mX / normal, mY / normal, mZ / normal);
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
			inline Real dot(const CustomVec3& v) const
			{
				return mX * v.mX + mY * v.mY + mZ * v.mZ;
			}

			/**
			* @brief Calculates the cross product of two vectors
			*/
			inline CustomVec3 cross(const CustomVec3& v) const
			{
				return CustomVec3(mY * v.mZ - mZ * v.mY,
					mZ * v.mX - mX * v.mZ,
					mX * v.mY - mY * v.mX);
			}

			/**
			* @brief Adds a vector to the current vector
			*/
			inline CustomVec3& operator+=(const CustomVec3& v)
			{
				mX += v.mX;
				mY += v.mY;
				mZ += v.mZ;
				return *this;
			}

			/**
			* @brief Subtracts a vector from the current vector
			*/
			inline CustomVec3& operator-=(const CustomVec3& v)
			{
				mX -= v.mX;
				mY -= v.mY;
				mZ -= v.mZ;
				return *this;
			}

			/**
			* @brief Multiplies the vector by a given value
			*/
			inline CustomVec3& operator*=(const Real& a)
			{
				mX *= a;
				mY *= a;
				mZ *= a;
				return *this;
			}

			/**
			* @brief Divides the vector by a given value
			*/
			inline CustomVec3& operator/=(const Real& a)
			{
				*this *= ((Real)1.0 / a);
				return *this;
			}

			/**
			* @brief Assigns a class with mX, mY, mZ parameters to CustomVec3
			*/
			template <typename Vector3Type>
			inline CustomVec3& operator=(const Vector3Type& v)
			{
				mX = v.mX;
				mY = v.mY;
				mZ = v.mZ;
				return *this;
			}

			/**
			 * @brief For eigen compatibility
			 */
			inline CustomVec3& noalias() { return *this; }

			//////////////////// CustomVec3 operator overloads ////////////////////

			friend inline CustomVec3 operator+(const CustomVec3& u, const CustomVec3& v) { return CustomVec3(u.mX + v.mX, u.mY + v.mY, u.mZ + v.mZ); }
			friend inline CustomVec3 operator-(const CustomVec3& v) { return CustomVec3(-v.mX, -v.mY, -v.mZ); }
			friend inline CustomVec3 operator-(const CustomVec3& u, const CustomVec3& v) { return -v + u; }
			friend inline CustomVec3 operator*(const Real a, const CustomVec3& v) { return CustomVec3(a * v.mX, a * v.mY, a * v.mZ); }
			friend inline CustomVec3 operator*(const CustomVec3& v, const Real a) { return a * v; }
			friend inline CustomVec3 operator/(const CustomVec3& v, const Real a) { return CustomVec3(v.mX / a, v.mY / a, v.mZ / a); }
			friend inline CustomVec3 operator/(const Real a, const CustomVec3& v) { return CustomVec3(a / v.mX, a / v.mY, a / v.mZ); }

			/**
			* @brief Performs an element-wise comparison
			* @return True if all element pairs are equal, false otherwise
			*/
			friend inline bool operator==(const CustomVec3& u, const CustomVec3& v) { return u.mX == v.mX && u.mY == v.mY && u.mZ == v.mZ;	}

			/**
			* @brief Performs an element-wise comparison
			* @return True if any element pairs are unequal, false otherwise
			*/
			friend inline bool operator!=(const CustomVec3& u, const CustomVec3& v)	{ return !(u==v); }

		private:
			Real mX;		// X coordinate
			Real mY;		// Y coordinate
			Real mZ;		// Z coordinate
		};


		/**
		* @brief prints a CustomVec3 using std::cout << vec3 << std::endl;
		*/ 
		inline std::ostream& operator<<(std::ostream& os, const CustomVec3& v)
		{
			os << "[ " << v.x() << " , " << v.y() << " , " << v.z() << " ]";
			return os;
		}

		//////////////////// CustomVec3 Functions ////////////////////

		/**
		* @return The unit vector of a given vector
		*/
		/*inline CustomVec3 UnitVector(const CustomVec3& v)
		{
			if (v.x() == 0.0 && v.y() == 0.0 && v.z() == 0.0)
				return v;
			return v / v.Normal();
		}*/

		/**
		* @return The unit vector of a given vector rounded decimal places based on ROUND_FACTOR
		*/
		/*inline CustomVec3 UnitVectorRound(CustomVec3 v)
		{
			v.RoundVec();
			return UnitVector(v);
		}*/

		///**
		//* @brief Calculates the dot product of two vectors
		//*/
		//inline Real Dot(const CustomVec3& u, const CustomVec3& v)
		//{
		//	return u.x() * v.x() + u.y() * v.y() + u.z() * v.z();
		//}

		///**
		//* @brief Calculates the cross product of two vectors
		//*/
		//inline CustomVec3 Cross(const CustomVec3& u, const CustomVec3& v)
		//{
		//	return CustomVec3(u.y() * v.z() - u.z() * v.y(),
		//		u.z() * v.x() - u.x() * v.z(),
		//		u.x() * v.y() - u.y() * v.x());
		//}

#if ALWAYS_USE_CUSTOM_VEC3
		// Eigen's Vec3 isn't fast for Vector 3's, so always use our custom one
		using Vec3 = CustomVec3;

		/**
		* @brief Rounds v to decimal places based on ROUND_FACTOR
		*/
		inline Vec3 Round(const Vec3& v)
		{
			return Vec3(Round(v.x()), Round(v.y()), Round(v.z()));
		}

#if MATRIX_LIBRARY == EIGEN_FLAG
		/**
		 * @brief Eigen's vector 3 are slower than our native one's, so we use the custom Vec3. In order to let the vectors interop,
		 *		  this function converts this to a matrix library compatible (Eigen) vector.
		 */
		inline EigenVec3 MakeCompatible(const Vec3 &vector) { return EigenVec3(vector.x(), vector.y(), vector.z()); }
#else
		/**
		 * @brief This is for compatibility with the Eigen version
		 */
		inline Vec3& MakeCompatible(Vec3& vector)
		{
			return vector;
		}

		/**
		  * @brief This is for compatibility with the Eigen version
		 */
		inline const Vec3& MakeCompatible(const Vec3& vector)
		{
			return vector;
		}
#endif // endif


#else

		use Vec3 = EigenVec3;

		/**
		 * @brief This is for compatibility with the mixed-format version
		 */
		inline Vec3& MakeCompatible(Vec3& vector)
		{
			return vector;
		}

		/**
		  * @brief This is for compatibility with the mixed-format version
		 */
		inline const Vec3& MakeCompatible(const Vec3& vector)
		{
			return vector;
		}
#endif

	}
}
#endif // Common_Vec3_h