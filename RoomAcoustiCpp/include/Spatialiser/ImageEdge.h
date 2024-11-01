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
			ImageEdge(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const size_t numBands);
			~ImageEdge() {}

			inline void UpdateIEMConfig(const IEMConfig& config) { lock_guard<std::mutex> lock(mMutex); mIEMConfigStore = config; }
			inline void SetListenerPosition(const Vec3& position) { lock_guard<std::mutex> lock(mMutex); mListenerPositionStore = position; }
			inline Vec3 GetListenerPosition() { lock_guard<std::mutex> lock(mMutex); return mListenerPositionStore; }
			
			void RunIEM();
			void UpdateLateReverbFilters();

		private:
			bool ReflectPointInRoom(const Vec3& point, VirtualSourceDataMap& vSources);

			void UpdateRValid();
			// EdgeZone FindEdgeZone(const vec3& point, const std::vector<size_t> planeIDs);

			bool FindWallIntersection(Absorption& absorption, Vec3& intersection, const Vec3& start, const Vec3& end, const Plane& plane) const;

			bool FindIntersection(Vec3& intersection, Absorption& absorption, const Vec3& start, const Vec3& end, const Plane& plane) const;
			bool FindIntersections(std::vector<Vec3>& intersections, VirtualSourceData& vSource, int bounceIdx) const;
			bool FindIntersections(std::vector<Vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const Vec3& start) const;

			bool FindWallObstruction(const Vec3& start, const Vec3& end, const Plane& plane) const;

			bool LineRoomObstruction(const Vec3& start, const Vec3& end);
			void LineRoomObstruction(const Vec3& start, const Vec3& end, bool& obstruction);
			bool LineRoomObstruction(const Vec3& start, const Vec3& end, int currentPlaneID);
			void LineRoomObstruction(const Vec3& start, const Vec3& end, int currentPlaneID, bool& obstruction);
			bool LineRoomObstruction(const Vec3& start, const Vec3& end, int currentPlaneId1, int currentPlaneId2);
			void LineRoomObstruction(const Vec3& start, const Vec3& end, int currentPlaneId1, int currentPlaneId2, bool& obstruction);

			size_t FirstOrderDiffraction(const Vec3& point, VirtualSourceDataMap& vSources);
			size_t FirstOrderReflections(const Vec3& point, VirtualSourceDataMap& vSources, size_t counter);
			void HigherOrderReflections(const Vec3& point, VirtualSourceDataMap& vSources);

			void EraseOldEntries(VirtualSourceDataMap& vSources);

			weak_ptr<Room> mRoom;
			weak_ptr<SourceManager> mSourceManager;
			weak_ptr<Reverb> mReverb;

			// Only accessed from background thread
			PlaneMap mPlanes;
			WallMap mWalls;
			EdgeMap mEdges;
			std::vector<IDPositionPair> mSources;
			std::vector<VirtualSourceDataMap> mVSources;
			IEMConfig mIEMConfig;
			Vec3 mListenerPosition;
			IEMConfig mIEMConfigStore;
			Vec3 mListenerPositionStore;

			size_t numAbsorptionBands;
			std::vector<Vec3> reverbDirections;
			std::vector<Absorption> reverbAbsorptions;

			VirtualSourceDataStore sp;

			bool currentCycle;

			std::mutex mMutex;
		};
	}
}

#endif