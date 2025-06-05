/*
* @class Reverb, ReverbSource
*
* @brief Declaration of Reverb and ReverbSource classes
*
*/

#if defined(_ANDROID)
// Common headers
#include "Common/Definitions.h"
#endif

#include <latch>

// Spatialiser headers
#include "Spatialiser/Reverb.h"
#include "Spatialiser/Globals.h"

// Unity headers
#include "Unity/Debug.h"
#include "Unity/UnityInterface.h"

// DSP headers
#include "DSP/Interpolate.h"

// Common headers
#include "Common/SphericalGeometries.h"

using namespace Common;
namespace RAC
{
	using namespace Unity;
	using namespace DSP;
	namespace Spatialiser
	{

		//////////////////// ReverbSource class ////////////////////

		ReleasePool ReverbSource::releasePool;

		////////////////////////////////////////

		ReverbSource::ReverbSource(Binaural::CCore* core, const Config& config, const Vec3& shift) : mCore(core), mShift(shift)
		{
			bInput = CMonoBuffer<float>(config.numFrames);
			bOutput.left = CMonoBuffer<float>(config.numFrames);
			bOutput.right = CMonoBuffer<float>(config.numFrames);
			InitSource();
			UpdateSpatialisationMode(config.spatialisationMode);
		}

		////////////////////////////////////////

		ReverbSource::~ReverbSource()
		{
#ifdef DEBUG_REMOVE
	Debug::Log("Remove reverb source", Colour::Red);
#endif
			{
				unique_lock<shared_mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
			}
		}

		////////////////////////////////////////

		void ReverbSource::InitSource()
		{
#ifdef DEBUG_INIT
			Debug::Log("Init reverb source", Colour::Green);
#endif
			{
				unique_lock<shared_mutex> lock(tuneInMutex);

				// Initialise source to core
				mSource = mCore->CreateSingleSourceDSP();
				mSource->DisablePropagationDelay();
				mSource->DisableDistanceAttenuationSmoothingAnechoic();
				mSource->DisableDistanceAttenuationAnechoic();
				mSource->DisableNearFieldEffect();
				mSource->DisableFarDistanceEffect();
			}
			
		}

		////////////////////////////////////////

		void ReverbSource::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			switch (mode)
			{
			case SpatialisationMode::quality:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
				break;
			}
			case SpatialisationMode::performance:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			case SpatialisationMode::none:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::NoSpatialization);
				break;
			}
			default:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::NoSpatialization);
				break;
			}
			}
		}

		////////////////////////////////////////

		void ReverbSource::UpdatePosition(const Vec3& listenerPosition)
		{
			CTransform newTransform;
			const Vec3 position = listenerPosition + mShift;
			newTransform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));
			const shared_ptr<CTransform> newTransformCopy = make_shared<CTransform>(newTransform);

			releasePool.Add(newTransformCopy);
			transform.store(newTransformCopy);
		}

		////////////////////////////////////////

		void ReverbSource::ProcessAudio(const Buffer& data, Buffer& outputBuffer)
		{
#ifdef PROFILE_AUDIO_THREAD
			BeginReverbSource();
#endif

			if (clearBuffers.load())
			{
				mSource->ResetSourceBuffers(); // Think this reallocates memory
				clearBuffers.store(false);
			}

			std::transform(data.begin(), data.end(), bInput.begin(),
				[&](auto value) { return static_cast<float>(value); });

#ifdef PROFILE_AUDIO_THREAD
			Begin3DTI();
#endif
			{
				shared_lock<shared_mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(*transform.load());
				mSource->SetBuffer(bInput);
				mSource->ProcessAnechoic(bOutput.left, bOutput.right);
			}

#ifdef PROFILE_AUDIO_THREAD
			End3DTI();
#endif

			int j = 0;
			for (int i = 0; i < data.Length(); i++)
			{
				outputBuffer[j++] = static_cast<Real>(bOutput.left[i]);
				outputBuffer[j++] = static_cast<Real>(bOutput.right[i]);
			}
#ifdef PROFILE_AUDIO_THREAD
			EndReverbSource();
#endif
		}

		//////////////////// Reverb class ////////////////////

		ReleasePool Reverb::releasePool;

		////////////////////////////////////////

		void Reverb::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			mConfig.spatialisationMode = mode;
			unique_lock<shared_mutex> lock(tuneInMutex);
			for (auto& source : mReverbSources)
				source->UpdateSpatialisationMode(mode);
		}

		////////////////////////////////////////

		void Reverb::UpdateLerpFactor(const Real lerpFactor)
		{
			mConfig.lerpFactor = lerpFactor;
		}

		////////////////////////////////////////

		void Reverb::InitSources(Binaural::CCore* core)
		{
			// Distribute reverb sources around the listener
			std::vector<Vec3> points;
			points.reserve(mConfig.numLateReverbChannels);
			switch (mConfig.numLateReverbChannels)
			{
			case 1:
			{ points.emplace_back(0.0, 0.0, 1.0); break; }
			case 2:
			{ points.emplace_back(-1.0, 0.0, 0.0); points.emplace_back(1.0, 0.0, 0.0); break; }
			case 4:
			{ Tetrahedron(points, true); break; }
			case 6:
			{ Octahedron(points); break; }	
			case 8:
			{ Cube(points); break; }
			case 12:
			{ Icosahedron(points, true); break; }
			case 16:
			{ Cube(points); Octahedron(points); break; }
			case 20:
			{ Dodecahedron(points, true); break; }
			case 24:
			{ Icosahedron(points, true); Icosahedron(points, false); break; }
			case 32:
			{ Icosahedron(points, true); Dodecahedron(points, true); break; }
			}

			mReverbSources.reserve(mConfig.numLateReverbChannels);
			threadResults.reserve(mConfig.numLateReverbChannels);
			for (int i = 0; i < mConfig.numLateReverbChannels; i++)
			{
				mReverbSources.emplace_back(std::make_unique<ReverbSource>(core, mConfig, points[i]));
				threadResults.emplace_back(2 * mConfig.numFrames);
			}
		}

		////////////////////////////////////////

		void Reverb::UpdateReverbSourcePositions(const Vec3& listenerPosition)
		{
			for (auto& reverbSource : mReverbSources)
				reverbSource->UpdatePosition(listenerPosition);
		}

		////////////////////////////////////////

		void Reverb::ProcessAudio(const Matrix& data, Buffer& outputBuffer)
		{
			if (!running.load())
				return;

#ifdef PROFILE_AUDIO_THREAD
			BeginReverb();
#endif
			mFDN.load()->ProcessAudio(data, reverbOutputs);
			
			std::latch latch(mReverbSources.size());
			size_t index = 0;
			for (auto& source : mReverbSources)
			{
				size_t threadIndex = index++; // Each thread gets a unique index
				audioThreadPool->enqueue([&, threadIndex] {
					source->ProcessAudio(reverbOutputs[threadIndex], threadResults[threadIndex]);
					// No need for a mutex, each thread writes to a unique index

					latch.count_down();
					});
			}

			// Wait for all threads to finish
			latch.wait();

			// Now safely merge the results **sequentially**
			for (const Buffer& localOutput : threadResults)
				outputBuffer += localOutput;

#ifdef PROFILE_AUDIO_THREAD
				EndReverb();
#endif
		}

		////////////////////////////////////////

		void Reverb::SetTargetT60(const Coefficients& T60)
		{
			if (!initialised.load())
				return;

#ifdef DEBUG_INIT
			Debug::Log("Init FDN: [" + RealToStr(T60[0]) + ", " + RealToStr(T60[1]) + ", " +
				RealToStr(T60[2]) + ", " + RealToStr(T60[3]) + ", " + RealToStr(T60[4]) + "]", Colour::Green);
#endif
			mFDN.load()->SetTargetT60(T60);
		}

		////////////////////////////////////////

		void Reverb::InitLateReverb(const Coefficients& T60, const Vec& dimensions, const FDNMatrix matrix)
		{
            std::shared_ptr<FDN> fdn;
            switch (matrix)
            {
            case FDNMatrix::householder:
                fdn = std::make_shared<HouseHolderFDN>(T60, dimensions, mConfig);
                break;
            case FDNMatrix::randomOrthogonal:
                fdn = std::make_shared<RandomOrthogonalFDN>(T60, dimensions, mConfig);
                break;
            default:
                fdn = std::make_shared<FDN>(T60, dimensions, mConfig);
                break;
            }
			releasePool.Add(fdn);
            mFDN.store(fdn);
			initialised.store(true);
		}

		////////////////////////////////////////

		void Reverb::UpdateReflectionFilters(const std::vector<Absorption>& absorptions)
		{
			if (!initialised.load())
				return;
			bool isZero = mFDN.load()->SetTargetReflectionFilters(absorptions);
			if (isZero)
			{
				mFDN.load()->Reset();
				running.store(false);
			}
			else
				running.store(true);
		}
	}
}
