/*
* @class Room
*
* @brief Declaration of Room class
* 
*/

#ifndef RoomAcoustiCpp_Room_h
#define RoomAcoustiCpp_Room_h

// C++ headers
#include <vector>
#include <mutex>

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Wall.h"
#include "Spatialiser/Edge.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		/**
		* @class Room
		* 
		* @brief Class that stores the geometry of the scene and calculates the reverb time
		*/
		class Room
		{
		public:

			/**
			* @brief Constructor that initialises a room with a given number of frequency bands for late reverberation
			* 
			* @params numFrequencyBands The number of frequency bands to use
			*/
			Room(const int numFrequencyBands) : nextPlane(0), nextWall(0), nextEdge(0), reverbFormula(ReverbFormula::Sabine), mVolume(0.0), numAbsorptionBands(numFrequencyBands), hasChanged(true) {}
			
			/**
			* @brief Default deconstructor
			*/
			~Room() {};

			/**
			* @brief Update the reverb time formula and recalculates the reverb time
			* 
			* @params formula The new reverb time formula
			*/
			inline Coefficients UpdateReverbTimeFormula(const ReverbFormula formula) { reverbFormula = formula; return GetReverbTime(); }

			/**
			* @brief Add a wall to the room
			*/
			size_t AddWall(Wall& wall);

			/**
			* @brief Update the vertex data of the wall with the given ID
			* 
			* @params id The ID of the wall to update
			* @params vData The new vertex data for the wall
			*/
			inline void UpdateWall(const size_t id, const Vertices& vData)
			{
				std::lock_guard<std::mutex> lock(mWallMutex);
				auto it = mWalls.find(id);
				if (it == mWalls.end()) { return; } // case: wall does not exist
				else { it->second.Update(vData); RecordChange(); } // case: wall does exist
			}

			/**
			* @brief Update the absorption of the wall with the given ID
			* 
			* @params id The ID of the wall to update
			* @params absorption The new absorption of the wall
			*/
			inline void UpdateWallAbsorption(const size_t id, const Absorption& absorption)
			{
				std::lock_guard<std::mutex> lock(mWallMutex);
				auto it = mWalls.find(id);
				if (it == mWalls.end()) { return; } // case: wall does not exist
				else { it->second.Update(absorption); RecordChange(); } // case: wall does exist
			}

			/**
			* @brief Remove the wall with the given ID
			* 
			* @params id The ID of the wall to remove
			*/
			void RemoveWall(const size_t id);

			/**
			* @brief Initialises edges between walls
			* 
			* @params id The ID of the wall to search for edges for
			*/
			void InitEdges(const size_t id);

			/**
			* @brief Update planes and grouping of coplanar walls
			*/
			void UpdatePlanes();

			/**
			* @brief Update edges and check connections between walls
			*/
			void UpdateEdges();

			/**
			* @return The predicted reverb time of the room
			*/
			Coefficients GetReverbTime();

			/**
			* @brief Get the predicted reverb time of the room with a given volume
			* 
			* @params volume The volume of the room
			* @return The predicted reverb time of the room
			*/
			Coefficients GetReverbTime(const Real volume) { mVolume = volume; return GetReverbTime(); }

			/**
			* @return True if the room geometry has changed since last check, false otherwise
			*/
			bool HasChanged()
			{
				if (hasChanged) { hasChanged = false; return true; }
				else { return false; }
			}

			/**
			* @return The planes of the room
			*/
			PlaneMap GetPlanes() { std::lock_guard<std::mutex> lock(mPlaneMutex); return mPlanes; }

			/**
			* @return The walls of the room
			*/
			WallMap GetWalls() { std::lock_guard<std::mutex> lock(mWallMutex); return mWalls; }

			/**
			* @return The edges of the room
			*/
			EdgeMap GetEdges() { std::lock_guard<std::mutex> lock(mEdgeMutex); return mEdges; }

		private:
			/**
			* @brief Assign a wall to a plane
			* 
			* @params id The wall ID to assign to a plane
			*/
			void AssignWallToPlane(const size_t id);

			/**
			* @brief Assign a wall to a plane
			* 
			* @params wallID The wall ID to assign to a plane
			* @params wall The wall to assign to a plane
			*/
			void AssignWallToPlane(const size_t wallID, Wall& wall);

			/**
			* @brief Remove a wall from a plane
			* 
			* @params planeID The plane ID to remove the wall from
			* @params wallID The wall ID to remove from the plane
			*/
			void RemoveWallFromPlane(const size_t planeID, const size_t wallID);

			/**
			* @brief Add a located edge to the room
			*/
			void AddEdge(const Edge& edge);

			/**
			* @brief Initialise edges between walls
			* 
			* @params id The ID of the wall to search for edges for
			* @params wallIDs The IDs of walls with already connected edges
			*/
			void InitEdges(const size_t id, const std::vector<size_t>& wallIDs);

			/**
			* @brief Locate edges between walls
			* 
			* @params idA The ID of the first wall
			* @params idB The ID of the second wall
			* @params edges To store the located edges
			* @params IDs To store the IDs of the previously located edges
			*/
			void FindEdges(const size_t idA, const size_t idB, std::vector<Edge>& edges, std::vector<size_t>& IDs);

			/**
			* @brief Locate edges between walls
			* 
			* @params wallA The first wall
			* @params wallB The second wall
			* @params idA The ID of the first wall
			* @params idB The ID of the second wall
			* @params edges To store the located edges
			*/
			void FindEdges(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges);

			/**
			* @brief Locate parallel edges between walls
			* 
			* @params wallA The first wall
			* @params wallB The second wall
			* @params idA The ID of the first wall
			* @params idB The ID of the second wall
			* @params edges To store the located edges
			*/
			void FindParallelEdges(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges);
			
			/**
			* Locate a single edge between walls
			* 
			* @params wallA The first wall
			* @params wallB The second wall
			* @params idA The ID of the first wall
			* @params idB The ID of the second wall
			* @params edges To store the located edge
			*/
			void FindEdge(const Wall& wallA, const Wall& wallB, const size_t idA, const size_t idB, std::vector<Edge>& edges);

			/**
			* @brief Update the edge with the given ID
			* 
			* @params id The ID of the edge to update
			* @params edge The new edge data
			* @return true If edge should be removed, false otherwise
			*/
			bool UpdateEdge(const size_t id, const Edge& edge);

			/**
			* @brief Update the edges with the given IDs
			* 
			* @params IDs The IDs of the edges to update
			* @params edges The new edge data
			*/
			inline std::vector<size_t> UpdateEdges(const std::vector<size_t>& IDs, const std::vector<Edge>& edges)
			{
				std::vector<size_t> removeIDs;
				int i = 0;
				for (const Edge& edge : edges)
				{
					if (UpdateEdge(IDs[i], edge))
						removeIDs.push_back(IDs[i]);
					i++;
				}
				return removeIDs;
			}

			/**
			* @brief Checks if an edge is coplanar with a plane not making up a wall attached to the edge
			* 
			* @params edge The edge to check
			* @return True if the edge is coplanar with a plane, false otherwise
			*/
			bool IsCoplanarEdge(const Edge& edge);

			/**
			* @brief Remove the edges with the given IDs
			* 
			* @params edgeIDs The IDs of the edges to remove
			* @params wallID The ID of the first wall the edge was atatched to
			*/
			inline void RemoveEdges(const std::vector<size_t>& edgeIDs, const size_t wallID)
			{
				std::lock_guard<std::mutex> lock(mEdgeMutex);
				for (const size_t edgeID : edgeIDs)
					RemoveEdge(edgeID, wallID);
			}

			/**
			* @brief Remove the edge with the given ID
			* 
			* @params edgeID The ID of the edge to remove
			* @params wallID The ID of the first wall the edge was atatched to
			*/
			void RemoveEdge(const size_t edgeID, const size_t wallID);

			/**
			* @brief Remove the edge with the given ID
			* 
			* @params idE The ID of the edge to remove
			*/
			void RemoveEdge(const size_t edgeID);

			/**
			* @brief Record a change in the room geometry
			*/
			void RecordChange() { hasChanged = true; }

			/**
			* @brief Calculate the reverb time of the room using the Sabine formula
			* 
			* @params absorption The total absorption of the room
			* @return The reverb time of the room
			*/
			Coefficients Sabine(const Coefficients& absorption);

			/**
			* @brief Calculate the reverb time of the room using the Eyring formula
			* 
			* @params absorption The total absorption of the room
			* @params surfaceArea The surface area of the room
			*/
			Coefficients Eyring(const Coefficients& absorption, const Real& surfaceArea);

			std::atomic<bool> hasChanged;		// True if the room geometry has changed since last check

			Real mVolume;						// The volume of the room
			ReverbFormula reverbFormula;		// The formula used to calculate the reverb time
			int numAbsorptionBands;				// The number of frequency bands to use for late reverberation

			WallMap mWalls;								// Stored walls
			std::vector<size_t> mEmptyWallSlots;		// Available wall IDs
			std::vector<TimerPair> mWallTimers;			// Wall IDs waiting to be made available
			size_t nextWall;							// Next wall ID if none are available

			PlaneMap mPlanes;							// Stored planes
			std::vector<size_t> mEmptyPlaneSlots;		// Available plane IDs
			std::vector<TimerPair> mPlaneTimers;		// Plane IDs waiting to be made available
			size_t nextPlane;							// Next plane ID if none are available

			EdgeMap mEdges;								// Stored edges
			std::vector<size_t> mEmptyEdgeSlots;		// Available edge IDs
			std::vector<TimerPair> mEdgeTimers;			// Edge IDs waiting to be made available
			size_t nextEdge;							// Next edge ID if none are available

			std::mutex mWallMutex;		// Protects mWalls. Must always be locked before Plane and Edge
			std::mutex mPlaneMutex;		// Protects mPlanes. Can be locked after Edge (not before)
			std::mutex mEdgeMutex;		// Protects mEdges. Cannot be locked after Plane
		};
	}
}

#endif