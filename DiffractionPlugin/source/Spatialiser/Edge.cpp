
#include "Spatialiser/Edge.h"

using namespace Spatialiser;

Edge::Edge() : zW(0.0f), mBase(vec3(0, 0, 0)), mTop(vec3(0, 1, 0)), mFaceNormals{ vec3(1, 0, 0), vec3(0, 0, 1) }, rValid(false)
{
	InitEdge();
}

Edge::Edge(const vec3& base, const vec3& top, const vec3& normal1, const vec3& normal2, const size_t& ID1, const size_t ID2)
	: zW(0.0f), mBase(base), mTop(top), mFaceNormals{ normal1, normal2 }, mPlaneIDs{ ID1, ID2 }, rValid(false)
{
	InitEdge();
}

void Edge::InitEdge()
{
	midPoint = (mTop + mBase) / 2;
	mEdgeVector = UnitVector(mTop - mBase);
	mEdgeNormal = UnitVector(mFaceNormals[0] + mFaceNormals[1]);

	if (UnitVector(Cross(mFaceNormals[0], mFaceNormals[1])) == mEdgeVector) // case true: angle is reflex
		t = PI_1 + acosf(Dot(mFaceNormals[0], mFaceNormals[1]));
	else
		t = PI_1 - acosf(Dot(mFaceNormals[0], mFaceNormals[1]));

	UpdateEdgeLength();
}