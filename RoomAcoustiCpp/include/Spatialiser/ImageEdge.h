/*
* @class ImageEdge
*
* @brief Declaration of ImageEdge class
*
*/

#ifndef RoomAcoustiCpp_ImageEdge_h
#define RoomAcoustiCpp_ImageEdge_h

// Common headers
#include "Common/Vec3.h" 

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/SourceManager.h"
#include "Spatialiser/Room.h"
#include "Spatialiser/Reverb.h"

namespace RAC
{
	namespace Spatialiser
	{
		class ImageEdge
		{
		public:
			ImageEdge(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const int numBands);
			~ImageEdge() {}

			inline void UpdateIEMConfig(const IEMConfig& config) { lock_guard<std::mutex> lock(mMutex); mIEMConfigStore = config; configChanged = true; }
			inline void SetListenerPosition(const Vec3& position) { lock_guard<std::mutex> lock(mMutex); mListenerPositionStore = position; }
			inline Vec3 GetListenerPosition() { lock_guard<std::mutex> lock(mMutex); return mListenerPositionStore; }
			
			bool RunIEM();
			void UpdateLateReverbFilters();

		private:
			SourceAudioData ReflectPointInRoom(const SourceData& source, VirtualSourceDataMap& vSources);

			void UpdateRValid();
			// EdgeZone FindEdgeZone(const vec3& point, const std::vector<size_t> planeIDs);

			bool FindWallIntersection(Absorption& absorption, Vec3& intersection, const Vec3& start, const Vec3& end, const Plane& plane) const;

			bool FindIntersection(Vec3& intersection, Absorption& absorption, const Vec3& start, const Vec3& end, const Plane& plane) const;
			bool FindIntersections(std::vector<Vec3>& intersections, VirtualSourceData& vSource, int bounceIdx) const;
			bool FindIntersections(std::vector<Vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const Vec3& start) const;

			bool FindWallObstruction(const Vec3& start, const Vec3& end, const Plane& plane) const;

			bool LineRoomObstruction(const Vec3& start, const Vec3& end) const;
			void LineRoomObstruction(const Vec3& start, const Vec3& end, bool& obstruction) const;
			bool LineRoomObstruction(const Vec3& start, const Vec3& end, size_t currentPlaneID) const;
			void LineRoomObstruction(const Vec3& start, const Vec3& end, size_t currentPlaneID, bool& obstruction) const;
			bool LineRoomObstruction(const Vec3& start, const Vec3& end, size_t currentPlaneId1, size_t currentPlaneId2) const;
			void LineRoomObstruction(const Vec3& start, const Vec3& end, size_t currentPlaneId1, size_t currentPlaneId2, bool& obstruction) const;

			size_t FirstOrderDiffraction(const SourceData& source, VirtualSourceDataMap& vSources);
			size_t FirstOrderReflections(const SourceData& source, VirtualSourceDataMap& vSources, size_t counter);
			void HigherOrderReflections(const SourceData& source, VirtualSourceDataMap& vSources);

			Real CalculateDirectivity(const SourceData& source, const Vec3& intersection) const;
			void InitVSource(const SourceData& source, const Vec3& intersection, VirtualSourceData& vSource, VirtualSourceDataMap& vSources, bool feedsFDN);
			void EraseOldEntries(VirtualSourceDataMap& vSources);

			weak_ptr<Room> mRoom;
			weak_ptr<SourceManager> mSourceManager;
			weak_ptr<Reverb> mReverb;

			// Only accessed from background thread
			PlaneMap mPlanes;
			WallMap mWalls;
			EdgeMap mEdges;
			std::vector<SourceData> mSources;
			std::vector<VirtualSourceDataMap> mVSources;
			std::vector<SourceAudioData> mSourceAudioDatas;
			std::vector<bool> mCurrentCycles;

			IEMConfig mIEMConfig;
			Vec3 mListenerPosition;
			IEMConfig mIEMConfigStore;
			Vec3 mListenerPositionStore;

			int numAbsorptionBands;
			std::vector<Vec3> reverbDirections;
			std::vector<Absorption> reverbAbsorptions;

			VirtualSourceDataStore sp;

			bool currentCycle;
			bool configChanged;

			std::mutex mMutex;
		};
	}
}

#endif