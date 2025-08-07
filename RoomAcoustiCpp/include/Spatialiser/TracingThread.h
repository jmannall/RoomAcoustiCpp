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

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		/**
		* @brief Class that runs the ray tracing
		*/
		class TracingThread
		{
		public:
			/**
			* @brief Constructor that initialises the TracingThread class
			*
			* @param room Pointer to the room class
			* @param sourceManager Pointer to the source manager class
			* @param reverb Pointer to the late reverb class
			* @param frequencyBands Frequency bands for graphic equalisers
			*/
			TracingThread(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb);

			/**
			* @brief Updates the stored listener position
			*/
			inline void SetListenerPosition(const Vec3& position) { lock_guard<std::mutex> lock(dataStoreMutex); mListenerPositionStore = position; }
			
			/**
			* @brief Process the ray-tracing and update...
			*/ // TODO: Update what?
			void RunTracing();

		private:
			weak_ptr<Room> mRoom;							// Pointer to the room class
			weak_ptr<SourceManager> mSourceManager;			// Pointer to the source manager class
			weak_ptr<Reverb> mReverb;						// Pointer to the late reverb class

			TriangleMeshSoA &triangles;

			RayPencil hemispherePencil;

			std::vector<std::vector<int>> pathIndexing;

			std::vector<Vec3> reverbDirections;

			std::vector<Source::Data> mSources;		// Store sources

			Vec3 mListenerPosition;					// The listener position (can be accessed freely)
			Vec3 mListenerPositionStore;			// Stores the listener position (Mutex must be locked to access)
			std::mutex dataStoreMutex;				// Protects mListenerPositionStore
		};
	}
}
#endif
