/*
* @brief The class that handles the core loop of the ray-tracing thread.
*/

#ifndef Tracing_Thread_h
#define Tracing_Thread_h

// Common headers
#include "Common/Vec3.h" 

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Room.h"
#include "Spatialiser/Reverb.h"
#include "Spatialiser/SourceManager.h"
#include "Spatialiser/TracingUtils.h" // Indirectly includes TracingTypes

#if defined _DEBUG || defined DEBUG_RTM
#pragma optimize("", off)
#endif

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		/**
		* @brief Class that runs the ray-tracing.
		*/
		class TracingThread
		{
		public:
			/**
			* @brief Constructor that initialises the TracingThread class.
			*
			* @param room Pointer to the room class.
			* @param sourceManager Pointer to the source manager class.
			* @param reverb Pointer to the late reverb class.
			*/
			TracingThread(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const std::shared_ptr<Config>& config);

			/**
			* @brief Updates the stored listener position.
			*/
			inline void SetListenerPosition(const Vec3& position) { lock_guard<std::mutex> lock(dataStoreMutex); mListenerPositionStore = position; }

			/**
			* @brief Change the number of traced rays.
			*
			* @param newNumRays The new number of rays. N.B.: this is the number of rays in each hemisphere; the actual traced directions are twice as many.
			*/
			void setNumberOfRays(int newNumRays);

			/**
			* @brief Set the propagation path indexing and decay rates of each mode.
			* N.B.: This also prompts the contruction of the TriangleMeshSoA in Room.
			*/
			void InitRoom(int paths, const std::vector<std::vector<int>>& indexing, const Vec<>& decayRates);

			/**
			* @brief Process the ray-tracing from every new position and update the related residues.
			*/
			void RunTracing();

		private:
			weak_ptr<Room> mRoom;							// Pointer to the room class
			weak_ptr<SourceManager> mSourceManager;			// Pointer to the source manager class
			weak_ptr<Reverb> mReverb;						// Pointer to the late reverb class
			std::mutex rayPencilMutex;						// Protects hemispherePencil

			// The geometry is assumed unchanging.
			int numPaths;									// Number of ART propagation paths
			std::vector<std::vector<int>> pathIndexing;		// Index of the ART propagation path from triangle A to triangle B

			// The number of reverb directions is assumed unchanging.
			int numFDNs, numReverbDirections;
			std::vector<Vec3> reverbDirections;
			// The indexing of ray directions to reverb directions may change because the number of rays may change.
			std::vector<int> rayClusters;			// This will have size `numRays`. For each ray, the index of the reverb direction that the ray falls into.
			std::vector<int> clustersSizes;			// This will have size `numReverbDirections`. For each reverb direction, the number of rays that fall into it.

			// The number of rays may change at runtime.
			int numRays;
			RayPencil hemispherePencil;				// A set of ray directions (relative to the frame of reference) used for all tracing

			// The number of sources may change at runtime.
			std::vector<Source::Data> mSources;		// Stores information about each source (id, position, ...)

			// The listener position may change at runtime.
			Vec3 mListenerPosition;					// The listener position (can be accessed freely)
			Vec3 mListenerPositionStore;			// Stores the listener position (Mutex must be locked to access)
			std::mutex dataStoreMutex;				// Protects mListenerPositionStore

			// These will be used as temporary "buffers" in the hot loop; memory is only allocated once.
			Vec<Real> decayPerSecond;						// This will have size `numFDNs`
			Vec<Real> energyContributions;					// This will have size `numPaths`
			Vec<Real> contributionDelays;					// This will have size `numPaths`
			Vec<Real> contributionDelayScaling;				// This will have size `numPaths`
			Coefficients<> sourceResidues;					// This will have size `numFDNs`
			std::vector<Coefficients<>> listenerResidues;	// This will have size `numFDNs, numReverbDirections`
			// These will be used exclusively inside `computeEnergyContributions`. All four will have size `numRays`.
			Vec<Real> rayDistances;				// This will have size `numRays`
			Vec<Real> rayCosines;				// This will have size `numRays`
			std::vector<int> frontIndices;		// This will have size `numRays`
			std::vector<int> backIndices;		// This will have size `numRays`

			/**
			* @brief Assigns each ray direction to the nearest reverb direction.
			*/
			void clusterReverbDirections();

			/**
			* @brief Computes the energy constributions of the ray pencil to each ART propagation path and stores them in the attribute `energyContributions`.
			* Makes internal use of the latest tracing results stored in hemispherePencil in conjunction with pathIndexing.
			*
			* @param reverbDirectionIdx If `reverbDirectionIdx == -1` (default), take all rays' contributions into account (omnidirectional).
			*						 Otherwise, only tally up the contributions of rays within the cluster with index `reverbDirectionIdx`.
			*/
			void computeEnergyContributions(int reverbDirectionIdx = -1);
		};
	}
}

#if defined _DEBUG || defined DEBUG_RTM
#pragma optimize("", on)
#endif

#endif
