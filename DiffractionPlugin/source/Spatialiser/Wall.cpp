
#include "Spatialiser/Wall.h""

using namespace Spatialiser;

Wall::Wall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption) : mNormal(normal), rValid(false), mNumVertices(numVertices), mAbsorption(absorption)
{
	Debug::Log("Vertecies: " + VecArrayToStr(mVertices), Color::Orange);
	Debug::Log("Normal: " + VecToStr(mNormal), Color::Orange);
	Update(vData);
	absorption.area = mAbsorption.area;
}

Absorption Wall::Update(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption)
{
	Absorption oldAbsorption = mAbsorption;
	mNormal = normal;
	mNumVertices = numVertices;
	mAbsorption = absorption;
	Update(vData);
	absorption.area = mAbsorption.area;
	return oldAbsorption;
}
void Wall::Update(const float* vData)
{
	int j = 0;
	mVertices.reserve(mNumVertices);
	std::fill_n(std::back_inserter(mVertices), mNumVertices, vec3());
	for (int i = 0; i < (int)mNumVertices; i++)
	{
		float x = vData[j++];
		float y = vData[j++];
		float z = vData[j++];
		mVertices[i] = vec3(x, y, z);
	}

	d = Dot(mNormal, mVertices[0]);
	CalculateArea();
}

void Wall::CalculateArea()
{
	mAbsorption.area = 0;
	for (int i = 0; i < mNumVertices - 1; i++)
	{
		mAbsorption.area += AreaOfTriangle(mVertices[0], mVertices[i], mVertices[i + 1]);
	}
}

float Wall::AreaOfTriangle(const vec3& v, const vec3& u, const vec3& w)
{
	return 0.5f * (Cross(v - u, v - w).Length());
}

#pragma region Geometry Checks

bool Wall::LineWallIntersection(const vec3& start, const vec3& end)
{
	vec3 intersection;
	return LineWallIntersection(intersection, start, end);
}

bool Wall::LineWallIntersection(vec3& intersection, const vec3& start, const vec3& end)
{
	vec3 grad = start - end;
	float scale = Dot(mNormal, grad);
	float k = Dot(start, mNormal) - d;
	intersection = start - grad * k / scale;

	// Check intersection lies in line segment
	vec3 grad2 = start - intersection;
	float scale2 = Dot(mNormal, grad2);
	if (scale2 > scale)
	{
		if (scale2 > 0)
			return false;
	}
	else if (scale2 < 0)
		return false;

	// Check intersection lies within plane
	float angleRot = 0.0f;
	for (int i = 0; i < mNumVertices; i++)
	{
		int idx = (i + 1) % mNumVertices;
		vec3 one = intersection - mVertices[i];
		vec3 two = intersection - mVertices[idx];
		float dotProduct = Dot(mNormal, Cross(one, two));
		angleRot += Sign(dotProduct) * acosf(Dot(one, two) / (one.Length() * two.Length()));
	}
	float eps = 0.001f;
	if (angleRot > PI_2 + eps)
		return false;
	else if (angleRot < PI_2 - eps)
		return false;
	else
		return true;
}

bool Wall::ReflectPointInWall(const vec3& point)
{
	vec3 dest;
	return ReflectPointInWall(dest, point);
}

bool Wall ::ReflectPointInWall(vec3& dest, const vec3& point)
{
	float k = PointWallPosition(point);
	if (k > 0) // Check source in front of plane
	{
		dest = point - 2 * mNormal * k;
		return true;
	}
	return false;
}

#pragma endregion
