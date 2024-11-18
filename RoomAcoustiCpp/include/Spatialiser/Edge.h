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
		
		class Plane;

		//////////////////// Data structures ////////////////////

		typedef std::pair<size_t, size_t> IDPair;
		typedef std::pair<Vec3, Vec3> Vec3Pair;
		typedef std::pair<Real, Real> RealPair;

		/**
		* @brief Describes the region a a point lies in around an edge 
		* 
		* @details NonShadowed - in front of both edge planes
		* CanBeShadowed - in front of one edge plane
		* Invalid - behind both edge planes
		*/
		enum EdgeZone
		{
			NonShadowed,
			CanBeShadowed,
			Invalid
		};

		//////////////////// Edge class ////////////////////

		/**
		* @brief Class that describes an edge
		*/
		class Edge
		{
		public:

			/**
			* @brief Default constructor
			*/
			Edge() : zW(0.0), t(0.0), mDs(0.0, 0.0), receiverZone(EdgeZone::Invalid) {}

			/**
			* @brief Constructor that initialises the Edge from the given parameters
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
			* @brief Default deconstructor
			*/
			~Edge() {};

			/**
			* @brief Calculates the edge parameters
			*/
			void Update();

			/**
			* @brief Reflects the edge in a plane
			*
			* @param plane The input plane
			*/
			void ReflectInPlane(const Plane& plane);

			/**
			* @brief Finds the vector between a point and the edge base
			*
			* @param point The input coordinate
			* @return The vector between the point and the edge base
			*/
			inline Vec3 GetAP(const Vec3& point) const { return point - mBase; }

			/**
			* @brief Finds the coordinate of a point given the z value along the edge
			*
			* @param z The z value along the edge
			* @return The coordinate at the z value along the edge
			*/
			inline Vec3 GetEdgeCoord(Real z) const { return mBase + z * mEdgeVector; }

			/**
			* @return The base coordinate of the edge
			*/
			inline Vec3 GetBase() const { return mBase; }

			/**
			* @return The top coordinate of the edge
			*/
			inline Vec3 GetTop() const { return mTop; }

			/**
			* @return The mid coordinate of the edge
			*/
			inline Vec3 GetMidPoint() const { return midPoint; }

			/**
			* @return The length of the edge
			*/
			inline Real GetLength() const { return zW; }

			/**
			* @return The exterior angle of the edge
			*/
			inline Real GetExteriorAngle() const { return t; }

			/**
			* @return The vector that lies between the face normals
			*/
			inline const Vec3& GetEdgeNormal() const { return mEdgeNormal; }

			/**
			* The vector between the base and the top of the edge
			*/
			inline const Vec3& GetEdgeVector() const { return mEdgeVector; }

			/**
			* @brief Gets the second wall ID
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
			* @return The plane IDs as an ID pair
			*/
			inline IDPair GetPlaneIDs() const { return mPlaneIds; }

			/**
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
			* @param listenerPosition The new listener position
			*/
			inline void SetReceiverZone(const Vec3& listenerPosition) { receiverZone = FindEdgeZone(listenerPosition); }

			/**
			* Gets the receiver edge zone
			* 
 			* @return The receiver edge zone
			*/
			inline EdgeZone GetReceiverZone() const { return receiverZone; }

			/**
			* Finds the edge zone of a given point
			* 
			* @param point The input point
			* @return The edge zone where the point is located
			*/
			EdgeZone FindEdgeZone(const Vec3& point) const;

		private:

			Vec3 mBase;				// Base coordinate of the edge
			Vec3 mTop;				// Top coordinate of the edge
			Vec3 midPoint;			// Midpoint of the edge
			Real t;					// Exterior angle of the edge
			Real zW;				// Length of the edge
			Vec3 mEdgeVector;		// Vector between the base and top of the edge
			Vec3 mEdgeNormal;		// Vector that lies between the face normals

			Vec3Pair mFaceNormals;		// Face normals of the edge
			RealPair mDs;				// The D values describing the planes containing the walls making up the edge
			IDPair mPlaneIds;			// The plane IDs of the planes containing the walls making up the edge
			IDPair mWallIds;			// The wall IDs of the walls making up the edge

			EdgeZone receiverZone;		// The edge zone where the receiver is located
		};
	}
}

#endif