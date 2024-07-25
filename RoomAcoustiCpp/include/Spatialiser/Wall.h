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

		/**
		* Class that represents a Wall in the room
		*
		* @details All walls are expected to be triangles
		*/
		class Wall
		{
		public:

			/**
			* Default constructor that initialises an empty wall.
			*/
			Wall() : mAbsorption(1), mPlaneId(0), d(0.0) {}

			/**
			* Constructor that initialises a wall.
			*
			* @param normal The normal of the wall face.
			* @param vData The vertices of the wall.
			* @param absorption The material absorption property of the wall.
			*/
			Wall(const vec3& normal, const Real* vData, const Absorption& absorption);

			/**
			* Default deconstructor.
			*/
			~Wall() {}

			/**
			* @brief Adds the id of a connecting edge to the wall
			* 
			* @param id The ID of the edge to add
			*/
			inline void AddEdge(const size_t id) { mEdges.push_back(id); std::sort(mEdges.begin(), mEdges.end()); }

			/**
			* @brief Removes the id of a connecting edge from the wall
			* 
			* @param id The ID of the edge to remove
			*/
			inline void RemoveEdge(const size_t id)
			{
				auto it = std::find(mEdges.begin(), mEdges.end(), id);
				if (it != mEdges.end())
					mEdges.erase(it);
			}

			/**
			* @brief Checks if the wall has any unclaimed connected edges
			* 
			* @return True if the wall has less connectd edges than the maximum possible, false otherwise
			*/
			inline bool EmptyEdges() const { return mEdges.size() < mVertices.size(); }

			/**
			* @brief Returns the normal of the wall
			* 
			* @return The normal of the wall
			*/
			inline vec3 GetNormal() const { return mNormal; }

			/**
			* @brief Returns the distance of the wall from the origin along the normal direction
			*
			* @return The distance of the wall from the origin along the normal direction
			*/
			inline Real GetD() const { return d; }

			/**
			* @brief Returns the vertices of the wall
			* 
			* @return The vertices of the wall
			*/
			inline std::vector<vec3> GetVertices() const { return mVertices; }

			/**
			* @brief Returns the material absorption properties of the wall
			*
			* @return The material absorption properties of the wall
			*/
			inline Absorption GetAbsorption() const { return mAbsorption; }

			/**
			* @brief Returns the area of the wall
			*
			* @return The area of the wall
			*/
			inline Real GetArea() const { return mAbsorption.mArea; }

			/**
			* @brief Returns the IDs of the connected edges
			*
			* @return The IDs of the connected edges
			*/
			inline std::vector<size_t> GetEdges() const { return mEdges; }

			/**
			* @brief Returns the ID of the plane the wall is part of
			*
			* @return The ID of the plane the wall is part of
			*/
			inline size_t GetPlaneID() const { return mPlaneId; }

			/**
			* @brief Sets the ID of the plane the wall is part of
			* 
			* @param id The ID of the plane the wall is part of
			*/
			inline void SetPlaneID(const size_t id) { mPlaneId = id; }

			/**
			* @brief Determines the position of a point relative to the wall
			* 
			* @details If the point is in front of the wall, the return value is positive. If the point is behind the wall, the return value is negative.
			* 
			* @param point The position of the point
			* 
			* @return The distance of the point from the wall in the direction of the normal
			*/
			inline Real PointWallPosition(const vec3& point) const { return Dot(point, mNormal) - d; }

			/**
			* @brief Determines if a given line intersects the wall
			*
			* @param start The beginning of the line
			* @param end The end of the line
			*
			* @return True if the line intersects the wall, false otherwise
			*/
			bool LineWallIntersection(const vec3& start, const vec3& end) const;

			/**
			* @brief Determines if a given line intersects the wall and stores the intersection point
			*
			* @param intersection A vec3 to store the intersection point
			* @param start The beginning of the line
			* @param end The end of the line
			*
			* @return True if the line intersects the wall, false otherwise
			*/
			bool LineWallIntersection(vec3& intersection, const vec3& start, const vec3& end) const;

			/**
			* @brief Determines if the wall obstructs a given line
			*
			* @param start The beginning of the line
			* @param end The end of the line
			*
			* @return True if the wall obstructs the line, false otherwise
			*/
			bool LineWallObstruction(const vec3& start, const vec3& end) const;

			/**
			* @brief Updates the wall normal, vertices, area and d value
			* 
			* @param normal The new normal of the wall
			* @param vData The new vertices of the wall
			*/
			void Update(const vec3& normal, const Real* vData);

		private:

			// Area
			/**
			* @brief Calculates the area of a triangle
			* 
			* @param v The first vertex of the triangle
			* @param u The second vertex of the triangle
			* @param w The third vertex of the triangle
			* 
			* @return The area of the triangle
			*/
			inline Real TriangleArea(const vec3& v, const vec3& u, const vec3& w) const { return 0.5* (Cross(v - u, v - w).Length()); }

			/**
			* @brief Calculates the area of the wall
			*/
			inline void CalculateArea() { mAbsorption.mArea = TriangleArea(mVertices[0], mVertices[1], mVertices[2]); }

			//////////////////// Member Variables ////////////////////

			/**
			* Wall parameters
			*/
			std::vector<vec3> mVertices;	// Vertices of the wall
			vec3 mNormal;					// Normal of the wall
			Real d;							// Distance of the wall from the origin along the normal direction
			Absorption mAbsorption;			// Material absorption of the wall

			/**
			* Connected IDs
			*/
			size_t mPlaneId;				// ID of the plane the wall is part of
			std::vector<size_t> mEdges;		// IDs of connected edges
		};
			
		/**
		* Class that represents a Plane in the room
		*
		* @details Stored collections of walls with identical normals and d values.
		*/
		class Plane
		{
		public:

			/**
			* Default constructor that initialises an empty plane.
			*/
			Plane() : d(0.0), rValid(false) {}

			/**
			* Constructor that initialises a plane.
			*
			* @param id The ID of the wall to add to the plane.
			* @param wall The wall being added to the plane.
			*/
			Plane(const size_t id, const Wall& wall) : d(wall.GetD()), rValid(false), mNormal(wall.GetNormal()) { AddWall(id); }

			/**
			* Default deconstructor.
			*/
			~Plane() {}

			/**
			* @brief Adds the ID of a wall to the plane
			* 
			* @param id The ID of the wall to add
			*/
			inline void AddWall(const size_t id) { mWalls.push_back(id); };

			/**
			* @brief Removes the ID of a wall from the plane
			* 
			* @param id The ID of the wall to remove
			*/
			inline bool RemoveWall(const size_t id)
			{
				auto it = std::find(mWalls.begin(), mWalls.end(), id);
				if (it != mWalls.end())
					mWalls.erase(it);
				return mWalls.size() == 0;
			}

			/**
			* @brief Returns the normal of the plane
			* 
			* @return The normal of the plane
			*/
			inline vec3 GetNormal() const { return mNormal; }

			/**
			* @brief Returns the distance of the plane from the origin along the normal direction
			*
			* @return The distance of the plane from the origin along the normal direction
			*/
			inline Real GetD() const { return d; }

			/**
			* @brief Returns the IDs of the walls in the plane
			* 
			* @return The IDs of the walls in the plane
			*/
			inline std::vector<size_t> GetWalls() const { return mWalls; }

			/**
			* @brief Returns whether the listener is in front of the plane
			* 
			* @return True if the listener is in front of the plane, false otherwise
			*/
			inline bool GetRValid() const { return rValid; }


			/**
			* @brief Sets whether the listener is in front of the plane
			*/
			inline void SetRValid(const bool valid) { rValid = valid; }

			/**
			* @brief Checks whether a wall is coplanar with the plane
			*
			* @param wall The wall to check
			* 
			* @return True if the wall and plane are coplanar, false otherwise
			*/
			bool IsCoplanar(const Wall& wall) const { return mNormal == wall.GetNormal() && d == wall.GetD(); }

			/**
			* @brief Determines the position of a point relative to the plane
			* 
			* @details If the point is in front of the plane, the return value is positive. If the point is behind the plane, the return value is negative.
			* 
			* @param point The position of the point
			* 
			* @return The distance of the point from the plane in the direction of the normal
			*/
			Real PointPlanePosition(const vec3& point) const { return Dot(point, mNormal) - d; }

			/**
			* @brief Determines if the plane obstruct a given line
			* 
			* @param start The beginning of the line
			* @param end The end of the line
			* 
			* @return True if the plane obstructs the line, false otherwise
			*/
			bool LinePlaneObstruction(const vec3& start, const vec3& end) const;

			/**
			* @brief Determines if a given line intersects the plane
			* 
			* @param start The beginning of the line
			* @param end The end of the line
			* 
			* @return True if the line intersects the plane, false otherwise
			*/
			bool LinePlaneIntersection(const vec3& start, const vec3& end) const;

			/**
			* @brief Reflects a point in the plane
			* 
			* @param point The point to reflect
			* 
			* @return True if the point is in front of the plane, false otherwise
			*/
			bool ReflectPointInPlane(const vec3& point) const;

			/**
			* @brief Reflects a point in the plane and stores the end position
			* 
			* @param dest A vec3 to store the reflected point
			* @param point The point to reflect
			* 
			* @return True if the point is in front of the plane, false otherwise
			*/
			bool ReflectPointInPlane(vec3& dest, const vec3& point) const;

			/**
			* @brief Reflects a point in the plane without checking if the point is in front of the plane
			* 
			* @param point The point to reflect and store the new position
			*/
			void ReflectPointInPlaneNoCheck(vec3& point) const;

			/**
			* @brief Reflects a normal in the plane
			* 
			* @param normal The normal to reflect and store the new normal
			*/
			void ReflectNormalInPlane(vec3& normal) const;

			/**
			* @brief Determines if a given edge lies in front of the plane
			* 
			* @param edge The edge to check
			* 
			* @return True if the edge lies in front of the plane, false otherwise
			*/
			bool EdgePlanePosition(const Edge& edge) const;

			/**
			* @brief Updates the plane normal and d value from a given wall
			* 
			* @param wall The wall to update the plane with
			*/
			inline void Update(const Wall& wall) { d = wall.GetD(); mNormal = wall.GetNormal(); };
		private:

			//////////////////// Member Variables ////////////////////

			/**
			* Plane parameters
			*/
			Real d;			// Distance of the plane from the origin along the normal direction
			vec3 mNormal;	// Normal of the plane
			bool rValid;	// True if the listener is in front of the plane

			/**
			* Connected IDs
			*/
			std::vector<size_t> mWalls;		// IDs of connected walls
		};
	}
}
#endif