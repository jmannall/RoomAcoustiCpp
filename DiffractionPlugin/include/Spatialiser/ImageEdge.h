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
			inline void SetListenerPosition(const vec3& position) { lock_guard<std::mutex> lock(mMutex); mListenerPositionStore = position; }
			inline vec3 GetListenerPosition() { lock_guard<std::mutex> lock(mMutex); return mListenerPositionStore; }
			
			void RunIEM();
			void UpdateLateReverbFilters();

		private:
			bool ReflectPointInRoom(const vec3& point, VirtualSourceDataMap& vSources);

			void UpdateRValid();
			// EdgeZone FindEdgeZone(const vec3& point, const std::vector<size_t> planeIDs);

			bool FindWallIntersection(const vec3& intersection, const vec3& start, const vec3& end, const Plane& plane) const;
			bool FindWallIntersection(Absorption& absorption, const vec3& intersection, const vec3& start, const vec3& end, const Plane& plane) const;

			bool FindIntersection(vec3& intersection, Absorption& absorption, const vec3& start, const vec3& end, const Plane& plane);
			bool FindIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx);
			//bool FindIntersectionsSpEd(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx);
			//bool FindIntersectionsEdSp(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx);
			//bool FindIntersectionsSpEdSp(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, int edgeIdx);
			bool FindIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const vec3& start);
			//bool FindRIntersections(std::vector<vec3>& intersections, VirtualSourceData& vSource, int bounceIdx, const vec3& start);

			bool LineRoomObstruction(const vec3& start, const vec3& end);
			void LineRoomObstruction(const vec3& start, const vec3& end, bool& obstruction);
			bool LineRoomObstruction(const vec3& start, const vec3& end, size_t currentWallID);
			void LineRoomObstruction(const vec3& start, const vec3& end, size_t currentWallID, bool& obstruction);
			bool LineRoomObstruction(const vec3& start, const vec3& end, size_t currentWallID1, size_t currentWallID2);

			// void HigherOrderSpecularDiffraction(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources);
			void FirstOrderDiffraction(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources);
			void FirstOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources);
			void HigherOrderReflections(const vec3& point, VirtualSourceDataStore& sp, VirtualSourceDataMap& vSources);

			shared_ptr<Room> mRoom;
			shared_ptr<SourceManager> mSourceManager;
			shared_ptr<Reverb> mReverb;

			// Only accessed from background thread
			PlaneMap mPlanes;
			WallMap mWalls;
			EdgeMap mEdges;
			std::vector<SourceData> mSources;
			IEMConfig mIEMConfig;
			vec3 mListenerPosition;
			IEMConfig mIEMConfigStore;
			vec3 mListenerPositionStore;

			size_t numAbsorptionBands;
			std::vector<vec3> reverbDirections;
			std::vector<Absorption> reverbAbsorptions;

			std::mutex mMutex;
		};
	}
}

#endif