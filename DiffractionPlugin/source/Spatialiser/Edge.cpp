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

		Edge::Edge() : zW(0.0), mBase(vec3(0.0, 0.0, 0.0)), mTop(vec3(0.0, 1.0, 0.0)), mFaceNormals{ vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0) }, mWallIds{ 0, 0 }
		{
			Update();
		}

		Edge::Edge(const EdgeData& data) : zW(0.0), mBase(data.base), mTop(data.top),
			mFaceNormals{ data.normal1, data.normal2 }, mWallIds{ data.id1, data.id2 }
		{
			Update();
		}

		Edge::Edge(const vec3& base, const vec3& top, const vec3& normal1, const vec3& normal2, const size_t id1, const size_t id2)
			: zW(0.0f), mBase(base), mTop(top), mFaceNormals{ normal1, normal2 }, mWallIds{ id1, id2 }
		{
			Update();
		}

		void Edge::Update()
		{
			midPoint = (mTop + mBase) / 2;
			mEdgeVector = UnitVectorRound(mTop - mBase);
			mEdgeNormal = UnitVectorRound(mFaceNormals[0] + mFaceNormals[1]);

			if (Dot(Cross(mFaceNormals[0], mFaceNormals[1]), mEdgeVector) > 0) // case true: angle is reflex
				t = PI_1 + acos(Dot(mFaceNormals[0], mFaceNormals[1]));
			else
				t = PI_1 - acos(Dot(mFaceNormals[0], mFaceNormals[1]));

			zW = (mTop - mBase).Length();
		}
	}
}