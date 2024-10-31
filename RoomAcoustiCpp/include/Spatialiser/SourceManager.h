/*
* @class SourceManager
*
* @brief Declaration of SourceManager class
*
*/

#ifndef RoomAcoustiCpp_SourceManager_h
#define RoomAcoustiCpp_SourceManager_h

// C++ headers
#include <unordered_map>
#include <mutex>
#include <ctime>
#include <shared_mutex>

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/Source.h"
#include "Spatialiser/Mutexes.h"

// 3DTI headers
#include "BinauralSpatializer/Core.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		//////////////////// SourceManager class ////////////////////

		class SourceManager
		{
		public:

			// Load and Destroy
			SourceManager(Binaural::CCore* core, const Config& config) : mSources(), mEmptySlots(), mCore(core), mConfig(config), nextSource(0) {};
			~SourceManager()
			{ 
				lock(updateMutex, processAudioMutex);
				unique_lock<shared_mutex> lk1(updateMutex, std::adopt_lock);
				lock_guard<mutex> lk2(processAudioMutex, std::adopt_lock); 
				Reset();
			};

			void UpdateSpatialisationMode(const SpatMode mode);
			void UpdateDiffractionModel(const DiffractionModel model);

			// Sources
			size_t Init();

			inline void Update(const size_t id, const vec3& position, const vec4& orientation, Real& distance)
			{
				shared_lock<shared_mutex> lock(updateMutex); // Lock before locate to ensure not deleted between found and mutex lock
				auto it = mSources.find(id);
				if (it != mSources.end()) { it->second.Update(position, orientation, distance); } // case: source does exist
			}

			inline void Remove(const size_t id)
			{
				lock(updateMutex, processAudioMutex);
				unique_lock<shared_mutex> lk1(updateMutex, std::adopt_lock);
				lock_guard<std::mutex> lk2(processAudioMutex, std::adopt_lock);

				size_t removed = mSources.erase(id);
				while (!mTimers.empty() && difftime(time(nullptr), mTimers.front().time) > 60)
				{
					mEmptySlots.push_back(mTimers.front().id);
					mTimers.erase(mTimers.begin());
				}

				if (removed == 0)
					return;

				sourceData.pop_back();
				mTimers.push_back(TimerPair(id, time(nullptr)));
			}

			std::vector<IDPositionPair> GetSourceData();
			inline vec3 GetSourcePosition(const size_t id)
			{
				shared_lock<shared_mutex> shared_lock(updateMutex); // Lock before locate to ensure not deleted between found and mutex lock
				lock_guard<mutex> lock(tuneInMutex);
				auto it = mSources.find(id);
				if (it != mSources.end()) { return it->second.GetPosition(); } // case: source does exist
				return vec3();
			}

			void UpdateSourceData(const size_t id, const bool visible, const VirtualSourceDataMap& vSources);

			// Audio
			void ProcessAudio(const size_t id, const Buffer& data, matrix& reverbInput, Buffer& outputBuffer);

		private:
			inline void Reset() { mSources.clear(); mEmptySlots.clear(); }

			// Sources
			SourceMap mSources;
			std::vector<size_t> mEmptySlots;
			std::vector<TimerPair> mTimers;
			size_t nextSource;
			std::vector<IDPositionPair> sourceData;

			// Mutexes
			std::shared_mutex updateMutex; // Locks during update step
			std::mutex processAudioMutex; // Locks during process audio step

			// Variables
			Binaural::CCore* mCore;
			Config mConfig;
		};
	}
}

#endif
