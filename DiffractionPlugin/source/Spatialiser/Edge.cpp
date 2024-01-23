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

namespace UIE
{
	using namespace Unity;
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Edge class ////////////////////

		Edge::Edge() : zW(0.0), mBase(vec3(0.0, 0.0, 0.0)), mTop(vec3(0.0, 1.0, 0.0)), mFaceNormals{ vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0) }
		{
			InitEdge();
		}

		Edge::Edge(const vec3& base, const vec3& top, const vec3& normal1, const vec3& normal2, const size_t& ID1, const size_t ID2)
			: zW(0.0f), mBase(base), mTop(top), mFaceNormals{ normal1, normal2 }, mPlaneIDs{ ID1, ID2 }
		{
			InitEdge();
		}

		void Edge::InitEdge()
		{
#if DEBUG_INIT
	Debug::Log("Edge init", Colour::Green);
#endif

			midPoint = (mTop + mBase) / 2;
			mEdgeVector = UnitVector(mTop - mBase);
			mEdgeNormal = UnitVector(mFaceNormals[0] + mFaceNormals[1]);

			if (UnitVector(Cross(mFaceNormals[0], mFaceNormals[1])) == mEdgeVector) // case true: angle is reflex
				t = PI_1 + acos(Dot(mFaceNormals[0], mFaceNormals[1]));
			else
				t = PI_1 - acos(Dot(mFaceNormals[0], mFaceNormals[1]));

#if DEBUG_INIT
	Debug::Log("Edge angle: " + RealToStr(t), Colour::Orange);
#endif
			UpdateEdgeLength();
		}
	}
}