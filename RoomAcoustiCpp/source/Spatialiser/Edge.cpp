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

		EdgeData::EdgeData(const Edge& edge)
		{
			base = edge.GetBase();
			top = edge.GetTop();
			normal1 = edge.GetFaceNormal(0);
			normal2 = edge.GetFaceNormal(1);
			std::vector<size_t> ids = edge.GetWallIDs();
			id1 = ids[0];
			id2 = ids[1];
		}

		//////////////////// Edge class ////////////////////

		Edge::Edge(const EdgeData& data) : zW(0.0), mBase(data.base), mTop(data.top),
			mFaceNormals{ data.normal1, data.normal2 }, mWallIds{ data.id1, data.id2 }, mDs(2), rZone(EdgeZone::Invalid)
		{
			Update();
		}

		Edge::Edge(const vec3& base, const vec3& top, const vec3& normal1, const vec3& normal2, const size_t id1, const size_t id2)
			: zW(0.0f), mBase(base), mTop(top), mFaceNormals{ normal1, normal2 }, mWallIds{ id1, id2 }, mDs(2), rZone(EdgeZone::Invalid)
		{
			Update();
		}

		void Edge::Update()
		{
			midPoint = (mTop + mBase) / 2;
			mEdgeVector = UnitVectorRound(mTop - mBase);

			if (mFaceNormals[0] == -mFaceNormals[1])
				mEdgeNormal = Cross(mEdgeVector, mFaceNormals[0]);
			else
				mEdgeNormal = UnitVectorRound(mFaceNormals[0] + mFaceNormals[1]);

			if (Dot(Cross(mFaceNormals[0], mFaceNormals[1]), mEdgeVector) >= 0) // case true: angle is reflex
				t = PI_1 + acos(Dot(mFaceNormals[0], mFaceNormals[1]));
			else
				t = PI_1 - acos(Dot(mFaceNormals[0], mFaceNormals[1]));

			zW = (mTop - mBase).Length();

			mDs[0] = Dot(mFaceNormals[0], mBase);
			mDs[1] = Dot(mFaceNormals[1], mBase);
		}

		void Edge::ReflectInPlane(const Plane& plane)
		{
			plane.ReflectPointInPlaneNoCheck(mBase);
			plane.ReflectPointInPlaneNoCheck(mTop);
			plane.ReflectNormalInPlane(mFaceNormals[0]);
			plane.ReflectNormalInPlane(mFaceNormals[1]);
			// Swap the normal order so that the external still travels from planeA to planeB (right hand rule)
			vec3 store = mFaceNormals[0];
			mFaceNormals[0] = mFaceNormals[1];
			mFaceNormals[1] = store;
			Update();
		}

		EdgeZone Edge::FindEdgeZone(const vec3& point) const
		{
			bool valid1 = Dot(point, mFaceNormals[0]) - mDs[0] > 0;
			bool valid2 = Dot(point, mFaceNormals[1]) - mDs[1] > 0;

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