/*
* @class Source
*
* @brief Declaration of Source class
*
*/

// Spatialiser headers
#include "Spatialiser/Source.h"
#include "Spatialiser/Directivity.h"

// DSP headers
#include "DSP/Interpolate.h"

// Unity headers
#include "Unity/Debug.h"
#include "Unity/UnityInterface.h"

#include <mutex>

using namespace Common;
namespace RAC
{
	using namespace Common;
	// using namespace Unity;
	using namespace DSP;
	namespace Spatialiser
	{

		//////////////////// Source class ////////////////////

		////////////////////////////////////////

		Source::Source(Binaural::CCore* core, const std::shared_ptr<Config>& config) : mCore(core), targetGain(0.0f), currentGain(0.0f), mAirAbsorption(1.0, config->fs), mDirectivity(SourceDirectivity::omni),
			directivityFilter(config->frequencyBands, config->Q, config->fs), reverbInputFilter(config->frequencyBands, config->Q, config->fs), feedsFDN(false), hasChanged(true)
		{
			dataMutex = std::make_shared<std::mutex>();
			imageSourcesMutex = std::make_shared<std::mutex>();
			imageSourceDataMutex = std::make_shared<std::mutex>();
			currentDataMutex = std::make_shared<std::mutex>();

			// Initialise source to core
			{
				unique_lock<shared_mutex> lock(tuneInMutex);
				mSource = mCore->CreateSingleSourceDSP();
				mSource->DisableFarDistanceEffect();
				mSource->EnablePropagationDelay();

				if (config->GetImpulseResponseMode())
				{
					mSource->DisableDistanceAttenuationSmoothingAnechoic();
					mSource->DisableInterpolation();
				}

				//Select spatialisation mode
				UpdateSpatialisationMode(config->GetSpatialisationMode());
			}

			ResetFDNSlots();
			bStore.ResizeBuffer(config->numFrames);
			bStoreReverb.ResizeBuffer(config->numFrames);
			bInput = CMonoBuffer<float>(config->numFrames);
			bOutput.left = CMonoBuffer<float>(config->numFrames);
			bOutput.right = CMonoBuffer<float>(config->numFrames);
			bMonoOutput = CMonoBuffer<float>(config->numFrames);
		}

		////////////////////////////////////////

		Source::~Source()
		{
			{
				unique_lock<shared_mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
			}
			Reset();
		}

		void Source::InitReverbSendSource(const bool impulseResponseMode)
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			mReverbSendSource = mCore->CreateSingleSourceDSP();
			mReverbSendSource->EnablePropagationDelay();
			mReverbSendSource->DisableDistanceAttenuationAnechoic();
			mReverbSendSource->DisableFarDistanceEffect();
			mReverbSendSource->DisableNearFieldEffect();
			mReverbSendSource->SetSpatializationMode(Binaural::TSpatializationMode::NoSpatialization);
			if (impulseResponseMode)
			{
				mReverbSendSource->DisableDistanceAttenuationSmoothingAnechoic();
				mReverbSendSource->DisableInterpolation();
			}
		}

		void Source::RemoveReverbSendSource()
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			mCore->RemoveSingleSourceDSP(mReverbSendSource);
			mReverbSendSource.reset();
		}

		////////////////////////////////////////

		void Source::UpdateSpatialisationMode(const SpatialisationMode mode)
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

			{
				lock_guard<mutex> lock(*imageSourcesMutex);
				for (auto& [key, imageSource] : mImageSources)
					imageSource.UpdateSpatialisationMode(mode);
			}
		}

		////////////////////////////////////////

		void Source::UpdateImpulseResponseMode(const Real lerpFactor, const bool mode)
		{
			{
				lock_guard<std::mutex> lock(*dataMutex);
				if (mode)
				{
					mSource->DisableDistanceAttenuationSmoothingAnechoic();
					mSource->DisableInterpolation();

					if (mReverbSendSource)
					{
						mReverbSendSource->DisableDistanceAttenuationSmoothingAnechoic();
						mReverbSendSource->DisableInterpolation();
					}
				}
				else
				{
					mSource->EnableDistanceAttenuationSmoothingAnechoic();
					mSource->EnableInterpolation();

					if (mReverbSendSource)
					{
						mReverbSendSource->EnableDistanceAttenuationSmoothingAnechoic();
						mReverbSendSource->EnableInterpolation();
					}
				}
			}

			{
				lock_guard<mutex> lock(*imageSourcesMutex);
				for (auto& [key, imageSource] : mImageSources)
					imageSource.UpdateImpulseResponseMode(mode);
			}
		}

		////////////////////////////////////////

		void Source::UpdateDiffractionModel(const DiffractionModel model, const int fs)
		{
			lock_guard<mutex> lock(*imageSourcesMutex);
			for (auto& [key, imageSource] : mImageSources)
				imageSource.UpdateDiffractionModel(model, fs);
		}

		////////////////////////////////////////

		void Source::UpdateDirectivity(const SourceDirectivity directivity, const Coefficients<>& frequencyBands, const int numLateReverbChannels)
		{
			{
				Coefficients reverbInput = Coefficients(frequencyBands.Length(), 1.0);
				lock_guard<mutex> lock(*dataMutex);
				mDirectivity = directivity;

				switch (mDirectivity)
				{
				case SourceDirectivity::omni:
					break;
				case SourceDirectivity::subcardioid:
					reverbInput = 1.0 / 1.3;	// 1 / Directivity Factor (DF) -> DF = 10 ^ (Directivity Index / 20) 
					break;
				case SourceDirectivity::cardioid:
					reverbInput = 1.0 / 1.7;
					break;
				case SourceDirectivity::supercardioid:
					reverbInput = 1.0 / 1.9;
					break;
				case SourceDirectivity::hypercardioid:
					reverbInput = 0.5;
					break;
				case SourceDirectivity::bidirectional:
					reverbInput = 1.0 / 1.7;
					break;
				case SourceDirectivity::genelec8020c:
					reverbInput = GENELEC.AverageResponse(frequencyBands);
					break;
				}
				// Divide energy between late reverb channels. Multiply by six to mimic shoebox room first reflections energy
				reverbInputFilter.SetTargetGains(6.0 * reverbInput / static_cast<Real>(numLateReverbChannels));
			}
			{ lock_guard<mutex> lock(*currentDataMutex); hasChanged = true; }
		}

		////////////////////////////////////////

		void Source::ResetFDNSlots()
		{
			freeFDNChannels.clear();
			freeFDNChannels.push_back(0);
		}

		////////////////////////////////////////

		void Source::ProcessDirect(const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer, const Real lerpFactor)
		{
			lock_guard<std::mutex> lock(*dataMutex);
			/*if (directivityFilter.Invalid())
				return;*/

#ifdef PROFILE_AUDIO_THREAD
			BeginSource();
			BeginAirAbsorption();
#endif
			mAirAbsorption.ProcessAudio(data, bStore, lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
			EndAirAbsorption();
#endif
			if (feedsFDN)
			{
				reverbInputFilter.ProcessAudio(bStore, bStoreReverb, lerpFactor);
				std::transform(bStoreReverb.begin(), bStoreReverb.end(), bInput.begin(),
					[](auto value) { return static_cast<float>(value); });

				// lock_guard<mutex> lock(tuneInMutex);
				{
					shared_lock<shared_mutex> lock(tuneInMutex);
					mReverbSendSource->SetSourceTransform(transform);
					mReverbSendSource->SetBuffer(bInput);
					mReverbSendSource->ProcessAnechoic(bOutput.left, bOutput.right);
				}
				for (int i = 0; i < reverbInput.Rows(); i++)
				{
					for (int j = 0; j < data.Length(); j++)
						reverbInput[i][j] += bOutput.left[j];
				}
			}
#ifdef PROFILE_AUDIO_THREAD
			BeginReflection();
#endif
			directivityFilter.ProcessAudio(bStore, bStore, lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
			EndReflection();
#endif
			std::transform(bStore.begin(), bStore.end(), bInput.begin(),
				[](auto value) { return static_cast<float>(value); });

#ifdef PROFILE_AUDIO_THREAD
			Begin3DTI();
#endif
			{
				shared_lock<shared_mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(transform);
				mSource->SetBuffer(bInput);
				mSource->ProcessAnechoic(bOutput.left, bOutput.right);
			}
#ifdef PROFILE_AUDIO_THREAD
			End3DTI();
#endif

			int j = 0;
			for (int i = 0; i < data.Length(); i++)
			{
				outputBuffer[j++] += static_cast<Real>(bOutput.left[i]);
				outputBuffer[j++] += static_cast<Real>(bOutput.right[i]);
			}
#ifdef PROFILE_AUDIO_THREAD
			EndSource();
#endif
		}

		////////////////////////////////////////

		void Source::ProcessAudio(const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer, const Real lerpFactor)
		{
			{
				lock_guard<mutex> lock(*imageSourcesMutex);
				std::latch latch(mImageSources.size() + 1);

				size_t index = 0;
				for (auto& [key, imageSource] : mImageSources)
				{
					size_t threadIndex = index++; // Each thread gets a unique index
					audioThreadPool->enqueue([&, threadIndex] {
						std::get<2>(threadResults[threadIndex]) = imageSource.ProcessAudio(data, std::get<0>(threadResults[threadIndex]), std::get<1>(threadResults[threadIndex]), lerpFactor);
						// No need for a mutex, each thread writes to a unique index

						latch.count_down();
						});
				}

				audioThreadPool->enqueue([&] {
					ProcessDirect(data, reverbInput, outputBuffer, lerpFactor);
					latch.count_down();
					});

				// Wait for all threads to finish
				latch.wait();

				// Now safely merge the results
				for (const auto& [localReverb, localOutput, fdnChannel] : threadResults)
				{
					outputBuffer += localOutput;
					if (fdnChannel != -1)
					{
						for (int i = 0; i < data.Length(); i++)
							reverbInput[fdnChannel][i] += localReverb[i];
					}
				}
			}
		}

		////////////////////////////////////////

		void Source::Update(const Vec3& position, const Vec4& orientation, const Real distance)
		{
			{
				lock_guard<std::mutex>lock(*dataMutex);
				mAirAbsorption.SetTargetDistance(distance);
			}
			{
				lock_guard<std::mutex>lock(*currentDataMutex);
				if (position == currentPosition && orientation == currentOrientation)
					return;
				hasChanged = true;
				currentPosition = position;
				currentOrientation = orientation;
			}
			lock_guard<std::mutex>lock(*dataMutex);
			transform.SetOrientation(CQuaternion(static_cast<float>(orientation.w), static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)));
			transform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));
		}

		////////////////////////////////////////

		void Source::UpdateImageSourceDataMap()
		{
			for (auto& [key, vSource] : currentImageSources)
			{
				auto it = targetImageSources.find(key);
				if (it == targetImageSources.end()) // case: old vSource
					vSource.Invisible();
			}

			for (auto& [key, vSource] : targetImageSources)
			{
				auto it = currentImageSources.find(key);
				if (it == currentImageSources.end()) // case: add new vSource
					currentImageSources.emplace(key, vSource);
				else // case: update exist vSource
					it->second = vSource;
			}
		}

		////////////////////////////////////////

		void Source::UpdateImageSources(const std::shared_ptr<Config>& config)
		{
			std::vector<std::string> keys;
			for (auto& [key, vSource] : currentImageSources)
			{
				if (UpdateImageSource(vSource, config))
					keys.push_back(key);
			}

			for (const std::string& key : keys)
				currentImageSources.erase(key);
		}

		////////////////////////////////////////

		bool Source::UpdateImageSource(const ImageSourceData& data, const std::shared_ptr<Config>& config)
		{
			auto it = mImageSources.find(data.GetKey());
			if (it == mImageSources.end())		// case: virtual source does not exist
			{
				int fdnChannel = -1;
				if (data.IsFeedingFDN())
					fdnChannel = AssignFDNChannel(config->numLateReverbChannels);
				{
					unique_lock<mutex> lock(*imageSourcesMutex, defer_lock);
					if (lock.try_lock())
					{
						mImageSources.try_emplace(data.GetKey(), mCore, config, data, fdnChannel);
						threadResults.resize(mImageSources.size(), { Buffer(config->numFrames), Buffer(2 * config->numFrames), -1 });
					}
					else
					{
#ifdef DEBUG_IMAGE_SOURCE
						Unity::Debug::Log("Failed to create image source", Unity::Colour::Red);
#endif
					}
				}
			}
			else
			{
				int fdnChannel = -1;
				if (data.IsFeedingFDN() && it->second.GetFDNChannel() < 0)
					fdnChannel = AssignFDNChannel(config->numLateReverbChannels);

				bool remove = it->second.Update(data, fdnChannel);

				assert(!(data.IsFeedingFDN() && it->second.GetFDNChannel() == -1));

				if (fdnChannel >= 0) // Add vSource old fdnChannel to freeFDNChannels (Also prevents leaking FDN channels if !data.visible and the channel is not assigned to vSource)
					freeFDNChannels.push_back(fdnChannel);
				if (remove)
				{
					if (it->second.GetFDNChannel() >= 0)
						freeFDNChannels.push_back(it->second.GetFDNChannel());

					size_t n = 0;
					{
						unique_lock<mutex> lock(*imageSourcesMutex, defer_lock);
						if (lock.try_lock())
						{
							n = mImageSources.erase(data.GetKey());
							BeginFDN();
							threadResults.resize(mImageSources.size(), { Buffer(config->numFrames), Buffer(2 * config->numFrames), -1 });
							EndFDN();
						}
					}

					if (n == 1) // Check vSource has been successfully erased
						return true;
				}
			}
			return false;
		}

		////////////////////////////////////////

		int Source::AssignFDNChannel(const int numLateReverbChannels)
		{
			if (freeFDNChannels.back() >= numLateReverbChannels)
				freeFDNChannels.back() = 0;
			int fdnChannel = static_cast<int>(freeFDNChannels.back());
			if (freeFDNChannels.size() > 1)
				freeFDNChannels.pop_back();
			else
				freeFDNChannels[0]++;
			return fdnChannel;
		}
	}
}