/*
* @class Reverb, ReverbSource
*
* @brief Declaration of Reverb and ReverbSource classes
*
*/

#ifndef RoomAcoustiCpp_Reverb_h
#define RoomAcoustiCpp_Reverb_h

// C++ headers
#include <mutex>

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/FDN.h"

// Unity headers
#include "Unity/Debug.h"

// DSP headers
#include "DSP/GraphicEQ.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// ReverbSource class ////////////////////

		class ReverbSource
		{
		public:
			ReverbSource(Binaural::CCore* core, const Config& config);
			ReverbSource(Binaural::CCore* core, const Config& config, const vec3& shift);
			~ReverbSource();

			void UpdateSpatialisationMode(const HRTFMode mode);
			inline void SetShift(const vec3& shift) { mShift = shift; }
			inline vec3 GetShift() const { return mShift; }
			void UpdatePosition(const vec3& position);
			void UpdateReflectionFilter(const Absorption& absorption);

			void AddInput(const Real& in, const int& i) { inputBuffer[i] = in; }
			void ProcessAudio(Buffer& outputBuffer);

			inline void Deactivate() { mSource = NULL; }
			inline void Reset() { mReflectionFilter.ClearBuffers(); }

#ifdef _TEST
#pragma optimize("", off)
			inline Coefficients GetAbsorption() const { return mAbsorption; }
#pragma optimize("", on)
#endif

		private:
			void Init();

			bool filterInitialised;
			vec3 mShift;
			Coefficients mAbsorption;
			GraphicEQ mReflectionFilter;
			shared_ptr<Binaural::CSingleSourceDSP> mSource;
			Config mConfig;

			Real targetGain;
			Real currentGain;

			Binaural::CCore* mCore;
			Buffer inputBuffer;
			CMonoBuffer<float> bInput;
			CEarPair<CMonoBuffer<float>> bOutput;

			std::shared_ptr<std::mutex> mMutex;
		};

		//////////////////// Reverb class ////////////////////

		class Reverb
		{
		public:
			Reverb(Binaural::CCore* core, const Config& config);
			Reverb(Binaural::CCore* core, const Config& config, const vec& dimensions, const Coefficients& T60);

			void UpdateSpatialisationMode(const HRTFMode mode);

			void UpdateReverb(const vec3& position);
			void UpdateReflectionFilters(const std::vector<Absorption>& absorptions, bool running);
			void ProcessAudio(const matrix& data, Buffer& outputBuffer);
			void UpdateReverbTime(const Coefficients& T60);
			inline void UpdateFDNModel(const FDNMatrix& model) { mFDN.SetFDNModel(model); }
			void SetFDNParameters(const Coefficients& T60, const vec& dimensions);
			inline void ResetFDN() { mFDN.Reset(); }
			inline std::vector<vec3> GetReverbSourceDirections()
			{
				std::vector<vec3> directions;
				for (ReverbSource& reverbSource : mReverbSources)
					directions.push_back(reverbSource.GetShift());
				return directions;
			}

		private:
			void InitSources();

			matrix input;
			rowvec out;
			Real* col;

			bool valid;
			bool runFDN;
			Real mTargetGain;
			Real mCurrentGain;

			FDN mFDN;
			std::vector<ReverbSource> mReverbSources;
			std::mutex mFDNMutex;

			Binaural::CCore* mCore;
			Config mConfig;
			std::mutex mCoreMutex; // This would ideally be per reverbSource but can't copy a class with a mutex
		};
	}
}

#endif