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
		VirtualSourceData() : valid(false), visible(false), feedsFDN(false), mFDNChannel(-1), order(0), reflection(false), diffraction(false) {};
		~VirtualSourceData() { Clear(); };

		inline void AddWallIDs(const std::vector<size_t>& ids, const Absorption& absorption)
		{
			for (int i = 0; i < ids.size(); i++)
			{
				IDs.push_back(ids[i]);
				isReflection.push_back(true);
				order++;
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
		}
		// Currently can only handle one edge per path/virtual source
		inline void AddEdgeID(const size_t& id, const Diffraction::Path path)
		{
			IDs.push_back(id);
			isReflection.push_back(false);
			order++;
			mDiffractionPath = path;
			diffraction = true;
		}
		inline void AddEdgeIDToStart(const size_t& id, const Diffraction::Path path)
		{
			IDs.insert(IDs.begin(), id);
			isReflection.insert(isReflection.begin(), false);
			order++;
			mDiffractionPath = path;
			diffraction = true;
		}
		size_t GetID() const { return IDs.back(); }
		size_t GetID(int i) const { return IDs[i]; }
		std::vector<size_t> GetWallIDs() const;
		inline bool IsReflection(int i) const { return isReflection[i]; }
		inline void GetAbsorption(float* g) const { mAbsorption.GetValues(g); }
		inline void GetAbsorption(Absorption& a) const { a = mAbsorption; }
		inline int GetOrder() const { return order; }
		inline vec3 GetPosition() const { return mPositions.back(); }
		vec3 GetPosition(int i) const;
		vec3 GetTransformPosition();
		void SetTransform(const vec3& vSourcePosition);
		void SetTransform(const vec3& vSourcePosition, const vec3& vEdgeSourcePosition);

		vec3 GetRPosition() const { return mRPositions.back(); }
		vec3 GetRPosition(int i) const;
		void SetRPosition(const vec3& vReceiverPosition);
		void SetRTransform(const vec3& vReceiverPosition, const vec3& vEdgeSourcePosition);

		inline void Visible(const bool& _feedsFDN) { visible = true; feedsFDN = _feedsFDN; }
		inline void Invisible() { visible = false; }
		inline void Valid() { valid = true; }
		inline void Invalid() { valid = false; }
		inline void RValid() { rValid = true; }
		inline void RInvalid() { rValid = false; }
		inline void Reset() { Invalid();  Invisible(); RInvalid();  }
		inline void Clear() { IDs.clear(); mPositions.clear(); }

		bool AppendVSource(VirtualSourceData& data, const vec3& listenerPosition);

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
		VirtualSource() : mCore(NULL), mSource(NULL), mFilter(4), mHPF(20.0f, FilterShape::hpf, 48000), isInitialised(false), mHRTFMode(HRTFMode::performance), feedsFDN(false), mFDNChannel(-1), btm(&mDiffractionPath, 48000) {};
		VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode) : mCore(core), mSource(NULL), mFilter(4), mHPF(20.0f, FilterShape::hpf, 48000), isInitialised(false), mHRTFMode(hrtfMode), feedsFDN(false), mFDNChannel(-1), btm(&mDiffractionPath, 48000) {};
		VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs, const VirtualSourceData& data, const int& fdnChannel);
		VirtualSource(const VirtualSource& vS);

		~VirtualSource();

		inline bool IsInit() const { return isInitialised; }
		inline bool Exists() const {
			if (mCore == NULL) { return false; }
			else { return true; }
		}

		void UpdateVirtualSource(const VirtualSourceData& data);
		void RemoveVirtualSources(const size_t& id);
		void ProcessAudio(const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer);

		inline void Deactivate() { mSource = NULL; }

		VirtualSourceMap mVirtualSources;
		VirtualSourceMap mVirtualEdgeSources;
	private:
		void Init();
		void Remove();
		void Update(const VirtualSourceData& data);

		void Reset() { mVirtualSources.clear(); mVirtualEdgeSources.clear(); } // mWallIDs.clear(); }

		shared_ptr<Binaural::CSingleSourceDSP> mSource;
		vec3 mPosition;
		// std::vector<size_t> wallIDs;
		// int mReflectionOrder;
		bool isInitialised;
		HRTFMode mHRTFMode;
		bool feedsFDN;
		int mFDNChannel;
		ParametricEQ mFilter;
		TransDF2 mHPF;
		Diffraction::Path mDiffractionPath;
		Diffraction::BTM btm;

		bool reflection;
		bool diffraction;

		Binaural::CCore* mCore;
	};
}