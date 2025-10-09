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
#include "Common/Definitions.h"
#include "Common/Types.h"
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Wall.h"
#include "Spatialiser/Edge.h"
#include "Spatialiser/TracingTypes.h"

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
			Room(const int numFrequencyBands) : roomData(numFrequencyBands), numFrequencyBands(numFrequencyBands), hasChanged(true) {}
			
			/**
			* @brief Default deconstructor
			*/
			~Room() {};

			inline void UpdateRoomData(const RoomData& data)
			{
				std::lock_guard<std::mutex> lock(roomDataMutex);
				roomData = data;
				roomData.Validate(numFrequencyBands);
			}

			/**
			* @brief Update the reverb time formula and recalculates the reverb time
			* 
			* @params formula The new reverb time formula
			* @return The new reverb time of the room
			*/
			inline Coefficients<> GetReverbTime(const ReverbFormula formula)
			{
				{ std::lock_guard<std::mutex> lock(roomDataMutex); roomData.formula = formula; }
				return GetReverbTime();
			}

			/**
			* @brief Calculates the reverb time of the room based on the current formula
			* 
			* @return The reverb time of the room
			*/
			Coefficients<> GetReverbTime();

			/**
			* @brief Sets the reverb time formula to custom and sets the custom T60
			*
			* @params t60 The new reverb time
			*/
			inline void UpdateReverbTime(const Coefficients<>& t60)
			{
				std::lock_guard<std::mutex> lock(roomDataMutex);
				roomData.formula = ReverbFormula::Custom;
				roomData.customT60 = t60;
			}

			size_t InitMaterial(const Coefficients<>& material);

			inline void UpdateMaterial(size_t id, const Coefficients<>& material)
			{
				std::lock_guard<std::mutex> lock(mMaterialMutex);
				auto it = mMaterials.find(id);
				if (it == mMaterials.end()) // case: material does not exist
					mMaterials.insert_or_assign(id, CalculateReflectance(material));
				else // case: material does exist
					it->second = CalculateReflectance(material);
				RecordChange();
			}

			void RemoveMaterial(size_t id);

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
			std::pair<Coefficients<>, Real> CalculateAbsorptionSurfaceArea();

			inline Vec<> GetDimensions() { std::lock_guard<std::mutex> lock(roomDataMutex); return roomData.dimensions; }

			/**
			* @return True if the room geometry has changed since last check, false otherwise
			*/
			bool HasChanged()
			{
				return hasChanged.exchange(false, std::memory_order_acq_rel);
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
			* @return The materials of the room
			*/
			MaterialMap GetMaterials() { std::lock_guard<std::mutex> lock(mMaterialMutex); return mMaterials; }

			/**
			* @return The walls of the room
			*/
			int GetNumberOfWalls() { std::lock_guard<std::mutex> lock(mWallMutex); return SizeToInt( mWalls.size() ); }

			/**
			* @return The edges of the room
			*/
			EdgeMap GetEdges() { std::lock_guard<std::mutex> lock(mEdgeMutex); return mEdges; }

			void CreateTriangleMeshSoA();

			inline const TriangleMeshSoA& GetTriangleMeshSoA() const { return mTriangleMeshSoA; }

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
			void RecordChange() { hasChanged.store(true, std::memory_order_release); }

			/**
			* @brief Calculate the reverb time of the room using the Sabine formula
			* 
			* @params absorption The total absorption of the room
			* @return The reverb time of the room
			*/
			Coefficients<> Sabine(const Coefficients<>& absorption) const;

			/**
			* @brief Calculate the reverb time of the room using the Eyring formula
			* 
			* @params absorption The total absorption of the room
			* @params surfaceArea The surface area of the room
			*/
			Coefficients<> Eyring(const Coefficients<>& absorption, const Real& surfaceArea) const;

			std::atomic<bool> hasChanged;		// True if the room geometry has changed since last check

			RoomData roomData;					// Data about the room
			int numFrequencyBands;				// Number of frequency bands for wall absorption

			WallMap mWalls;								// Stored walls
			std::vector<size_t> mEmptyWallSlots;		// Available wall IDs
			std::vector<TimerPair> mWallTimers;			// Wall IDs waiting to be made available
			size_t nextWall{ 0 };							// Next wall ID if none are available
			TriangleMeshSoA mTriangleMeshSoA;			// Triangle mesh for ray tracing

			PlaneMap mPlanes;							// Stored planes
			std::vector<size_t> mEmptyPlaneSlots;		// Available plane IDs
			std::vector<TimerPair> mPlaneTimers;		// Plane IDs waiting to be made available
			size_t nextPlane{ 0 };							// Next plane ID if none are available

			MaterialMap mMaterials;						// Stored materials (stores reflectance)
			std::vector<size_t> mEmptyMaterialSlots;	// Available material IDs
			size_t nextMaterial{ 0 };					// Next material ID if none are available

			EdgeMap mEdges;								// Stored edges
			std::vector<size_t> mEmptyEdgeSlots;		// Available edge IDs
			std::vector<TimerPair> mEdgeTimers;			// Edge IDs waiting to be made available
			size_t nextEdge{ 0 };							// Next edge ID if none are available

			std::mutex mWallMutex;		// Protects mWalls. Must always be locked before Plane and Edge
			std::mutex mPlaneMutex;		// Protects mPlanes. Can be locked after Edge (not before)
			std::mutex mMaterialMutex;		// Protects mMaterials
			std::mutex mEdgeMutex;		// Protects mEdges. Cannot be locked after Plane
			std::mutex roomDataMutex;	// Protects roomData
		};
	}
}

#endif