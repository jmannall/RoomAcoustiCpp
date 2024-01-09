/*
*
*  \Reverb class
*
*/

#ifndef Spatialiser_Reverb_h
#define Spatialiser_Reverb_h

// C++ headers
#include <mutex>

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/FDN.h"

// Unity headers
#include "Unity/Debug.h"

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// ReverbSource class ////////////////////

		class ReverbSource
		{
		public:
			ReverbSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs);
			~ReverbSource();

			inline void SetShift(const vec3& shift) { mShift = shift; }
			void Update(const vec3& position);
			void UpdateReflectionFilter();
			void UpdateReflectionFilter(const Absorption& absorption);
			void ProcessAudio(const Real* data, const size_t& numFrames, Buffer& outputBuffer);

			inline void Deactivate() { mSource = NULL; }
		private:
			void Init();

			bool valid;
			vec3 mShift;
			Absorption mAbsorption;
			ParametricEQ mReflectionFilter;
			shared_ptr<Binaural::CSingleSourceDSP> mSource;
			HRTFMode mHRTFMode;

			Binaural::CCore* mCore;
		};

		//////////////////// Reverb class ////////////////////

		class Reverb
		{
		public:
			Reverb(Binaural::CCore* core, HRTFMode hrtfMode, const vec& dimensions, int fs);
			Reverb(Binaural::CCore* core, HRTFMode hrtfMode, const vec& dimensions, const FrequencyDependence& T60, int fs);

			void UpdateReverbSources(const vec3& position);
			void UpdateReflectionFilters(const ReverbWall& id, const Absorption& absorption);
			void UpdateReflectionFilters(const ReverbWall& id, const Absorption& absorption, const Absorption& oldAbsorption);
			void ProcessAudio(const matrix& data, Buffer& outputBuffer);
			void SetFDNParameters(const FrequencyDependence& T60, const vec& dimensions);
			// size_t NumChannels() const { return mNumChannels; }

			inline void UpdateValid(bool v) { valid = v; }
		private:
			void InitSources(int fs);

			bool valid;
			size_t mNumChannels;
			FDN mFDN;
			std::vector<ReverbSource> mReverbSources;
			HRTFMode mHRTFMode;

			Binaural::CCore* mCore;
			std::mutex mCoreMutex; // This would ideally be per reverbSource but can't copy a class with a mutex
		};
	}
}

#endif