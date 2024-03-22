/*
*
*  \Wall class
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

namespace UIE
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

		bool Plane::ReflectEdgeInPlane(const Edge& edge) const
		{
			bool valid = ReflectPointInPlane(edge.GetEdgeCoord(EPS)); // Prevents false in case edge base is coplanar
			if (valid)
				valid = ReflectPointInPlane(edge.GetEdgeCoord(edge.zW - EPS));
			return valid;
		}

		bool Plane::LinePlaneIntersection(vec3& intersection, const vec3& start, const vec3& end) const
		{
			Real kS = PointPlanePosition(start);
			Real kE = PointPlanePosition(end);

			if (kS * kE < 0)	// point lies on plane when kS || kE == 0. Therefore no intersection
			{
				vec3 grad = start - end;
				Real scale = Dot(mNormal, grad);
				if (scale == 0)
					return false;
				intersection = start - grad * kS / scale;

				Real test = PointPlanePosition(intersection);
				if (test != 0)
					intersection -= mNormal * test;

				return true;

				// Check intersection lies in line segment (can this check be made without this part?)
				/*vec3 grad2 = start - intersection;
				Real scale2 = Dot(plane.GetNormal(), grad2);
				if (scale2 > scale)
				{
					if (scale2 > 0)
						return false;
				}
				else if (scale2 < 0)
					return false;*/
			}
			else
				return false;
		}

		//////////////////// Wall class ////////////////////

		Wall::Wall(const vec3& normal, const Real* vData, size_t numVertices, const Absorption& absorption) : mNormal(normal), mPlaneId(0), mNumVertices(numVertices), mAbsorption(absorption)
		{
			mNormal = UnitVectorRound(normal);

			mVertices = std::vector<vec3>(mNumVertices);
			triangleAreas = std::vector<Real>(mNumVertices - 2, 0);
			Update(vData);

#ifdef DEBUG_INIT
	Debug::Log("Vertices: " + VecArrayToStr(mVertices), Colour::Orange);
	Debug::Log("Normal: " + VecToStr(mNormal), Colour::Orange);
#endif
		}

		// Update
		void Wall::Update(const vec3& normal, const Real* vData, size_t numVertices)
		{
			mNormal = UnitVectorRound(normal);
			if (mNumVertices == numVertices)
				Update(vData);
			else
				Debug::Log("Cannot update wall because the number of vertices has changed", Colour::Red);
		}

		void Wall::Update(const Real* vData)
		{
			int j = 0;
			for (int i = 0; i < (int)mNumVertices; i++)
			{
				// Round as otherwise comparing identical vertices from unity still returns false
				Real x = Round(vData[j++], NUM_PRECISION);
				Real y = Round(vData[j++], NUM_PRECISION);
				Real z = Round(vData[j++], NUM_PRECISION);
				mVertices[i] = vec3(x, y, z);
			}

			d = Dot(mNormal, mVertices[0]);
			CalculateArea();

			min = mVertices[0];
			max = mVertices[0];

			// create bounding box of mVertices
			for (int i = 1; i < mNumVertices; i++)
			{
				min.Min(mVertices[i]);
				max.Max(mVertices[i]);
			}
			vec3 shift = vec3(EPS, EPS, EPS);
			min -= shift;
			max += shift;
		}

		// Area
		void Wall::CalculateArea()
		{
			mAbsorption.area = 0;
			for (int i = 0; i < mNumVertices - 2; i++)
			{
				triangleAreas[i] = AreaOfTriangle(mVertices[0], mVertices[i + 1], mVertices[i + 2]);
				mAbsorption.area += triangleAreas[i];
			}
		}

		Real Wall::AreaOfTriangle(const vec3& v, const vec3& u, const vec3& w) const
		{
			return 0.5 * (Cross(v - u, v - w).Length());
		}

		// Geometry
		bool Wall::LineWallIntersection(const vec3& start, const vec3& end) const
		{
			vec3 intersection;
			return LineWallIntersection(intersection, start, end);
		}

		bool Wall::LineWallIntersection(const vec3& intersection, const vec3& start, const vec3& end) const
		{
			//vec3 grad = start - end;
			//Real scale = Dot(mNormal, grad);
			//if (scale == 0)
			//	return false;
			//Real k = Dot(start, mNormal) - d;
			//intersection = start - grad * k / scale;

			//// Check intersection lies in line segment
			//vec3 grad2 = start - intersection;
			//Real scale2 = Dot(mNormal, grad2);
			//if (scale2 > scale)
			//{
			//	if (scale2 > 0)
			//		return false;
			//}
			//else if (scale2 < 0)
			//	return false;



			// Check intersection lies within wall
			//Real angleRot = 0.0;
			//for (int i = 0; i < mNumVertices; i++)
			//{
			//	int idx = (i + 1) % mNumVertices;
			//	vec3 one = intersection - mVertices[i];
			//	vec3 two = intersection - mVertices[idx];
			//	Real dotProduct = Dot(mNormal, Cross(one, two));
			//	angleRot += Sign(dotProduct) * acos(Dot(one, two) / (one.Length() * two.Length()));
			//}

			//bool ret = false;
			//Real eps = 0.0001;
			//if (angleRot < PI_2 + eps && angleRot > PI_2 - eps)
			//	ret = true;
			//if (angleRot < PI_1 + eps && angleRot > PI_1 - eps) // lies on edge of wall
			//	ret = true;

			/*if (angleRot > PI_2 + eps)
				return false;
			else if (angleRot < PI_2 - eps)
				return false;
			else
				return true;*/

			// Check intersection lies within bounding box
			if (intersection.x < min.x || intersection.y < min.y || intersection.z < min.z)
				return false;
			if (intersection.x > max.x || intersection.y > max.y || intersection.z > max.z)
				return false;

			Real area = 0;
			for (int i = 0, j = 1; i < mNumVertices; i++, j++)
			{
				if (j >= mNumVertices)
					j = 0;
				area += AreaOfTriangle(intersection, mVertices[i], mVertices[j]);
			}

			if (Round(area, 4) == Round(mAbsorption.area, 4))
				return true;

			return false;
			// Check intersection lies within wall
			//int i = 0;
			//while (!ret3 && i < triangleAreas.size())
			//{
			//	alpha = AreaOfTriangle(intersection, mVertices[0], mVertices[i + 1]);
			//	beta = AreaOfTriangle(intersection, mVertices[i + 1], mVertices[i + 2]);
			//	gamma = AreaOfTriangle(intersection, mVertices[i + 2], mVertices[0]);
			//	if (Round(alpha + beta + gamma, 8) == Round(triangleAreas[i], 8))
			//	{
			//		area = triangleAreas[i];
			//		ret3 = true;
			//	}
			//	i++;
			//}

			/*Debug::Log("Intersection: angle " + BoolToStr(ret) + " bounding " + BoolToStr(ret2) + " triangle " + BoolToStr(ret3), Colour::Orange);

			if (!ret && !ret2 && !ret3)
				Debug::Log("Vertices: " + VecArrayToStr(mVertices) + " Intersection: " + VecToStr(intersection) + " Area: " + RealToStr(mAbsorption.area) + " Check: " + RealToStr(area) + " Angle: " + RealToStr(angleRot), Colour::Yellow);*/

			/*if (!ret && ret2 && ret3)
				Debug::Log("Vertices: " + VecArrayToStr(mVertices) + " Intersection: " + VecToStr(intersection) + " Area: " + RealToStr(area) + " Check: " + RealToStr(alpha + beta + gamma) + " Angle: " + RealToStr(angleRot), Colour::Green);*/

		}
	}
}
