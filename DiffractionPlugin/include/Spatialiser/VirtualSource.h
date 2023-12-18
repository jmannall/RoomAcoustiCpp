#pragma once

#include "vec3.h"
#include "matrix.h"
#include <vector>
#include <unordered_map>
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"
#include "Debug.h"
#include "Spatialiser/Wall.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/Diffraction/Path.h"
#include "Spatialiser/Diffraction/Models.h"
#include "AudioManager.h"

namespace Spatialiser
{
	class VirtualSourceData
	{
	public:
		VirtualSourceData() : key(""), valid(false), rValid(false), visible(false), feedsFDN(false), mFDNChannel(-1), order(0), reflection(false), diffraction(false) {};
		~VirtualSourceData() { Clear(); };

		inline void AddWallIDs(const std::vector<size_t>& ids, const Absorption& absorption)
		{
			for (int i = 0; i < ids.size(); i++)
			{
				IDs.push_back(ids[i]);
				isReflection.push_back(true);
				order++;
				key = key + IntToStr(ids[i]) + "r";
			}
			mAbsorption *= absorption;
			reflection = true;
		}
		inline void AddWallID(const size_t& id, const Absorption& absorption)
		{
			IDs.push_back(id);
			isReflection.push_back(true);
			order++;
			mAbsorption *= absorption;
			reflection = true;
			key = key + IntToStr(id) + "r";
		}
		// Currently can only handle one edge per path/virtual source
		inline void AddEdgeID(const size_t& id, const Diffraction::Path path)
		{
			IDs.push_back(id);
			isReflection.push_back(false);
			order++;
			mDiffractionPath = path;
			diffraction = true;
			key = key + IntToStr(id) + "d";
		}
		inline void AddEdgeIDToStart(const size_t& id, const Diffraction::Path path)
		{
			IDs.insert(IDs.begin(), id);
			isReflection.insert(isReflection.begin(), false);
			order++;
			mDiffractionPath = path;
			diffraction = true;
			key = IntToStr(id) + "d" + key;
		}
		size_t GetID() const { return IDs.back(); }
		size_t GetID(int i) const { return IDs[i]; }
		std::string GetKey() const { return key; }
		std::vector<size_t> GetWallIDs() const;
		inline bool IsReflection(int i) const { return isReflection[i]; }
		inline void GetAbsorption(float* g) const { mAbsorption.GetValues(g); }
		inline void GetAbsorption(Absorption& a) const { a = mAbsorption; }
		inline size_t GetOrder() const { return order; }
		inline vec3 GetPosition() const { return mPositions.back(); }
		vec3 GetPosition(int i) const;
		vec3 GetTransformPosition();
		void SetTransform(const vec3& vSourcePosition);
		void SetTransform(const vec3& vSourcePosition, const vec3& vEdgeSourcePosition);
		void UpdateTransform(const vec3& vEdgeSourcePosition);

		vec3 GetRPosition() const { return mRPositions.back(); }
		vec3 GetRPosition(int i) const;
		inline std::vector<vec3> GetRPositions() const { return mRPositions; };
		void SetRPosition(const vec3& vReceiverPosition);
		inline void SetRPositions(const std::vector<vec3>& vReceiverPositions) { mRPositions = vReceiverPositions; };
		void SetRTransform(const vec3& vReceiverPosition, const vec3& vEdgeSourcePosition);

		inline void UpdateDiffractionPath(const Diffraction::Path path) { mDiffractionPath = path; }
		inline Edge GetEdge() const { return mDiffractionPath.GetEdge(); }
		inline vec3 GetApex() const { return mDiffractionPath.GetApex(); }
		inline bool GetSValid() const { return mDiffractionPath.sValid; }
		inline bool GetRValid() const { return mDiffractionPath.rValid; }

		inline void Visible(const bool& _feedsFDN) { visible = true; feedsFDN = _feedsFDN; }
		inline void Invisible() { visible = false; }
		inline void Valid() { valid = true; }
		inline void Invalid() { valid = false; }
		inline void RValid() { rValid = true; }
		inline void RInvalid() { rValid = false; }
		inline void Reset() { Invalid();  Invisible(); RInvalid(); }
		inline void Clear() { IDs.clear(); mPositions.clear(); }

		bool AppendVSource(VirtualSourceData& data, const vec3& listenerPosition);

		VirtualSourceData Trim(const int i);

		bool reflection;
		bool diffraction;
		bool valid;
		bool visible;
		bool rValid;
		bool feedsFDN;
		int mFDNChannel;
		Common::CTransform transform;
		Diffraction::Path mDiffractionPath;

	private:
		std::string key;
		std::vector<size_t> IDs;
		std::vector<bool> isReflection;
		std::vector<vec3> mPositions;
		std::vector<vec3> mRPositions;
		size_t order;
		Absorption mAbsorption;
	};

	class VirtualSource // Divide into sub classes types: reflection, diffraction, both
	{
		using VirtualSourceMap = std::unordered_map<size_t, VirtualSource>;
	public:
		VirtualSource() : mCore(NULL), mSource(NULL), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(4), isInitialised(false), mHRTFMode(HRTFMode::performance), feedsFDN(false), mFDNChannel(-1), btm(&mDiffractionPath, 48000), udfa(&mDiffractionPath, 48000), reflection(false), diffraction(false) {};
		VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs) : mCore(core), mSource(NULL), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(4, fs), isInitialised(false), mHRTFMode(hrtfMode), feedsFDN(false), mFDNChannel(-1), btm(&mDiffractionPath, fs), udfa(&mDiffractionPath, fs), reflection(false), diffraction(false) {};
		VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs, const VirtualSourceData& data, const int& fdnChannel);
		VirtualSource(const VirtualSource& vS);

		~VirtualSource();

		inline VirtualSource operator=(const VirtualSource& vS) {
			mCore = vS.mCore; mSource = vS.mSource; mPosition = vS.mPosition; mFilter = vS.mFilter; isInitialised = vS.isInitialised; mHRTFMode = vS.mHRTFMode; feedsFDN = vS.feedsFDN; mFDNChannel = vS.mFDNChannel; mDiffractionPath = vS.mDiffractionPath; btm = vS.btm; udfa = vS.udfa, mTargetGain = vS.mTargetGain; mCurrentGain = vS.mCurrentGain; reflection = vS.reflection; diffraction = vS.diffraction; mVirtualSources = vS.mVirtualSources; mVirtualEdgeSources = vS.mVirtualEdgeSources;
			return *this;
		}

		inline bool IsInit() const { return isInitialised; }
		inline bool Exists() const {
			if (mCore == NULL) { return false; }
			else { return true; }
		}
		inline int GetFDNChannel() const { return mFDNChannel; }

		bool UpdateVirtualSource(const VirtualSourceData& data);
		void RemoveVirtualSources(const size_t& id);
		int ProcessAudio(const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer, const float lerpFactor);

		inline void Deactivate() { mSource = NULL; }

		VirtualSourceMap mVirtualSources;
		VirtualSourceMap mVirtualEdgeSources;

		std::mutex vWallMutex;
		std::mutex vEdgeMutex;

	private:
		void Init(const VirtualSourceData& data);
		void Remove();
		void Update(const VirtualSourceData& data);

		void Reset() { mVirtualSources.clear(); mVirtualEdgeSources.clear(); } // mWallIDs.clear(); }

		// Constants
		Binaural::CCore* mCore;
		HRTFMode mHRTFMode;
		bool feedsFDN;
		int mFDNChannel;

		shared_ptr<Binaural::CSingleSourceDSP> mSource;
		vec3 mPosition;

		float mCurrentGain;
		float mTargetGain;
		ParametricEQ mFilter;
		Diffraction::Path mDiffractionPath;
		Diffraction::UDFA udfa;
		Diffraction::BTM btm;

		bool isInitialised;
		bool reflection;
		bool diffraction;

		std::mutex audioMutex;
	};
}