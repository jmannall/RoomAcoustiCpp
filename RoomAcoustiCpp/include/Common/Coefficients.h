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
			Coefficients(const int len) : Coefficients(len, 0.0) {}

			/**
			* @brief Constructor that initialises the Coefficients with a given value
			*
			* @param len The number of coefficients
			* @param in The initialisation value
			*/
			Coefficients(const int len, const Real in) : mCoefficients(len, in) {}

			/**
			* @brief Constructor that initialises the Coefficients from a std::vector
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
			inline Real& operator[](const size_t i) { assert(i < mCoefficients.size()); return mCoefficients[i]; };
			
			/**
			* @brief Access the coefficient at a specified index
			*
			* @param i The index of the coefficient to return
			* @return The value of the coefficient at the specified index
			*/
			inline Real operator[](const size_t i) const { assert(i < mCoefficients.size()); return mCoefficients[i]; };

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

			inline auto begin() { return mCoefficients.begin(); }

			inline auto end() { return mCoefficients.end(); }

			inline const auto begin() const { return mCoefficients.begin(); }

			inline const auto end() const { return mCoefficients.end(); }

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
		* @return True if any coefficient entries are not equal to a, false otherwise
		*/
		inline bool operator!=(const Coefficients& v, const Real a)
		{
			return !(v == a);
		}

		/**
		* @brief Performs an element-wise comparison
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
		* @brief Performs an element-wise comparison
		* @return True if any element pairs are unequal, false otherwise
		*/
		inline bool operator!=(const Coefficients& u, const Coefficients& v)
		{
			return !(u == v);
		}

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs satisfy the condition, false otherwise
		*/
		inline bool operator>(const Coefficients& u, const Coefficients& v)
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
		inline bool operator<(const Coefficients& u, const Coefficients& v)
		{
			assert(u.Length() == v.Length());
			for (int i = 0; i < u.Length(); i++)
				if (u[i] >= v[i])
					return false;
			return true;
		}

		//////////////////// -- ////////////////////

		/**
		* @brief Class that stores absorption coefficients
		* 
		* @details Stores reflectance -> sqrt(1 - R). Where R is the absortion property of the material
		*/
		class Absorption : public Coefficients
		{
		public:
			/**
			* Constructor that initialises the Absorption with ones
			*
			* @param len The number of coefficients
			*/
			Absorption(int len) : Coefficients(len, 1.0), mArea(0.0) {}

			/**
			* Constructor that initialises the Absorption from a std::vector
			*
			* @param R The material absorption
			*/
			Absorption(const std::vector<Real>& R) : Coefficients(static_cast<int>(R.size())), mArea(0.0)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
				{
					assert(mCoefficients[i] <= 1.0);
					mCoefficients[i] = sqrt(1.0 - R[i]);
				}
			}

			/**
			* @brief Default deconstructor
			*/
			~Absorption() {}

			/**
			* @brief Resets the coefficients to ones
			*/
			void Reset() { std::fill(mCoefficients.begin(), mCoefficients.end(), 1.0); }

			/**
			* @brief Sets all absorption entries to a single value
			*
			* @param x The new value
			*/
			inline Absorption& operator=(Real x)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = x;
				return *this;
			}

			/**
			* @brief Inverts the sign of all absorption entries
			*/
			inline Absorption& operator-()
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = -mCoefficients[i];
				return *this;
			}

			/**
			* @brief Adds a set of absorption coefficients to the current absorption. The areas are added together
			*/
			inline Absorption& operator+=(const Absorption& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += v[i];
				mArea += v.mArea;
				return *this;
			}

			/**
			* @brief Subtracts a set of absorption coefficients to the current absorption. The input area is subtracted
			*/
			inline Absorption& operator-=(const Absorption& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] -= v[i];
				mArea -= v.mArea;
				return *this;
			}

			/**
			* @brief Multiplies absorption entries by a given set of absorption coefficients. No change to area
			*/
			inline Absorption& operator*=(const Absorption& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] *= v[i];
				return *this;
			}

			/**
			* @brief Divides absorption entries by a given set of absorption coefficients. No change to area
			*/
			inline Absorption& operator/=(const Absorption& v)
			{
				assert(mCoefficients.size() == v.Length());
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] /= v[i];
				return *this;
			}

			/**
			* @brief Adds a single value to all absorption entries. No change to area
			*/
			inline Absorption& operator+=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += a;
				return *this;
			}

			/**
			* @brief Multiplies absorption entries by a given value. No change to area
			*/
			inline Absorption& operator*=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] *= a;
				return *this;
			}

			Real mArea;		// Area covered by the absorption coefficients

		private:
		};

		//////////////////// Absorption operator overloads ////////////////////

		inline Absorption operator+(Absorption u, const Absorption& v) { return u += v; }
		inline Absorption operator-(Absorption u, const Absorption& v) { return u -= v; }
		inline Absorption operator*(Absorption u, const Absorption& v) { return u *= v; }
		inline Absorption operator/(Absorption u, const Absorption& v) { return u /= v; }

		inline Absorption operator+(Absorption v, const Real a) { return v += a; }
		inline Absorption operator-(Absorption v, const Real a) { return v += (-a); }
		inline Absorption operator-(const Real a, Absorption v) { return -v += a; }
		inline Absorption operator*(Absorption v, const Real a) { return v *= a; }
		inline Absorption operator/(Absorption v, const Real a) { return v *= (1.0 / a); }

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs and the areas are equal, false otherwise
		*/
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

		/**
		* @brief Performs an element-wise comparison
		* @return True if any element pairs or the areas are unequal, false otherwise
		*/
		inline bool operator!=(const Absorption& u, const Absorption& v)
		{
			return !(u == v);
		}
	}
}

#endif