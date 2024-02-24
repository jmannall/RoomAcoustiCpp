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

		//////////////////// Wall class ////////////////////

		Wall::Wall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption) : mNormal(normal), mPlaneId(0), mNumVertices(numVertices), mAbsorption(absorption)
		{
			mNormal = UnitVectorRound(normal);
			Update(vData);
			absorption.area = mAbsorption.area;

#ifdef DEBUG_INIT
	Debug::Log("Vertecies: " + VecArrayToStr(mVertices), Colour::Orange);
	Debug::Log("Normal: " + VecToStr(mNormal), Colour::Orange);
#endif
		}

		// Update
		void Wall::Update(const vec3& normal, const Real* vData, size_t numVertices)
		{
			Absorption oldAbsorption = mAbsorption;

			mNormal = UnitVectorRound(normal);
			mNumVertices = numVertices;
			Update(vData);
		}

		void Wall::Update(const Real* vData)
		{
			int j = 0;
			mVertices.reserve(mNumVertices);
			std::fill_n(std::back_inserter(mVertices), mNumVertices, vec3());
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
		}

		// Area
		void Wall::CalculateArea()
		{
			mAbsorption.area = 0;
			for (int i = 0; i < mNumVertices - 1; i++)
				mAbsorption.area += AreaOfTriangle(mVertices[0], mVertices[i], mVertices[i + 1]);
		}

		Real Wall::AreaOfTriangle(const vec3& v, const vec3& u, const vec3& w)
		{
			return 0.5 * (Cross(v - u, v - w).Length());
		}

		// Geometry
		bool Wall::LineWallIntersection(const vec3& start, const vec3& end) const
		{
			vec3 intersection;
			return LineWallIntersection(intersection, start, end);
		}

		bool Wall::LineWallIntersection(vec3& intersection, const vec3& start, const vec3& end) const
		{
			vec3 grad = start - end;
			Real scale = Dot(mNormal, grad);
			Real k = Dot(start, mNormal) - d;
			intersection = start - grad * k / scale;

			// Check intersection lies in line segment
			vec3 grad2 = start - intersection;
			Real scale2 = Dot(mNormal, grad2);
			if (scale2 > scale)
			{
				if (scale2 > 0)
					return false;
			}
			else if (scale2 < 0)
				return false;

			// Check intersection lies within plane
			Real angleRot = 0.0;
			for (int i = 0; i < mNumVertices; i++)
			{
				int idx = (i + 1) % mNumVertices;
				vec3 one = intersection - mVertices[i];
				vec3 two = intersection - mVertices[idx];
				Real dotProduct = Dot(mNormal, Cross(one, two));
				angleRot += Sign(dotProduct) * acos(Dot(one, two) / (one.Length() * two.Length()));
			}

			Real eps = 0.00001;
			if (angleRot > PI_2 + eps)
				return false;
			else if (angleRot < PI_2 - eps)
				return false;
			else
				return true;
		}
	}
}
