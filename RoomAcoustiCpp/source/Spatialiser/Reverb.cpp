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
			newTransform.SetPosition(CVector3(static_cast<float>(position.x()), static_cast<float>(position.y()), static_cast<float>(position.z())));
			const shared_ptr<CTransform> newTransformCopy = make_shared<CTransform>(newTransform);

			releasePool.Add(newTransformCopy);
#ifdef __ANDROID__
			std::atomic_store(&transform, newTransformCopy);
#else
			transform.store(newTransformCopy, std::memory_order_release);
#endif
		}

		////////////////////////////////////////

		void ReverbSource::ProcessAudio(Buffer<>& outputBuffer, const AudioData& audioData)
		{
			PROFILE_ReverbSource
			if (audioData.clearBuffers)
				mSource->ResetSourceBuffers(); // Think this reallocates memory

			if (audioData.spatialisationMode != currentSpatialisationMode)
				SetSpatialisationMode(audioData.spatialisationMode);

			const int numFrames = ToInt(inputBuffer->Length());

			for (int i = 0; i < numFrames; i++)
				bInput[i] = static_cast<float>((*inputBuffer)[i]);
			/*std::transform(inputBuffer->begin(), inputBuffer->end(), bInput.begin(),
				[&](auto value) { return static_cast<float>(value); });*/

			{
				PROFILE_Spatialisation
				shared_lock<shared_mutex> lock(tuneInMutex);
#ifdef __ANDROID__
				mSource->SetSourceTransform(*std::atomic_load(&transform));
#else
				mSource->SetSourceTransform(*transform.load(std::memory_order_acquire));
#endif
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

		void Reverb::buildDelaySets(Matrix<int>& delayLineLengths, int fs,
			Real minDiffSeconds, Real minLineSeconds, Real maxLineSeconds)
		{
			const int numFDNs = ToInt(delayLineLengths.Rows());
			const int fdnSize = ToInt(delayLineLengths.Cols());

			// Constraints in number of samples.
			int minDiff = static_cast<int>(minDiffSeconds * fs);
			int minLine = static_cast<int>(minLineSeconds * fs);
			int maxLine = static_cast<int>(maxLineSeconds * fs);

			// Number of FDNs which have been assigned values for all of their lines.
			int numFilledFDNs = 0;
			// Number of assigned line lengths for each FDN.
			std::vector<int> numAssigned(numFDNs);
			// Set of prime factors already present among each FDN's lines.
			std::vector<std::unordered_set<int>> factorSetPerFDN(numFDNs);

			// Two variables which will be used inside the loop.
			std::vector<int> factors;
			std::vector<int> priorityOrder(numFDNs);

			// Consider every acceptable delay line length, in ascending order.
			for (int candidate = minLine; candidate < maxLine; ++candidate)
			{
				// Sort the FDNs based on how many lines have already been assigned to each;
				// prioritize ones with fewer assigned values.
				// https://gist.github.com/HViktorTsoi/58eabb4f7c5a303ced400bcfa816f6f5
				std::iota(priorityOrder.begin(), priorityOrder.end(), 0);
				std::sort(priorityOrder.begin(), priorityOrder.end(),
					[&](int a, int b) { return numAssigned[a] < numAssigned[b]; });

				// For each FDN, in order of priority, consider whether the current candidate
				// delay line length can be assigned to it.
				for (int fdnIdx : priorityOrder)
				{
					// Has the FDN been assigned values for all lines already?
					if (numAssigned[fdnIdx] >= fdnSize)
						break; // No need to continue: priority order ensures all following FDNs are also full.

					// Is the candidate at least `minDiff` larger than the latest (i.e., largest) value assigned to this FDN?
					// N.B. Only checked if at least one value has already been assigned.
					if (numAssigned[fdnIdx] > 0)
					{
						if (delayLineLengths(fdnIdx, numAssigned[fdnIdx] - 1) > candidate - minDiff)
							continue;
					}

					// If this point is reached, consider the candidate seriously.
					// Perform a prime factorization, needed to check if it's co-prime with all existing lines.
					factors = primeFactorization(candidate);

					// Is the candidate co-prime (i.e., does not share any prime factors) with all existing lines?
					bool invalid = false;
					for (int f : factors)
					{
						if (factorSetPerFDN[fdnIdx].count(f))
						{
							invalid = true;
							break;
						}
					}
					if (invalid) continue;

					// If this point is reached, assign the candidate to this FDN.
					delayLineLengths(fdnIdx, numAssigned[fdnIdx]) = candidate;
					// Add the prime factors of the candidate to the FDN's set.
					for (int f : factors)
						factorSetPerFDN[fdnIdx].insert(f);
					// Update the number of values assigned to this FDN.
					++numAssigned[fdnIdx];
					// If it was the last value needed by the FDN, update the tracker.
					if (numAssigned[fdnIdx] == fdnSize)
						++numFilledFDNs;
					// Avoid adding the same length to any other FDNs.
					break;
				}
				// If all FDNs are complete, stop the search.
				if (numFilledFDNs == numFDNs)
					break;
			}
			// If not all FDNs are complete (unassigned lines), fill any remaining ones with large values.
			// TODO: Warning? Exception?
			if (numFilledFDNs != numFDNs)
			{
				for (int fdnIdx = 0; fdnIdx < numFDNs; fdnIdx++)
				{
					for (int lineIdx = numAssigned[fdnIdx]; lineIdx < fdnSize; lineIdx++)
						delayLineLengths(fdnIdx, lineIdx) = maxLine + lineIdx;
				}
			}
		}

		////////////////////////////////////////

		void SingleFDN::SetTargetT60(const Coefficients<>& T60)
		{
			assert(IsValid());

#ifdef DEBUG_INIT
			Debug::Log("Init FDN: [" + CoefficientToStr(T60) + "]", Colour::Green);
#endif

#ifdef __ANDROID__
			std::atomic_load(&mFDN)->SetTargetT60(T60);
#else
			mFDN.load(std::memory_order_acquire)->SetTargetT60(T60);
#endif
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
#ifdef __ANDROID__
			std::atomic_store(&mFDN, fdn);
#else
			mFDN.store(fdn, std::memory_order_release);
#endif

			initialised.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		void SingleFDN::SetTargetOutputFilters(const std::vector<Coefficients<>>& gains)
		{
			PROFILE_UpdateAudioData
			assert(IsValid());

#ifdef __ANDROID__
			bool isZero = std::atomic_load(&mFDN)->SetTargetReflectionFilters(gains);
#else
			bool isZero = mFDN.load(std::memory_order_acquire)->SetTargetReflectionFilters(gains);
#endif
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

			// Set the minimum difference between delay line lengths based on the number of lines.
			// This avoids having excessively long lines when there are many, or excessively colored sets when there are few.
			Real minDiffSeconds;
			if (numFDNs * fdnSize > 100)
				minDiffSeconds = 2e-4;
			else if (numFDNs * fdnSize > 75)
				minDiffSeconds = 4e-4;
			else if (numFDNs * fdnSize > 50)
				minDiffSeconds = 6e-4;
			else if (numFDNs * fdnSize > 25)
				minDiffSeconds = 8e-4;
			else
				minDiffSeconds = 2e-3;

			Matrix<int> delayLineLengthSets(numFDNs, fdnSize);
			buildDelaySets(delayLineLengthSets, dspConfig->GetData().fs);
			// TODO: Detect failure (unassigned lines) and do something about it.
			Vec<int> delayLineLengths;

			FDNPtr fdns = std::make_shared<std::vector<std::unique_ptr<FDN<Complex>>>>(numFDNs);
			for (int i = 0; i < numFDNs; i++)
			{
				delayLineLengths = delayLineLengthSets.Row(i);
				
				switch (data.feedbackMatrix)
				{
				case FDNMatrix::householder:
					fdns->at(i) = std::make_unique<HouseHolderFDN<Complex>>(data.t60s(i), delayLineLengths, dspConfig);
					break;
				case FDNMatrix::randomOrthogonal:
					fdns->at(i) = std::make_unique<RandomOrthogonalFDN<Complex>>(data.t60s(i), delayLineLengths, dspConfig);
					break;
				default:
					fdns->at(i) = std::make_unique<FDN<Complex>>(data.t60s(i), delayLineLengths, dspConfig);
					break;
				}
				fdns->at(i)->SetMinimumReverbTime(data.minimumT60);
			}
			releasePool.Add(fdns);

#ifdef __ANDROID__
			std::atomic_store(&mFDNs, fdns);
#else
			mFDNs.store(fdns, std::memory_order_release);
#endif
			OctaveBand temporaryFilter(dspConfig->GetData().frequencyBands, dspConfig->GetData().fs);
			delayOffset = temporaryFilter.GetLatency();
			SetPrecedingDelay(data.delay, dspConfig->GetData().fs);

			initialised.store(true, std::memory_order_release);
		}
		
		////////////////////////////////////////

		void RAVES::SetTargetListenerResidues(size_t id, const Coefficients<>& residues)
		{
			assert(IsValid());

#ifdef __ANDROID__
			auto fdns = std::atomic_load(&mFDNs);
#else
			auto fdns = mFDNs.load();
#endif
			fdns->at(id)->SetTargetResidues(residues);

			running.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		void RAVES::ProcessReverberator(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData)
		{
			for (Buffer<>& buffer : outputBuffers)
				buffer.Reset();

			assert(IsValid());

#ifdef __ANDROID__
			auto fdns = std::atomic_load(&mFDNs);
#else
			auto fdns = mFDNs.load(); // Parallelise the processing of multiple FDNs
#endif
			for (int i = 0; i < fdns->size(); i++)
#if MATRIX_LIBRARY == EIGEN_FLAG
				fdns->at(i)->SubmitAudio(data.Row(i));
#else
				fdns->at(i)->SubmitAudio(data, i);
#endif

			audioThreadPool->ProcessFDNs(*fdns, outputBuffers, audioData); 
			/*for (int i = 0; i < fdns->size(); i++)
				fdns->at(i)->ProcessAudio(outputBuffers, lerpFactor);*/
		}
	}
}
