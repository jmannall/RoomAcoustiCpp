/*
*
*  \Wall class
*
*/

#ifndef Spatialiser_Wall_h
#define Spatialiser_Wall_h

// C++ headers
#include <vector>

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"
#include "Common/Coefficients.h"

// Spatialiser headers
#include "Spatialiser/Edge.h"
#include "Spatialiser/Types.h"

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Wall class ////////////////////

		class Wall
		{
		public:

			// Load and Destroy
			Wall() : mNumVertices(0), mAbsorption(1), mPlaneId(0), d(0.0), mReverbWall(ReverbWall::none) {}
			Wall(const vec3& normal, const Real* vData, size_t numVertices, const Absorption& absorption, const ReverbWall& reverbWall);
			~Wall() {}

			// Edges
			inline void AddEdge(const size_t& id) { mEdges.push_back(id); }
			inline void RemoveEdge(const size_t& id)
			{
				auto it = std::find(mEdges.begin(), mEdges.end(), id);
				if (it != mEdges.end())
					mEdges.erase(it);
			}

			// Getters
			inline vec3 GetNormal() const { return mNormal; }
			inline Real GetD() const { return d; }
			inline std::vector<vec3> GetVertices() const { return mVertices; }
			inline std::vector<size_t> GetEdges() const { return mEdges; }
			inline size_t GetPlaneID() const { return mPlaneId; }

			// Setters
			inline void SetPlaneID(const size_t& id) { mPlaneId = id; }

			// Geometry
			inline Real PointWallPosition(const vec3& point) const { return Dot(point, mNormal) - d; }
			bool LineWallIntersection(const vec3& start, const vec3& end) const;
			bool LineWallIntersection(vec3& intersection, const vec3& start, const vec3& end) const;

			// Absorption
			void Update(const vec3& normal, const Real* vData, size_t numVertices);
			inline Absorption GetAbsorption() { return mAbsorption; }
			inline Real GetArea() { return mAbsorption.area; }

			inline ReverbWall GetReverbWall() { return mReverbWall; }

		private:

			// Update
			void Update(const Real* vData);

			// Area
			void CalculateArea();
			Real AreaOfTriangle(const vec3& v, const vec3& u, const vec3& w);

			// Member variables
			Real d;
			vec3 mNormal;
			size_t mPlaneId;
			std::vector<vec3> mVertices;
			size_t mNumVertices;
			Absorption mAbsorption;
			ReverbWall mReverbWall;
			std::vector<size_t> mEdges;
		};
			
		//////////////////// Plane class ////////////////////

		class Plane
		{
		public:
			Plane() : d(0.0), rValid(false) {}
			Plane(const size_t& id, const Wall& wall) : d(wall.GetD()), rValid(false), mNormal(wall.GetNormal()) { AddWall(id); }
			~Plane() {}

			// Walls
			inline void AddWall(const size_t& id) { mWalls.push_back(id); };
			inline bool RemoveWall(const size_t& id)
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
			inline void SetRValid(const bool& valid) { rValid = valid; }

			// Geometry
			bool IsCoplanar(const Wall& wall) const { return mNormal == wall.GetNormal() && d == wall.GetD(); }
			Real PointPlanePosition(const vec3& point) const { return Dot(point, mNormal) - d; }
			bool ReflectPointInPlane(const vec3& point) const;
			bool ReflectPointInPlane(vec3& dest, const vec3& point) const;
			void ReflectPointInPlaneNoCheck(vec3& point) const;
			bool ReflectEdgeInPlane(const Edge& edge) const;

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