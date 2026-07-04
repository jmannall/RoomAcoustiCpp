/*
* @class Coefficients, Absorption
*
* @brief Declaration of Coefficient and Absorption classes
*
*/

#ifndef Common_Coefficients_h
#define Common_Coefficients_h

// C++ headers
#include <assert.h>
#include <vector>
#include <array>
#include <cmath>
#include <iostream>

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"

// Eigen headers
#if MATRIX_LIBRARY == EIGEN_FLAG
#include <Eigen/Dense>
#endif

namespace RAC
{
	namespace Common
	{
#if MATRIX_LIBRARY == EIGEN_FLAG
		template<typename T = Real, int Size = Eigen::Dynamic>
		using Coefficients = Eigen::Array<T, Size, 1>;
#elif MATRIX_LIBRARY == CUSTOM_FLAG
		/**
		* @brief Class that stores abitrary coefficients
		*/
		template <typename T = Real, size_t Size = 0>
		class Coefficients
		{
			using Container = std::conditional_t<
				Size == 0,
				std::vector<T>,
				std::array<T, Size>>;
		public:

			Coefficients() : mCoefficients() {}

			/**
			* @brief Constructor that initialises the Coefficients with a value
			*
			* @param in The value for the coefficients
			* @details Used for std::array (predetermined size)
			*/
			Coefficients(const T in) requires (Size != 0) { mCoefficients.fill(in); }

			/**
			* @brief Constructor that initialises the Coefficients with zeros
			*
			* @param len The number of coefficients
			*/
			Coefficients(const int len) requires (Size == 0) : Coefficients(std::vector<T>(len, 0.0)) {}

			/**
			* @brief Constructor that initialises the Coefficients from a std::vector
			*
			* @param coefficients The vector of coefficients
			*/
			Coefficients(const std::vector<T>& coefficients) requires (Size == 0) : mCoefficients(coefficients) {}

			/**
			* @brief Constructor that initialises the Coefficients from a std::array
			*
			* @param coefficients The array of coefficients
			*/
			Coefficients(const std::array<T, Size>& coefficients) requires (Size != 0) : mCoefficients(coefficients) {}

			/**
			* @brief Default deconstructor
			*/
			~Coefficients() {};

			// Static factory: zeros
			static Coefficients Zero(const int length) requires (Size == 0) { return Coefficients(length); }

			static Coefficients Zero() requires (Size != 0) { return Coefficients(0.0); }

			// Static factory: constant
			static Coefficients Constant(const int length, const T value) requires (Size == 0) { return Coefficients(length, value); }

			static Coefficients Constant(const T value) requires (Size != 0) { return Coefficients(value); }

			/**
			* @brief Sets all coefficient entries to a single value
			*
			* @param x The new value
			*/
			inline void Reset()
			{
				std::fill(mCoefficients.begin(), mCoefficients.end(), 0.0);
			}

			/**
			* @brief Sets all coefficient entries to a single value
			*
			* @param x The new value
			*/
			inline void SetConstant(const T value)
			{
				std::fill(mCoefficients.begin(), mCoefficients.end(), value);
			}

			/**
			* @brief Returns the number of coefficients
			*
			* @return The number of coefficients
			*/
			inline int Length() const { return static_cast<int>(mCoefficients.size()); }

			/**
			* @brief Calculates the natural (base e) logarithm of the coefficients
			*/
			inline Coefficients Log() const
			{
				Coefficients<T, Size> result(*this);
				for (int i = 0; i < mCoefficients.size(); i++)
					result[i] = std::log(mCoefficients[i]);
				return result;
			}

			/**
			* @brief Calculates 10 raised to the power of the coefficients
			*/
			inline Coefficients Pow(Real exponent) const
			{
				Coefficients<T, Size> result(*this);
				for (int i = 0; i < mCoefficients.size(); i++)
					result[i] = std::pow(mCoefficients[i], exponent);
				return result;
			}

			/**
			* @brief Calculates 10 raised to the power of the coefficients
			*/
			inline Coefficients Pow10() const
			{
				Coefficients<T, Size> result(*this);
				for (int i = 0; i < mCoefficients.size(); i++)
					result[i] = RAC::Common::Pow10(mCoefficients[i]);
				return result;
			}

			/**
			* @brief Calculates the square root of the coefficients
			*/
			inline Coefficients Sqrt() const
			{
				Coefficients<T, Size> result(*this);
				for (int i = 0; i < mCoefficients.size(); i++)
					result[i] = std::sqrt(mCoefficients[i]);
				return result;
			}

			/**
			* @brief Calculates the square of the coefficients
			*/
			inline Coefficients Square() const
			{
				Coefficients<T, Size> result(*this);
				for (int i = 0; i < mCoefficients.size(); i++)
					result[i] = mCoefficients[i] * mCoefficients[i];
				return result;
			}

			/**
			* @brief Calculates the absolute value of the coefficients
			*/
			inline Coefficients Abs() const
			{
				Coefficients<T, Size> result(*this);
				for (int i = 0; i < mCoefficients.size(); i++)
					result[i] = std::abs(mCoefficients[i]);
				return result;
			}

			/**
			* @brief Calculates the sin value of the coefficients
			*/
			inline Coefficients Sin() const
			{
				Coefficients<T, Size> result(*this);
				for (int i = 0; i < mCoefficients.size(); i++)
					result[i] = std::sin(mCoefficients[i]);
				return result;
			}

			/**
			* @brief Calculates the cos value of the coefficients
			*/
			inline Coefficients Cos() const
			{
				Coefficients<T, Size> result(*this);
				for (int i = 0; i < mCoefficients.size(); i++)
					result[i] = std::cos(mCoefficients[i]);
				return result;
			}

			/**
			* @brief Calculates the sum of the coefficients
			*/
			inline Real Sum()
			{
				Real output = 0.0;
				for (int i = 0; i < mCoefficients.size(); i++)
					output += mCoefficients[i];
				return output;
			}

			/**
			* @brief Access the coefficient at a specified index
			*
			* @param i The index of the coefficient to return
			* @return A reference to the coefficient at the specified index
			*/
			inline Real& operator[](const size_t i)
			{
				RAC_DEBUG_ASSERT(i >= 0, "Index out of bounds");
				RAC_DEBUG_ASSERT(i < ToInt(this->mCoefficients.size()), "Index out of bounds");
				return mCoefficients[i];
			};
			
			/**
			* @brief Access the coefficient at a specified index
			*
			* @param i The index of the coefficient to return
			* @return The value of the coefficient at the specified index
			*/
			inline Real operator[](const size_t i) const
			{
				RAC_DEBUG_ASSERT(i >= 0, "Index out of bounds");
				RAC_DEBUG_ASSERT(i < ToInt(this->mCoefficients.size()), "Index out of bounds");
				return mCoefficients[i];
			};

			/**
			* @brief Inverts the sign of all coefficient entries
			*/
			inline Coefficients& operator-()
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = -mCoefficients[i];
				return *this;
			}

			/**
			* @brief Adds a set of coefficients to the current coefficients
			*/
			inline Coefficients& operator+=(const Coefficients& v)
			{
				RAC_DEBUG_ASSERT(ToInt(mCoefficients.size()) == v.Length(), "Coefficients must have the same length");
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += v[i];
				return *this;
			}

			/**
			* @brief Subtracts a set of coefficients from the current coefficients
			*/
			inline Coefficients& operator-=(const Coefficients& v)
			{
				RAC_DEBUG_ASSERT(ToInt(mCoefficients.size()) == v.Length(), "Coefficients must have the same length");
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] -= v[i];
				return *this;
			}

            /**
            * @brief Multiplies coefficient entries by a given set of coefficient
            */
            inline Coefficients& operator*=(const Coefficients& v)
            {
				RAC_DEBUG_ASSERT(ToInt(mCoefficients.size()) == v.Length(), "Coefficients must have the same length");
				for (int i = 0; i < mCoefficients.size(); i++)
				mCoefficients[i] *= v[i];
				return *this;
            }

            /**
            * @brief Divides coefficient entries by a given set of coefficients
            */
            inline Coefficients& operator/=(const Coefficients& v)
            {
				RAC_DEBUG_ASSERT(ToInt(mCoefficients.size()) == v.Length(), "Coefficients must have the same length");
				for (int i = 0; i < mCoefficients.size(); i++)
				mCoefficients[i] /= v[i];
				return *this;
            }

			/**
			* @brief Adds a single value to all coefficient entries
			*/
			inline Coefficients& operator+=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += a;
				return *this;
			}

			/**
			* @brief Multiplies coefficient entries by a given value
			*/
			inline Coefficients& operator*=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] *= a;
				return *this;
			}

			/**
			* @brief Divides coefficient entries by a given value
			*/
			inline Coefficients& operator/=(const Real a)
			{
				return *this *= (1 / a);
			}

			/**
			* @brief Performs an element-wise comparison
			* @return False if any element pairs are unequal, true otherwise
			*/
			inline bool IsApprox(const Coefficients& a) const
			{
				if (mCoefficients.size() != a.Length())
					return false;
				for (int i = 0; i < a.Length(); i++)
					if (mCoefficients[i] != a[i])
						return false;
				return true;
			}

			/**
			* @brief Determines whether coefficient entries are less than a given value
			*
			* @param a The test value
			* @return True if all entries are less than a, false otherwise
			*/
			inline bool IsLessThan(const Real a) const
			{
				bool valid = true;
				int i = 0;
				while (valid && i < mCoefficients.size())
				{
					valid = mCoefficients[i] < a;
					i++;
				}
				return valid;
			}

			/**
			* @brief Determines whether coefficient entries are greater than a given value
			*
			* @param a The test value
			* @return True if all entries are greater than a, false otherwise
			*/
			inline bool IsGreaterThan(const Real a) const
			{
				bool valid = true;
				int i = 0;
				while (valid&& i < mCoefficients.size())
				{
					valid = mCoefficients[i] > a;
					i++;
				}
				return valid;
			}

			/**
			* @brief Determines whether coefficient entries are less or equal to a given value
			*
			* @param a The test value
			* @return True if all entries are less than or equal to a, false otherwise
			*/
			inline bool IsLessEqThan(const Real a) const
			{
				bool valid = true;
				int i = 0;
				while (valid && i < mCoefficients.size())
				{
					valid = mCoefficients[i] <= a;
					i++;
				}
				return valid;
			}

			/**
			* @brief Determines whether coefficient entries are greater than or equal to a given value
			*
			* @param a The test value
			* @return True if all entries are greater than or equal to a, false otherwise
			*/
			inline bool IsGreaterEqThan(const Real a) const
			{
				bool valid = true;
				int i = 0;
				while (valid && i < mCoefficients.size())
				{
					valid = mCoefficients[i] >= a;
					i++;
				}
				return valid;
			}

			inline auto begin() { return mCoefficients.begin(); }

			inline auto end() { return mCoefficients.end(); }

			inline const auto begin() const { return mCoefficients.begin(); }

			inline const auto end() const { return mCoefficients.end(); }

		protected:
			Container mCoefficients; // Array or vector of coefficients

		private:
			/**
			* @brief Constructor that initialises the Coefficients with a given value
			*
			* @param len The number of coefficients
			* @param in The initialisation value
			*/
			Coefficients(const int len, const T in) requires (Size == 0) : mCoefficients(std::vector<T>(len, in)) {}
		};

		//////////////////// Coefficient operator overloads ////////////////////

		template <typename T, size_t Size>
		inline Coefficients<T, Size> operator-(Coefficients<T, Size> u, const Coefficients<T, Size>& v) { return u -= v; }
		template <typename T, size_t Size>
		inline Coefficients<T, Size> operator+(Coefficients<T, Size> u, const Coefficients<T, Size>& v) { return u += v; }
		template <typename T, size_t Size>
		inline Coefficients<T, Size> operator*(Coefficients<T, Size> u, const Coefficients<T, Size>& v) { return u *= v; }
		template <typename T, size_t Size>
		inline Coefficients<T, Size> operator/(Coefficients<T, Size> u, const Coefficients<T, Size>& v) { return u /= v; }
		
		template <typename T, size_t Size>		
		inline Coefficients<T, Size> operator+(Coefficients<T, Size> v, const Real a) { return v += a; }
		template <typename T, size_t Size>		
		inline Coefficients<T, Size> operator+(const Real a, Coefficients<T, Size> v) { return v += a; }
		template <typename T, size_t Size>		
		inline Coefficients<T, Size> operator-(Coefficients<T, Size> v, const Real a) { return v += (-a); }
		template <typename T, size_t Size>		
		inline Coefficients<T, Size> operator-(const Real a, Coefficients<T, Size> v) { return -v += a; }
		template <typename T, size_t Size>		
		inline Coefficients<T, Size> operator*(Coefficients<T, Size> v, const Real a) { return v *= a; }
		template <typename T, size_t Size>		
		inline Coefficients<T, Size> operator*(const Real a, Coefficients<T, Size> v) { return v *= a; }
		template <typename T, size_t Size>		
		inline Coefficients<T, Size> operator/(Coefficients<T, Size> v, const Real a) { return v *= (1.0 / a); }
		template <typename T, size_t Size>		
		inline Coefficients<T, Size> operator/(const Real a, const Coefficients<T, Size>& v) { Coefficients<T, Size> u = Coefficients<T, Size>(v.Length(), a);  return u /= v; }

		/**
		* @brief prints a Coeffcient using std::cout << coefficient << std::endl;
		*/
		inline std::ostream& operator<<(std::ostream& os, const Coefficients<>& v)
		{
			if (v.Length() == 0)
			{
				os << "[ ]";
				return os;
			}
			os << "[ " << v[0];
			for (int i = 1; i < v.Length(); i++)
				os << ", " << v[i];
			os << " ]";
			return os;
		}
#endif // MATRIX_LIBRARY == CUSTOM_FLAG

		inline Coefficients<> CalculateReflectance(const Coefficients<>& values) { return ((Real)1.0 - values).Sqrt(); }

		/**
		* @brief prints a Coefficients using std::cout << vec3 << std::endl;
		*/
		inline std::ostream& operator<<(std::ostream& os, const Coefficients<>& v)
		{
			int numCoefficients = ToInt(v.Length());
			os << "[ ";
			for (int i = 0; i < numCoefficients; i++)
			{
				os << v[i];
				if (i < numCoefficients - 1)
					os << " , ";
			}
			os << " ]";
			return os;
		}
	}
}

#endif