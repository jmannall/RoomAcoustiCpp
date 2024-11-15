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
#include "Spatialiser/Mutexes.h"

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
		struct SourceData
		{
			size_t id;
			Vec3 position;
			Vec3 forward;
			Vec4 orientation;
			SourceDirectivity directivity;
			bool hasChanged;
		};

		typedef std::pair<Real, bool> SourceAudioData;

		//////////////////// Source class ////////////////////

		class Source
		{
		public:

			// Load and Destroy
			Source(Binaural::CCore* core, const Config& config);
			~Source();

			void UpdateSpatialisationMode(const SpatMode mode);
			void UpdateDiffractionModel(const DiffractionModel model);
			inline void UpdateDirectivity(const SourceDirectivity directivity)
			{
				{ 
					lock_guard<mutex> lock(*dataMutex);
					mDirectivity = directivity;
					switch (mDirectivity)
					{
					case SourceDirectivity::omni:
						reverbEnergy = 1.0;
						break;
					case SourceDirectivity::cardioid:
						reverbEnergy = 0.5;
						break;
					default:
						reverbEnergy = 1.0;
						break;
					}
				}
				{ lock_guard<mutex> lock(*currentDataMutex); hasChanged = true; }
			}
			
			// Getters
			inline shared_ptr<Binaural::CSingleSourceDSP>& GetSource() { return mSource; }

			// Updates
			void Update(const Vec3& position, const Vec4& orientation, const Real distance);

			inline void UpdateData(const SourceAudioData source, const VirtualSourceDataMap& vSources)
			{ 
				{ lock_guard<mutex> lock(*dataMutex); targetGain = source.first; feedsFDN = source.second; }
				targetVSources = vSources;
				// { lock_guard<std::mutex> lock(*vSourceDataMutex); targetVSources = vSources; }
				UpdateVirtualSourceDataMap();
				UpdateVirtualSources();
				/*{
					lock_guard<mutex> lock(tuneInMutex);
					for (auto& [key, vSource] : mVSources)
						vSource.UpdateTransform();
				}*/
			}

			Vec3 GetPosition() const { lock_guard<std::mutex>lock(*currentDataMutex); return currentPosition; };
			Vec4 GetOrientation() const { lock_guard<std::mutex>lock(*currentDataMutex); return currentOrientation; };

			SourceDirectivity GetDirectivity() { lock_guard<mutex> lock(*dataMutex); return mDirectivity; }

			bool HasChanged() { lock_guard<mutex> lock(*currentDataMutex); if (hasChanged) { hasChanged = false; return true; } return false; }

			inline CTransform GetTransform() { return mSource->GetCurrentSourceTransform(); }
			
			// Audio
			void ProcessAudio(const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer);

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
			CEarPair<CMonoBuffer<float>> bOutput;	// 3DTI stereo output buffer
			CMonoBuffer<float> bMonoOutput;			// 3DTI Mono output buffer
			shared_ptr<Binaural::CSingleSourceDSP> mSource;
			AirAbsorption mAirAbsorption;
			Real targetGain;
			Real currentGain;

			Real reverbEnergy;
			SourceDirectivity mDirectivity;
			bool feedsFDN;

			std::vector<int> freeFDNChannels;

			Vec3 currentPosition;
			Vec4 currentOrientation;
			bool hasChanged;
			CTransform transform;
			
			VirtualSourceMap mVSources;

			VirtualSourceDataMap currentVSources;
			VirtualSourceDataMap targetVSources;

			// Mutexes
			shared_ptr<std::mutex> vWallMutex; // Protects mVirtualSources
			shared_ptr<std::mutex> vEdgeMutex; // Protects mVirtualEdgeSources
			shared_ptr<std::mutex> dataMutex; // Protects isVisible, targetGain, currentGain, mDirectivity
			shared_ptr<std::mutex> vSourceDataMutex; // Protects currentVSources, targetVSources
			shared_ptr<std::mutex> vSourcesMutex; // Protects mVSources
			shared_ptr<std::mutex> currentDataMutex; // Protects currentPosition, currentOrientation, hasChanged
		};
	}
}

#endif