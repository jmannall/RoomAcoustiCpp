/*
* @class Source, SourceData
*
* @brief Declaration of Source and SourceData classes
*
*/

#ifndef RoomAcoustiCpp_Source_h
#define RoomAcoustiCpp_Source_h

// C++ headers
#include <unordered_map>

// Common headers
#include "Common/Matrix.h"
#include "Common/Types.h" 
#include "Common/Vec3.h"
#include "Common/Vec4.h"

// Spatialiser headers
#include "Spatialiser/VirtualSource.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/AirAbsorption.h"

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"

// Unity headers
#include "Unity/Debug.h"

using namespace Common;
namespace RAC
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
			VirtualSourceDataMap vSources;

		private:
		};

		//////////////////// Source class ////////////////////

		class Source
		{
		public:

			// Load and Destroy
			Source(Binaural::CCore* core, const Config& config);
			~Source();

			void UpdateSpatialisationMode(const HRTFMode mode);
			void UpdateSpatialisationMode(const SPATConfig config);

			// Getters
			inline shared_ptr<Binaural::CSingleSourceDSP>& GetSource() { return mSource; }

			// Updates
			void Update(const vec3& position, const vec4& orientation, const Real distance);
			void UpdateVirtualSources(const VirtualSourceDataMap& data);
			bool UpdateVirtualSource(const VirtualSourceData& data, std::vector<VirtualSourceData>& newVSources);

			inline SourceData GetData(const size_t& id) { lock_guard<std::mutex> lock(*dataMutex); mData.id = id; return mData; }
			inline void UpdateData(const SourceData& data) { lock_guard<std::mutex> lock(*dataMutex); mData.visible = data.visible; mData.vSources = data.vSources; }

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
			Buffer bStore;
			CMonoBuffer<float> bInput;
			CEarPair<CMonoBuffer<float>> bOutput;
			shared_ptr<Binaural::CSingleSourceDSP> mSource;
			AirAbsorption mAirAbsorption;
			Real targetGain;
			Real currentGain;
			VirtualSourceDataMap oldData;

			// IEM tree structure
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