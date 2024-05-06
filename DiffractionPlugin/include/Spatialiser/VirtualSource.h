/*
* @class VirtualSource, VirtualsourceData
*
* @brief Declaration of VirtualSource and VirtualsourceData classes
*
* @remarks Currently, it can only handle one diffracting edge per path/virtual source
*
*/

#ifndef RoomAcoustiCpp_VirtualSource_h
#define RoomAcoustiCpp_VirtualSource_h

// C++ headers
#include <vector>
#include <unordered_map>

// Common headers
#include "Common/Vec3.h"
#include "Common/Matrix.h"

// Spatialiser headers
#include "Spatialiser/Wall.h"
#include "Spatialiser/Types.h"
#include "Diffraction/Path.h"
#include "Diffraction/Models.h"
#include "Spatialiser/AirAbsorption.h"

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"

// Unity headers
#include "Unity/Debug.h"

// DSP headers
#include "DSP/GraphicEQ.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	using namespace Unity;
	namespace Spatialiser
	{

		struct Part
		{
			bool isReflection;
			size_t id;
			Part(const size_t _id, const bool _isReflection) : id(_id), isReflection(_isReflection) {};
		};

		//////////////////// VirtualSourceData class ////////////////////

		class VirtualSourceData
		{
		public:

			// Load and Destroy
			VirtualSourceData(size_t numBands) : valid(false), visible(false), feedsFDN(false), mFDNChannel(-1), order(0), reflection(false), diffraction(false), key(""), mAbsorption(numBands), distance(1.0) {};
			~VirtualSourceData() { /*Clear();*/ };

			// Plane
			/*inline void AddPlaneIDs(const std::vector<size_t>& ids)
			{
				for (size_t id : ids)
				{
					pathParts.push_back(Part(id, true));
					order++;
					key = key + std::to_string(id) + "r";
				}
				reflection = true;
			}*/
			inline void AddPlaneID(const size_t id)
			{
				pathParts.emplace_back(id, true);
				order++;
				reflection = true;
				key = key + std::to_string(id) + "r";
			}
			//inline void AddPlaneIDToStart(const size_t id) // IDs are added in reverse order (from listener to source)
			//{
			//	pathParts.insert(pathParts.begin(), Part(id, true));
			//	key = std::to_string(id) + "r" + key;
			//}
			/*inline void RemovePlaneIDs()
			{
				key = "";
				mAbsorption.Reset();
				if (diffraction)
				{
					std::vector<Part>::iterator iter;
					for (iter = pathParts.begin(); iter != pathParts.end(); )
					{
						if (iter->isReflection)
							iter = pathParts.erase(iter);
						else
						{
							key = key + std::to_string(iter->id) + "d";
							++iter;
						}
					}
				}
				else
					pathParts.clear();
			}*/

			// Absorption
			inline void AddAbsorption(const Absorption& absorption) { mAbsorption *= absorption; }
			inline void ResetAbsorption() { mAbsorption = 1.0; }
			inline Absorption& GetAbsorptionRef() { return mAbsorption; }

			// Edge
			inline void AddEdgeID(const size_t id)
			{
				pathParts.emplace_back(id, false);
				order++;
				diffraction = true;
				key = key + std::to_string(id) + "d";
			}
			/*inline void AddEdgeIDToStart(const size_t id, const Diffraction::Path path)
			{
				pathParts.insert(pathParts.begin(), Part(id, false));
				order++;
				mDiffractionPath = path;
				diffraction = true;
				key = std::to_string(id) + "d" + key;
			}*/
			/*inline void FlipPath()
			{
				std::reverse(pathParts.begin(), pathParts.end());
				key = "";
				for (Part part : pathParts)
					key = key + std::to_string(part.id) + "r";
			}*/

			// Getters
			size_t GetID() const { return pathParts.back().id; }
			size_t GetID(int i) const { return pathParts[i].id; }
			// std::vector<size_t> GetPlaneIDs() const;
			std::string GetKey() const { return key; }
			inline bool IsReflection(int i) const { return pathParts[i].isReflection; }
			inline Absorption GetAbsorption() const { return mAbsorption; }
			inline size_t GetOrder() const { return order; }

			// Transforms
			void SetTransform(const vec3& vSourcePosition);
			void SetTransform(const vec3& vSourcePosition, const vec3& vEdgeSourcePosition);
			// void UpdateTransform(const vec3& vEdgeSourcePosition);

			inline vec3 GetPosition() const { return mPositions.back(); }
			vec3 GetPosition(int i) const;
			// vec3 GetTransformPosition();
			
			// Receiver position
			// void SetRPosition(const vec3& vReceiverPosition);
			// inline void SetRPositions(const std::vector<vec3>& vReceiverPositions) { mRPositions = vReceiverPositions; };
			// void SetRTransform(const vec3& vReceiverPosition, const vec3& vEdgeSourcePosition);
			
			// vec3 GetRPosition() const { return mRPositions.back(); }
			// vec3 GetRPosition(const int i) const;
			// inline std::vector<vec3> GetRPositions() const { return mRPositions; };

			// Diffraction
			inline void UpdateDiffractionPath(const vec3& source, const vec3& receiver, const Edge& edge)
			{
				mDiffractionPath.UpdateParameters(source, receiver, edge);
				//vec3 position = mDiffractionPath.CalculateVirtualPostion();
				SetTransform(mDiffractionPath.GetApex(), mDiffractionPath.CalculateVirtualPostion());
			}
			inline void UpdateDiffractionPath(const vec3& source, const vec3& receiver, const Plane& plane)
			{
				mDiffractionPath.ReflectEdgeInPlane(plane);
				mDiffractionPath.UpdateParameters(source, receiver);
				//vec3 position = mDiffractionPath.CalculateVirtualPostion();
				SetTransform(mDiffractionPath.GetApex(), mDiffractionPath.CalculateVirtualPostion());
			}
			const inline Edge& GetEdge() const { return mDiffractionPath.GetEdge(); }
			inline vec3 GetApex() const { return mDiffractionPath.GetApex(); }
			// inline bool GetSValid() const { return mDiffractionPath.sValid; }
			// inline bool GetRValid() const { return mDiffractionPath.rValid; }

			// Visibility and Validity
			inline void Visible(const bool fdn) { visible = true; feedsFDN = fdn; }
			inline void Invisible() { visible = false; }
			inline void Valid() { valid = true; }
			inline void Invalid() { valid = false; }
			// inline void RValid() { rValid = true; }
			// inline void RInvalid() { rValid = false; }

			// Reset
			inline void Reset() { Invalid();  Invisible(); ResetAbsorption(); }
			inline void Clear()
			{ 
				Reset();
				pathParts.clear();
				mPositions.clear();
				reflection = false;
				diffraction = false;
				order = 0;
				key = "";
			}
			inline void Update(const VirtualSourceData& vS)
			{
				pathParts = vS.pathParts;
				mPositions = vS.mPositions;
				reflection = vS.reflection;
				diffraction = vS.diffraction;
				if (diffraction)
					mDiffractionPath = vS.mDiffractionPath;
				order = vS.order;
				key = vS.key;
			}

			void SetDistance(const vec3& listenerPosition);

			VirtualSourceData Trim(const int i);

			// Status data
			bool reflection;
			bool diffraction;
			bool valid;
			bool visible;
			// bool rValid;
			bool feedsFDN;

			// DSP 
			int mFDNChannel;
			CTransform transform;
			Diffraction::Path mDiffractionPath;
			Real distance;

		private:
			std::string key;
			std::vector<Part> pathParts;
			// std::vector<size_t> planeIds;
			std::vector<vec3> mPositions;
			// std::vector<vec3> mRPositions;
			size_t order;
			Absorption mAbsorption;
		};

		//////////////////// VirtualSource class ////////////////////

		class VirtualSource // Divide into sub classes types: reflection, diffraction, both
		{
		public:

			// Load and Destroy
			VirtualSource(const Config& config) : mCore(NULL), mSource(NULL), order(0), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(config.frequencyBands, config.Q, config.fs),
				isInitialised(false), feedsFDN(false), mFDNChannel(-1), btm(&mDiffractionPath, 48000), reflection(false), diffraction(false), mConfig(config), mAirAbsorption(mConfig.fs)
			{
				vWallMutex = std::make_shared<std::mutex>();
				vEdgeMutex = std::make_shared<std::mutex>();
				audioMutex = std::make_shared<std::mutex>();
			}
			VirtualSource(Binaural::CCore* core, const Config& config);
			VirtualSource(Binaural::CCore* core, const Config& config, const VirtualSourceData& data, const int fdnChannel);
			~VirtualSource();

			inline bool IsInit() const { return isInitialised; }
			inline bool Exists() const
			{
				if (mCore == NULL)
					return false;
				return true;
			}
			inline int GetFDNChannel() const { return mFDNChannel; }

			void UpdateSpatialisationMode(const HRTFMode mode);
			void UpdateSpatialisationMode(const SPATConfig config);

			// Updates
			bool UpdateVirtualSource(const VirtualSourceData& data, int& fdnChannel);

			// Audio
			void ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer);

			// Deactivate
			inline void Deactivate() { mSource = NULL; }

			VirtualSourceMap mVirtualSources;
			VirtualSourceMap mVirtualEdgeSources;

			shared_ptr<std::mutex> vWallMutex;
			shared_ptr<std::mutex> vEdgeMutex;

		private:
			void Init(const VirtualSourceData& data);
			void Remove();
			void Update(const VirtualSourceData& data, int& fdnChannel);

			void Reset() { mVirtualSources.clear(); mVirtualEdgeSources.clear(); } // mWallIDs.clear(); }

			// Constants
			Binaural::CCore* mCore;
			Config mConfig;
			bool feedsFDN;
			int mFDNChannel;
			int order;

			shared_ptr<Binaural::CSingleSourceDSP> mSource;

			Buffer bStore;
			CMonoBuffer<float> bInput;
			CEarPair<CMonoBuffer<float>> bOutput;
			CMonoBuffer<float> bMonoOutput;

			Real mCurrentGain;
			Real mTargetGain;
			GraphicEQ mFilter;
			Diffraction::Path mDiffractionPath;
			Diffraction::BTM btm;
			AirAbsorption mAirAbsorption;

			bool isInitialised;
			bool reflection;
			bool diffraction;

			// Mutexes
			shared_ptr<std::mutex> audioMutex;
		};
	}
}

#endif