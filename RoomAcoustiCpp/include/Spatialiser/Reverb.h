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

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"
#include "Common/ThreadPool.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/FDN.h"

// DSP headers
#include "DSP/GraphicEQ.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		/**
		* @brief Handles spatialisation processing for a single late reverberation channel
		*/
		class ReverbSource
		{
		public:
			/**
			* @brief Constructor that intialises a reverb source with a given position offset
			* 
			* @params core The 3DTI processing core
			* @params config The spatialiser configuration
			* @params shift The position offset relative to the listener
			*/
			ReverbSource(Binaural::CCore* core, const Config& config, const Vec3& shift);

			/**
			* @brief Default deconstructor
			*/
			~ReverbSource();

			/**
			* @brief Update the spatialisation mode for the HRTF processing
			*
			* @params mode New spatialisation mode
			*/
			void UpdateSpatialisationMode(const SpatialisationMode mode);

			/**
			* @brief Update the interpolation factor for interpolations
			*
			* @params lerpFactor New interpolation factor
			*/
			inline void UpdateLerpFactor(const Real lerpFactor)
			{
				lock_guard<mutex> lock(*mReflectionFilterMutex);
				mConfig.lerpFactor = lerpFactor;
			}

			/**
			* @brief Set position offset relative to the listener
			*/
			inline void SetShift(const Vec3& shift) { mShift = shift; }

			/**
			* @return The position shift relative to the listener
			*/
			inline Vec3 GetShift() const { return mShift; }

			/**
			* @brief Updates the absolute position of the reverb source so maintain a correct shift relative to the listener
			* 
			* @params listenerPosition The current listener position
			*/
			void UpdatePosition(const Vec3& listenerPosition);

			/**
			* @brief Update the reflection filter for directional dependent reverberation level
			* 
			* @params absorption New reflection filter target gain
			*/
			void UpdateReflectionFilter(const Coefficients& absorption);

			/**
			* @brief Add an input to the reverb source input buffer
			* 
			* @params in The input value
			* @params i The buffer sample index to write to
			*/
			void AddInput(const Real in, const int& i) { inputBuffer[i] = in; }

			/**
			* @brief Update the reverb source input buffer
			*
			* @params in The new input buffer
			*/
#ifdef USE_MOD_ART
			void AddInput(const Buffer& in) { inputBuffer = in; }
#endif

			/**
			* @brief Process the current reverb source audio buffer
			* 
			* @params outputBuffer The output buffer to write to
			*/
			void ProcessAudio(Buffer& outputBuffer);

#ifdef USE_MOD_ART
			void ProcessAudio_MOD_ART(Buffer& outputBuffer);
#endif

			/**
			* @brief Reset the internal buffers to zero
			*/
			inline void Reset() { lock_guard<mutex> lock(*mReflectionFilterMutex); mReflectionFilter.ClearBuffers(); mSource->ResetSourceBuffers(); }

#ifdef _TEST
#pragma optimize("", off)
			inline Coefficients GetAbsorption() const { return mAbsorption; }
#pragma optimize("", on)
#endif

		private:
			/**
			* @brief Initialises the reverb source
			*/
			void Init();

			Config mConfig;		// Spatialiser configuration
			Vec3 mShift;		// Position shift relative to the listener

			Buffer inputBuffer;		// Input buffer
			Real targetGain;		// Target gain
			Real currentGain;		// Current gain

			bool filterInitialised;									// True if the reflection filter has been initialised
			Coefficients mAbsorption;								// Reflection filter target gain
			GraphicEQ mReflectionFilter;							// Frequency dependent reflection filter
			std::shared_ptr<std::mutex> mReflectionFilterMutex;		// Protects mReflectionFilter

			Binaural::CCore* mCore;								// 3DTI core
			shared_ptr<Binaural::CSingleSourceDSP> mSource;		// 3DTI source
			CMonoBuffer<float> bInput;							// 3DTI Input buffer	
			CEarPair<CMonoBuffer<float>> bOutput;				// 3DTI Output buffer
		};

		/**
		* @brief Handles late reverberation processing
		*/
		class Reverb
		{
		public:
			/**
			* @brief Constructor that intialises a default late reverberation with a 1s T60
			*
			* @params core The 3DTI processing core
			* @params config The spatialiser configuration
			*/
			Reverb(Binaural::CCore* core, const Config& config) : Reverb(core, config, Vec({ 2.5, 4.0, 6.0 }), Coefficients(config.frequencyBands.Length())) {}
			
			/**
			* @brief Constructor that intialises late reveberation with a target T60 and given primary room dimensions
			*
			* @params core The 3DTI processing core
			* @params config The spatialiser configuration
			* @params dimensions Primary room dimensions that determine delay line lengths
			* @params T60 Target decay time
			*/
			Reverb(Binaural::CCore* core, const Config& config, const Vec& dimensions, const Coefficients& T60);

			/**
			* @brief Default deconstructor
			*/
			~Reverb() {}

			/**
			* @brief Update the spatialisation mode for the HRTF processing
			* 
			* @params mode New spatialisation mode
			*/
			void UpdateSpatialisationMode(const SpatialisationMode mode);

			/**
			* @brief Update the interpolation factor for interpolations
			*
			* @params lerpFactor New interpolation factor
			*/
			void UpdateLerpFactor(const Real lerpFactor);

			/**
			* @brief Updates the reverb source positions relative to the listener
			* 
			* @params listenerPosition The listener position
			*/
			void UpdateReverbSourcePositions(const Vec3& listenerPosition);

			/**
			* @brief Update reflection filters for directional dependent reverberation level
			* 
			* @params absorptions New reflection filter target gains
			* @params running True if including late reveberation in audio prcoessing, false otherwise
			*/
			bool UpdateReflectionFilters(const std::vector<Absorption>& absorptions, bool running);

			/**
			* @brief Processes a single audio buffer
			* 
			* @params data Multichannel audio data input
			* @params ouputBuffer Stereo output buffer to write to
			*/
			void ProcessAudio(const Matrix& data, Buffer& outputBuffer);

#ifdef USE_MOD_ART
			void ProcessAudio_MOD_ART(const Matrix& data, Buffer& outputBuffer);
#endif

			/**
			* @brief Updates the target T60
			*
			* @params T60 The new decay time
			*/
			void UpdateReverbTime(const Coefficients& T60);

			/**
			* @brief Initialises the FDN matrix
			*
			* @params matrixType The new FDN matrix type
			*/
			inline void InitFDNMatrix(const FDNMatrix& matrixType) { mFDN.InitFDNMatrix(matrixType); }

			/**
			* @brief Updates the FDN delay line lengths
			*
			* @params dimensions The new primary room dimensions that determine the delay line lengths
			* @params T60 The target T60 in order to update the absorption filters for the given delay lengths
			*/
			void UpdateFDNDelayLines(const Vec& dimensions, const Coefficients& T60);

			/**
			* @brief Resets the FDN and ReverbSources internal buffers to zero
			*/
			inline void ResetFDN()
			{ 
				mFDN.Reset(); 
				for (ReverbSource& reverbSource : mReverbSources)
					reverbSource.Reset();
			}

			/**
			* @brief Calculate the end limits for reverb source directions
			* 
			* @params directions The vector to add reverb source directions to
			*/
			inline void GetReverbSourceDirections(std::vector<Vec3>& directions) const
			{
				directions.reserve(mReverbSources.size());
				for (const ReverbSource& reverbSource : mReverbSources)
					directions.emplace_back(100.0 * reverbSource.GetShift());
			}

			inline void SetReverbGain(const Real gain) { reverbGain = gain; }

		private:
			/**
			* @brief Initialse the ReverbSources
			*/
			void InitSources();

			Config mConfig;									// Spatialiser configuration
			Coefficients mT60;								// Target decay time
			FDN mFDN;										// FDN used to processing late reverbation
			std::mutex mFDNMutex;							// Protects mFDN
			std::vector<ReverbSource> mReverbSources;		// Reverb sources to binauralise the FDN output

			std::atomic<bool> valid;			// True if T60 > 0.0 and T60 < 20.0 seconds
			std::atomic<bool> runFDN;			// True if audio thread should process late revebreration
			std::atomic<Real> reverbGain;		// Reverberation Gain
			std::atomic<Real> mTargetGain;		// Target reverberation level
			std::atomic<Real> mCurrentGain;		// Current reverberation level

			Binaural::CCore* mCore;		// 3DTI core

			std::vector<Buffer> threadResults;
		};
	}
}

#endif