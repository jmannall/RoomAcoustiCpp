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
#include <cmath>

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
		class Coefficients
		{
		public:
			/**
			* Constructor that initialises the Coefficients with zeros
			*
			* @param len The number of coefficients
			*/
			Coefficients(const size_t len) : mCoefficients(len, 0.0) {}

			/**
			* @brief Constructor that initialises the Coefficients with a given value
			*
			* @param len The number of coefficients
			* @param in The initialisation value
			*/
			Coefficients(const size_t len, const Real in) : mCoefficients(len, in) {}

			/**
			* @brief Constructor that initialises the Coefficients for a std::vector
			*
			* @param coefficients The vector of coefficients
			*/
			Coefficients(const std::vector<Real>& coefficients) : mCoefficients(coefficients) {}

			/**
			* @brief Default deconstructor
			*/
			~Coefficients() {};

			/**
			* @brief Updates the coefficients
			*
			* @param coefficients The new vector of coefficients
			*/
			inline void Update(const std::vector<Real>& coefficients) { mCoefficients = coefficients; }

			/**
			* @brief Returns the number of coefficients
			*
			* @return The number of coefficients
			*/
			inline size_t Length() const { return mCoefficients.size(); }

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
			* @brief Calculates 10 to the power of the coefficients
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
			* @brief Returns the coefficient at a specified index
			*
			* @param i The index of the coefficient to return
			*/
			inline Real& operator[](const int i) { assert(0 <= i && i < mCoefficients.size()); return mCoefficients[i]; };
			inline Real operator[](const int i) const { assert(0 <= i && i < mCoefficients.size()); return mCoefficients[i]; };

			/**
			* @brief Sets all coefficeint entries to a single value
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

			inline Coefficients& operator+=(const Coefficients& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += v[i];
				return *this;
			}

			inline Coefficients& operator-=(const Coefficients& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] -= v[i];
				return *this;
			}

			inline Coefficients& operator*=(const Coefficients& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] *= v[i];
				return *this;
			}

			inline Coefficients& operator/=(const Coefficients& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] /= v[i];
				return *this;
			}

			/**
			* @brief Adds a single value all coefficient entries
			* 
			* @param a The value to increase by
			*/
			inline Coefficients& operator+=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += a;
				return *this;
			}

			/**
			* @brief Multiplies coefficient entries by a given value
			*
			* @param a The value to multiply by
			*/
			inline Coefficients& operator*=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] *= a;
				return *this;
			}

			/**
			* @brief Divides coefficient entries by a given value
			*
			* @param a The value to divide by
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

		protected:
			std::vector<Real> mCoefficients; // Stored coefficients
		};

		//////////////////// Coefficient operator overloads ////////////////////

		inline Coefficients operator+(Coefficients u, const Coefficients& v) { return u += v; }
		inline Coefficients operator-(Coefficients u, const Coefficients& v) { return u -= v; }
		inline Coefficients operator*(Coefficients u, const Coefficients& v) { return u *= v; }
		inline Coefficients operator/(Coefficients u, const Coefficients& v) { return u /= v; }

		inline Coefficients operator+(Coefficients v, const Real a) { return v += a; }
		inline Coefficients operator+(const Real a, Coefficients v) { return v += a; }
		inline Coefficients operator-(Coefficients v, const Real a) { return v += (-a); }
		inline Coefficients operator-(const Real a, Coefficients v) { return -v += a; }
		inline Coefficients operator*(Coefficients v, const Real a) { return v *= a; }
		inline Coefficients operator*(const Real a, Coefficients v) { return v *= a; }
		inline Coefficients operator/(Coefficients v, const Real a) { return v *= (1.0 / a); }
		inline Coefficients operator/(const Real a, const Coefficients& v) { Coefficients u = Coefficients(v.Length(), a);  return u /= v; }

		/**
		* @param v Coefficients to compare
		* @param a Real value to compare
		* @return True if all coefficient entries are equal to a, false otherwise
		*/
		inline bool operator==(const Coefficients& v, const Real a)
		{
			for (int i = 0; i < v.Length(); i++)
				if (v[i] != a)
					return false;
			return true;
		}

		/**
		* @param v Coefficients to compare
		* @param a Real value to compare
		* @return True if any coefficient entries are not equal to a, false otherwise
		*/
		inline bool operator!=(const Coefficients& v, const Real a)
		{
			return !(v == a);
		}

		/**
		* @brief Elementwise comparison
		* @return True if all element pairs are equal, false otherwise
		*/
		inline bool operator==(const Coefficients& u, const Coefficients& v)
		{
			if (u.Length() != v.Length())
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] != v[i])
					return false;
			return true;
		}

		/**
		* @brief Elementwise comparison
		* @return True if any element pairs are unequal, false otherwise
		*/
		inline bool operator!=(const Coefficients& u, const Coefficients& v)
		{
			return !(u == v);
		}

		/**
		* @brief Elementwise comparison
		* @return True if all element pairs satisfy the condition, false otherwise
		*/
		inline bool operator>(const Coefficients& u, const Coefficients& v)
		{
			if (u.Length() != v.Length())
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] <= v[i])
					return false;
			return true;
		}

		/**
		* @brief Elementwise comparison
		* @return True if all element pairs satisfy the condition, false otherwise
		*/
		inline bool operator<(const Coefficients& u, const Coefficients& v)
		{
			if (u.Length() != v.Length())
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] >= v[i])
					return false;
			return true;
		}

		//////////////////// -- ////////////////////

		/**
		* @brief Class that stores absorption coefficients
		* 
		* @detials Stores reflectance -> sqrt(1 - R). Where R is the absortion property of the material
		*/
		class Absorption : public Coefficients
		{
		public:

			// Load and Destroy
			Absorption() : Coefficients(1, 1.0), mArea(0.0) {}
			Absorption(size_t len) : Coefficients(len, 1.0), mArea(0.0) {}
			Absorption(const std::vector<Real>& coefficients) : Coefficients(coefficients.size()), mArea(0.0)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
				{
					assert(coefficients[i] <= 1.0);
					mCoefficients[i] = sqrt(1.0 - coefficients[i]);
				}
			}
			~Absorption() {}

			void Reset() { std::fill(mCoefficients.begin(), mCoefficients.end(), 1.0); }

			// Operators
			inline Absorption& operator=(Real x)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = x;
				return *this;
			}

			inline Absorption& operator-()
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = -mCoefficients[i];
				return *this;
			}

			inline Absorption& operator+=(const Absorption& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += v[i];
				mArea += v.mArea;
				return *this;
			}

			inline Absorption& operator-=(const Absorption& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] -= v[i];
				mArea -= v.mArea;
				return *this;
			}

			inline Absorption& operator*=(const Absorption& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] *= v[i];
				return *this;
			}

			inline Absorption& operator/=(const Absorption& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] /= v[i];
				return *this;
			}

			inline Absorption& operator+=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += a;
				return *this;
			}

			inline Absorption& operator-=(const Real a)
			{
				return *this += -a;
			}

			inline Absorption& operator*=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] *= a;
				return *this;
			}

			inline Absorption& operator/=(const Real a)
			{
				return *this *= (1 / a);
			}

			// Member variables
			Real mArea;

		private:
		};

		inline Absorption operator+(Absorption u, const Absorption& v) { return u += v; }
		inline Absorption operator-(Absorption u, const Absorption& v) { return u -= v; }
		inline Absorption operator*(Absorption u, const Absorption& v) { return u *= v; }
		inline Absorption operator/(Absorption u, const Absorption& v) { return u /= v; }

		inline Absorption operator+(Absorption v, const Real a) { return v += a; }
		inline Absorption operator-(Absorption v, const Real a) { return v += (-a); }
		inline Absorption operator-(const Real a, Absorption v) { return -v += a; }
		inline Absorption operator*(Absorption v, const Real a) { return v *= a; }
		inline Absorption operator/(Absorption v, const Real a) { return v /= a; }

		inline bool operator==(const Absorption& u, const Absorption& v)
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

		inline bool operator!=(const Absorption& u, const Absorption& v)
		{
			return !(u == v);
		}

		inline Absorption Sqrt(Absorption v)
		{
			for (int i = 0; i < v.Length(); i++)
				v[i] = sqrt(v[i]);
			return v;
		}
	}
}

#endif