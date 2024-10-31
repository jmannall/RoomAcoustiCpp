/*
*
*  \Edge class
*
*/

// Common headers
#include "Common/Vec3.h"
#include "Common/Types.h"
#include "Common/Definitions.h"

// Unity headers
#include "Unity/Debug.h" 

// Spatialiser headers
#include "Spatialiser/Edge.h"
#include "Spatialiser/Types.h"

namespace RAC
{
	using namespace Unity;
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Edge class ////////////////////

		Edge::Edge(const vec3& base, const vec3& top, const vec3& normal1, const vec3& normal2, const size_t wallId1, const size_t wallId2, const size_t planeId1, const size_t planeId2)
			: zW(0.0f), mBase(base), mTop(top), mFaceNormals(normal1, normal2), mWallIds(wallId1, wallId2), mPlaneIds(planeId1, planeId2), mDs(0.0, 0.0), rZone(EdgeZone::Invalid)
		{
			Update();
		}

		void Edge::Update()
		{
			midPoint = (mTop + mBase) / 2;
			mEdgeVector = UnitVector(mTop - mBase);

			if (mFaceNormals.first == -mFaceNormals.second)
				mEdgeNormal = Cross(mEdgeVector, mFaceNormals.first);
			else
				mEdgeNormal = UnitVector(mFaceNormals.first + mFaceNormals.second);

			Real test1 = Dot(mFaceNormals.first, mFaceNormals.second);
			Real test2 = acos(test1);

			if (Dot(Cross(mFaceNormals.first, mFaceNormals.second), mEdgeVector) >= 0) // case true: angle is reflex
				t = PI_1 + acos(Dot(mFaceNormals.first, mFaceNormals.second));
			else
				t = PI_1 - acos(Dot(mFaceNormals.first, mFaceNormals.second));

			zW = (mTop - mBase).Length();

			mDs.first = Dot(mFaceNormals.first, mBase);
			mDs.second = Dot(mFaceNormals.second, mBase);
		}

		void Edge::ReflectInPlane(const Plane& plane)
		{
			plane.ReflectPointInPlaneNoCheck(mBase);
			plane.ReflectPointInPlaneNoCheck(mTop);
			plane.ReflectNormalInPlane(mFaceNormals.first);
			plane.ReflectNormalInPlane(mFaceNormals.second);
			// Swap the normal order so that the external still travels from planeA to planeB (right hand rule)
			vec3 store = mFaceNormals.first;
			mFaceNormals.first = mFaceNormals.second;
			mFaceNormals.second = store;
			Update();
		}

		EdgeZone Edge::FindEdgeZone(const vec3& point) const
		{
			bool valid1 = Dot(point, mFaceNormals.first) - mDs.first > 0;
			bool valid2 = Dot(point, mFaceNormals.second) - mDs.second > 0;

			if (valid1 == valid2)
				if (valid1)
					return EdgeZone::NonShadowed;
				else
					return EdgeZone::Invalid;
			else
				return EdgeZone::CanBeShadowed;
		}
	}
}