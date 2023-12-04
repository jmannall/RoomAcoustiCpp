#pragma once

#ifndef SPATIALISER_HRTFMANAGER_H_
#define SPATIALISER_HRTFMANAGER_H_

#include "Spatialiser/VirtualSource.h"
#include "Spatialiser/Room.h"
#include "Spatialiser/Types.h"
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"
#include <unordered_map>
#include "Debug.h"

namespace Spatialiser
{
#pragma region SourceManager

	using VirtualSourceMap = std::unordered_map<size_t, VirtualSource>;

	class SourceNew
	{
	public:
		SourceNew(Binaural::CCore* core, const size_t& numFDNChannels, HRTFMode hrtfMode, int fs);
		~SourceNew();

		void ResetFDNSlots();
		void Update(const Common::CTransform& transform, const SourceData& data);
		inline shared_ptr<Binaural::CSingleSourceDSP>& GetSource() { return mSource; }
		/*inline void GetSources(std::vector<shared_ptr<Binaural::CSingleSourceDSP>>& sources)
		{
			if (isVisible)
			{
				sources.push_back(mSource);
			}
			for (int i = 0; i < mVirtualSources.size(); i++)
			{
				mVirtualSources[i].GetSources(sources);
			}
		}*/
		void UpdateVirtualSources(const std::vector<VirtualSourceData>& data);
		void UpdateVirtualSource(const VirtualSourceData& data);
		inline void LogWallRemoval(const size_t& id) { removedWalls.push_back(id); }
		inline void LogEdgeRemoval(const size_t& id) { removedEdges.push_back(id); }
		void ProcessAudio(const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer);

		inline void Deactivate() { mSource = NULL; }
		inline void Reset() { mVirtualSources.clear(); }

	private:
		void RemoveVirtualSources();

		shared_ptr<Binaural::CSingleSourceDSP> mSource;
		VirtualSourceMap mVirtualSources;
		VirtualSourceMap mVirtualEdgeSources;
		size_t mNumFDNChannels;
		std::vector<int> freeFDNChannels;
		bool isVisible;
		HRTFMode mHRTFMode;
		int sampleRate;
		std::vector<size_t> removedWalls;
		std::vector<size_t> removedEdges;

		Binaural::CCore* mCore;
	};

	class HRTFManager
	{
		using SourceMap = std::unordered_map<size_t, SourceNew>;
		using MutexMap = std::unordered_map<size_t, std::mutex>;
	public:
		HRTFManager(Binaural::CCore* core, const size_t& numFDNChannels, HRTFMode hrtfMode, int fs) : mSources(), mEmptySlots(), mCore(core), mNumFDNChannels(numFDNChannels), mHRTFMode(hrtfMode), sampleRate(fs) {};
		~HRTFManager() { Reset(); };

		size_t Init();
		inline void Update(const size_t& id, const Common::CTransform& transform, const SourceData& data)
		{
			lock_guard <mutex> lock(mCoreMutex); // Lock before locate to ensure not deleted between found and mutex lock
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
			lock_guard <mutex> lock(mCoreMutex);
			mSources.erase(id);
			mEmptySlots.push_back(id);
		}
		inline void LogWallRemoval(const size_t& id)
		{ 
			lock_guard <mutex> lock(mCoreMutex);
			for (auto it = mSources.begin(); it != mSources.end(); it++)
			{
				it->second.LogWallRemoval(id);
			}
		}
		inline void LogEdgeRemoval(const size_t& id)
		{
			lock_guard <mutex> lock(mCoreMutex);
			for (auto it = mSources.begin(); it != mSources.end(); it++)
			{
				it->second.LogEdgeRemoval(id);
			}
		}
		void ProcessAudio(const size_t& id, const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer);

	private:
		inline void Reset() { mSources.clear(); mEmptySlots.clear(); }
		// vec3 GetSourcePosition(shared_ptr<Binaural::CSingleSourceDSP> source);

		SourceMap mSources;
		MutexMap mMutexes; // For later extension to multiple sources?
		std::vector<size_t> mEmptySlots;
		std::mutex mCoreMutex;

		Binaural::CCore* mCore;
		size_t mNumFDNChannels;
		HRTFMode mHRTFMode;
		int sampleRate;
	};
#pragma endregion
}

#endif SPATIALISER_HRTFMANAGER_H_
