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

// Spatialiser headers
#include "Spatialiser/Reverb.h"
#include "Spatialiser/Globals.h"

// Unity headers
#include "Unity/Debug.h"

// DSP headers
#include "DSP/Interpolate.h"

// Common headers
#include "Common/SphericalGeometries.h"
#include "Common/RACProfiler.h"

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

		ReverbSource::ReverbSource(Binaural::CCore* core, const std::shared_ptr<Config> config, const Vec3& shift, const Buffer<>* inBuffer) : mCore(core), mShift(shift), inputBuffer(inBuffer)
		{
			bInput = CMonoBuffer<float>(config->numFrames);
			bOutput.left = CMonoBuffer<float>(config->numFrames);
			bOutput.right = CMonoBuffer<float>(config->numFrames);
			InitSource();
			UpdateSpatialisationMode(config->GetSpatialisationMode());
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
			transform.store(newTransformCopy, std::memory_order_release);
		}

		////////////////////////////////////////

		void ReverbSource::ProcessAudio(Buffer<>& outputBuffer)
		{
			PROFILE_ReverbSource
			if (clearBuffers.load(std::memory_order_acquire))
			{
				mSource->ResetSourceBuffers(); // Think this reallocates memory
				clearBuffers.store(false, std::memory_order_release);
			}

			const int numFrames = inputBuffer->Length();

			std::transform(inputBuffer->begin(), inputBuffer->end(), bInput.begin(),
				[&](auto value) { return static_cast<float>(value); });

			{
				PROFILE_Spatialisation
				shared_lock<shared_mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(*transform.load(std::memory_order_acquire));
				mSource->SetBuffer(bInput);
				mSource->ProcessAnechoic(bOutput.left, bOutput.right);
			}

			int j = 0;
			for (int i = 0; i < numFrames; i++)
			{
				outputBuffer[j++] += static_cast<Real>(bOutput.left[i]);
				outputBuffer[j++] += static_cast<Real>(bOutput.right[i]);
			}
		}

		//////////////////// Reverb class ////////////////////

		ReleasePool Reverb::releasePool;

		////////////////////////////////////////

		void Reverb::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			for (auto& source : mReverbSources)
				source->UpdateSpatialisationMode(mode);
		}

		////////////////////////////////////////

		std::vector<Vec3> Reverb::CalculateSourcePositions(const int numLateReverbChannels) const
		{
			// Distribute reverb sources around the listener
			std::vector<Vec3> points;
			points.reserve(numLateReverbChannels);
			switch (numLateReverbChannels)
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
			return points;
		}

		////////////////////////////////////////

		void Reverb::UpdateReverbSourcePositions(const Vec3& listenerPosition)
		{
			for (auto& reverbSource : mReverbSources)
				reverbSource->UpdatePosition(listenerPosition);
		}

		////////////////////////////////////////

		void Reverb::ProcessAudio(const Matrix& data, Buffer<>& outputBuffer, const Real lerpFactor)
		{
			PROFILE_LateReverb
			if (!running.load(std::memory_order_acquire))
				return;

			mFDN.load(std::memory_order_acquire)->ProcessAudio(data, reverbSourceInputs, lerpFactor);
			
			audioThreadPool->ProcessReverbSources(mReverbSources, outputBuffer);
			/*for (auto& source : mReverbSources)
				source->ProcessAudio(outputBuffer);*/
		}

		////////////////////////////////////////

		void Reverb::SetTargetT60(const Coefficients<>& T60)
		{
			if (!initialised.load(std::memory_order_acquire))
				return;

#ifdef DEBUG_INIT
			Debug::Log("Init FDN: [" + RealToStr(T60[0]) + ", " + RealToStr(T60[1]) + ", " +
				RealToStr(T60[2]) + ", " + RealToStr(T60[3]) + ", " + RealToStr(T60[4]) + "]", Colour::Green);
#endif
			mFDN.load(std::memory_order_acquire)->SetTargetT60(T60);
		}

		////////////////////////////////////////

		void Reverb::InitLateReverb(const Coefficients<>& T60, const Vec& dimensions, const FDNMatrix matrix, const std::shared_ptr<Config> config)
		{
            std::shared_ptr<FDN> fdn;
            switch (matrix)
            {
            case FDNMatrix::householder:
                fdn = std::make_shared<HouseHolderFDN>(T60, dimensions, config);
                break;
            case FDNMatrix::randomOrthogonal:
                fdn = std::make_shared<RandomOrthogonalFDN>(T60, dimensions, config);
                break;
            default:
                fdn = std::make_shared<FDN>(T60, dimensions, config);
                break;
            }
			releasePool.Add(fdn);
            mFDN.store(fdn, std::memory_order_release);
			initialised.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		void Reverb::UpdateReflectionFilters(const std::vector<Absorption<>>& absorptions)
		{
			PROFILE_UpdateAudioData
			if (!initialised.load(std::memory_order_acquire))
				return;
			bool isZero = mFDN.load(std::memory_order_acquire)->SetTargetReflectionFilters(absorptions);
			if (isZero)
			{
				mFDN.load(std::memory_order_acquire)->Reset();
				running.store(false, std::memory_order_release);
			}
			else
				running.store(true, std::memory_order_release);
		}
	}
}
