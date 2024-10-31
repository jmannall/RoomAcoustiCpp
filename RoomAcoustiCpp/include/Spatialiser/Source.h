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

		typedef std::pair<size_t, vec3> IDPositionPair;

		//////////////////// Source class ////////////////////

		class Source
		{
		public:

			// Load and Destroy
			Source(Binaural::CCore* core, const Config& config);
			~Source();

			void UpdateSpatialisationMode(const SpatMode mode);
			void UpdateDiffractionModel(const DiffractionModel model);

			// Getters
			inline shared_ptr<Binaural::CSingleSourceDSP>& GetSource() { return mSource; }

			// Updates
			void Update(const vec3& position, const vec4& orientation, const Real distance);

			inline void UpdateData(const bool visible, const VirtualSourceDataMap& vSources)
			{ 
				BeginAirAbsorption();
				{ lock_guard<std::mutex> lock(*dataMutex); isVisible = visible; }
				EndAirAbsorption();
				Begin3DTI();
				{ lock_guard<std::mutex> lock(*vSourceDataMutex); targetVSources = vSources; }
				End3DTI();
			}
			vec3 GetPosition();
			
			// Audio
			void ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer);

			// Reset
			inline void Deactivate() { mSource = nullptr; }
			inline void Reset()
			{ 
				{ lock_guard<std::mutex> lock(*vSourcesMutex); mVSources.clear(); }
				ResetFDNSlots();
			}

		private:

			void UpdateVirtualSourceDataMap();
			void UpdateVirtualSources();
			bool UpdateVirtualSource(const VirtualSourceData& data);
			int AssignFDNChannel();

			void ResetFDNSlots();

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

			std::vector<int> freeFDNChannels;
			
			bool isVisible;
			VirtualSourceMap mVSources;

			VirtualSourceDataMap currentVSources;
			VirtualSourceDataMap targetVSources;

			// Mutexes
			shared_ptr<std::mutex> vWallMutex; // Protects mVirtualSources
			shared_ptr<std::mutex> vEdgeMutex; // Protects mVirtualEdgeSources
			shared_ptr<std::mutex> dataMutex; // Protects isVisible, targetGain, currentGain
			shared_ptr<std::mutex> vSourceDataMutex; // Protects currentVSources, targetVSources
			shared_ptr<std::mutex> vSourcesMutex; // Protects mVSources
		};
	}
}

#endif