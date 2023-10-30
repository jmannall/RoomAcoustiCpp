#pragma once

#include "vec3.h"
#include "Definitions.h"

namespace Spatialiser
{
	// faceNormals defined using right hand curl rule that rotates from plane 0 to plane 1 through the exterior wedge.
	class Edge
	{
	public:
		Edge();
		Edge(const vec3& base, const vec3& top, const vec3& normal1, const vec3& normal2, const size_t& ID1, const size_t ID2);
		~Edge() {};

		void InitEdge();
		void UpdateEdgeLength() { zW = (mTop - mBase).Length(); }
		vec3 GetAP(vec3 point) const { return point - mBase; }
		vec3 GetEdgeCoord(float z) const { return mBase + z * mEdgeVector; }
		vec3 GetMidPoint() const { return midPoint; }
		size_t GetWallID(const size_t& id) const
		{
			if (id == mPlaneIDs[0])
				return mPlaneIDs[1];
			else
				return mPlaneIDs[0];
		}
		inline bool GetRValid() const { return rValid; }
		inline void SetRValid(const bool& valid) { rValid = valid; }

		float t;
		float zW;
		vec3 mEdgeVector;
		vec3 mEdgeNormal;

	private:
		bool rValid;
		vec3 mBase;
		vec3 mTop;
		vec3 midPoint;
		vec3 mFaceNormals[2];
		size_t mPlaneIDs[2];
	};
}