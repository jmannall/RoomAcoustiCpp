/*
* @class Edge
*
* @brief Declaration of Edge class
*
* @faceNormals defined using right hand curl rule that rotates from plane 0 to plane 1 through the exterior wedge.
*
*/

#ifndef RoomAcoustiCpp_Edge_h
#define RoomAcoustiCpp_Edge_h

// C++ headers
#include <vector>

// Common headers
#include "Common/Vec3.h"
#include "Common/Types.h"

// Spatialiser headers
#include "Spatialiser/Wall.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		class Edge;
		class Plane;

		struct EdgeData
		{
			vec3 base, top, normal1, normal2;
			size_t id1, id2;
			EdgeData(const vec3& _base, const vec3& _top, const vec3& _normal1, const vec3& _normal2, const size_t _id1, const size_t _id2)
				: base(_base), top(_top), normal1(_normal1), normal2(_normal2), id1(_id1), id2(_id2) {};

			EdgeData(const Edge& edge);
		};

		enum EdgeZone
		{
			NonShadowed,
			CanBeShadowed,
			Invalid
		};

		//////////////////// Edge class ////////////////////

		class Edge
		{
		public:

			// Load and Destroy
			Edge() : zW(0.0), t(0.0), mDs(2), rZone(EdgeZone::Invalid) {}
			Edge(const EdgeData& data);
			Edge(const vec3& base, const vec3& top, const vec3& normal1, const vec3& normal2, const size_t id1, const size_t id2);
			~Edge() {};

			// Edge
			void Update();
			void ReflectInPlane(const Plane& plane);

			// Getters
			inline vec3 GetAP(const vec3& point) const { return point - mBase; }
			inline vec3 GetEdgeCoord(Real z) const { return mBase + z * mEdgeVector; }
			inline vec3 GetBase() const { return mBase; }
			inline vec3 GetTop() const { return mTop; }
			inline vec3 GetMidPoint() const { return midPoint; }
			inline size_t GetWallID(const size_t id) const
			{
				if (id == mWallIds[0])
					return mWallIds[1];
				else
					return mWallIds[0];
			}
			inline std::vector<size_t> GetWallIDs() const { return mWallIds; }
			inline vec3 GetFaceNormal(const size_t i) const { return mFaceNormals[i]; }

			inline bool AttachedToPlane(const std::vector<size_t>& ids) const
			{
				for (size_t id : ids)
				{
					if (id == mWallIds[0] || id == mWallIds[1])
						return true;
				}
				return false;
			}

			void Update(const EdgeData& data)
			{
				mBase = data.base;
				mTop = data.top;
				mFaceNormals[0] = data.normal1;
				mFaceNormals[1] = data.normal2;
				Update();
			}

			inline void SetRZone(const EdgeZone zone) { rZone = zone; }
			inline EdgeZone GetRZone() const { return rZone; }

			EdgeZone FindEdgeZone(const vec3& point) const;

			// Edge parameters
			Real t;
			Real zW;
			vec3 mEdgeVector;
			vec3 mEdgeNormal;

		private:
			//bool rValid;
			vec3 midPoint;

			// Edge data
			vec3 mBase;
			vec3 mTop;
			std::vector<vec3> mFaceNormals;
			std::vector<Real> mDs;
			std::vector<size_t> mWallIds;

			EdgeZone rZone;
		};
	}
}

#endif