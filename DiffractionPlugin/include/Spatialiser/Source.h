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
#include "Common/Vec3.h"
#include "Common/Vec4.h"

// Spatialiser headers
#include "Spatialiser/VirtualSource.h"
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
			SourceData() : id(0), mPosition(vec3()), visible(false), vSources() {}
			SourceData(const size_t ID, vec3 position) : id(ID), mPosition(position), visible(false), vSources() {}
			SourceData(const SourceData& data) : id(data.id), mPosition(data.mPosition), visible(data.visible), vSources(data.vSources) {}

			// Operators
			inline SourceData operator=(const SourceData& data)
			{
				id = data.id;
				mPosition = data.mPosition;
				visible = data.visible;
				vSources = data.vSources;
				return *this;
			}

			// Member variables
			size_t id;
			vec3 mPosition;
			bool visible;
			// std::mutex mMutex;
			VirtualSourceDataMap vSources;

		private:
		};

		//////////////////// Source class ////////////////////

		class Source
		{
		public:

			// Load and Destroy
			Source(Binaural::CCore* core, const Config& config);
			/*Source(const Source& s) : mCore(s.mCore), mConfig(s.mConfig), mSource(s.mSource),
				bInput(s.bInput), targetGain(s.targetGain), currentGain(s.currentGain), mVirtualSources(s.mVirtualSources), mVirtualEdgeSources(s.mVirtualEdgeSources),
				oldData(s.oldData), freeFDNChannels(s.freeFDNChannels) {};*/
			~Source();

			void UpdateSpatialisationMode(const HRTFMode& mode);
			void UpdateSpatialisationMode(const SPATConfig& config);

			// Operators
			/*inline Source operator=(const Source& s) {
				mCore = s.mCore; mConfig = s.mConfig; mSource = s.mSource;
				targetGain = s.targetGain; currentGain = s.currentGain; mVirtualSources = s.mVirtualSources; mVirtualEdgeSources = s.mVirtualEdgeSources;
				oldData = s.oldData; freeFDNChannels = s.freeFDNChannels;
				return *this;
			}*/

			// Getters
			inline shared_ptr<Binaural::CSingleSourceDSP>& GetSource() { return mSource; }

			// Updates
			void Update(const vec3& position, const vec4& orientation);
			void UpdateVirtualSources(const VirtualSourceDataMap& data);
			bool UpdateVirtualSource(const VirtualSourceData& data, std::vector<VirtualSourceData>& newVSources);

			inline SourceData GetData(const size_t& id) { lock_guard<std::mutex> lock(*dataMutex); mData.id = id; return mData; }
			inline void UpdateData(const SourceData& data) { lock_guard<std::mutex> lock(*dataMutex); mData = data; }

			//inline void LogWallRemoval(const size_t& id) { removedWalls.push_back(id); }
			//inline void LogEdgeRemoval(const size_t& id) { removedEdges.push_back(id); }

			// Audio
			void ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer);

			// Reset
			inline void Deactivate() { mSource = NULL; }
			inline void Reset() { mVirtualSources.clear(); ResetFDNSlots(); }
			void ResetFDNSlots();

		private:
			// void RemoveVirtualSources();

			// Constants
			Binaural::CCore* mCore;
			Config mConfig;

			// Audio data
			CMonoBuffer<float> bInput;
			CEarPair<CMonoBuffer<float>> bOutput;
			shared_ptr<Binaural::CSingleSourceDSP> mSource;
			Real targetGain;
			Real currentGain;
			VirtualSourceDataMap oldData;

			// ISM tree structure
			VirtualSourceMap mVirtualSources;
			VirtualSourceMap mVirtualEdgeSources;

			std::vector<int> freeFDNChannels;
			
			SourceData mData;

			// Mutexes
			shared_ptr<std::mutex> vWallMutex; // Protects mVirtualSources
			shared_ptr<std::mutex> vEdgeMutex; // Protects mVirtualEdgeSources
			shared_ptr<std::mutex> dataMutex; // Protects mData and current/target Gain
		};
	}
}

#endif