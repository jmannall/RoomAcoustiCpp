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

		typedef std::pair<size_t, size_t> IDPair;
		typedef std::pair<Vec3, Vec3> Vec3Pair;
		typedef std::pair<Real, Real> RealPair;

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
			Edge() : zW(0.0), t(0.0), mDs(0.0, 0.0), rZone(EdgeZone::Invalid) {}

			/**
			* Constructor that initialises the Edge from the given parameters
			* 
			* @param base The base coordinate of the edge
			* @param top The top coordinate of the edge
			* @param normal1 The normal of the first face
			* @param normal2 The normal of the second face
			* @param wallId1 The ID of the first wall
			* @param wallId2 The ID of the second wall
			* @param planeId1 The ID of the first plane
			* @param planeId2 The ID of the second plane
			*/
			Edge(const Vec3& base, const Vec3& top, const Vec3& normal1, const Vec3& normal2, const size_t wallId1, const size_t wallId2, const size_t planeId1, const size_t planeId2);

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
			inline void Update(const Edge& edge)
			{
				mBase = edge.GetBase();
				mTop = edge.GetTop();
				mFaceNormals = edge.GetFaceNormals();
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
			inline Vec3 GetAP(const Vec3& point) const { return point - mBase; }

			/**
			* Finds the coordinate of a point given the z value along the edge
			*
			* @param z The z value along the edge
			* @return The coordinate at the z value along the edge
			*/
			inline Vec3 GetEdgeCoord(Real z) const { return mBase + z * mEdgeVector; }

			/**
			* Gets the base coordinates
			*
			* @return The base coordinate of the edge
			*/
			inline Vec3 GetBase() const { return mBase; }

			/**
			* Gets the top coordinates
			*
			* @return The top coordinate of the edge
			*/
			inline Vec3 GetTop() const { return mTop; }

			/**
			* Gets the mid coordinates
			*
			* @return The mid coordinate of the edge
			*/
			inline Vec3 GetMidPoint() const { return midPoint; }

			/**
			* Gets the second wall ID
			*
			* @param id The first wall ID
			* @return The wall ID that does not match the input ID
			*/
			inline size_t GetWallID(const size_t id) const
			{
				if (id == mWallIds.first)
					return mWallIds.second;
				else
					return mWallIds.first;
			}

			/**
			* Gets the plane IDs
			*
			* @return The plane IDs as an ID pair
			*/
			inline IDPair GetPlaneIDs() const { return mPlaneIds; }

			/**
			* Gets the wall IDs
			*
			* @return The wall IDs as an ID pair
			*/
			inline IDPair GetWallIDs() const { return mWallIds; }

			/**
			* Gets the face normals
			*
			* @return The face normals as a vector pair
			*/
			inline Vec3Pair GetFaceNormals() const { return mFaceNormals; }

			/**
			* Returns true if the edge includes the given plane ID
			*
			* @param id The plane ID to check
			* @return True if the edge includes any of the IDs, else false
			*/
			inline bool IncludesPlane(const size_t id) const
			{
				if (id == mPlaneIds.first || id == mPlaneIds.second)
					return true;
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
			EdgeZone FindEdgeZone(const Vec3& point) const;

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
			Vec3 mEdgeVector;

			/**
			* The vector that lies between the face normals
 			*/
			Vec3 mEdgeNormal;

		private:
			/**
			* The midpoint along the edge
			*/
			Vec3 midPoint;

			/**
			* The base coordinate of the edge
			*/
			Vec3 mBase;

			/**
			* The top coordinate of the edge
			*/
			Vec3 mTop;

			/**
			* The face normals of the edge
			*/
			Vec3Pair mFaceNormals;

			/**
			* The D values describing the planes wherein the walls connected to the edge lie
			*/
			RealPair mDs;

			/**
			* The plane IDs of the planes making up the edge
			*/
			IDPair mPlaneIds;

			/**
			* The wall IDs of the walls connected to the edge
			*/
			IDPair mWallIds;

			/**
			* The receiver edge zone
			*/
			EdgeZone rZone;
		};
	}
}

#endif