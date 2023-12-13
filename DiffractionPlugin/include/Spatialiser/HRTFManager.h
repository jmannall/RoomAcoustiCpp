/*
*
*  \Spatialiser source manager
*
*/

#ifndef Spatialiser_SourceManager_h
#define Spatialiser_SourceManager_h

#include "Spatialiser/VirtualSource.h"
#include "Spatialiser/Source.h"
#include "Spatialiser/Room.h"
#include "Spatialiser/Types.h"
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"
#include <unordered_map>
#include "Debug.h"

namespace Spatialiser
{

	using VirtualSourceMap = std::unordered_map<size_t, VirtualSource>;

	

	class HRTFManager
	{
		using SourceMap = std::unordered_map<size_t, Source>;
		using MutexMap = std::unordered_map<size_t, std::mutex>;
	public:
		HRTFManager(Binaural::CCore* core, const size_t& numFDNChannels, HRTFMode hrtfMode, int fs) : mSources(), mEmptySlots(), mCore(core), mNumFDNChannels(numFDNChannels), mHRTFMode(hrtfMode), sampleRate(fs) {};
		~HRTFManager() { Reset(); };

		size_t Init();
		inline void Update(const size_t& id, const Common::CTransform& transform, const SourceData& data)
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
		inline void LogWallRemoval(const size_t& id)
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
		}
		void ProcessAudio(const size_t& id, const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer, const float lerpFactor);

	private:
		inline void Reset() { mSources.clear(); mEmptySlots.clear(); }
		// vec3 GetSourcePosition(shared_ptr<Binaural::CSingleSourceDSP> source);

		SourceMap mSources;
		MutexMap mMutexes; // For later extension to multiple sources?
		std::vector<size_t> mEmptySlots;
		
		std::mutex updateMutex; // Locks during update step
		std::mutex processAudioMutex; // Locks during process audio step

		Binaural::CCore* mCore;
		size_t mNumFDNChannels;
		HRTFMode mHRTFMode;
		int sampleRate;
	};
}

#endif
