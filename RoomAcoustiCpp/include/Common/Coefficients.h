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

namespace RAC
{
	namespace Common
	{
		/**
		* @brief Class that stores abitrary coefficients
		*/
		template <class T = std::vector<Real>>
		class Coefficients
		{
		public:

			/**
			* @brief Constructor that initialises the Coefficients with a value
			*
			* @param in The value for the coefficients
			* @details Used for std::array (predetermined size)
			*/
			template <typename U = T, std::enable_if_t<!std::is_constructible<U, size_t, Real>::value, int> = 0>
			Coefficients(const Real in) { mCoefficients.fill(in); }

			/**
			* @brief Constructor that initialises the Coefficients with zeros
			*
			* @param len The number of coefficients
			*/
			template <typename U = T, std::enable_if_t<std::is_constructible<U, size_t, Real>::value, int> = 0>
			Coefficients(const int len) : Coefficients(len, 0.0) {}

			/**
			* @brief Constructor that initialises the Coefficients with a given value
			*
			* @param len The number of coefficients
			* @param in The initialisation value
			*/
			template <typename U = T, std::enable_if_t<std::is_constructible<U, size_t, Real>::value, int> = 0>
			Coefficients(const int len, const Real in) : mCoefficients(len, in) {}

			/**
			* @brief Constructor that initialises the Coefficients from a std::vector
			*
			* @param coefficients The vector of coefficients
			*/
			Coefficients(const T& coefficients) : mCoefficients(coefficients) {}

			/**
			* @brief Default deconstructor
			*/
			~Coefficients() {};

			/**
			* @brief Updates the coefficients
			*
			* @param coefficients The new vector of coefficients
			*/
			inline void Update(const T& coefficients) { mCoefficients = coefficients; }

			/**
			* @brief Returns the number of coefficients
			*
			* @return The number of coefficients
			*/
			inline int Length() const { return static_cast<int>(mCoefficients.size()); }

			/**
			* @brief Calculates the natural (base e) logarithm of the coefficients
			*/
			inline Coefficients Log()
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = log(mCoefficients[i]);
				return *this;
			}

			/**
			* @brief Calculates 10 raised to the power of the coefficients
			*/
			inline Coefficients Pow10()
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = RAC::Common::Pow10(mCoefficients[i]);
				return *this;
			}

			/**
			* @brief Calculates the square root of the coefficients
			*/
			inline Coefficients Sqrt()
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = sqrt(mCoefficients[i]);
				return *this;
			}

			/**
			* @brief Access the coefficient at a specified index
			*
			* @param i The index of the coefficient to return
			* @return A reference to the coefficient at the specified index
			*/
			inline Real& operator[](const size_t i) { assert(i < this->mCoefficients.size()); return mCoefficients[i]; };
			
			/**
			* @brief Access the coefficient at a specified index
			*
			* @param i The index of the coefficient to return
			* @return The value of the coefficient at the specified index
			*/
			inline Real operator[](const size_t i) const { assert(i < this->mCoefficients.size()); return mCoefficients[i]; };

			/**
			* @brief Sets all coefficient entries to a single value
			*
			* @param x The new value
			*/
			inline Coefficients& operator=(Real x)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = x;
				return *this;
			}

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
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += v[i];
				return *this;
			}

			/**
			* @brief Subtracts a set of coefficients from the current coefficients
			*/
			inline Coefficients& operator-=(const Coefficients& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] -= v[i];
				return *this;
			}

            /**
            * @brief Multiplies coefficient entries by a given set of coefficient
            */
            inline Coefficients& operator*=(const Coefficients& v)
            {
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
				mCoefficients[i] *= v[i];
				return *this;
            }

            /**
            * @brief Divides coefficient entries by a given set of coefficients
            */
            inline Coefficients& operator/=(const Coefficients& v)
            {
				assert(mCoefficients.size() == v.Length());
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
			* @brief Determines whether coefficient entries are less than a given value
			*
			* @param a The test value
			* @return True if all entries are less than a, false otherwise
			*/
			inline bool operator<(const Real a) const
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
			inline bool operator>(const Real a) const
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
			inline bool operator<=(const Real a) const
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
			inline bool operator>=(const Real a) const
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
			T mCoefficients; // Array or vector of coefficients
		};

		/**
		* @brief Calculates the sine of the coefficients
		*/
		template<typename T>
		inline Coefficients<T> Sin(Coefficients<T> v)
		{
			for (int i = 0; i < v.Length(); i++)
				v[i] = sin(v[i]);
			return v;
		}

		/**
		* @brief Calculates the cosine of the coefficients
		*/
		template<typename T>
		inline Coefficients<T> Cos(Coefficients<T> v)
		{
			for (int i = 0; i < v.Length(); i++)
				v[i] = cos(v[i]);
			return v;
		}

		/**
		* @brief Calculates the absolute value of the coefficients
		*/
		template<typename T>
		inline Coefficients<T> Abs(Coefficients<T> v)
		{
			for (int i = 0; i < v.Length(); i++)
				v[i] = abs(v[i]);
			return v;
		}

		/**
		* @brief Calculates the sum of the coefficients
		*/
		template<typename T>
		inline Real Sum(Coefficients<T> v)
		{
			Real output = 0.0;
			for (int i = 0; i < v.Length(); i++)
				output += v[i];
			return output;
		}

		//////////////////// Coefficient operator overloads ////////////////////

		template<typename T>
		inline Coefficients<T> operator-(Coefficients<T> u, const Coefficients<T>& v) { return u -= v; }
		template<typename T>		
		inline Coefficients<T> operator+(Coefficients<T> u, const Coefficients<T>& v) { return u += v; }
		template<typename T>		
		inline Coefficients<T> operator*(Coefficients<T> u, const Coefficients<T>& v) { return u *= v; }
		template<typename T>		
		inline Coefficients<T> operator/(Coefficients<T> u, const Coefficients<T>& v) { return u /= v; }
		
		template<typename T>		
		inline Coefficients<T> operator+(Coefficients<T> v, const Real a) { return v += a; }
		template<typename T>		
		inline Coefficients<T> operator+(const Real a, Coefficients<T> v) { return v += a; }
		template<typename T>		
		inline Coefficients<T> operator-(Coefficients<T> v, const Real a) { return v += (-a); }
		template<typename T>		
		inline Coefficients<T> operator-(const Real a, Coefficients<T> v) { return -v += a; }
		template<typename T>		
		inline Coefficients<T> operator*(Coefficients<T> v, const Real a) { return v *= a; }
		template<typename T>		
		inline Coefficients<T> operator*(const Real a, Coefficients<T> v) { return v *= a; }
		template<typename T>		
		inline Coefficients<T> operator/(Coefficients<T> v, const Real a) { return v *= (1.0 / a); }
		template<typename T>		
		inline Coefficients<T> operator/(const Real a, const Coefficients<T>& v) { Coefficients<T> u = Coefficients<T>(v.Length(), a);  return u /= v; }

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

		/**
		* @return True if all coefficient entries are equal to a, false otherwise
		*/
		template <typename T>
		inline bool operator==(const Coefficients<T>& v, const Real a)
		{
			for (int i = 0; i < v.Length(); i++)
				if (v[i] != a)
					return false;
			return true;
		}

		/**
		* @return True if any coefficient entries are not equal to a, false otherwise
		*/
		template <typename T>
		inline bool operator!=(const Coefficients<T>& v, const Real a)
		{
			return !(v == a);
		}

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs are equal, false otherwise
		*/
		template <typename T>
		inline bool operator==(const Coefficients<T>& u, const Coefficients<T>& v)
		{
			if (u.Length() != v.Length())
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] != v[i])
					return false;
			return true;
		}

		/**
		* @brief Performs an element-wise comparison
		* @return True if any element pairs are unequal, false otherwise
		*/
		template <typename T>
		inline bool operator!=(const Coefficients<T>& u, const Coefficients<T>& v)
		{
			return !(u == v);
		}

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs satisfy the condition, false otherwise
		*/
		template <typename T>
		inline bool operator>(const Coefficients<T>& u, const Coefficients<T>& v)
		{
			assert(u.Length() == v.Length());
			for (int i = 0; i < u.Length(); i++)
				if (u[i] <= v[i])
					return false;
			return true;
		}

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs satisfy the condition, false otherwise
		*/
		template <typename T>
		inline bool operator<(const Coefficients<T>& u, const Coefficients<T>& v)
		{
			assert(u.Length() == v.Length());
			for (int i = 0; i < u.Length(); i++)
				if (u[i] >= v[i])
					return false;
			return true;
		}

		template <typename T>
		inline Coefficients<T> Pow(Coefficients<T> u, Real x)
		{
			for (int i = 0; i < u.Length(); i++)
				u[i] = pow(u[i], x);
			return u;
		}

		//////////////////// -- ////////////////////

		/**
		* @brief Class that stores absorption coefficients
		* 
		* @details Stores reflectance -> sqrt(1 - R). Where R is the absortion property of the material
		*/
		template <typename T = std::vector<Real>>
		class Absorption : public Coefficients<T>
		{
		public:
			/**
			* Constructor that initialises the Absorption with ones
			*
			* @param len The number of coefficients
			*/
			Absorption(int len) : Coefficients<T>(len, 1.0), mArea(0.0) {}

			/**
			* Constructor that initialises the Absorption from a std::vector
			*
			* @param R The material absorption
			*/
			Absorption(const T& R) : Coefficients<T>(static_cast<int>(R.size())), mArea(0.0)
			{
				for (int i = 0; i < this->mCoefficients.size(); i++)
				{
					// Clip between 0 and 1
					if (R[i] < 0.0)
						this->mCoefficients[i] = 1.0;
					else if (R[i] > 1.0)
						this->mCoefficients[i] = 0.0;
					else
						this->mCoefficients[i] = sqrt(1.0 - R[i]);
				}
			}

			/**
			* @brief Default deconstructor
			*/
			~Absorption() {}

			/**
			* @brief Resets the coefficients to ones
			*/
			void Reset() { std::fill(this->mCoefficients.begin(), this->mCoefficients.end(), 1.0); }

			/**
			* @brief Sets all absorption entries to a single value
			*
			* @param x The new value
			*/
			inline Absorption& operator=(Real x)
			{
				for (int i = 0; i < this->mCoefficients.size(); i++)
					this->mCoefficients[i] = x;
				return *this;
			}

			/**
			* @brief Inverts the sign of all absorption entries
			*/
			inline Absorption& operator-()
			{
				for (int i = 0; i < this->mCoefficients.size(); i++)
					this->mCoefficients[i] = -this->mCoefficients[i];
				return *this;
			}

			/**
			* @brief Adds a set of absorption coefficients to the current absorption. The areas are added together
			*/
			inline Absorption& operator+=(const Absorption& v)
			{
				assert(this->mCoefficients.size() == v.Length());
				for (int i = 0; i < this->mCoefficients.size(); i++)
					this->mCoefficients[i] += v[i];
				mArea += v.mArea;
				return *this;
			}

			/**
			* @brief Subtracts a set of absorption coefficients to the current absorption. The input area is subtracted
			*/
			inline Absorption& operator-=(const Absorption& v)
			{
				assert(this->mCoefficients.size() == v.Length());
				for (int i = 0; i < this->mCoefficients.size(); i++)
					this->mCoefficients[i] -= v[i];
				mArea -= v.mArea;
				return *this;
			}

			/**
			* @brief Multiplies absorption entries by a given set of absorption coefficients. No change to area
			*/
			inline Absorption& operator*=(const Absorption& v)
			{
				assert(this->mCoefficients.size() == v.Length());
				for (int i = 0; i < this->mCoefficients.size(); i++)
					this->mCoefficients[i] *= v[i];
				return *this;
			}

			/**
			* @brief Divides absorption entries by a given set of absorption coefficients. No change to area
			*/
			inline Absorption& operator/=(const Absorption& v)
			{
				assert(this->mCoefficients.size() == v.Length());
				for (int i = 0; i < this->mCoefficients.size(); i++)
					this->mCoefficients[i] /= v[i];
				return *this;
			}

			/**
			* @brief Adds a single value to all absorption entries. No change to area
			*/
			inline Absorption& operator+=(const Real a)
			{
				for (int i = 0; i < this->mCoefficients.size(); i++)
					this->mCoefficients[i] += a;
				return *this;
			}

			/**
			* @brief Multiplies absorption entries by a given value. No change to area
			*/
			inline Absorption& operator*=(const Real a)
			{
				for (int i = 0; i < this->mCoefficients.size(); i++)
					this->mCoefficients[i] *= a;
				return *this;
			}

			Real mArea;		// Area covered by the absorption coefficients

		private:
		};

		//////////////////// Absorption operator overloads ////////////////////

		template<typename T>
		inline Absorption<T> operator+(Absorption<T> u, const Absorption<T>& v) { return u += v; }
		template<typename T>		
		inline Absorption<T> operator-(Absorption<T> u, const Absorption<T>& v) { return u -= v; }
		template<typename T>		
		inline Absorption<T> operator*(Absorption<T> u, const Absorption<T>& v) { return u *= v; }
		template<typename T>		
		inline Absorption<T> operator/(Absorption<T> u, const Absorption<T>& v) { return u /= v; }

		template<typename T>		
		inline Absorption<T> operator+(Absorption<T> v, const Real a) { return v += a; }
		template<typename T>		
		inline Absorption<T> operator-(Absorption<T> v, const Real a) { return v += (-a); }
		template<typename T>		
		inline Absorption<T> operator-(const Real a, Absorption<T> v) { return -v += a; }
		template<typename T>		
		inline Absorption<T> operator*(Absorption<T> v, const Real a) { return v *= a; }
		template<typename T>		
		inline Absorption<T> operator/(Absorption<T> v, const Real a) { return v *= (1.0 / a); }

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs and the areas are equal, false otherwise
		*/
		template<typename T>
		inline bool operator==(const Absorption<T>& u, const Absorption<T>& v)
		{
			if (u.Length() != v.Length())
				return false;
			if (u.mArea != v.mArea)
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] != v[i])
					return false;
			return true;
		}

		/**
		* @brief Performs an element-wise comparison
		* @return True if any element pairs or the areas are unequal, false otherwise
		*/
		template<typename T>
		inline bool operator!=(const Absorption<T>& u, const Absorption<T>& v) { return !(u == v); }
	}
}

#endif