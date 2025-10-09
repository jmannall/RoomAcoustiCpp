/*
* @class vec4
*
* @brief Declaration of vec4 class
*
*/

#ifndef Common_Vec4_h
#define Common_Vec4_h

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"

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
#if DATA_TYPE_DOUBLE
		using Vec4 = Eigen::Quaterniond;
#else
		using Vec4 = Eigen::Quaternionf;
#endif
		/**
		* @brief Rotates a vector by the quaternion
		* @details Assumes a normalised quaternion
		*
		* @param v The vector to rotate
		*
		* @return The rotated vector
		*/
		inline Vec3 RotateVector(const Vec3& v, const Vec4& orientation) { return orientation.Conjugate() * v; }

#elif MATRIX_LIBRARY == CUSTOM_FLAG

		/**
		* @class Class that stores a quaternion data
		*/
		class Vec4
		{
		public:
			
			/**
			* @brief Default constructor that initialises a zero quaternion
			*/
			Vec4() : mW(0.0), mVec(0.0, 0.0, 0.0) {}

			/**
			* @brief Constructor that initialises a quaternion with specified values
			*
			* @param w The w component of the quaternion
			* @param x The x component of the quaternion
			* @param y The y component of the quaternion
			* @param z The z component of the quaternion
			*/
			Vec4(const Real w, const Real x, const Real y, const Real z) : mW(w), mVec(x, y, z) {}
#if DATA_TYPE_DOUBLE

			/**
			* @brief Constructor that initialises a double quaternion from floats
			*
			* @param w The w component of the quaternion
			* @param x The x component of the quaternion
			* @param y The y component of the quaternion
			* @param z The z component of the quaternion
			*/
			Vec4(const float w, const float x, const float y, const float z) : mW(static_cast<Real>(w)), mVec(static_cast<Real>(x), static_cast<Real>(y), static_cast<Real>(z)) {}
#else

			/**
			* @brief Constructor that initialises a float quaternion from doubles
			*
			* @param w The w component of the quaternion
			* @param x The x component of the quaternion
			* @param y The y component of the quaternion
			* @param z The z component of the quaternion
			*/
			Vec4(const double w, const double x, const double y, const double z) : mW(static_cast<Real>(w)), mVec(static_cast<Real>(x), static_cast<Real>(y), static_cast<Real>(z)) {}
#endif

			/**
			* @brief Constructor that initialises a quaternion from a Real and Vec3
			*
			* @param w The w component of the quaternion
			* @param vec The x, y, z components of the quaternion
			*/
			Vec4(const Real w, const Vec3 vec) : mW(w), mVec(vec) {}

			/**
			* @brief Constructor that initialises a quaternion from a Vec3 with a zero w component
			*
			* @param vec The x, y, z components of the quaternion
			*/
			Vec4(const Vec3 vec) : Vec4(0.0, vec) {}

			/**
			* @brief Default deconstructor
			*/
			~Vec4() {}

			inline Real w() const { return mW; }
			inline Real x() const { return mVec.x(); }
			inline Real y() const { return mVec.y(); }
			inline Real z() const { return mVec.z(); }
			
			inline Real& w() { return mW; }
			inline Real& x() { return mVec.x(); }
			inline Real& y() { return mVec.y(); }
			inline Real& z() { return mVec.z(); }

			inline Vec3 vec() const { return mVec; }
			inline Vec3& vec() { return mVec; }

			/**
			* @return The square of the normal of the quaternion
			*/
			inline Real SquareNormal() const { return mW * mW + mVec.SquareNormal(); }

			/**
			* @return The length of the vector
			*/
			inline Real Normal() const { return std::sqrt(SquareNormal()); }

			/**
			* @brief Normalises the vector
			*/
			inline void Normalise()
			{
				if (mW == 0.0 && x() == 0.0 && y() == 0.0 && z() == 0.0)
					return;
				mW /= Normal();
				mVec /= Normal();
			}

			/**
			* @brief Return the normalised vector
			*/
			inline Vec4 Normalised() const
			{
				if (mW == 0.0 && x() == 0.0 && y() == 0.0 && z() == 0.0)
					return Vec4();
				Real normal = Normal();
				return Vec4(mW / normal, mVec / normal);
			}

			/**
			* @return The inverse of the quaternion
			*/
			Vec4 InverseMatrix() const
			{
				Real normSquared = SquareNormal();
				if (normSquared == 0.0f)
					return Vec4();
                return Conjugate() / normSquared;
			}

			/**
			* @return The conjugate of the quaternion
			*/
			inline Vec4 Conjugate() const { return Vec4(mW, -mVec); }

			inline Real dot(const Vec4& v) const
			{
				return mW * v.w() + x() * v.x() + y() * v.y() + z() * v.z();
			}

			/**
			* @brief Performs a quaternion multiplication
			*/
			inline Vec4 operator*(const Vec4& v) const
			{
				// Vec3 vecPart = mW * v.vec() + v.w() * mVec + mVec.cross(v.vec());
				return Vec4(mW * v.w() - mVec.dot(v.vec()),
					mW * v.vec() + v.w() * mVec + mVec.cross(v.vec()));

				/*return Vec4(mW * v.w() - x() * v.x() - y() * v.y() - z() * v.z(),
					mW * v.x() + x() * v.w() - y() * v.z() + z() * v.y(),
					mW * v.y() + x() * v.z() + y() * v.w() - z() * v.x(),
					mW * v.z() - x() * v.y() + y() * v.x() + z() * v.w());*/
			}

			/**
			* @brief Performs a quaternion multiplication
			*/
			// TODO: fix this (assumes a normalised quaternion)
			//inline Vec3 operator*(const Vec3& v) const
			//{
			//	// t = 2 * (q_vec x v)
			//	 Vec3 t = 2.0 * mVec.cross(v);

			//	Vec3 ret = v + mW * t + mVec.cross(t);
			//	return ret;
			//}

			/**
			* @brief Divides the quaternioin by a given value 
			*/
			inline Vec4 operator/(const Real a) const
			{ 
				return Vec4(mW / a, x() / a, y() / a, z() / a);
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

			Real mW;		// W component of the quaternion
			Vec3 mVec;		// Vector component of the quaternion
		private:
		};

		//////////////////// Vec4 operator overloads ////////////////////

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs are equal, false otherwise
		*/
		inline bool operator==(const Vec4& u, const Vec4& v)
		{
			if (u.w() == v.w())
			{
				if (u.vec() == v.vec())
					return true;
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
		* @brief Rotates a vector by the quaternion
		*
		* @param v The vector to rotate
		*
		* @return The rotated vector
		*/
		inline Vec3 RotateVector(const Vec3& v, const Vec4& orientation)
		{
			Vec4 rotatedVector = orientation.Conjugate() * Vec4(v) * orientation.Conjugate().InverseMatrix();
			return rotatedVector.vec();
		}
#endif
		/**
		* @return The inverted w, x, y, z of a quaternion
		*/
		inline Vec4 operator-(const Vec4& v) { return Vec4(-v.w(), -v.vec()); }

		/**
		* @return The forward vector of the quaternion
		*
		* @param quaternion Unit quaternion representing the orientation
		*/
		inline Vec3 Forward(const Vec4& q)
		{
			Real x = 2.0 * (q.x() * q.z() + q.w() * q.y());
			Real y = 2.0 * (q.y() * q.z() - q.w() * q.x());
			Real z = 1.0 - 2.0 * (q.x() * q.x() + q.y() * q.y());
			// TODO: Check if normalise is necessary
			return Vec3(x, y, z).Normalised();
			// (quaternion * Vec3::UnitZ()).normalized();
		}
	}
}
#endif // Common_Vec4_h