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

		//////////////////// Utility functions ////////////////////

		/**
		* @brief Determines if a given line intersects a triangle
		* 
		* @param v1 The first vertex of the triangle
		* @param v2 The second vertex of the triangle
		* @param v3 The third vertex of the triangle
		* @param origin The origin of the line
		* @param dir The direction of the line
		* @param returnIntersection True if data to calculate the intersection point should be returned, false otherwise
		* 
		* @return A pair containing a boolean indicating if the line intersects the triangle and the distance to the intersection point
		*/
		std::pair<bool, Vec3> IntersectTriangle(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& origin, const Vec3& dir, const bool returnIntersection);

		/**
		* Class that represents a Wall in the room
		*
		* @details All walls are stored as triangles
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
			* @param vData The vertices of the wall.
			* @param absorption The material absorption property of the wall.
			*/
			Wall(const Vertices& vData, const Absorption& absorption);

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
			inline Vec3 GetNormal() const { return mNormal; }

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
			inline Vertices GetVertices() const { return mVertices; }

			/**
			* @return True if the wall contains the given vertex, false otherwise
			*/
			inline bool VertexMatch(const Vec3& x) const { return mVertices[0] == x || mVertices[1] == x || mVertices[2] == x; }

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
			inline Real PointWallPosition(const Vec3& point) const { return Dot(point, mNormal) - d; }

			/**
			* @brief Determines if a given line intersects the wall and stores the intersection point
			*
			* @param intersection A vec3 to store the intersection point
			* @param start The beginning of the line
			* @param end The end of the line
			*
			* @return True if the line intersects the wall, false otherwise
			*/
			bool LineWallIntersection(const Vec3& start, const Vec3& end, Vec3& intersection) const;
			
			/**
			* @brief Determines if the wall obstructs a given line
			*
			* @param start The beginning of the line
			* @param end The end of the line
			*
			* @return True if the wall obstructs the line, false otherwise
			*/
			inline bool LineWallObstruction(const Vec3& start, const Vec3& end) const { return IntersectTriangle(mVertices[0], mVertices[1], mVertices[2], start, start - end, false).first; }

			/**
			* @brief Updates the wall normal, vertices, area and d value
			* 
			* @param vData The new vertices of the wall
			*/
			void Update(const Vertices& vData);

			/**
			* @brief Updates the wall absorption
			*
			* @param absorption The new absorption of the wall
			*/
			inline void Update(const Absorption& absorption) { Real area = GetArea(); mAbsorption = absorption; mAbsorption.mArea = area; }

		private:
			/**
			* @brief Calculates the area of the wall (area of a triangle)
			*/
			inline void CalculateArea() { mAbsorption.mArea = 0.5 * (Cross(mVertices[0] - mVertices[1], mVertices[0] - mVertices[2]).Length()); }

			Vertices mVertices;				// Vertices of the wall
			Vec3 mNormal;					// Normal of the wall
			Real d;							// Distance of the wall from the origin along the normal direction
			Absorption mAbsorption;			// Material absorption of the wall

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
			Plane() : d(0.0), receiverValid(false) {}

			/**
			* Constructor that initialises a plane.
			*
			* @param id The ID of the wall to add to the plane.
			* @param wall The wall being added to the plane.
			*/
			Plane(const size_t id, const Wall& wall) : d(wall.GetD()), receiverValid(false), mNormal(wall.GetNormal()) { AddWall(id); }

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
			inline Vec3 GetNormal() const { return mNormal; }

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
			inline bool GetReceiverValid() const { return receiverValid; }


			/**
			* @brief Sets whether the listener is in front of the plane
			* 
			* @param listenerPosition The new listener position
			*/
			inline void SetReceiverValid(const Vec3& listenerPosition) { receiverValid = ReflectPointInPlane(listenerPosition); }

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
			Real PointPlanePosition(const Vec3& point) const { return Dot(point, mNormal) - d; }

			/**
			* @brief Determines if the plane obstruct a given line
			* 
			* @param start The beginning of the line
			* @param end The end of the line
			* 
			* @return True if the plane obstructs the line, false otherwise
			*/
			bool LinePlaneObstruction(const Vec3& start, const Vec3& end) const;

			/**
			* @brief Determines if a given line intersects the plane
			* 
			* @param start The beginning of the line
			* @param end The end of the line
			* 
			* @return True if the line intersects the plane, false otherwise
			*/
			bool LinePlaneIntersection(const Vec3& start, const Vec3& end) const;

			/**
			* @brief Reflects a point in the plane
			* 
			* @param point The point to reflect
			* 
			* @return True if the point is in front of the plane, false otherwise
			*/
			bool ReflectPointInPlane(const Vec3& point) const;

			/**
			* @brief Reflects a point in the plane and stores the end position
			* 
			* @param dest A vec3 to store the reflected point
			* @param point The point to reflect
			* 
			* @return True if the point is in front of the plane, false otherwise
			*/
			bool ReflectPointInPlane(Vec3& dest, const Vec3& point) const;

			/**
			* @brief Reflects a point in the plane without checking if the point is in front of the plane
			* 
			* @param point The point to reflect and store the new position
			*/
			void ReflectPointInPlaneNoCheck(Vec3& point) const;

			/**
			* @brief Reflects a normal in the plane
			* 
			* @param normal The normal to reflect and store the new normal
			*/
			void ReflectNormalInPlane(Vec3& normal) const;

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

			Real d;					// Distance of the plane from the origin along the normal direction
			Vec3 mNormal;			// Normal of the plane
			bool receiverValid;		// True if the listener is in front of the plane

			std::vector<size_t> mWalls;		// IDs of connected walls
		};
	}
}
#endif