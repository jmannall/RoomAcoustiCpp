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

		ReleasePool Source::releasePool;

		////////////////////////////////////////

		void Source::Init(const std::shared_ptr<Config>& config)
		{
			if (mSource == nullptr)
			{
				InitBuffers(config->numFrames);
				InitSource();
			}

			mAirAbsorption = std::make_unique<AirAbsorption>(1.0, config->fs);
			directivityFilter = std::make_unique<GraphicEQ>(config->frequencyBands, config->Q, config->fs);
			reverbInputFilter = std::make_unique<GraphicEQ>(config->frequencyBands, config->Q, config->fs);

			mDirectivity.store(SourceDirectivity::omni);

			feedsFDN.store(false);

			ResetFDNSlots();
			AllowAccess();
		}

		////////////////////////////////////////

		void Source::Remove()
		{
			PreventAccess();
			clearInputBuffer.store(true);
		}

		////////////////////////////////////////

		void Source::InitSource()
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			mSource = mCore->CreateSingleSourceDSP();
			mSource->DisableFarDistanceEffect();
			mSource->EnablePropagationDelay();
		}

		////////////////////////////////////////

		void Source::RemoveSource()
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			mCore->RemoveSingleSourceDSP(mSource);
			mSource.reset();
		}

		////////////////////////////////////////

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
			feedsFDN.store(true);
		}

		void Source::RemoveReverbSendSource()
		{
			feedsFDN.store(false);
			unique_lock<shared_mutex> lock(tuneInMutex);
			mCore->RemoveSingleSourceDSP(mReverbSendSource);
			mReverbSendSource.reset();
		}

		////////////////////////////////////////

		void Source::SetSpatialisationMode(const SpatialisationMode mode)
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
			currentSpatialisationMode = spatialisationMode;
		}

		////////////////////////////////////////

		void Source::SetImpulseResponseMode(const bool mode)
		{
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
			currentImpulseResponseMode = mode;
		}

		////////////////////////////////////////

		void Source::UpdateDirectivity(const SourceDirectivity directivity, const Coefficients<>& frequencyBands, const int numLateReverbChannels)
		{
			Coefficients reverbInput = Coefficients(frequencyBands.Length(), 1.0);
			switch (directivity)
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
			mDirectivity.store(directivity);
			// Divide energy between late reverb channels. Multiply by six to mimic shoebox room first reflections energy
			reverbInputFilter->SetTargetGains(6.0 * reverbInput / static_cast<Real>(numLateReverbChannels));
			hasChanged.store(true);
		}

		////////////////////////////////////////

		void Source::ResetFDNSlots()
		{
			freeFDNChannels.clear();
			freeFDNChannels.push_back(0);
		}

		////////////////////////////////////////

		void Source::Reset()
		{
			if (!CanEdit())
				return;
			if (mSource == nullptr)
				return;
			RemoveImageSources();
			ClearBuffers();
			RemoveSource();
			if (mReverbSendSource)
				RemoveReverbSendSource();
			ClearPointers();
		}

		////////////////////////////////////////

		void Source::ProcessAudio(Buffer& outputBuffer, Matrix& reverbInput, const Real lerpFactor)
		{
			if (!GetAccess())
				return;

#ifdef PROFILE_AUDIO_THREAD
			BeginSource();
#endif
			const int numFrames = inputBuffer.Length();

			if (currentImpulseResponseMode != impulseResponseMode.load())
				SetImpulseResponseMode(impulseResponseMode.load());

			if (currentSpatialisationMode != spatialisationMode.load())
				SetSpatialisationMode(spatialisationMode.load());

#ifdef PROFILE_AUDIO_THREAD
			BeginAirAbsorption();
#endif
			mAirAbsorption->ProcessAudio(inputBuffer, bStore, lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
			EndAirAbsorption();
#endif
			if (feedsFDN.load())
			{
				reverbInputFilter->ProcessAudio(bStore, bStoreReverb, lerpFactor);
				std::transform(bStoreReverb.begin(), bStoreReverb.end(), bInput.begin(),
					[](auto value) { return static_cast<float>(value); });

				{
					shared_lock<shared_mutex> lock(tuneInMutex);
					mReverbSendSource->SetSourceTransform(*transform.load());
					mReverbSendSource->SetBuffer(bInput);
					mReverbSendSource->ProcessAnechoic(bOutput.left, bOutput.right);
				}
				for (int i = 0; i < reverbInput.Rows(); i++)
				{
					for (int j = 0; j < numFrames; j++)
						reverbInput[i][j] += bOutput.left[j];
				}
			}
#ifdef PROFILE_AUDIO_THREAD
			BeginReflection();
#endif
			directivityFilter->ProcessAudio(bStore, bStore, lerpFactor);
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
				mSource->SetSourceTransform(*transform.load());
				mSource->SetBuffer(bInput);
				mSource->ProcessAnechoic(bOutput.left, bOutput.right);
			}
#ifdef PROFILE_AUDIO_THREAD
			End3DTI();
#endif

			int j = 0;
			for (int i = 0; i < numFrames; i++)
			{
				outputBuffer[j++] += static_cast<Real>(bOutput.left[i]);
				outputBuffer[j++] += static_cast<Real>(bOutput.right[i]);
			}
#ifdef PROFILE_AUDIO_THREAD
			EndSource();
#endif
			FreeAccess();
		}

		////////////////////////////////////////

		void Source::Update(const Vec3& position, const Vec4& orientation, const Real distance)
		{
			if (!GetAccess())
				return;
			mAirAbsorption->SetTargetDistance(distance);
			
			if (position == currentPosition && orientation == currentOrientation)
			{
				FreeAccess();
				return;
			}
			{
				lock_guard<std::mutex>lock(*dataMutex);
				currentPosition = position;
				currentOrientation = orientation;
			}
			hasChanged.store(true);
			UpdateTransform(position, orientation);
			FreeAccess();
		}

		////////////////////////////////////////
		
		void Source::UpdateData(const Source::AudioData source, const ImageSourceDataMap& imageSourceData, const std::shared_ptr<Config>& config)
		{
			if (!GetAccess())
				return;
			directivityFilter->SetTargetGains(source.directivity);

			if (source.feedsFDN && !mReverbSendSource)
			{
				InitReverbSendSource(config->GetImpulseResponseMode());
				UpdateImpulseResponseMode(config->GetImpulseResponseMode());
			}
			else if (!source.feedsFDN && mReverbSendSource)
				RemoveReverbSendSource();
			else
				feedsFDN.store(source.feedsFDN);
			UpdateImageSourceDataMap(imageSourceData);
			UpdateImageSources(config);
			FreeAccess();
		}

		////////////////////////////////////////

		std::optional<Source::Data> Source::GetData()
		{
			if (!GetAccess())
				return std::nullopt;
			lock_guard<std::mutex>lock(*dataMutex);
			Data data(-1, currentPosition, currentOrientation, mDirectivity.load(), hasChanged.load());
			FreeAccess();
			return data;
		}

		////////////////////////////////////////

		void Source::InitBuffers(int numFrames)
		{
			bStore = Buffer(numFrames);
			bStoreReverb = Buffer(numFrames);
			bInput = CMonoBuffer<float>(numFrames);
			bOutput.left = CMonoBuffer<float>(numFrames);
			bOutput.right = CMonoBuffer<float>(numFrames);
			bMonoOutput = CMonoBuffer<float>(numFrames);
		}

		////////////////////////////////////////

		void Source::ClearBuffers()
		{
			bInput.clear();
			bOutput.left.clear();
			bOutput.right.clear();
			bMonoOutput.clear();
			bStore.ResizeBuffer(1);
		}

		////////////////////////////////////////

		void Source::ClearPointers()
		{
			directivityFilter.reset();
			reverbInputFilter.reset();
			mAirAbsorption.reset();
			transform.load().reset();
		}

		////////////////////////////////////////

		void Source::UpdateTransform(const Vec3& position, const Vec4& orientation)
		{
			std::shared_ptr<CTransform> transformCopy = std::make_shared<CTransform>();
			transformCopy->SetOrientation(CQuaternion(static_cast<float>(orientation.w), static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)));
			transformCopy->SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));
			transform.store(transformCopy);
			releasePool.Add(transformCopy);
		}

		////////////////////////////////////////

		void Source::UpdateImageSourceDataMap(const ImageSourceDataMap& imageSourceData)
		{
			for (auto& [key, vSource] : currentImageSources)
			{
				auto it = imageSourceData.find(key);
				if (it == imageSourceData.end()) // case: old vSource
					vSource.second.Invisible();
			}

			for (auto& [key, vSource] : imageSourceData)
			{
				auto it = currentImageSources.find(key);
				if (it == currentImageSources.end()) // case: add new vSource
					currentImageSources.emplace(key, vSource);
				else // case: update exist vSource
					it->second.second = vSource.second;
			}
		}

		////////////////////////////////////////

		void Source::UpdateImageSources(const std::shared_ptr<Config>& config)
		{
			std::vector<std::string> keys;
			for (auto& [key, vSource] : currentImageSources)
			{
				if (UpdateImageSource(vSource.first, vSource.second, config))
					keys.push_back(key);
			}

			for (const std::string& key : keys)
				currentImageSources.erase(key);
		}
		
		////////////////////////////////////////

		bool Source::UpdateImageSource(int& id, ImageSourceData& data, const std::shared_ptr<Config>& config)
		{
			if (id < 0)		// case: virtual source does not exist
			{
				int fdnChannel = -1;
				if (data.IsFeedingFDN())
					fdnChannel = AssignFDNChannel(config->numLateReverbChannels);

				id = imageSources.NextID();
				if (id < 0)		// No free slots
					return false;

				imageSources.at(id).Init(&inputBuffer, config, data, fdnChannel);
			}
			else
			{
				int fdnChannel = -1;
				if (data.IsFeedingFDN() && imageSources.at(id).GetFDNChannel() < 0)
					fdnChannel = AssignFDNChannel(config->numLateReverbChannels);

				bool remove = imageSources.at(id).Update(data, fdnChannel);

				assert(!(data.IsFeedingFDN() && imageSources.at(id).GetFDNChannel() == -1));

				if (fdnChannel >= 0) // Add vSource old fdnChannel to freeFDNChannels (Also prevents leaking FDN channels if !data.visible and the channel is not assigned to vSource)
					freeFDNChannels.push_back(fdnChannel);
				if (remove)
				{
					int currentFDNChannel = imageSources.at(id).GetFDNChannel();
					if (currentFDNChannel >= 0)
						freeFDNChannels.push_back(currentFDNChannel);
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