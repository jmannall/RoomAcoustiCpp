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
#include "AudioManager.h"

namespace Spatialiser
{
	class VirtualSourceData
	{
	public:
		VirtualSourceData() : valid(false), visible(false), feedsFDN(false), mFDNChannel(-1), reflectionOrder(0),diffractionOrder(0) {};
		~VirtualSourceData() { Clear(); };

		inline void AddWallID(const size_t& id, const Absorption& absorption)
		{ 
			wallIDs.push_back(id);
			reflectionOrder++;
			mAbsorption *= absorption;
		}
		inline void AddEdgeID(const size_t& id)
		{
			edgeIDs.push_back(id);
			diffractionOrder++;
		}
		size_t GetWallID(int i) const { return wallIDs[i]; }
		void GetAbsorption(float* g) const { mAbsorption.GetValues(g); }
		int GetReflectionOrder() const { return reflectionOrder; }
		vec3 GetPosition() const { return mPositions.back(); }
		vec3 GetPosition(int i) const;
		void SetTransform(const vec3& pos);

		inline void Visible(const bool& _feedsFDN) { visible = true; feedsFDN = _feedsFDN; }
		inline void Invisible() { visible = false; }
		inline void Valid() { valid = true; }
		inline void Invalid() { valid = false; }
		inline void Reset() { Invalid();  Invisible(); }
		inline void Clear() { wallIDs.clear(); edgeIDs.clear(); mPositions.clear(); }

		bool valid;
		bool visible;
		bool feedsFDN;
		int mFDNChannel;
		Common::CTransform transform;

	private:
		std::vector<size_t> wallIDs;
		std::vector<size_t> edgeIDs;
		std::vector<vec3> mPositions;
		size_t reflectionOrder;
		size_t diffractionOrder;
		Absorption mAbsorption;
	};

	class VirtualSource
	{
		using VirtualSourceMap = std::unordered_map<size_t, VirtualSource>;
	public:
		VirtualSource() : mCore(NULL), mSource(NULL), mFilter(4), isInitialised(false), mHRTFMode(HRTFMode::performance), feedsFDN(false), mFDNChannel(-1) {};
		VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode) : mCore(core), mSource(NULL), mFilter(4), isInitialised(false), mHRTFMode(hrtfMode), feedsFDN(false), mFDNChannel(-1) {};
		VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs, const VirtualSourceData& data, const int& fdnChannel);
		~VirtualSource();

		bool IsInit() const { return isInitialised; }

		void UpdateVirtualSource(const VirtualSourceData& data);
		void RemoveVirtualSources(const size_t& id);
		void ProcessAudio(const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer);

		inline void Deactivate() { mSource = NULL; }

		VirtualSourceMap mChildren;
	private:
		void Init();
		void Remove();
		void Update(const VirtualSourceData& data);

		void Reset() { mChildren.clear(); } // mWallIDs.clear(); }

		shared_ptr<Binaural::CSingleSourceDSP> mSource;
		vec3 mPosition;
		// std::vector<size_t> wallIDs;
		// int mReflectionOrder;
		bool isInitialised;
		HRTFMode mHRTFMode;
		bool feedsFDN;
		int mFDNChannel;
		ParametricEQ mFilter;

		Binaural::CCore* mCore;
	};
}