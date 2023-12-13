/*
*
*  \Spatialiser source
*
*/

#ifndef Spatialiser_Source_h
#define Spatialiser_Source_h

#include "Spatialiser/VirtualSource.h"
#include "Spatialiser/Room.h"
#include "Spatialiser/Types.h"
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"
#include <unordered_map>
#include "Debug.h"

namespace Spatialiser
{
	using VirtualSourceMap = std::unordered_map<size_t, VirtualSource>;

	class Source
	{
	public:
		Source(Binaural::CCore* core, const size_t& numFDNChannels, HRTFMode hrtfMode, int fs);
		Source(const Source& s) : mCore(s.mCore), mNumFDNChannels(s.mNumFDNChannels), mHRTFMode(s.mHRTFMode), sampleRate(s.sampleRate), mSource(s.mSource),
			targetGain(s.targetGain), currentGain(s.currentGain), isVisible(s.isVisible), mVirtualSources(s.mVirtualSources), mVirtualEdgeSources(s.mVirtualEdgeSources),
			oldData(s.oldData), freeFDNChannels(s.freeFDNChannels), removedWalls(s.removedWalls), removedEdges(s.removedEdges) {};
		~Source();

		inline Source operator=(const Source& s) {
			mCore = s.mCore; mNumFDNChannels = s.mNumFDNChannels; mHRTFMode = s.mHRTFMode; sampleRate = s.sampleRate; mSource = s.mSource;
			targetGain = s.targetGain; currentGain = s.currentGain; isVisible = s.isVisible; mVirtualSources = s.mVirtualSources; mVirtualEdgeSources = s.mVirtualEdgeSources;
			oldData = s.oldData; freeFDNChannels = s.freeFDNChannels; removedWalls = s.removedWalls; removedEdges = s.removedEdges;
			return *this;
		}

		inline shared_ptr<Binaural::CSingleSourceDSP>& GetSource() { return mSource; }

		void Update(const Common::CTransform& transform, const SourceData& data);
		void UpdateVirtualSources(const VirtualSourceStore& data);
		bool UpdateVirtualSource(const VirtualSourceData& data, std::vector<VirtualSourceData>& newVSources);

		void ResetFDNSlots();
		inline void LogWallRemoval(const size_t& id) { removedWalls.push_back(id); }
		inline void LogEdgeRemoval(const size_t& id) { removedEdges.push_back(id); }

		void ProcessAudio(const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer, const float lerpFactor);

		inline void Deactivate() { mSource = NULL; }
		inline void Reset() { mVirtualSources.clear(); }

	private:
		void RemoveVirtualSources();

		// Constants
		Binaural::CCore* mCore;
		size_t mNumFDNChannels;
		HRTFMode mHRTFMode;
		int sampleRate;

		// Audio data
		shared_ptr<Binaural::CSingleSourceDSP> mSource;
		float targetGain;
		float currentGain;
		bool isVisible;

		// ISM tree structure
		VirtualSourceMap mVirtualSources;
		VirtualSourceMap mVirtualEdgeSources;
		VirtualSourceStore oldData;

		std::vector<size_t> freeFDNChannels;
		std::vector<size_t> removedWalls;
		std::vector<size_t> removedEdges;

		std::mutex vWallMutex; // Protects mVirtualSources
		std::mutex vEdgeMutex; // Protects mVirtualEdgeSources
		std::mutex audioMutex; // Protects audio data
	};
}

#endif