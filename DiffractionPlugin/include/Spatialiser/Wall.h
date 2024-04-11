/*
* @class Wall, Plane
*
* @brief Declaration of Wall and Plane classes
*
*/

#ifndef RoomAcoustiCpp_Wall_h
#define RoomAcoustiCpp_Wall_h

// C++ headers
#include <vector>
#include <algorithm>

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"
#include "Common/Coefficients.h"

// Spatialiser headers
#include "Spatialiser/Edge.h"
#include "Spatialiser/Types.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Wall class ////////////////////

		class Wall
		{
		public:

			// Load and Destroy
			Wall() : mNumVertices(0), mAbsorption(1), mPlaneId(0), d(0.0) {}
			Wall(const vec3& normal, const Real* vData, size_t numVertices, const Absorption& absorption);
			~Wall() {}

			// Edges
			inline void AddEdge(const size_t id) { mEdges.push_back(id); std::sort(mEdges.begin(), mEdges.end()); }
			inline void RemoveEdge(const size_t id)
			{
				auto it = std::find(mEdges.begin(), mEdges.end(), id);
				if (it != mEdges.end())
					mEdges.erase(it);
			}
			inline bool EmptyEdges() const { return mEdges.size() < mVertices.size(); }

			// Getters
			inline vec3 GetNormal() const { return mNormal; }
			inline Real GetD() const { return d; }
			inline std::vector<vec3> GetVertices() const { return mVertices; }
			inline void GetVertices(float** wallVertices)
			{
				mFVertices.resize(mVertices.size() * 3);
				for (int i = 0; i < mVertices.size(); i++)
				{
					mFVertices[i * 3] = static_cast<float>(mVertices[i].x);
					mFVertices[i * 3 + 1] = static_cast<float>(mVertices[i].y);
					mFVertices[i * 3 + 2] = static_cast<float>(mVertices[i].z);
				}
				*wallVertices = &mFVertices[0];
			}
			inline std::vector<size_t> GetEdges() const { return mEdges; }
			inline size_t GetPlaneID() const { return mPlaneId; }

			// Setters
			inline void SetPlaneID(const size_t id) { mPlaneId = id; }

			// Geometry
			inline Real PointWallPosition(const vec3& point) const { return Dot(point, mNormal) - d; }
			bool LineWallIntersection(const vec3& start, const vec3& end) const;
			bool LineWallIntersection(const vec3& intersection, const vec3& start, const vec3& end) const;

			// Absorption
			void Update(const vec3& normal, const Real* vData, size_t numVertices);
			inline Absorption GetAbsorption() const { return mAbsorption; }
			inline Real GetArea() const { return mAbsorption.mArea; }

		private:

			// Update
			void Update(const Real* vData);

			// Area
			void CalculateArea();
			Real AreaOfTriangle(const vec3& v, const vec3& u, const vec3& w) const;

			// Member variables
			Real d;
			vec3 mNormal;
			size_t mPlaneId;
			std::vector<vec3> mVertices;
			vec3 min;
			vec3 max;
			std::vector<float> mFVertices;
			size_t mNumVertices;
			Absorption mAbsorption;
			std::vector<Real> triangleAreas;
			std::vector<size_t> mEdges;
		};
			
		//////////////////// Plane class ////////////////////

		class Plane
		{
		public:
			Plane() : d(0.0), rValid(false) {}
			Plane(const size_t id, const Wall& wall) : d(wall.GetD()), rValid(false), mNormal(wall.GetNormal()) { AddWall(id); }
			~Plane() {}

			// Walls
			inline void AddWall(const size_t id) { mWalls.push_back(id); };
			inline bool RemoveWall(const size_t id)
			{
				auto it = std::find(mWalls.begin(), mWalls.end(), id);
				if (it != mWalls.end())
					mWalls.erase(it);
				return mWalls.size() == 0;
			}

			// Getters
			inline vec3 GetNormal() const { return mNormal; }
			inline Real GetD() const { return d; }
			inline bool GetRValid() const { return rValid; }
			inline std::vector<size_t> GetWalls() const { return mWalls; }

			// Setters
			inline void SetRValid(const bool valid) { rValid = valid; }

			// Geometry
			bool IsCoplanar(const Wall& wall) const { return mNormal == wall.GetNormal() && d == wall.GetD(); }
			Real PointPlanePosition(const vec3& point) const { return Dot(point, mNormal) - d; }
			bool FindIntersectionPoint(vec3& intersection, const vec3& start, const vec3& end, const Real k) const;
			bool LinePlaneObstruction(vec3& intersection, const vec3& start, const vec3& end) const;
			bool LinePlaneIntersection(vec3& intersection, const vec3& start, const vec3& end) const;
			bool ReflectPointInPlane(const vec3& point) const;
			bool ReflectPointInPlane(vec3& dest, const vec3& point) const;
			void ReflectPointInPlaneNoCheck(vec3& point) const;
			bool ReflectEdgeInPlane(const Edge& edge) const;

			inline void Update(const Wall& wall) { d = wall.GetD(); mNormal = wall.GetNormal(); };
		private:

			// Member variables
			Real d;
			bool rValid;
			vec3 mNormal;

			std::vector<size_t> mWalls;
		};
	}
}
#endif