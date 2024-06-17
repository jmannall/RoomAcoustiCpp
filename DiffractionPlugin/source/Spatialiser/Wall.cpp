/*
* @class Wall, Plane
*
* @brief Declaration of Wall and Plane classes
*
*/

// Common headers
#include "Common/Vec3.h"
#include "Common/Types.h"
#include "Common/Definitions.h"

// Unity headers
#include "Unity/Debug.h"

// Spatialiser headers
#include "Spatialiser/Wall.h"
#include "Spatialiser/Edge.h"
#include "Spatialiser/Types.h"

namespace RAC
{
	using namespace Common;
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// Plane class ////////////////////

		bool Plane::ReflectPointInPlane(const vec3& point) const
		{
			Real k = PointPlanePosition(point);
			if (k > 0) // Check source in front of plane
				return true;
			return false;
		}

		bool Plane::ReflectPointInPlane(vec3& dest, const vec3& point) const
		{
			Real k = PointPlanePosition(point);
			if (k > 0) // Check source in front of plane
			{
				dest = point - 2.0 * mNormal * k;
				return true;
			}
			return false;
		}

		void Plane::ReflectPointInPlaneNoCheck(vec3& point) const
		{
			Real k = PointPlanePosition(point);
			point += -2.0 * mNormal * k;
		}

		void Plane::ReflectNormalInPlane(vec3& normal) const
		{
			normal += -2.0 * mNormal * Dot(normal, mNormal);
		}

		bool Plane::EdgePlanePosition(const Edge& edge) const
		{
			bool valid = ReflectPointInPlane(edge.GetEdgeCoord(EPS)); // Prevents false in case edge base is coplanar
			if (valid)
				valid = ReflectPointInPlane(edge.GetEdgeCoord(edge.zW - EPS));

			return valid;
		}

		bool Plane::FindIntersectionPoint(vec3& intersection, const vec3& start, const vec3& end, const Real k) const
		{
			vec3 grad = start - end;
			Real scale = Dot(mNormal, grad);
			if (scale == 0)
				return false;
			intersection = start - grad / scale * k; // Division first to prevent nummerical error

			Real test = PointPlanePosition(intersection);
			if (test != 0)
				intersection -= mNormal * test;

			return true;
		}

		bool Plane::LinePlaneObstruction(const vec3& start, const vec3& end) const
		{
			Real kS = PointPlanePosition(start);
			Real kE = PointPlanePosition(end);

			return kS * kE < 0;	// point lies on plane when kS || kE == 0. Therefore, not obstructed
			//if (kS * kE < 0)	// point lies on plane when kS || kE == 0. Therefore, not obstructed
			//	return true;
			//	// return FindIntersectionPoint(intersection, start, end, kS);
			//else
			//	return false;
		}

		bool Plane::LinePlaneIntersection(const vec3& start, const vec3& end) const
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
			//if (kS * kE <= 0)	// point lies on plane when kS || kE == 0.
			//	return true;
			//	// return FindIntersectionPoint(intersection, start, end, kS);
			//else
			//	return false;
		}

		//////////////////// Wall class ////////////////////

		Wall::Wall(const vec3& normal, const Real* vData, const Absorption& absorption) : mNormal(normal), mPlaneId(0), mAbsorption(absorption)
		{
			mVertices = std::vector<vec3>(3);

			Update(normal, vData);
			// mNormal = UnitVectorRound(normal);
			// Update(vData);

#ifdef DEBUG_INIT
	Debug::Log("Vertices: " + VecArrayToStr(mVertices), Colour::Orange);
	Debug::Log("Normal: " + VecToStr(mNormal), Colour::Orange);
#endif
		}

		// Update
		void Wall::Update(const vec3& normal, const Real* vData)
		{
			mNormal = UnitVectorRound(normal);

			int j = 0;
			for (int i = 0; i < mVertices.size(); i++)
			{
				// Round as otherwise comparing identical vertices from unity still returns false
				Real x = Round(vData[j++]);
				Real y = Round(vData[j++]);
				Real z = Round(vData[j++]);
				mVertices[i] = vec3(x, y, z);
			}

			d = Dot(mNormal, mVertices[0]);

			CalculateArea();
		}

		//void Wall::Update(const Real* vData)
		//{
		//	int j = 0;
		//	for (int i = 0; i < (int)mNumVertices; i++)
		//	{
		//		// Round as otherwise comparing identical vertices from unity still returns false
		//		Real x = Round(vData[j++]);
		//		Real y = Round(vData[j++]);
		//		Real z = Round(vData[j++]);
		//		mVertices[i] = vec3(x, y, z);
		//	}

		//	d = Dot(mNormal, mVertices[0]);
		//	CalculateArea();

		//	min = mVertices[0];
		//	max = mVertices[0];

		//	// create bounding box of mVertices
		//	for (int i = 1; i < mNumVertices; i++)
		//	{
		//		min.Min(mVertices[i]);
		//		max.Max(mVertices[i]);
		//	}
		//	vec3 shift = vec3(EPS, EPS, EPS);
		//	min -= shift;
		//	max += shift;
		//}

		// Geometry
		bool Wall::LineWallIntersection(const vec3& start, const vec3& end) const
		{
			vec3 intersection;
			return LineWallIntersection(intersection, start, end);
		}

		// Fast, minimum storage ray/triangle intersection. Möller, Trumbore. 2005
		bool IntersectTriangle(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& origin, const vec3& dir)
		{
			vec3 E1 = v2 - v1;
			vec3 E2 = v3 - v1;
			vec3 pVec = Cross(dir, E2);
			Real det = Dot(E1, pVec);

			if (det > -MIN_VALUE && det < MIN_VALUE)
				return false;    // This ray is parallel to this triangle.

			Real invdet = 1.0 / det;

			vec3 tVec = origin - v1;
			Real u = Dot(tVec, pVec) * invdet;

			if (u < 0.0 || u > 1.0)
				return false;

			vec3 qVec = Cross(tVec, E1);
			Real v = Dot(dir, qVec) * invdet;

			if (v < 0.0 || u + v > 1.0)
				return false;

			return true;
		}

		bool IntersectTriangle(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& origin, const vec3& dir, Real t)
		{
			vec3 E1 = v2 - v1;
			vec3 E2 = v3 - v1;
			vec3 pVec = Cross(dir, E2);
			Real det = Dot(E1, pVec);

			if (det > -MIN_VALUE && det < MIN_VALUE)
				return false;    // This ray is parallel to this triangle.

			Real invdet = 1.0 / det;

			vec3 tVec = origin - v1;
			Real u = Dot(tVec, pVec) * invdet;

			if (u < 0.0 || u > 1.0)
				return false;

			vec3 qVec = Cross(tVec, E1);
			Real v = Dot(dir, qVec) * invdet;

			if (v < 0.0 || u + v > 1.0)
				return false;

			t = Dot(E2, qVec) * invdet;
			return true;
		}

		bool IntersectTriangle(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& origin, const vec3& dir, vec3& intersection)
		{
			Real t = 0.0;
			if (IntersectTriangle(v1, v2, v3, origin, dir, t))
			{
				// intersection = v1 + E1 * u + E2 * v;
				intersection = origin + dir * t;
				return true;
			}
			return false;
		}

		bool Wall::LineWallIntersection(vec3& intersection, const vec3& start, const vec3& end) const
		{
			return IntersectTriangle(mVertices[0], mVertices[1], mVertices[2], start, start - end, intersection);

			// Check intersection lies within bounding box
			/*if (intersection.x < min.x || intersection.y < min.y || intersection.z < min.z)
				return false;
			if (intersection.x > max.x || intersection.y > max.y || intersection.z > max.z)
				return false;*/

			/*Real area = 0;
			for (int i = 0, j = 1; i < mNumVertices; i++, j++)
			{
				if (j >= mNumVertices)
					j = 0;
				area += AreaOfTriangle(intersection, mVertices[i], mVertices[j]);
			}

			bool test = IntersectTriangle(mVertices[0], mVertices[1], mVertices[2], start, end - start);

			if (Round(area, 4) == Round(mAbsorption.mArea, 4))
				return true;

			return false;*/
		}

		bool Wall::LineWallObstruction(const vec3& start, const vec3& end) const
		{
			return IntersectTriangle(mVertices[0], mVertices[1], mVertices[2], start, start - end);
		}
	}
}
