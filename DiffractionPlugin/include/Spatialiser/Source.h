/*
*
*  \Source class
*
*/

#ifndef Spatialiser_Source_h
#define Spatialiser_Source_h

// C++ headers
#include <unordered_map>

// Common headers
#include "Common/AudioManager.h"
#include "Common/Matrix.h"
#include "Common/Types.h" 

// Spatialiser headers
#include "Spatialiser/VirtualSource.h"
// #include "Spatialiser/Room.h"
#include "Spatialiser/Types.h"

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"

// Unity headers
#include "Unity/Debug.h"

using namespace Common;
namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// SourceData class ////////////////////

		class SourceData
		{
		public:

			// Load and Destroy
			SourceData() : position(vec3()), visible(false), vSources() {}
			SourceData(vec3 _position) : position(_position), visible(false), vSources() {}
			SourceData(const SourceData& data) : position(data.position), visible(data.visible), vSources(data.vSources) {}

			// Operators
			inline SourceData operator=(const SourceData& data)
			{
				position = data.position;
				visible = data.visible;
				vSources = data.vSources;
				return *this;
			}

			// Member variables
			vec3 position;
			bool visible;
			std::mutex mMutex;
			VirtualSourceDataMap vSources;

		private:
		};

		//////////////////// Source class ////////////////////

		class Source
		{
		public:

			// Load and Destroy
			Source(Binaural::CCore* core, const size_t& numFDNChannels, HRTFMode hrtfMode, int fs);
			Source(const Source& s) : mCore(s.mCore), mNumFDNChannels(s.mNumFDNChannels), mHRTFMode(s.mHRTFMode), sampleRate(s.sampleRate), mSource(s.mSource),
				targetGain(s.targetGain), currentGain(s.currentGain), isVisible(s.isVisible), mVirtualSources(s.mVirtualSources), mVirtualEdgeSources(s.mVirtualEdgeSources),
				oldData(s.oldData), freeFDNChannels(s.freeFDNChannels) {};
			~Source();

			// Operators
			inline Source operator=(const Source& s) {
				mCore = s.mCore; mNumFDNChannels = s.mNumFDNChannels; mHRTFMode = s.mHRTFMode; sampleRate = s.sampleRate; mSource = s.mSource;
				targetGain = s.targetGain; currentGain = s.currentGain; isVisible = s.isVisible; mVirtualSources = s.mVirtualSources; mVirtualEdgeSources = s.mVirtualEdgeSources;
				oldData = s.oldData; freeFDNChannels = s.freeFDNChannels;
				return *this;
			}

			// Getters
			inline shared_ptr<Binaural::CSingleSourceDSP>& GetSource() { return mSource; }

			// Updates
			void Update(const CTransform& transform, const SourceData& data);
			void UpdateVirtualSources(const VirtualSourceDataMap& data);
			bool UpdateVirtualSource(const VirtualSourceData& data, std::vector<VirtualSourceData>& newVSources);

			//inline void LogWallRemoval(const size_t& id) { removedWalls.push_back(id); }
			//inline void LogEdgeRemoval(const size_t& id) { removedEdges.push_back(id); }

			// Audio
			void ProcessAudio(const Real* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer, const Real lerpFactor);

			// Reset
			inline void Deactivate() { mSource = NULL; }
			inline void Reset() { mVirtualSources.clear(); }
			void ResetFDNSlots();

		private:
			// void RemoveVirtualSources();

			// Constants
			Binaural::CCore* mCore;
			size_t mNumFDNChannels;
			HRTFMode mHRTFMode;
			int sampleRate;

			// Audio data
			shared_ptr<Binaural::CSingleSourceDSP> mSource;
			Real targetGain;
			Real currentGain;
			bool isVisible;
			VirtualSourceDataMap oldData;

			// ISM tree structure
			VirtualSourceMap mVirtualSources;
			VirtualSourceMap mVirtualEdgeSources;

			std::vector<size_t> freeFDNChannels;
			//std::vector<size_t> removedWalls;
			//std::vector<size_t> removedEdges;

			// Mutexes
			std::mutex vWallMutex; // Protects mVirtualSources
			std::mutex vEdgeMutex; // Protects mVirtualEdgeSources
			std::mutex audioMutex; // Protects audio data
		};
	}
}

#endif