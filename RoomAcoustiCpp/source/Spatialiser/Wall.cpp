/*
* @class Wall, Plane
*
* @brief Declaration of Wall and Plane classes
*
*/

// Common headers
#include "Common/Definitions.h"

// Unity headers
#include "Unity/Debug.h"

// Spatialiser headers
#include "Spatialiser/Wall.h"

namespace RAC
{
	using namespace Common;
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// Plane Class ////////////////////

		////////////////////////////////////////

		bool Plane::ReflectPointInPlane(const Vec3& point) const
		{
			Real k = PointPlanePosition(point);
			if (k > 0) // Check source in front of plane
				return true;
			return false;
		}

		////////////////////////////////////////

		bool Plane::ReflectPointInPlane(Vec3& dest, const Vec3& point) const
		{
			Real k = PointPlanePosition(point);
			if (k > 0) // Check source in front of plane
			{
				dest = point - 2.0 * mNormal * k;
				return true;
			}
			return false;
		}

		////////////////////////////////////////

		void Plane::ReflectPointInPlaneNoCheck(Vec3& point) const
		{
			Real k = PointPlanePosition(point);
			point += -2.0 * mNormal * k;
		}

		////////////////////////////////////////

		void Plane::ReflectNormalInPlane(Vec3& normal) const
		{
			normal += -2.0 * mNormal * Dot(normal, mNormal);
		}

		////////////////////////////////////////

		bool Plane::EdgePlanePosition(const Edge& edge) const
		{
			bool valid = ReflectPointInPlane(edge.GetEdgeCoord(EPS)); // Prevents false in case edge base is coplanar
			if (valid)
				valid = ReflectPointInPlane(edge.GetEdgeCoord(edge.GetLength() - EPS));

			return valid;
		}

		////////////////////////////////////////

		bool Plane::LinePlaneObstruction(const Vec3& start, const Vec3& end) const
		{
			Real kS = PointPlanePosition(start);
			Real kE = PointPlanePosition(end);

			return kS * kE < 0;	// point lies on plane when kS || kE == 0. Therefore, not obstructed
		}

		////////////////////////////////////////

		bool Plane::LinePlaneIntersection(const Vec3& start, const Vec3& end) const
		{
			Real kS = PointPlanePosition(start);
			Real kE = PointPlanePosition(end);
			/*
			* In the unlikely case intersection points of a reflection between two adjacent planes
			* lies on the edge between the two planes, kS or kE will equal zero. Therefore, for this
			* case to be a valid reflection a point lying on a plane must be considered as intersecting
			* with the plane.
			*/
			return kS * kE <= 0;	// point lies on plane when kS || kE == 0.
		}

		//////////////////// Wall utility functions ////////////////////

		////////////////////////////////////////

		std::pair<bool, Vec3> IntersectTriangle(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& origin, const Vec3& dir, const bool returnIntersection)
		{
			Vec3 E1 = v2 - v1;
			Vec3 E2 = v3 - v1;
			Vec3 pVec = Cross(dir, E2);
			Real det = Dot(E1, pVec);

			if (det > -MIN_VALUE && det < MIN_VALUE)
				return { false, Vec3() };    // This ray is parallel to this triangle.

			Real invdet = 1.0 / det;

			Vec3 tVec = origin - v1;
			Real u = Dot(tVec, pVec) * invdet;

			if (u < 0.0 || u > 1.0)
				return { false, Vec3() };

			Vec3 qVec = Cross(tVec, E1);
			Real v = Dot(dir, qVec) * invdet;

			if (v < 0.0 || u + v > 1.0)
				return { false, Vec3() };

			if (returnIntersection)
				// return { true, Dot(E2, qVec) * invdet };
				return { true, v1 + E1 * u + E2 * v };
			else
				return { true, Vec3() };
		}

		//////////////////// Wall Class ////////////////////

		////////////////////////////////////////

		Wall::Wall(const Vertices& vData, const Absorption& absorption) : mPlaneId(0), mAbsorption(absorption)
		{
			Update(vData);

#ifdef DEBUG_WALL
	Debug::Log("Vertices: " + VerticesToStr(mVertices), Colour::Orange);
	Debug::Log("Normal: " + VecToStr(mNormal), Colour::Orange);
#endif
		}

		////////////////////////////////////////

		void Wall::Update(const Vertices& vData)
		{
			mVertices = vData;

			// Round as otherwise comparing identical vertices from unity still returns false
			mVertices[0].RoundVec();
			mVertices[1].RoundVec();
			mVertices[2].RoundVec();

			mNormal = UnitVector(Cross(vData[1] - vData[0], vData[2] - vData[0]));

			d = Round(Dot(mNormal, vData[0]));
			mNormal.RoundVec();

			CalculateArea();	
		}

		////////////////////////////////////////

		bool Wall::LineWallIntersection(const Vec3& start, const Vec3& end, Vec3& intersection) const
		{
			Vec3 dir = start - end;
			auto [valid, i] = IntersectTriangle(mVertices[0], mVertices[1], mVertices[2], start, dir, true);
			if (valid)
			{
				// intersection = v1 + E1 * u + E2 * v;
				// intersection = start + dir * t;
				intersection = i;
				return true;
			}
			return false;
		}
	}
}
