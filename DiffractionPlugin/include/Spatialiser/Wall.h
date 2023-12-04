#pragma once

#include "vec3.h"
#include "Spatialiser/Edge.h"
#include "UnityGAPlugin.h"
#include "Debug.h"

namespace Spatialiser
{
	class FrequencyDependence
	{
	public:
		FrequencyDependence() : low(1.0f), midLow(1.0f), mid(1.0f), midHigh(1.0f), high(1.0f) {}
		FrequencyDependence(float l, float mL, float m, float mH, float h) : low(l), midLow(mL), mid(m), midHigh(mH), high(h) {}

		inline void GetValues(float* g) const { g[0] = low; g[1] = midLow; g[2] = mid; g[3] = midHigh; g[4] = high; };
	
		inline FrequencyDependence operator+=(const float& a)
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

		inline FrequencyDependence operator-=(const float& a)
		{
			low -= a;
			midLow -= a;
			mid -= a;
			midHigh -= a;
			high -= a;
			return *this;
		}

		inline FrequencyDependence operator*=(const float& a)
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

		inline FrequencyDependence operator/=(const float& a)
		{
			low /= a;
			midLow /= a;
			mid /= a;
			midHigh /= a;
			high /= a;
			return *this;
		}

		inline FrequencyDependence Log()
		{
			low = log(low);
			midLow = log(midLow);
			mid = log(mid);
			midHigh = log(midHigh);
			high = log(high);
			return *this;
		}

	protected:
		float low, midLow, mid, midHigh, high;

	};

	inline FrequencyDependence operator+(const FrequencyDependence& f, const float& a)
	{
		float g[5];
		f.GetValues(g);
		return FrequencyDependence(a + g[0], a + g[1], a + g[2], a + g[3], a + g[4]);
	}

	inline FrequencyDependence operator-(const float& a, const FrequencyDependence& f)
	{
		float g[5];
		f.GetValues(g);
		return FrequencyDependence(a - g[0], a - g[1], a - g[2], a - g[3], a - g[4]);
	}

	inline FrequencyDependence operator+(const FrequencyDependence& v, const FrequencyDependence& u)
	{
		float g1[5];
		float g2[5];
		v.GetValues(g1);
		u.GetValues(g2);
		return FrequencyDependence(g1[0] + g2[0], g1[1] + g2[1], g1[2] + g2[2], g1[3] + g2[3], g1[4] + g2[4]);
	}

	inline FrequencyDependence operator*(const float& a, const FrequencyDependence& f)
	{
		float g[5];
		f.GetValues(g);
		return FrequencyDependence(a * g[0], a * g[1], a * g[2], a * g[3], a * g[4]);
	}

	inline FrequencyDependence operator*(const FrequencyDependence& f, const float& a)
	{
		return a * f;
	}

	inline FrequencyDependence operator*(const FrequencyDependence& v, const FrequencyDependence& u)
	{
		float g1[5];
		float g2[5];
		v.GetValues(g1);
		u.GetValues(g2);
		return FrequencyDependence(g1[0] * g2[0], g1[1] * g2[1], g1[2] * g2[2], g1[3] * g2[3], g1[4] * g2[4]);
	}

	inline FrequencyDependence operator/(const float& a, const FrequencyDependence& f)
	{
		float g[5];
		f.GetValues(g);
		return FrequencyDependence(a / g[0], a / g[1], a / g[2], a / g[3], a / g[4]);
	}

	inline FrequencyDependence operator/(const FrequencyDependence& f, const float& a)
	{
		float g[5];
		f.GetValues(g);
		return FrequencyDependence(g[0] / a, g[1] / a, g[2] / a, g[3] / a, g[4] / a);
	}

	inline bool operator<(const FrequencyDependence& f, const float& a)
	{
		float g[5];
		f.GetValues(g);
		bool valid = true;
		int i = 0;
		while (valid && i < 5)
		{
			valid = g[i] < a;
			i++;
		}
		return valid;
	}

	inline bool operator>(const FrequencyDependence& f, const float& a)
	{
		float g[5];
		f.GetValues(g);
		bool valid = true;
		int i = 0;
		while (valid && i < 5)
		{
			valid = g[i] > a;
			i++;
		}
		return valid;
	}

	class Absorption : public FrequencyDependence // Stores sqrt(1 - R). Where R is the absortion property of the material in the pressure domain
	{												// Processing done in the energy domain
	public:
		Absorption() : FrequencyDependence(1.0f, 1.0f, 1.0f, 1.0f, 1.0f), area(0.0f) {}
		Absorption(float l, float mL, float m, float mH, float h) : FrequencyDependence(sqrtf(1.0f - l), sqrtf(1.0f - mL), sqrtf(1.0f - m), sqrtf(1.0f - mH), sqrtf(1.0f - h)), area(0.0f) {}
		Absorption(float l, float mL, float m, float mH, float h, float _area) : FrequencyDependence(l, mL, m, mH, h), area(_area) {} // Is this correct. Is this used?
		~Absorption() {}

		inline Absorption operator=(const FrequencyDependence& v)
		{
			float g[5];
			v.GetValues(g);
			low = g[0];
			midLow = g[1];
			mid = g[2];
			midHigh = g[3];
			high = g[4];
			return *this;
		}

		float area;
	private:
		
	};

	inline Absorption operator+(const Absorption& a, const Absorption& b)
	{
		float g1[5];
		float g2[5];
		a.GetValues(g1);
		b.GetValues(g2);
		return Absorption(g1[0] + g2[0], g1[1] + g2[1], g1[2] + g2[2], g1[3] + g2[3], g1[4] + g2[4], a.area + b.area);
	}

	inline Absorption operator-(const Absorption& a, const Absorption& b)
	{
		float g1[5];
		float g2[5];
		a.GetValues(g1);
		b.GetValues(g2);
		return Absorption(g1[0] - g2[0], g1[1] - g2[1], g1[2] - g2[2], g1[3] - g2[3], g1[4] - g2[4], a.area - b.area);
	}

	inline Absorption operator*(const float& a, const Absorption& f)
	{
		float g[5];
		f.GetValues(g);
		return Absorption(a * g[0], a * g[1], a * g[2], a * g[3], a * g[4], f.area);
	}

	inline Absorption operator*(const Absorption& f, const float& a)
	{
		return a * f;
	}

	class Wall
	{
	public:
		Wall() : d(0.0f), rValid(false), mNumVertices(0), mAbsorption() {}
		Wall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption);
		~Wall() {}

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
		inline vec3 GetNormal() const { return mNormal; }
		inline std::vector<vec3> GetVertices() const { return mVertices; }
		inline std::vector<size_t> GetEdges() const { return mEdges; }
		inline float GetD() const { return d; }
		inline bool GetRValid() const { return rValid; }
		inline void SetRValid(const bool& valid) { rValid = valid; }

		float PointWallPosition(const vec3& point) const { return Dot(point, mNormal) - d; }
		bool LineWallIntersection(const vec3& start, const vec3& end) const;
		bool LineWallIntersection(vec3& intersection, const vec3& start, const vec3& end) const;
		bool ReflectPointInWall(const vec3& point) const;
		bool ReflectPointInWall(vec3& dest, const vec3& point) const;
		bool ReflectEdgeInWall(const Edge& edge) const;

		Absorption Update(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption);

		Absorption GetAbsorption() { return mAbsorption; }
		float GetArea() { return mAbsorption.area; }

	private:
		void Update(const float* vData);

		void CalculateArea();
		float AreaOfTriangle(const vec3& v, const vec3& u, const vec3& w);
		float d;
		bool rValid;
		vec3 mNormal;
		std::vector<vec3> mVertices;
		std::vector<size_t> mEdges;
		size_t mNumVertices;
		Absorption mAbsorption;
	};

}