/*
* @class Coefficients, Absorption
*
* @brief Declaration of Coefficient and Absorption class
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
		//////////////////// Coefficients class ////////////////////

		class Coefficients
		{
		public:
			Coefficients(const size_t len) : mCoefficients(len, 0.0) {}
			Coefficients(const size_t len, const Real in) : mCoefficients(len, in) {}
			Coefficients(const std::vector<Real>& coefficients) : mCoefficients(coefficients) {}
			~Coefficients() {};

			inline void Update(const std::vector<Real>& coefficients) { mCoefficients = coefficients; }
			inline size_t Length() const { return mCoefficients.size(); }

			inline Coefficients Log()
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = log(mCoefficients[i]);
				return *this;
			}

			inline Coefficients Pow(Real exp)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = pow(exp, mCoefficients[i]);
				return *this;
			}

			inline Coefficients Sqrt()
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = sqrt(mCoefficients[i]);
				return *this;
			}

			// Operators
			inline Real& operator[](const int i) { return mCoefficients[i]; };
			inline Real operator[](const int i) const { return mCoefficients[i]; };

			inline Coefficients& operator=(Real x)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = x;
				return *this;
			}

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

			inline Coefficients& operator+=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] += a;
				return *this;
			}

			inline Coefficients& operator*=(const Real a)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] *= a;
				return *this;
			}

			inline Coefficients& operator/=(const Real a)
			{
				return *this *= (1 / a);
			}

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
			std::vector<Real> mCoefficients;
		};

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

		inline bool operator==(const Coefficients& v, const Real a)
		{
			for (int i = 0; i < v.Length(); i++)
				if (v[i] != a)
					return false;
			return true;
		}

		inline bool operator!=(const Coefficients& v, const Real a)
		{
			return !(v == a);
		}

		inline bool operator==(const Coefficients& u, const Coefficients& v)
		{
			if (u.Length() != v.Length())
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] != v[i])
					return false;
			return true;
		}

		inline bool operator!=(const Coefficients& u, const Coefficients& v)
		{
			return !(u == v);
		}

		inline bool operator>(const Coefficients& u, const Coefficients& v)
		{
			if (u.Length() != v.Length())
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] <= v[i])
					return false;
			return true;
		}

		inline bool operator<(const Coefficients& u, const Coefficients& v)
		{
			if (u.Length() != v.Length())
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] >= v[i])
					return false;
			return true;
		}

		//////////////////// Absorption class ////////////////////

		class Absorption : public Coefficients // Stores 1 - sqrt(R). Where R is the absortion property of the material in the pressure domain
		{
		public:

			// Load and Destroy
			Absorption() : Coefficients(1, 1.0), mArea(0.0) {}
			Absorption(size_t len) : Coefficients(len, 1.0), mArea(0.0) {}
			Absorption(size_t len, const Real x) : Coefficients(len, x), mArea(0.0) {}
			Absorption(const std::vector<Real>& coefficients) : Coefficients(coefficients.size()), mArea(0.0)
			{
				for (int i = 0; i < mCoefficients.size(); i++)
					mCoefficients[i] = sqrt(1.0 - coefficients[i]);
			}
			Absorption(const std::vector<Real>& coefficients, Real area) : Coefficients(coefficients), mArea(area) {}
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