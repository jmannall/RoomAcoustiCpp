
#include "Spatialiser/Wall.h""

using namespace Spatialiser;

Wall::Wall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption) : mNormal(normal), rValid(false), mNumVertices(numVertices), mAbsorption(absorption)
{
	Update(vData);
	absorption.area = mAbsorption.area;
	Debug::Log("Vertecies: " + VecArrayToStr(mVertices), Color::Orange);
	Debug::Log("Normal: " + VecToStr(mNormal), Color::Orange);
}

Absorption Wall::Update(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption)
{
	Absorption oldAbsorption = mAbsorption;

	mNormal = vec3(Round(normal.x, 4), Round(normal.y, 4), Round(normal.z, 4));
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
		// Round as otherwise comparing identical vertices from unity still returns false
		float x = Round(vData[j++], 4);
		float y = Round(vData[j++], 4);
		float z = Round(vData[j++], 4);
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

bool Wall::LineWallIntersection(const vec3& start, const vec3& end) const
{
	vec3 intersection;
	return LineWallIntersection(intersection, start, end);
}

bool Wall::LineWallIntersection(vec3& intersection, const vec3& start, const vec3& end) const
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
	double angleRot = 0.0f;
	float angleRotF = 0.0f;
	for (int i = 0; i < mNumVertices; i++)
	{
		int idx = (i + 1) % mNumVertices;
		vec3 one = intersection - mVertices[i];
		vec3 two = intersection - mVertices[idx];
		float dotProduct = Dot(mNormal, Cross(one, two));
		angleRot += (double)Sign(dotProduct) * acos((double)Dot(one, two) / ((double)one.Length() * (double)two.Length()));
		angleRotF += Sign(dotProduct) * acosf(Dot(one, two) / (one.Length() * two.Length()));
	}
	//if (angleRotF == PI_2)
	//	return true;
	//return false;
	float eps = 0.00001f;
	if (angleRotF > PI_2 + eps)
		return false;
	else if (angleRotF < PI_2 - eps)
		return false;
	else
		return true;
}

bool Wall::ReflectPointInWall(const vec3& point) const
{
	float k = PointWallPosition(point);
	if (k > 0) // Check source in front of plane
		return true;
	return false;
}

bool Wall ::ReflectPointInWall(vec3& dest, const vec3& point) const
{
	float k = PointWallPosition(point);
	if (k > 0) // Check source in front of plane
	{
		dest = point - 2 * mNormal * k;
		return true;
	}
	return false;
}

void Wall::ReflectPointInWallNoCheck(vec3& point) const
{
	float k = PointWallPosition(point);
	point += -2 * mNormal * k;
}

bool Wall::ReflectEdgeInWall(const Edge& edge) const
{
	bool valid = ReflectPointInWall(edge.GetEdgeCoord(EPS)); // Prevents false in case edge base is coplanar
	if (valid)
		valid = ReflectPointInWall(edge.GetEdgeCoord(edge.zW - EPS));
	return valid;
}

#pragma endregion
