#pragma once

#include <iostream>
#include <vector>
#include <math.h>

#pragma region vec3
class vec3
{
public:
	vec3(const float x_ = 0, const float y_ = 0, const float z_ = 0) : x(x_), y(y_), z(z_) {}
	~vec3() {}

	float Length() { return sqrtf(x * x + y * y + z * z); }

	// add another vector to a vector like a += v;
	inline vec3 operator+=(const vec3& v)
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		return *this;
	}

	/*inline bool operator==(const vec3& v)
	{
		if (this->x == v.x);
		{
			if (this->y == v.y);
			{
				if (this->z == v.z);
					return true;
			}
		}
		return false;
	}*/

	float x;
	float y;
	float z;

private:
};

inline bool operator==(const vec3& a, const vec3& b)
{
	if (a.x == b.x)
	{
		if (a.y == b.y)
		{
			if (a.z == b.z)
				return true;
		}
	}
	return false;
}

inline bool operator!=(const vec3& a, const vec3& b)
{
	if (a == b)
		return false;
	return true;
}

// add two vectors together like vec3 c = a + b;
inline vec3 operator+(const vec3& a, const vec3& b)
{
	return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline vec3 operator-(const vec3& a)
{
	return vec3(-a.x, -a.y, -a.z);
}

inline vec3 operator-(const vec3& a, const vec3& b)
{
	return -b + a;
}

// multiply a vector with a scalar like vec3 a = 3.5f * v; // this would need a second overload with the other order so vec3 a = v * 3.5f; would work as well, e.g. operator*( vec3 , float )
inline vec3 operator*(const float f, const vec3& v)
{
	return vec3(f * v.x, f * v.y, f * v.z);
}

inline vec3 operator*(const vec3& v, const float f)
{
	return f * v;
}

inline vec3 operator/(const vec3& v, const float f)
{
	return vec3(v.x / f, v.y / f, v.z / f);
}

inline vec3 operator/(const float f, const vec3& v)
{
	return v / (1 / f);
}

// print the vec3 directly using std::cout << vec3 << std::endl;
inline std::ostream& operator<<(std::ostream& os, const vec3& v)
{
	os << "[ " << v.x << " , " << v.y << " , " << v.z << " ]";
	return os;
}

inline vec3 UnitVector(vec3 v)
{
	float len = v.Length();
	if (len == 0)
	{
		return vec3(0.0f, 0.0f, 0.0f);
	}
	return v / len;
}

inline float Dot(vec3 v, vec3 u)
{
	return v.x * u.x + v.y * u.y + v.z * u.z;
}

inline vec3 Cross(vec3 v, vec3 u)
{
	return vec3(v.y * u.z - v.z * u.y,
		v.z * u.x - v.x * u.z,
		v.x * u.y - v.y * u.x);
}
#pragma endregion