/*
* @class Edge
*
* @brief Declaration of Edge class
*
* @remarks Face normals defined using right hand curl rule that rotates from wall 0 to wall 1 through the exterior wedge.
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
		//////////////////// Class predelarations ////////////////////
		
		class Edge;
		class Plane;

		//////////////////// Data structures ////////////////////

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

		/**
		 * Class that describes an edge
		 */
		class Edge
		{
		public:

			/**
			 * Default constructor
			 */
			Edge() : zW(0.0), t(0.0), mDs(2), rZone(EdgeZone::Invalid) {}

			/**
			 * Constructor that initialises the Edge from an EdgeData struct
			 *
			 * @param data The input EdgeData struct
			 */
			Edge(const EdgeData& data);

			/**
			 * Constructor that initialises the Edge from the given parameters
			 * 
			 * @param base The base coordinate of the edge
			 * @param top The top coordinate of the edge
			 * @param normal1 The normal of the first face
			 * @param normal2 The normal of the second face
			 * @param id1 The ID of the first wall
			 * @param id2 The ID of the second wall
			 */
			Edge(const vec3& base, const vec3& top, const vec3& normal1, const vec3& normal2, const size_t id1, const size_t id2);

			/**
			 * Default deconstructor
			 */
			~Edge() {};

			/**
			 * Calculates the edge parameters
			 */
			void Update();

			/**
			 * Updates the edge data from an EdgeData struct and updates the edge parameters
			 *
			 * @param data The input EdgeData struct
			 */
			inline void Update(const EdgeData& data)
			{
				mBase = data.base;
				mTop = data.top;
				mFaceNormals[0] = data.normal1;
				mFaceNormals[1] = data.normal2;
				Update();
			}

			/**
			 * Reflects the edge in a plane
			 *
			 * @param plane The input plane
			 */
			void ReflectInPlane(const Plane& plane);

			/**
			 * Finds the vector between a point and the edge base
			 *
			 * @param point The input coordinate
			 * @return The vector between the point and the edge base
			 */
			inline vec3 GetAP(const vec3& point) const { return point - mBase; }

			/**
			 * Finds the coordinate of a point given the z value along the edge
			 *
			 * @param z The z value along the edge
			 * @return The coordinate at the z value along the edge
			 */
			inline vec3 GetEdgeCoord(Real z) const { return mBase + z * mEdgeVector; }

			/**
			 * Gets the base coordinates
			 *
			 * @return The base coordinate of the edge
			 */
			inline vec3 GetBase() const { return mBase; }

			/**
			 * Gets the top coordinates
			 *
			 * @return The top coordinate of the edge
			 */
			inline vec3 GetTop() const { return mTop; }

			/**
			 * Gets the mid coordinates
			 *
			 * @return The mid coordinate of the edge
			 */
			inline vec3 GetMidPoint() const { return midPoint; }

			/**
			 * Gets the second wall ID
			 *
			 * @param id The first wall ID
			 * @return The wall ID that does not match the input ID
			 */
			inline size_t GetWallID(const size_t id) const
			{
				if (id == mWallIds[0])
					return mWallIds[1];
				else
					return mWallIds[0];
			}

			/**
			 * Gets the wall IDs
			 *
			 * @return The wall IDs as a vector
			 */
			inline std::vector<size_t> GetWallIDs() const { return mWallIds; }

			/**
			 * Gets the face normal at the given index
			 *
			 * @param i The index of the face normal
			 * @return The face normal at the given index
			 */
			inline vec3 GetFaceNormal(const size_t i) const { return mFaceNormals[i]; }

			/**
			 * Iterates through a vector of IDs and returns true if the edge is attached to any of the IDs
			 *
			 * @param ids The vector of IDs to check
			 * @return True if the edge is attached to any of the IDs, else false
			 */
			inline bool AttachedToPlane(const std::vector<size_t>& ids) const
			{
				for (size_t id : ids)
				{
					if (id == mWallIds[0] || id == mWallIds[1])
						return true;
				}
				return false;
			}

			/**
			 * Sets the receiver edge zone
			 * 
			 * @param zone The new receiver edge zone
			 */
			inline void SetRZone(const EdgeZone zone) { rZone = zone; }

			/**
			 * Gets the receiver edge zone
			 * 
 			 * @return The receiver edge zone
			 */
			inline EdgeZone GetRZone() const { return rZone; }

			/**
			 * Finds the edge zone of a given point
			 * 
			 * @param point The input point
			 * @return The edge zone where the point is located
			 */
			EdgeZone FindEdgeZone(const vec3& point) const;

			/**
			 * The exterior angle of the edge
			 */
			Real t;

			/**
			 * The length of the edge
			 */
			Real zW;

			/**
			 * The vector between the base and top of the edge
			 */
			vec3 mEdgeVector;

			/**
			 * The vector that lies between the face normals
 			 */
			vec3 mEdgeNormal;

		private:
			/**
			 * The midpoint along the edge
			 */
			vec3 midPoint;

			/**
			 * The base coordinate of the edge
			 */
			vec3 mBase;

			/**
			 * The top coordinate of the edge
			 */
			vec3 mTop;

			/**
			 * The face normals of the edge
			 */
			std::vector<vec3> mFaceNormals;

			/**
			 * The D values describing the planes wherein the walls connected to the edge lie
			 */
			std::vector<Real> mDs;

			/**
			 * The wall IDs of the walls connected to the edge
			 */
			std::vector<size_t> mWallIds;

			/**
			 * The receiver edge zone
			 */
			EdgeZone rZone;
		};
	}
}

#endif