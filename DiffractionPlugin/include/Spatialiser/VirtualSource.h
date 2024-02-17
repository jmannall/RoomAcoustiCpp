/*
*
*  \VirtualSource class
*  \Currently can only handle one edge per path/virtual source
*
*/

#ifndef Spatialiser_VirtualSource_h
#define Spatialiser_VirtualSource_h

// C++ headers
#include <vector>
#include <unordered_map>

// Common headers
#include "Common/Vec3.h"
#include "Common/Matrix.h"
#include "Common/AudioManager.h"

// Spatialiser headers
#include "Spatialiser/Wall.h"
#include "Spatialiser/Types.h"
#include "Diffraction/Path.h"
#include "Diffraction/Models.h"

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"

// Unity headers
#include "Unity/Debug.h"

using namespace Common;
namespace UIE
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
			VirtualSourceData() : valid(false), rValid(false), visible(false), feedsFDN(false), mFDNChannel(-1), order(0), reflection(false), diffraction(false), key("") {};
			~VirtualSourceData() { Clear(); };

			// Wall
			inline void AddWallIDs(const std::vector<size_t>& ids, const Absorption& absorption)
			{
				for (size_t id : ids)
				{
					pathParts.push_back(Part(id, true));
					order++;
					key = key + IntToStr(id) + "r";
				}
				mAbsorption *= absorption;
				reflection = true;
			}
			inline void AddWallID(const size_t& id, const Absorption& absorption) // IDs are added in reverse order (from listener to source)
			{
				pathParts.push_back(Part(id, true));
				mAbsorption *= absorption;
				key = key + IntToStr(id) + "r";
			}
			inline void AddWallIDToStart(const size_t& id, const Absorption& absorption) // IDs are added in reverse order (from listener to source)
			{
				pathParts.insert(pathParts.begin(), Part(id, true));
				mAbsorption *= absorption;
				key = IntToStr(id) + "r" + key;
			}
			inline void AddPlaneID(const size_t& id)
			{
				planeIds.push_back(id);
				order++;
				reflection = true;
			}
			inline void RemoveWallIDs()
			{
				key = "";
				mAbsorption = Absorption();
				if (diffraction)
				{
					std::vector<Part>::iterator iter;
					for (iter = pathParts.begin(); iter != pathParts.end(); )
					{
						if (iter->isReflection)
							iter = pathParts.erase(iter);
						else
						{
							key = key + IntToStr(iter->id) + "d";
							++iter;
						}
					}
				}
				else
					pathParts.clear();
			}

			// Edge
			inline void AddEdgeID(const size_t& id, const Diffraction::Path path)
			{
				pathParts.push_back(Part(id, false));
				order++;
				mDiffractionPath = path;
				diffraction = true;
				key = key + IntToStr(id) + "d";
			}
			inline void AddEdgeIDToStart(const size_t& id, const Diffraction::Path path)
			{
				pathParts.insert(pathParts.begin(), Part(id, false));
				order++;
				mDiffractionPath = path;
				diffraction = true;
				key = IntToStr(id) + "d" + key;
			}

			// Getters
			size_t GetID() const { return pathParts.back().id; }
			size_t GetID(int i) const { return pathParts[i].id; }
			size_t GetPlaneID() const { return planeIds.back(); }
			size_t GetPlaneID(int i) const { return planeIds[i]; }
			std::string GetKey() const { return key; }
			std::vector<size_t> GetWallIDs() const;
			inline bool IsReflection(int i) const { return pathParts[i].isReflection; }
			inline void GetAbsorption(Real* g) const { mAbsorption.GetValues(g); }
			inline void GetAbsorption(Absorption& a) const { a = mAbsorption; }
			inline size_t GetOrder() const { return order; }

			// Transforms
			void SetTransform(const vec3& vSourcePosition);
			void SetTransform(const vec3& vSourcePosition, const vec3& vEdgeSourcePosition);
			void UpdateTransform(const vec3& vEdgeSourcePosition);

			inline vec3 GetPosition() const { return mPositions.back(); }
			vec3 GetPosition(int i) const;
			vec3 GetTransformPosition();
			
			// Receiver position
			void SetRPosition(const vec3& vReceiverPosition);
			inline void SetRPositions(const std::vector<vec3>& vReceiverPositions) { mRPositions = vReceiverPositions; };
			void SetRTransform(const vec3& vReceiverPosition, const vec3& vEdgeSourcePosition);
			
			vec3 GetRPosition() const { return mRPositions.back(); }
			vec3 GetRPosition(int i) const;
			inline std::vector<vec3> GetRPositions() const { return mRPositions; };

			// Diffraction
			inline void UpdateDiffractionPath(const Diffraction::Path path) { mDiffractionPath = path; }
			inline Edge GetEdge() const { return mDiffractionPath.GetEdge(); }
			inline vec3 GetApex() const { return mDiffractionPath.GetApex(); }
			inline bool GetSValid() const { return mDiffractionPath.sValid; }
			inline bool GetRValid() const { return mDiffractionPath.rValid; }

			// Visibility and Validity
			inline void Visible(const bool& _feedsFDN) { visible = true; feedsFDN = _feedsFDN; }
			inline void Invisible() { visible = false; }
			inline void Valid() { valid = true; }
			inline void Invalid() { valid = false; }
			inline void RValid() { rValid = true; }
			inline void RInvalid() { rValid = false; }

			// Reset
			inline void Reset() { Invalid();  Invisible(); RInvalid(); }
			inline void Clear() { pathParts.clear(); planeIds.clear(), mPositions.clear(); mRPositions.clear(); }

			VirtualSourceData Trim(const int i);

			// Status data
			bool reflection;
			bool diffraction;
			bool valid;
			bool visible;
			bool rValid;
			bool feedsFDN;

			// DSP 
			int mFDNChannel;
			CTransform transform;
			Diffraction::Path mDiffractionPath;

		private:
			std::string key;
			std::vector<Part> pathParts;
			std::vector<size_t> planeIds;
			std::vector<vec3> mPositions;
			std::vector<vec3> mRPositions;
			size_t order;
			Absorption mAbsorption;
		};

		//////////////////// VirtualSource class ////////////////////

		class VirtualSource // Divide into sub classes types: reflection, diffraction, both
		{
		public:

			// Load and Destroy
			VirtualSource() : mCore(NULL), mSource(NULL), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(4), isInitialised(false), feedsFDN(false), mFDNChannel(-1), btm(&mDiffractionPath, 48000), reflection(false), diffraction(false) {};
			VirtualSource(Binaural::CCore* core, const Config& config);
			VirtualSource(Binaural::CCore* core, const Config& config, const VirtualSourceData& data, const int fdnChannel);
			VirtualSource(const VirtualSource& vS);
			~VirtualSource();

			// Operators
			inline VirtualSource operator=(const VirtualSource& vS)
			{
				mCore = vS.mCore; mSource = vS.mSource; mFilter = vS.mFilter; isInitialised = vS.isInitialised; mConfig = vS.mConfig; feedsFDN = vS.feedsFDN; mFDNChannel = vS.mFDNChannel; mDiffractionPath = vS.mDiffractionPath; btm = vS.btm; mTargetGain = vS.mTargetGain;
				mCurrentGain = vS.mCurrentGain; reflection = vS.reflection; diffraction = vS.diffraction; mVirtualSources = vS.mVirtualSources; mVirtualEdgeSources = vS.mVirtualEdgeSources; bInput = vS.bInput; bStore = vS.bStore; bOutput = vS.bOutput, bMonoOutput = vS.bMonoOutput;
				return *this;
			}

			inline bool IsInit() const { return isInitialised; }
			inline bool Exists() const
			{
				if (mCore == NULL)
					return false;
				return true;
			}
			inline int GetFDNChannel() const { return mFDNChannel; }

			// Updates
			bool UpdateVirtualSource(const VirtualSourceData& data, int& fdnChannel);

			// Audio
			void ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer);

			// Deactivate
			inline void Deactivate() { mSource = NULL; }

			VirtualSourceMap mVirtualSources;
			VirtualSourceMap mVirtualEdgeSources;

			std::mutex vWallMutex;
			std::mutex vEdgeMutex;

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

			shared_ptr<Binaural::CSingleSourceDSP> mSource;

			Buffer bStore;
			CMonoBuffer<float> bInput;
			CEarPair<CMonoBuffer<float>> bOutput;
			CMonoBuffer<float> bMonoOutput;

			Real mCurrentGain;
			Real mTargetGain;
			ParametricEQ mFilter;
			Diffraction::Path mDiffractionPath;
			Diffraction::BTM btm;

			bool isInitialised;
			bool reflection;
			bool diffraction;

			// Mutexes
			std::mutex audioMutex;
		};
	}
}

#endif