/*
*
*  \FrequencyDependence class
*
*/

#ifndef Common_FrequencyDependence_h
#define Common_FrequencyDependence_h

// Common headers
#include "Common/Types.h"

namespace UIE
{
	namespace Common
	{
		//////////////////// Coefficients class ////////////////////

		class Coefficients
		{
		public:
			Coefficients(const size_t len) : coefficients(len) {}
			Coefficients(const size_t len, const Real in) : coefficients(len, in) {}
			Coefficients(const std::vector<Real>& c) : coefficients(c) {}
			~Coefficients() {};

			inline void Update(std::vector<Real> c) { coefficients = c; }
			inline size_t Length() const { return coefficients.size(); }

			inline Coefficients Log()
			{
				for (int i = 0 ; i < coefficients.size(); i++)
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

			inline Coefficients& operator+=(const Real& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] += a;
				return *this;
			}

			inline Coefficients& operator-=(const Real& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] -= a;
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
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] /= a;
				return *this;
			}

			inline Coefficients& operator+=(const Coefficients& v)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] += v[i];
				return *this;
			}

			inline Coefficients& operator-=(const Coefficients& v)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] -= v[i];
				return *this;
			}

			inline Coefficients& operator*=(const Coefficients& v)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] *= v[i];
				return *this;
			}

			inline Coefficients& operator/=(const Coefficients& v)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] /= v[i];
				return *this;
			}

			inline Coefficients& operator+(const Coefficients& v) const
			{
				Coefficients ret = *this;
				return ret += v;
			}

			inline Coefficients& operator-(const Coefficients& v) const
			{
				Coefficients ret = *this;
				return ret -= v;
			}

			inline Coefficients& operator*(const Coefficients& v) const
			{
				Coefficients ret = *this;
				return ret *= v;
			}

			inline Coefficients& operator/(const Coefficients& v) const
			{
				Coefficients ret = *this;
				return ret /= v;
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

		inline Coefficients operator/(Real a, Coefficients v)
		{
			Coefficients ret = Coefficients(v.Length());
			for (int i = 0; i < v.Length(); i++)
				ret[i] = a / v[i];
			return ret;
		}

		//////////////////// Absorption class ////////////////////

		class Absorption : public Coefficients // Stores sqrt(1 - R). Where R is the absortion property of the material in the pressure domain
		{												// Processing done in the energy domain
		public:

			// Load and Destroy
			Absorption(size_t len) : Coefficients(len, 1.0), area(0.0) {}
			Absorption(const std::vector<Real> c) : Coefficients(c.size()), area(0.0)
			{
				for (int i = 0; i < c.size(); i++)
					coefficients[i] = sqrt(1.0 - c[i]);
			}
			Absorption(const std::vector<Real> c, Real _area) : Coefficients(c), area(_area) {}
			~Absorption() {}

			void Reset() { std::fill(coefficients.begin(), coefficients.end(), 1.0); }

			// Operators
			inline Absorption& operator+=(const Absorption& v)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] += v[i];
				area += v.area;
				return *this;
			}

			inline Absorption& operator-=(const Absorption& v)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] -= v[i];
				area -= v.area;
				return *this;
			}

			inline Absorption& operator*=(const Absorption& v)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] *= v[i];
				return *this;
			}

			inline Absorption& operator/=(const Absorption& v)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] /= v[i];
				return *this;
			}

			inline Absorption& operator+(const Absorption& v) const
			{
				Absorption ret = *this;
				return ret += v;
			}

			inline Absorption& operator-(const Absorption& v) const
			{
				Absorption ret = *this;
				return ret -= v;
			}

			inline Absorption& operator*(const Absorption& v) const
			{
				Absorption ret = *this;
				return ret *= v;
			}

			inline Absorption& operator/(const Absorption& v) const
			{
				Absorption ret = *this;
				return ret /= v;
			}

			/*inline Absorption& operator+=(const Absorption& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] += a[i];
				area += a.area;
				return *this;
			}

			inline Absorption& operator-=(const Absorption& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] -= a[i];
				area -= a.area;
				return *this;
			}

			inline Absorption& operator*=(const Absorption& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] *= a[i];
				return *this;
			}

			inline Absorption& operator*=(const Real& a)
			{
				for (int i = 0; i < coefficients.size(); i++)
					coefficients[i] *= a;
				return *this;
			}

			inline Absorption operator+(const Absorption& a) const
			{
				Absorption ret = *this;
				return ret += a;
			}

			inline Absorption operator-(const Absorption& a) const
			{
				Absorption ret = *this;
				return ret -= a;
			}

			inline Absorption operator*(const Absorption& a) const
			{
				Absorption ret = *this;
				return ret *= a;
			}

			inline Absorption operator*(const Real& a) const
			{
				Absorption ret = *this;
				return ret *= a;
			}*/

			// Member variables
			Real area;

		private:
		};
	}

	////////////////////// FrequencyDependence class ////////////////////

	//class FrequencyDependence
	//{
	//public:

	//	// Load and Destroy
	//	FrequencyDependence() : low(1.0), midLow(1.0), mid(1.0), midHigh(1.0), high(1.0) {}
	//	FrequencyDependence(Real l, Real mL, Real m, Real mH, Real h) : low(l), midLow(mL), mid(m), midHigh(mH), high(h) {}
	//	~FrequencyDependence() {};

	//	// Getters
	//	inline void GetValues(Real* g) const { g[0] = low; g[1] = midLow; g[2] = mid; g[3] = midHigh; g[4] = high; };

	//	inline FrequencyDependence Log()
	//	{
	//		low = log(low);
	//		midLow = log(midLow);
	//		mid = log(mid);
	//		midHigh = log(midHigh);
	//		high = log(high);
	//		return *this;
	//	}

	//	// Operators
	//	inline FrequencyDependence operator+=(const Real& a)
	//	{
	//		low += a;
	//		midLow += a;
	//		mid += a;
	//		midHigh += a;
	//		high += a;
	//		return *this;
	//	}

	//	inline FrequencyDependence operator+=(const FrequencyDependence& v)
	//	{
	//		low += v.low;
	//		midLow += v.midLow;
	//		mid += v.mid;
	//		midHigh += v.midHigh;
	//		high += v.high;
	//		return *this;
	//	}

	//	inline FrequencyDependence operator-=(const Real& a)
	//	{
	//		low -= a;
	//		midLow -= a;
	//		mid -= a;
	//		midHigh -= a;
	//		high -= a;
	//		return *this;
	//	}

	//	inline FrequencyDependence operator*=(const Real& a)
	//	{
	//		low *= a;
	//		midLow *= a;
	//		mid *= a;
	//		midHigh *= a;
	//		high *= a;
	//		return *this;
	//	}

	//	inline FrequencyDependence operator*=(const FrequencyDependence& v)
	//	{
	//		low *= v.low;
	//		midLow *= v.midLow;
	//		mid *= v.mid;
	//		midHigh *= v.midHigh;
	//		high *= v.high;
	//		return *this;
	//	}

	//	inline FrequencyDependence operator/=(const Real& a)
	//	{
	//		low /= a;
	//		midLow /= a;
	//		mid /= a;
	//		midHigh /= a;
	//		high /= a;
	//		return *this;
	//	}

	//protected:

	//	// Member variables
	//	Real low, midLow, mid, midHigh, high;

	//};

	////////////////////// Operators ////////////////////

	//inline FrequencyDependence operator+(const FrequencyDependence& v, const Real& a)
	//{
	//	Real g[5];
	//	v.GetValues(g);
	//	return FrequencyDependence(a + g[0], a + g[1], a + g[2], a + g[3], a + g[4]);
	//}

	//inline FrequencyDependence operator-(const FrequencyDependence& v)
	//{
	//	Real g[5];
	//	v.GetValues(g);
	//	return FrequencyDependence(-g[0], -g[1], -g[2], -g[3], -g[4]);
	//}

	//inline FrequencyDependence operator+(const Real& a, const FrequencyDependence& v)
	//{
	//	return v + a;
	//}

	//inline FrequencyDependence operator-(const Real& a, const FrequencyDependence& v)
	//{
	//	return -v + a;
	//}

	//inline FrequencyDependence operator-(const FrequencyDependence& v, const Real& a)
	//{
	//	return -a + v;
	//}

	//inline FrequencyDependence operator+(const FrequencyDependence& v, const FrequencyDependence& u)
	//{
	//	Real g1[5];
	//	Real g2[5];
	//	v.GetValues(g1);
	//	u.GetValues(g2);
	//	return FrequencyDependence(g1[0] + g2[0], g1[1] + g2[1], g1[2] + g2[2], g1[3] + g2[3], g1[4] + g2[4]);
	//}

	//inline FrequencyDependence operator*(const Real& a, const FrequencyDependence& f)
	//{
	//	Real g[5];
	//	f.GetValues(g);
	//	return FrequencyDependence(a * g[0], a * g[1], a * g[2], a * g[3], a * g[4]);
	//}

	//inline FrequencyDependence operator*(const FrequencyDependence& f, const Real& a)
	//{
	//	return a * f;
	//}

	//inline FrequencyDependence operator*(const FrequencyDependence& v, const FrequencyDependence& u)
	//{
	//	Real g1[5];
	//	Real g2[5];
	//	v.GetValues(g1);
	//	u.GetValues(g2);
	//	return FrequencyDependence(g1[0] * g2[0], g1[1] * g2[1], g1[2] * g2[2], g1[3] * g2[3], g1[4] * g2[4]);
	//}

	//inline FrequencyDependence operator/(const Real& a, const FrequencyDependence& v)
	//{
	//	Real g[5];
	//	v.GetValues(g);
	//	return FrequencyDependence(a / g[0], a / g[1], a / g[2], a / g[3], a / g[4]);
	//}

	//inline FrequencyDependence operator/(const FrequencyDependence& v, const Real& a)
	//{
	//	return (1.0 / a) * v;
	//}

	//inline bool operator<(const FrequencyDependence& v, const Real& a)
	//{
	//	Real g[5];
	//	v.GetValues(g);
	//	bool valid = true;
	//	int i = 0;
	//	while (valid && i < 5)
	//	{
	//		valid = g[i] < a;
	//		i++;
	//	}
	//	return valid;
	//}

	//inline bool operator>(const FrequencyDependence& v, const Real& a)
	//{
	//	Real g[5];
	//	v.GetValues(g);
	//	bool valid = true;
	//	int i = 0;
	//	while (valid&& i < 5)
	//	{
	//		valid = g[i] > a;
	//		i++;
	//	}
	//	return valid;
	//}
}

#endif