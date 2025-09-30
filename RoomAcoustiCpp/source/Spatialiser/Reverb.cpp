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

		ReverbSource::ReverbSource(Binaural::CCore* core, const std::shared_ptr<DSPConfig> dspConfig, const Vec3& shift, const Buffer<>* inBuffer) : mCore(core), mShift(shift), inputBuffer(inBuffer)
		{
			int numFrames = dspConfig->GetData().numFrames;
			bInput = CMonoBuffer<float>(numFrames);
			bOutput.left = CMonoBuffer<float>(numFrames);
			bOutput.right = CMonoBuffer<float>(numFrames);
			InitSource(dspConfig);
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

		void ReverbSource::InitSource(const std::shared_ptr<DSPConfig>& dspConfig)
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
				mSource->DisableInterpolation();
				mSource->DisableNearFieldEffect();
				mSource->DisableFarDistanceEffect();
				SetSpatialisationMode(dspConfig->GetSpatialisationMode());
			}
			
		}

		////////////////////////////////////////

		void ReverbSource::SetSpatialisationMode(const SpatialisationMode mode)
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
			currentSpatialisationMode = mode;
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

		void ReverbSource::ProcessAudio(Buffer<>& outputBuffer, const AudioData& audioData)
		{
			PROFILE_ReverbSource
			if (audioData.clearBuffers)
				mSource->ResetSourceBuffers(); // Think this reallocates memory

			if (audioData.spatialisationMode != currentSpatialisationMode)
				SetSpatialisationMode(audioData.spatialisationMode);

			const int numFrames = SizeToInt(inputBuffer->Length());

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

		std::vector<Vec3> Reverb::CalculateSourcePositions(const int numReverbSources) const
		{
			// Distribute reverb sources around the listener
			std::vector<Vec3> points;
			points.reserve(numReverbSources);
			switch (numReverbSources)
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

		void Reverb::ProcessAudio(const Matrix<>& data, Buffer<>& outputBuffer, const AudioData& audioData)
		{
			PROFILE_LateReverb
			if (!running.load(std::memory_order_acquire))
				return;

			ProcessReverberator(data, reverbSourceInputs, audioData);

			audioThreadPool->ProcessReverbSources(mReverbSources, outputBuffer, audioData);
			/*for (auto& source : mReverbSources)
				source->ProcessAudio(outputBuffer);*/
		}

		////////////////////////////////////////

		void SingleFDN::SetTargetT60(const Coefficients<>& T60)
		{
			if (!initialised.load(std::memory_order_acquire))
				return;

#ifdef DEBUG_INIT
			Debug::Log("Init FDN: [" + CoefficientToStr(T60[0]) + "]", Colour::Green);
#endif

			mFDN.load(std::memory_order_acquire)->SetTargetT60(T60);
		}

		////////////////////////////////////////

		void SingleFDN::InitLateReverb(const Coefficients<>& T60, const Vec<>& delayLineLengths, const LateReverbData& data, const std::shared_ptr<DSPConfig>& dspConfig)
		{
            std::shared_ptr<FDN<>> fdn;
            switch (data.feedbackMatrix)
            {
            case FDNMatrix::householder:
                fdn = std::make_shared<HouseHolderFDN<>>(T60, delayLineLengths, dspConfig);
                break;
            case FDNMatrix::randomOrthogonal:
                fdn = std::make_shared<RandomOrthogonalFDN<>>(T60, delayLineLengths, dspConfig);
                break;
            default:
                fdn = std::make_shared<FDN<>>(T60, delayLineLengths, dspConfig);
                break;
            }
			releasePool.Add(fdn);
            mFDN.store(fdn, std::memory_order_release);

			initialised.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		void SingleFDN::SetTargetOutputFilters(const std::vector<Absorption<>>& gains)
		{
			PROFILE_UpdateAudioData
			if (!initialised.load(std::memory_order_acquire))
				return;

			bool isZero = mFDN.load(std::memory_order_acquire)->SetTargetReflectionFilters(gains);
			if (isZero)
			{
				running.store(false, std::memory_order_release);
				// TODO: reset internal buffers of late reverb to zero
			}
			else
				running.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		void RAVES::InitLateReverb(const MoDARTData& data, const std::shared_ptr<DSPConfig>& dspConfig)
		{
			int numFDNs = dspConfig->GetNumFDNs();
			int fdnSize = dspConfig->GetData().fdnSize;

			std::vector<int> delayLineLengths(fdnSize, -1);

			FDNPtr fdns = std::make_shared<std::vector<std::unique_ptr<FDN<Complex>>>>(numFDNs);
			for (int i = 0; i < numFDNs; i++)
			{
				// TODO: Be smarter about this
				delayLineLengths = GetSetOfPrimes(100+i, fdnSize, std::max(12, numFDNs));

				// TODO: Check if any of the values are -1 (error in generating the list of primes) and adapt accordingly

				switch (data.feedbackMatrix)
				{
				case FDNMatrix::householder:
					fdns->at(i) = std::make_unique<HouseHolderFDN<Complex>>(data.t60s[i], delayLineLengths, dspConfig);
					break;
				case FDNMatrix::randomOrthogonal:
					fdns->at(i) = std::make_unique<RandomOrthogonalFDN<Complex>>(data.t60s[i], delayLineLengths, dspConfig);
					break;
				default:
					fdns->at(i) = std::make_unique<FDN<Complex>>(data.t60s[i], delayLineLengths, dspConfig);
					break;
				}
				fdns->at(i)->SetMinimumReverbTime(data.minimumT60);
			}
			releasePool.Add(fdns);

			mFDNs.store(fdns, std::memory_order_release);
			OctaveBand temporaryFilter(dspConfig->GetData().frequencyBands, dspConfig->GetData().fs);
			delayOffset = temporaryFilter.GetLatency();
			SetPrecedingDelay(data.delay, dspConfig->GetData().fs);

			initialised.store(true, std::memory_order_release);
		}
		
		////////////////////////////////////////

		void RAVES::SetTargetListenerResidues(size_t id, const Coefficients<>& residues)
		{
			if (!initialised.load(std::memory_order_acquire))
				return;

			auto fdns = mFDNs.load();
			fdns->at(id)->SetTargetResidues(residues);

			running.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		void RAVES::ProcessReverberator(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData)
		{
			for (Buffer<>& buffer : outputBuffers)
				buffer.Reset();

			if (!initialised.load(std::memory_order_acquire))
				return;

			auto fdns = mFDNs.load(); // Parallelise the processing of multiple FDNs
			for (int i = 0; i < fdns->size(); i++)
				fdns->at(i)->SubmitAudio(data.GetRowStartPtr(i));

			audioThreadPool->ProcessFDNs(*fdns, outputBuffers, audioData);
			/*for (int i = 0; i < fdns->size(); i++)
				fdns->at(i)->ProcessAudio(outputBuffers, lerpFactor);*/
		}
	}
}
