/*
*
*  \FrequencyDependence class
*
*/

#ifndef Common_FrequencyDependence_h
#define Common_FrequencyDependence_h

// C++ headers
#include <assert.h>
#include <vector>
#include <cmath>

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"

namespace UIE
{
	namespace Common
	{
		//////////////////// Coefficients class ////////////////////

		class Coefficients
		{
		public:
			Coefficients(const size_t len) : coefficients(len, 0.0) {}
			Coefficients(const size_t len, const Real in) : coefficients(len, in) {}
			Coefficients(const std::vector<Real>& c) : coefficients(c) {}
			~Coefficients() {};

			inline void Update(std::vector<Real> c) { coefficients = c; }
			inline size_t Length() const { return coefficients.size(); }

			inline Coefficients Log()
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] = log(coefficients[i]);
				return *this;
			}

			inline Coefficients Pow(Real exp)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] = pow(exp, coefficients[i]);
				return *this;
			}

			// Operators
			inline Real& operator[](const int& i) { return coefficients[i]; };
			inline Real operator[](const int& i) const { return coefficients[i]; };

			// Operators
			inline Coefficients& operator-()
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] = -coefficients[i];
				return *this;
			}

			inline Coefficients& operator+=(const Coefficients& v)
			{
				assert(coefficients.size() == v.Length());
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] += v[i];
				return *this;
			}

			inline Coefficients& operator-=(const Coefficients& v)
			{
				assert(coefficients.size() == v.Length());
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] -= v[i];
				return *this;
			}

			inline Coefficients& operator*=(const Coefficients& v)
			{
				assert(coefficients.size() == v.Length());
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] *= v[i];
				return *this;
			}

			inline Coefficients& operator/=(const Coefficients& v)
			{
				assert(coefficients.size() == v.Length());
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] /= v[i];
				return *this;
			}

			inline Coefficients& operator+=(const Real& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] += a;
				return *this;
			}

			inline Coefficients& operator*=(const Real& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] *= a;
				return *this;
			}

			inline Coefficients& operator/=(const Real& a)
			{
				return *this *= (1 / a);
			}

			inline bool operator<(const Real& a) const
			{
				bool valid = true;
				int i = 0;
				while (valid && i < coefficients.size())
				{
					valid = coefficients[i] < a;
					i++;
				}
				return valid;
			}

			inline bool operator>(const Real& a) const
			{
				bool valid = true;
				int i = 0;
				while (valid&& i < coefficients.size())
				{
					valid = coefficients[i] > a;
					i++;
				}
				return valid;
			}

		protected:
			std::vector<Real> coefficients;
		};

		inline bool Equals(const Coefficients& a, const Coefficients& b)
		{
			if (a.Length() != b.Length())
				return false;
			for (int i = 0; i < a.Length(); i++)
				if (a[i] > b[i] + EPS || a[i] < b[i] - EPS)
					return false;
			return true;
		}

		inline Coefficients operator+(Coefficients a, const Coefficients& b) { return a += b; }
		inline Coefficients operator-(Coefficients a, const Coefficients& b) { return a -= b; }
		inline Coefficients operator*(Coefficients a, const Coefficients& b) { return a *= b; }
		inline Coefficients operator/(Coefficients a, const Coefficients& b) { return a /= b; }

		inline Coefficients operator+(Coefficients a, const Real& b) { return a += b; }
		inline Coefficients operator-(Coefficients a, const Real& b) { return a += (-b); }
		inline Coefficients operator-(const Real& b, Coefficients a) { return -a += b; }
		inline Coefficients operator*(Coefficients a, const Real& b) { return a *= b; }
		inline Coefficients operator/(Coefficients a, const Real& b) { return a *= (1.0 / b); }
		inline Coefficients operator/(const Real& b, const Coefficients& a) { Coefficients c = Coefficients(a.Length(), b);  return c /= a; }

		inline bool operator==(const Coefficients& a, const Real& b)
		{
			for (int i = 0; i < a.Length(); i++)
				if (a[i] != b)
					return false;
			return true;
		}

		inline bool operator!=(const Coefficients& a, const Real& b)
		{
			return !(a == b);
		}

		inline bool operator==(const Coefficients& a, const Coefficients& b)
		{
			if (a.Length() != b.Length())
				return false;
			for (int i = 0; i < a.Length(); i++)
				if (a[i] != b[i])
					return false;
			return true;
		}

		inline bool operator!=(const Coefficients& a, const Coefficients& b)
		{
			return !(a == b);
		}

		inline bool operator>(const Coefficients& a, const Coefficients& b)
		{
			if (a.Length() != b.Length())
				return false;
			for (int i = 0; i < a.Length(); i++)
				if (a[i] <= b[i])
					return false;
			return true;
		}

		inline bool operator<(const Coefficients& a, const Coefficients& b)
		{
			if (a.Length() != b.Length())
				return false;
			for (int i = 0; i < a.Length(); i++)
				if (a[i] >= b[i])
					return false;
			return true;
		}

		//////////////////// Absorption class ////////////////////

		class Absorption : public Coefficients // Stores 1 - sqrt(R). Where R is the absortion property of the material in the pressure domain
		{
		public:

			// Load and Destroy
			Absorption() : Coefficients(1, 1.0), area(0.0) {}
			Absorption(size_t len) : Coefficients(len, 1.0), area(0.0) {}
			Absorption(size_t len, const Real& x) : Coefficients(len, x), area(0.0) {}
			Absorption(const std::vector<Real>& c) : Coefficients(c.size()), area(0.0)
			{
				for (int i = 0; i < c.size(); i++)
					coefficients[i] = sqrt(1.0 - c[i]);
			}
			Absorption(const std::vector<Real>& c, Real _area) : Coefficients(c), area(_area) {}
			~Absorption() {}

			void Reset() { std::fill(coefficients.begin(), coefficients.end(), 1.0); }

			// Operators
			inline Absorption& operator-()
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] = -coefficients[i];
				return *this;
			}

			inline Absorption& operator+=(const Absorption& v)
			{
				assert(coefficients.size() == v.Length());
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] += v[i];
				area += v.area;
				return *this;
			}

			inline Absorption& operator-=(const Absorption& v)
			{
				assert(coefficients.size() == v.Length());
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] -= v[i];
				area -= v.area;
				return *this;
			}

			inline Absorption& operator*=(const Absorption& v)
			{
				assert(coefficients.size() == v.Length());
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] *= v[i];
				return *this;
			}

			inline Absorption& operator/=(const Absorption& v)
			{
				assert(coefficients.size() == v.Length());
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] /= v[i];
				return *this;
			}

			inline Absorption& operator+=(const Real& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] += a;
				return *this;
			}

			inline Absorption& operator-=(const Real& a)
			{
				return *this += -a;
			}

			inline Absorption& operator*=(const Real& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] *= a;
				return *this;
			}

			inline Absorption& operator/=(const Real& a)
			{
				return *this *= (1 / a);
			}

			// Member variables
			Real area;

		private:
		};

		inline Absorption operator+(Absorption a, const Absorption& b) { return a += b; }
		inline Absorption operator-(Absorption a, const Absorption& b) { return a -= b; }
		inline Absorption operator*(Absorption a, const Absorption& b) { return a *= b; }
		inline Absorption operator/(Absorption a, const Absorption& b) { return a /= b; }

		inline Absorption operator+(Absorption a, const Real& b) { return a += b; }
		inline Absorption operator-(Absorption a, const Real& b) { return a += (-b); }
		inline Absorption operator-(const Real& b, Absorption a) { return -a += b; }
		inline Absorption operator*(Absorption a, const Real& b) { return a *= b; }
		inline Absorption operator/(Absorption a, const Real& b) { return a /= b; }

		inline bool operator==(const Absorption& a, const Absorption& b)
		{
			if (a.Length() != b.Length())
				return false;
			if (a.area != b.area)
				return false;
			for (int i = 0; i < a.Length(); i++)
				if (a[i] != b[i])
					return false;
			return true;
		}

		inline bool operator!=(const Absorption& a, const Absorption& b)
		{
			return !(a == b);
		}

		inline Absorption Sqrt(Absorption a)
		{
			for (int i = 0; i < a.Length(); i++)
				a[i] = sqrt(a[i]);
			return a;
		}
	}
}

#endif