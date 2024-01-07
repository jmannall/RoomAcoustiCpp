/*
*
*  \Edge class
*  \faceNormals defined using right hand curl rule that rotates from plane 0 to plane 1 through the exterior wedge.
*
*/

#ifndef Spatialiser_Edge_h
#define Spatialiser_Edge_h

// Common headers
#include "Common/Vec3.h"
#include "Common/Types.h"

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Edge class ////////////////////

		class Edge
		{
		public:

			// Load and Destroy
			Edge();
			Edge(const vec3& base, const vec3& top, const vec3& normal1, const vec3& normal2, const size_t& ID1, const size_t ID2);
			~Edge() {};

			// Edge
			void InitEdge();
			void UpdateEdgeLength() { zW = (mTop - mBase).Length(); }

			// Getters
			vec3 GetAP(vec3 point) const { return point - mBase; }
			vec3 GetEdgeCoord(Real z) const { return mBase + z * mEdgeVector; }
			vec3 GetBase() const { return mBase; }
			vec3 GetTop() const { return mTop; }
			vec3 GetMidPoint() const { return midPoint; }
			size_t GetWallID(const size_t& id) const
			{
				if (id == mPlaneIDs[0])
					return mPlaneIDs[1];
				else
					return mPlaneIDs[0];
			}
			bool AttachedToPlane(size_t id) const
			{
				if (id == mPlaneIDs[0] || id == mPlaneIDs[1])
					return true;
				return false;
			}

			//inline bool GetRValid() const { return rValid; }
			//inline void SetRValid(const vec3& receiver)
			//{
			//	// Check receiver in front of either plane (in front of both defo not in shadow zone)
			//	// Be easy if save wall to edge - currently check path is valid in ISM anyway which performs similar checks
			//	// This way maybe quicker? as avoids initialising a path if unnessesary?
			//}

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
			vec3 mFaceNormals[2];
			size_t mPlaneIDs[2];
		};
	}
}

#endif