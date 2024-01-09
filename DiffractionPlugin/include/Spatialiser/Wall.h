/*
*
*  \Wall class
*
*/

#ifndef Spatialiser_Wall_h
#define Spatialiser_Wall_h

// C++ headers
#include <vector>

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/Edge.h"

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// FrequencyDependence class ////////////////////

		class FrequencyDependence
		{
		public:

			// Load and Destroy
			FrequencyDependence() : low(1.0), midLow(1.0), mid(1.0), midHigh(1.0), high(1.0) {}
			FrequencyDependence(Real l, Real mL, Real m, Real mH, Real h) : low(l), midLow(mL), mid(m), midHigh(mH), high(h) {}
			~FrequencyDependence() {};

			// Getters
			inline void GetValues(Real* g) const { g[0] = low; g[1] = midLow; g[2] = mid; g[3] = midHigh; g[4] = high; };

			inline FrequencyDependence Log()
			{
				low = log(low);
				midLow = log(midLow);
				mid = log(mid);
				midHigh = log(midHigh);
				high = log(high);
				return *this;
			}

			// Operators
			inline FrequencyDependence operator+=(const Real& a)
			{
				low += a;
				midLow += a;
				mid += a;
				midHigh += a;
				high += a;
				return *this;
			}

			inline FrequencyDependence operator+=(const FrequencyDependence& v)
			{
				low += v.low;
				midLow += v.midLow;
				mid += v.mid;
				midHigh += v.midHigh;
				high += v.high;
				return *this;
			}

			inline FrequencyDependence operator-=(const Real& a)
			{
				low -= a;
				midLow -= a;
				mid -= a;
				midHigh -= a;
				high -= a;
				return *this;
			}

			inline FrequencyDependence operator*=(const Real& a)
			{
				low *= a;
				midLow *= a;
				mid *= a;
				midHigh *= a;
				high *= a;
				return *this;
			}

			inline FrequencyDependence operator*=(const FrequencyDependence& v)
			{
				low *= v.low;
				midLow *= v.midLow;
				mid *= v.mid;
				midHigh *= v.midHigh;
				high *= v.high;
				return *this;
			}

			inline FrequencyDependence operator/=(const Real& a)
			{
				low /= a;
				midLow /= a;
				mid /= a;
				midHigh /= a;
				high /= a;
				return *this;
			}

		protected:

			// Member variables
			Real low, midLow, mid, midHigh, high;

		};

		//////////////////// Operators ////////////////////

		inline FrequencyDependence operator+(const FrequencyDependence& v, const Real& a)
		{
			Real g[5];
			v.GetValues(g);
			return FrequencyDependence(a + g[0], a + g[1], a + g[2], a + g[3], a + g[4]);
		}

		inline FrequencyDependence operator-(const FrequencyDependence& v)
		{
			Real g[5];
			v.GetValues(g);
			return FrequencyDependence(-g[0], -g[1], -g[2], -g[3], -g[4]);
		}

		inline FrequencyDependence operator+(const Real& a, const FrequencyDependence& v)
		{
			return v + a;
		}

		inline FrequencyDependence operator-(const Real& a, const FrequencyDependence& v)
		{
			return -v + a;
		}

		inline FrequencyDependence operator-(const FrequencyDependence& v, const Real& a)
		{
			return -a + v;
		}

		inline FrequencyDependence operator+(const FrequencyDependence& v, const FrequencyDependence& u)
		{
			Real g1[5];
			Real g2[5];
			v.GetValues(g1);
			u.GetValues(g2);
			return FrequencyDependence(g1[0] + g2[0], g1[1] + g2[1], g1[2] + g2[2], g1[3] + g2[3], g1[4] + g2[4]);
		}

		inline FrequencyDependence operator*(const Real& a, const FrequencyDependence& f)
		{
			Real g[5];
			f.GetValues(g);
			return FrequencyDependence(a * g[0], a * g[1], a * g[2], a * g[3], a * g[4]);
		}

		inline FrequencyDependence operator*(const FrequencyDependence& f, const Real& a)
		{
			return a * f;
		}

		inline FrequencyDependence operator*(const FrequencyDependence& v, const FrequencyDependence& u)
		{
			Real g1[5];
			Real g2[5];
			v.GetValues(g1);
			u.GetValues(g2);
			return FrequencyDependence(g1[0] * g2[0], g1[1] * g2[1], g1[2] * g2[2], g1[3] * g2[3], g1[4] * g2[4]);
		}

		inline FrequencyDependence operator/(const Real& a, const FrequencyDependence& v)
		{
			Real g[5];
			v.GetValues(g);
			return FrequencyDependence(a / g[0], a / g[1], a / g[2], a / g[3], a / g[4]);
		}

		inline FrequencyDependence operator/(const FrequencyDependence& v, const Real& a)
		{
			return (1.0 / a) * v;
		}

		inline bool operator<(const FrequencyDependence& v, const Real& a)
		{
			Real g[5];
			v.GetValues(g);
			bool valid = true;
			int i = 0;
			while (valid && i < 5)
			{
				valid = g[i] < a;
				i++;
			}
			return valid;
		}

		inline bool operator>(const FrequencyDependence& v, const Real& a)
		{
			Real g[5];
			v.GetValues(g);
			bool valid = true;
			int i = 0;
			while (valid&& i < 5)
			{
				valid = g[i] > a;
				i++;
			}
			return valid;
		}

		//////////////////// Absorption class ////////////////////

		class Absorption : public FrequencyDependence // Stores sqrt(1 - R). Where R is the absortion property of the material in the pressure domain
		{												// Processing done in the energy domain
		public:

			// Load and Destroy
			Absorption() : FrequencyDependence(1.0, 1.0, 1.0, 1.0, 1.0), area(0.0) {}
			Absorption(Real l, Real mL, Real m, Real mH, Real h) : FrequencyDependence(sqrt(1.0 - l), sqrt(1.0 - mL), sqrt(1.0 - m), sqrt(1.0 - mH), sqrt(1.0 - h)), area(0.0) {}
			Absorption(Real l, Real mL, Real m, Real mH, Real h, Real _area) : FrequencyDependence(l, mL, m, mH, h), area(_area) {} // Is this correct. Is this used?
			~Absorption() {}

			// Operators
			inline Absorption operator=(const FrequencyDependence& v)
			{
				Real g[5];
				v.GetValues(g);
				low = g[0];
				midLow = g[1];
				mid = g[2];
				midHigh = g[3];
				high = g[4];
				return *this;
			}

			// Member variables
			Real area;

		private:
		};

		//////////////////// Operators ////////////////////

		inline Absorption operator+(const Absorption& a, const Absorption& b)
		{
			Real g1[5];
			Real g2[5];
			a.GetValues(g1);
			b.GetValues(g2);
			return Absorption(g1[0] + g2[0], g1[1] + g2[1], g1[2] + g2[2], g1[3] + g2[3], g1[4] + g2[4], a.area + b.area);
		}

		inline Absorption operator-(const Absorption& a, const Absorption& b)
		{
			Real g1[5];
			Real g2[5];
			a.GetValues(g1);
			b.GetValues(g2);
			return Absorption(g1[0] - g2[0], g1[1] - g2[1], g1[2] - g2[2], g1[3] - g2[3], g1[4] - g2[4], a.area - b.area);
		}

		inline Absorption operator*(const Real& a, const Absorption& f)
		{
			Real g[5];
			f.GetValues(g);
			return Absorption(a * g[0], a * g[1], a * g[2], a * g[3], a * g[4], f.area);
		}

		inline Absorption operator*(const Absorption& f, const Real& a)
		{
			return a * f;
		}

		//////////////////// Wall class ////////////////////

		class Wall
		{
		public:

			// Load and Destroy
			Wall() : d(0.0), rValid(false), mNumVertices(0), mAbsorption() {}
			Wall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption);
			~Wall() {}

			// Edges
			inline void AddEdge(const size_t& id) { mEdges.push_back(id); }
			inline void RemoveEdge(const size_t& id)
			{
				auto it = mEdges.begin();
				while (it != mEdges.end())
				{
					if (*it == id)
					{
						mEdges.erase(it);
						return;
					}
					else
					{
						it++;
					}
				}
			}

			// Getters
			inline vec3 GetNormal() const { return mNormal; }
			inline std::vector<vec3> GetVertices() const { return mVertices; }
			inline std::vector<size_t> GetEdges() const { return mEdges; }
			inline Real GetD() const { return d; }
			inline bool GetRValid() const { return rValid; }

			// Setters
			inline void SetRValid(const bool& valid) { rValid = valid; }

			// Geometry
			Real PointWallPosition(const vec3& point) const { return Dot(point, mNormal) - d; }
			bool LineWallIntersection(const vec3& start, const vec3& end) const;
			bool LineWallIntersection(vec3& intersection, const vec3& start, const vec3& end) const;
			bool ReflectPointInWall(const vec3& point) const;
			bool ReflectPointInWall(vec3& dest, const vec3& point) const;
			void ReflectPointInWallNoCheck(vec3& point) const;
			bool ReflectEdgeInWall(const Edge& edge) const;

			// Absorption
			Absorption Update(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption);
			Absorption GetAbsorption() { return mAbsorption; }
			Real GetArea() { return mAbsorption.area; }

		private:

			// Update
			void Update(const Real* vData);

			// Area
			void CalculateArea();
			Real AreaOfTriangle(const vec3& v, const vec3& u, const vec3& w);

			// Member variables
			Real d;
			bool rValid;
			vec3 mNormal;
			std::vector<vec3> mVertices;
			std::vector<size_t> mEdges;
			size_t mNumVertices;
			Absorption mAbsorption;
		};
	}
}
#endif