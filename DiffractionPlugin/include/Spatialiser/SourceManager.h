/*
*
*  \Source manager class
*
*/

#ifndef Spatialiser_SourceManager_h
#define Spatialiser_SourceManager_h

// C++ headers
#include <unordered_map>
#include <mutex>

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Source.h"

// 3DTI headers
#include "BinauralSpatializer/Core.h"

using namespace Common;
namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{
		//////////////////// SourceManager class ////////////////////

		class SourceManager
		{
		public:

			// Load and Destroy
			SourceManager(Binaural::CCore* core, const Config& config) : mSources(), mEmptySlots(), mCore(core), mConfig(config) {};
			~SourceManager() { Reset(); };

			// Sources
			size_t Init();
			inline void Update(const size_t& id, const CTransform& transform, const SourceData& data)
			{
				lock_guard <mutex> lock(updateMutex); // Lock before locate to ensure not deleted between found and mutex lock
				auto it = mSources.find(id);
				if (it == mSources.end())		// case: source does not exist
				{
					// Source does not exist
				}
				else
				{
					it->second.Update(transform, data);
				}
			}
			inline void Remove(const size_t& id)
			{
				std::lock(updateMutex, processAudioMutex);
				std::lock_guard<std::mutex> lk1(updateMutex, std::adopt_lock);
				std::lock_guard<std::mutex> lk2(processAudioMutex, std::adopt_lock);
				mSources.erase(id);
				mEmptySlots.push_back(id);
			}
			/*inline void LogWallRemoval(const size_t& id)
			{
				std::lock(updateMutex, processAudioMutex);
				std::lock_guard<std::mutex> lk1(updateMutex, std::adopt_lock);
				std::lock_guard<std::mutex> lk2(processAudioMutex, std::adopt_lock);
				for (auto it = mSources.begin(); it != mSources.end(); it++)
				{
					it->second.LogWallRemoval(id);
				}
			}
			inline void LogEdgeRemoval(const size_t& id)
			{
				std::lock(updateMutex, processAudioMutex);
				std::lock_guard<std::mutex> lk1(updateMutex, std::adopt_lock);
				std::lock_guard<std::mutex> lk2(processAudioMutex, std::adopt_lock);
				for (auto it = mSources.begin(); it != mSources.end(); it++)
				{
					it->second.LogEdgeRemoval(id);
				}
			}*/

			// Audio
			void ProcessAudio(const size_t& id, const Buffer& data, matrix& reverbInput, Buffer& outputBuffer);

		private:
			inline void Reset() { mSources.clear(); mEmptySlots.clear(); }

			// Sources
			SourceMap mSources;
			std::vector<size_t> mEmptySlots;

			// Mutexes
			std::mutex updateMutex; // Locks during update step
			std::mutex processAudioMutex; // Locks during process audio step

			// Variables
			Binaural::CCore* mCore;
			Config mConfig;
		};
	}
}

#endif
