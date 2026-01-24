/*
*
*  \Edge class
*
*/

// Common headers
#include "Common/Vec3.h"
#include "Common/Types.h"
#include "Common/Definitions.h"
#include "Common/Debug.h" 

// Spatialiser headers
#include "Spatialiser/Edge.h"
#include "Spatialiser/Types.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Edge class ////////////////////

		Edge::Edge(const Vec3& base, const Vec3& top, const Vec3& normal1, const Vec3& normal2, const size_t planeId1, const size_t planeId2)
			: zW(0.0f), mBase(base), mTop(top), mFaceNormals(normal1, normal2), mPlaneIds(planeId1, planeId2), mDs(REAL_CONST(0.0), REAL_CONST(0.0)), receiverZone(EdgeZone::Invalid)
		{
			Update();
		}

		void Edge::Update()
		{
			mEdgeVector = (mTop - mBase).Normalised();

			if (mFaceNormals.first == -mFaceNormals.second)
				mEdgeNormal = mEdgeVector.cross(mFaceNormals.first);
			else
				mEdgeNormal = (mFaceNormals.first + mFaceNormals.second).Normalised();

			// Offset slightly in direction of edge normal to avoid numerical issues when checking
			// coplanarity when planes are back to back
			midPoint = (mTop + mBase) / 2 + mEdgeNormal * REAL_CONST(10.0) * EPS_GENERAL;

			if (mFaceNormals.first.cross(mFaceNormals.second).dot(mEdgeVector) >= 0) // case true: angle is reflex
				t = PI_1 + acos(mFaceNormals.first.dot(mFaceNormals.second));
			else
				t = PI_1 - acos(mFaceNormals.first.dot(mFaceNormals.second));

			zW = (mTop - mBase).Normal();

			mDs.first = mFaceNormals.first.dot(mBase);
			mDs.second = mFaceNormals.second.dot(mBase);
		}

		void Edge::ReflectInPlane(const Plane& plane)
		{
			plane.ReflectPointInPlaneNoCheck(mBase);
			plane.ReflectPointInPlaneNoCheck(mTop);
			plane.ReflectNormalInPlane(mFaceNormals.first);
			plane.ReflectNormalInPlane(mFaceNormals.second);
			// Swap the normal order so that the external still travels from planeA to planeB (right hand rule)
			std::swap(mFaceNormals.first, mFaceNormals.second);
			Update();
		}

		EdgeZone Edge::FindEdgeZone(const Vec3& point) const
		{
			bool valid1 = point.dot(mFaceNormals.first) - mDs.first > 0;
			bool valid2 = point.dot(mFaceNormals.second) - mDs.second > 0;

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