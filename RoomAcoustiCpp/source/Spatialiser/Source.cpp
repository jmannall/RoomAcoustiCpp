/*
* @class Source
*
* @brief Declaration of Source class
*
*/

// C++ headers
#include <mutex>

//Common headers
#include "Common/RACProfiler.h"
 
// Spatialiser headers
#include "Spatialiser/Source.h"
#include "Spatialiser/Directivity.h"
#include "Spatialiser/Globals.h"

// DSP headers
#include "DSP/Interpolate.h"

// Unity headers
#include "Unity/Debug.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{

		//////////////////// Source class ////////////////////

		ReleasePool Source::releasePool;

		////////////////////////////////////////

		void Source::Init(const std::shared_ptr<DSPConfig>& dspConfig, const Vec<int>& frequencyIndexing)
		{
			InitSource(dspConfig);
			const DSPData& data = dspConfig->GetData();
			InitBuffers(data.numFrames);

			mAirAbsorption = std::make_unique<AirAbsorption>((Real)1.0, data.fs);
			directivityFilter = std::make_unique<GraphicEQ<>>(data.frequencyBands, data.Q, data.fs);
			reverbInputFilter = std::make_unique<GraphicEQ<>>(data.frequencyBands, data.Q, data.fs);

			mDirectivity.store(SourceDirectivity::omni, std::memory_order_release);

			updateFlags.RecordChange();

			if (dspConfig->GetLateReverbModel() == LateReverbModel::raves)
				InitMoDARTParameters(frequencyIndexing, data.numFrames);

			ResetFDNSlots();
			AllowAccess();
			isReset.store(false, std::memory_order_release);
		}

		////////////////////////////////////////

		void Source::Remove()
		{
			PreventAccess();
			RemoveImageSources();
			clearInputBuffer.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		void Source::InitSource(const std::shared_ptr<DSPConfig>& dspConfig)
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			mSource = mCore->CreateSingleSourceDSP();
			mSource->DisableFarDistanceEffect();
			mSource->EnablePropagationDelay();
			SetSpatialisationMode(dspConfig->GetSpatialisationMode());
			SetImpulseResponseMode(dspConfig->GetImpulseResponseMode());
		}

		////////////////////////////////////////

		void Source::RemoveSource()
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			mCore->RemoveSingleSourceDSP(mSource);
			mSource.reset();
		}

		////////////////////////////////////////

		void Source::InitReverbSendSource(const std::shared_ptr<DSPConfig>& dspConfig)
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			mReverbSendSource = mCore->CreateSingleSourceDSP();
			mReverbSendSource->EnablePropagationDelay();
			mReverbSendSource->DisableDistanceAttenuationAnechoic();
			mReverbSendSource->DisableFarDistanceEffect();
			mReverbSendSource->DisableNearFieldEffect();
			mReverbSendSource->SetSpatializationMode(Binaural::TSpatializationMode::NoSpatialization);
			SetImpulseResponseMode(dspConfig->GetImpulseResponseMode());
		}

		void Source::RemoveReverbSendSource()
		{
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
			currentSpatialisationMode = mode;
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
			GetAccess();
			Coefficients<> reverbInput = Coefficients<>::Constant(frequencyBands.Length(), 1.0);
			switch (directivity)
			{
			default:
			case SourceDirectivity::omni:
				break;
			case SourceDirectivity::subcardioid:
				reverbInput.SetConstant((Real)(1.0 / 1.3));	// 1 / Directivity Factor (DF) -> DF = 10 ^ (Directivity Index / 20) 
				break;
			case SourceDirectivity::cardioid:
				reverbInput.SetConstant((Real)(1.0 / 1.7));
				break;
			case SourceDirectivity::supercardioid:
				reverbInput.SetConstant((Real)(1.0 / 1.9));
				break;
			case SourceDirectivity::hypercardioid:
				reverbInput.SetConstant((Real)(0.5));
				break;
			case SourceDirectivity::bidirectional:
				reverbInput.SetConstant((Real)(1.0 / 1.7));
				break;
			case SourceDirectivity::genelec8020c:
				reverbInput = GENELEC.AverageResponse(frequencyBands);
				break;
			case SourceDirectivity::qscK8:
				reverbInput = QSC_K8.AverageResponse(frequencyBands);
				break;
			}
			mDirectivity.store(directivity, std::memory_order_release);
			// Divide energy between late reverb channels. Multiply by six to mimic shoebox room first reflections energy
			reverbInputFilter->SetTargetGains((Real)6.0 * reverbInput / static_cast<Real>(numLateReverbChannels));
			updateFlags.RecordChange();
			FreeAccess();
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
			if (isReset.load(std::memory_order_acquire))
				return;
			if (!CanEdit())
				return;
			if (!mSource) // TODO: Is this check necessary?
				return;
			{
				lock_guard<std::mutex>lock(*imageSourcesMutex);
				currentImageSources.clear();
			}
			ClearBuffers();
			RemoveSource();
			if (mReverbSendSource)
				RemoveReverbSendSource();
			ClearPointers();
			isReset.store(true, std::memory_order_release);
		}

		////////////////////////////////////////

		void Source::ProcessAudio(Buffer<>& outputBuffer, const AudioData& audioData)
		{
			if (!GetAccess())
				return;

#ifdef __ANDROID__
			if (!std::atomic_load(&transform)) // Check if the source position has been updated before using
			{
				FreeAccess();
				return;
			}
#else
			if (!transform.load(std::memory_order_acquire)) // Check if the source position has been updated before using
			{
				FreeAccess();
				return;
			}
#endif

			PROFILE_Source
			const int numFrames = ToInt(inputBuffer.Length());

			if (audioData.impulseResponseMode != currentImpulseResponseMode)
			{
				shared_lock<shared_mutex> lock(tuneInMutex);
				SetImpulseResponseMode(audioData.impulseResponseMode);
			}

			if (audioData.spatialisationMode != currentSpatialisationMode)
				SetSpatialisationMode(audioData.spatialisationMode);

			if (audioData.lateReverbModel == LateReverbModel::raves && audioData.lateReverbEnabled)
			{
				modartSendProcessed = true;
				if (audioData.clearBuffers)
					octaveBandFilter.ClearBuffers();
				PROFILE_OctaveBand
				for (int i = 0; i < numFrames; i++)
				{
					const Buffer<>& bands = octaveBandFilter.GetOutput(inputBuffer[i], audioData.lerpFactor);
					for (int j = 0; j < bands.Length(); j++)
						frequencyBands(j, i) = bands[j];
				}
			}

			if (!audioData.earlyReverbEnabled)
			{
				FreeAccess();
				return;
			}
			mAirAbsorption->ProcessAudio(inputBuffer, bStore, audioData.lerpFactor);

			{
				PROFILE_Reflection
				directivityFilter->ProcessAudio(bStore, bStore, audioData.lerpFactor);
			}

			for (int i = 0; i < numFrames; i++)
				bInput[i] = static_cast<float>(bStore[i]);
			/*std::transform(bStore.begin(), bStore.end(), bInput.begin(),
				[](auto value) { return static_cast<float>(value); });*/

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
			FreeAccess();
		}

		////////////////////////////////////////

		void Source::ProcessMoDARTSend(Matrix<>& reverbInput, const Real lerpFactor)
		{
			if (!GetAccess())
				return;

			if (!modartSendProcessed)
			{
				FreeAccess();
				return;
			}

			const int numFrames = ToInt(inputBuffer.Length());
			const int numBands = octaveBandFilter.NumBands();

			for (int i = 0; i < reverbInput.Rows(); i++)
			{
				int bandIndex = octaveBandFilter.GetBandIndex(ravesResidues[i].frequencyIndex);
				for (int j = 0; j < numFrames; j++)
				{
					Complex input = ravesResidues[i].GetOutput(frequencyBands(bandIndex, j), lerpFactor);
					reverbInput(i, 2 * j) += input.real();
					reverbInput(i, 2 * j + 1) += input.imag();
				}
			}
			modartSendProcessed = false;
			FreeAccess();
		}

		////////////////////////////////////////

		void Source::ProcessSingleFDNSend(Matrix<>& reverbInput, const Real lerpFactor)
		{
			if (!GetAccess())
				return;

			PROFILE_Source
			const int numFrames = ToInt(inputBuffer.Length());

			{
				PROFILE_Reflection
				reverbInputFilter->ProcessAudio(inputBuffer, bStoreReverb, lerpFactor);
			}

			for (int i = 0; i < numFrames; i++)
				bInput[i] = static_cast<float>(bStoreReverb[i]);
			/*std::transform(bStoreReverb.begin(), bStoreReverb.end(), bInput.begin(),
				[](auto value) { return static_cast<float>(value); });*/

			{
				shared_lock<shared_mutex> lock(tuneInMutex);
				if (!mReverbSendSource) // Check if the reverb send source hasn't been removed
				{
					FreeAccess();
					return;
				}
#ifdef __ANDROID__
				mReverbSendSource->SetSourceTransform(*std::atomic_load(&transform));
#else
				mReverbSendSource->SetSourceTransform(*transform.load(std::memory_order_acquire));
#endif
				mReverbSendSource->SetBuffer(bInput);
				mReverbSendSource->ProcessAnechoic(bOutput.left, bOutput.right);
			}
			for (int i = 0; i < reverbInput.Rows(); i++)
			{
				for (int j = 0; j < numFrames; j++)
					reverbInput(i, j) += static_cast<Real>(bOutput.left[j]);
				/*auto start = reverbInput.begin() + i * numFrames;
				for (int j = 0; j < numFrames; j++)
					start[j] += static_cast<Real>(bOutput.left[j]);*/
			}
			FreeAccess();
		}

		////////////////////////////////////////

		void Source::Update(const Vec3& position, const Vec4& orientation, const Real distance)
		{
			if (!GetAccess())
				return;
			mAirAbsorption->SetTargetDistance(distance);
			
#ifdef __ANDROID__
			if (position == currentPosition && orientation == currentOrientation && std::atomic_load(&transform))
			{
				FreeAccess();
				return;
			}
#else
			if (position == currentPosition && orientation == currentOrientation && transform.load(std::memory_order_acquire))
			{
				FreeAccess();
				return;
			}
#endif
			UpdateTransform(position, orientation);
			if ((position - currentPosition).Normal() > EPS_POSITION || 2.0 * std::acos(std::abs(orientation.dot(currentOrientation))) > EPS_ORIENTATION)
			{
				{
					lock_guard<std::mutex>lock(*dataMutex);
					currentPosition = position;
					currentOrientation = orientation;
				}
				updateFlags.RecordChange();
			}
			FreeAccess();
		}

		////////////////////////////////////////
		
		void Source::UpdateData(const Source::DSPParameters source, ImageSourceDataMap& imageSourceData, const std::shared_ptr<DSPConfig>& dspConfig)
		{
			if (!GetAccess())
				return;
			directivityFilter->SetTargetGains(source.directivity);

			if (source.feedsFDN && dspConfig->GetLateReverbModel() == LateReverbModel::fdn && !mReverbSendSource)
			{
				InitReverbSendSource(dspConfig);
				feedsFDN.store(true, std::memory_order_release);
			}
			if ((!source.feedsFDN || dspConfig->GetLateReverbModel() != LateReverbModel::fdn) && mReverbSendSource)
			{
				feedsFDN.store(false, std::memory_order_release);
				RemoveReverbSendSource();
			}

			UpdateImageSourceDataMap(imageSourceData);
			UpdateImageSources(dspConfig);
			FreeAccess();
		}

		////////////////////////////////////////

		std::optional<Source::Data> Source::GetData(ThreadID id)
		{
			if (!GetAccess())
				return std::nullopt;
#ifdef __ANDROID__
			if (!std::atomic_load(&transform)) // Check if the source position has been updated before using
			{
				FreeAccess();
				return std::nullopt;
			}
#else
			if (!transform.load(std::memory_order_acquire)) // Check if the source position has been updated before using
			{
				FreeAccess();
				return std::nullopt;
			}
#endif
			lock_guard<std::mutex>lock(*dataMutex);
			Data data(-1, currentPosition, currentOrientation, mDirectivity.load(std::memory_order_acquire), updateFlags.HasChanged(id));
			FreeAccess();
			return data;
		}

		////////////////////////////////////////

		void Source::InitBuffers(int numFrames)
		{
			bInput = CMonoBuffer<float>(numFrames);
			bOutput.left = CMonoBuffer<float>(numFrames);
			bOutput.right = CMonoBuffer<float>(numFrames);
			bMonoOutput = CMonoBuffer<float>(numFrames);
		}

		////////////////////////////////////////

		void Source::ClearBuffers()
		{
			frequencyBands = Matrix<>();
			octaveBandFilter.ClearBuffers();
			bInput.clear();
			bOutput.left.clear();
			bOutput.right.clear();
			bMonoOutput.clear();
		}

		////////////////////////////////////////

		void Source::ClearPointers()
		{
			directivityFilter.reset();
			reverbInputFilter.reset();
			mAirAbsorption.reset();
#ifdef __ANDROID__
			std::atomic_load(&transform).reset();
#else
			transform.load(std::memory_order_acquire).reset();
#endif
		}

		////////////////////////////////////////

		void Source::UpdateTransform(const Vec3& position, const Vec4& orientation)
		{
			std::shared_ptr<CTransform> transformCopy = std::make_shared<CTransform>();
			transformCopy->SetOrientation(CQuaternion(static_cast<float>(orientation.w()), static_cast<float>(orientation.x()), static_cast<float>(orientation.y()), static_cast<float>(orientation.z())));
			transformCopy->SetPosition(CVector3(static_cast<float>(position.x()), static_cast<float>(position.y()), static_cast<float>(position.z())));
#ifdef __ANDROID__
			std::atomic_store(&transform, transformCopy);
#else
			transform.store(transformCopy, std::memory_order_release);
#endif
			releasePool.Add(transformCopy);
		}

		////////////////////////////////////////

		void Source::UpdateImageSourceDataMap(ImageSourceDataMap& imageSourceData)
		{
			for (auto& [key, vSource] : currentImageSources)
			{
				auto it = imageSourceData.find(key);
				if (it == imageSourceData.end()) // case: old vSource
					vSource.second->Invisible();
			}

			for (auto& [key, vSource] : imageSourceData)
			{
				auto it = currentImageSources.find(key);
				if (it == currentImageSources.end()) // case: add new vSource
				{
					lock_guard<std::mutex>lock(*imageSourcesMutex);
					if (!GetAccess())
						return;
					currentImageSources.emplace(key, std::pair<int, std::shared_ptr<ImageSourceData>>(-1, std::make_shared<ImageSourceData>(*vSource)));
					FreeAccess();
				}
				else // case: update exist vSource
					it->second.second.swap(vSource);
			}
		}

		////////////////////////////////////////

		void Source::UpdateImageSources(const std::shared_ptr<DSPConfig>& config)
		{
			std::vector<std::string> keys;
			for (auto& [key, vSource] : currentImageSources)
			{
				if (UpdateImageSource(vSource.first, vSource.second, config))
					keys.push_back(key);
			}

			lock_guard<std::mutex>lock(*imageSourcesMutex);
			if (!GetAccess())
				return;
			for (const std::string& key : keys)
				currentImageSources.erase(key);
			FreeAccess();
		}
		
		////////////////////////////////////////

		bool Source::UpdateImageSource(int& id, std::shared_ptr<ImageSourceData>& data, const std::shared_ptr<DSPConfig>& dspConfig)
		{
			if (id < 0)		// case: virtual source does not exist
			{
				int fdnChannel = -1;
				if (data->IsFeedingFDN())
					fdnChannel = AssignFDNChannel(dspConfig->GetData().fdnSize);

				id = imageSources.NextID();
				if (id < 0)		// No free slots
					return false;

				imageSources.at(id).Init(&inputBuffer, dspConfig, data, fdnChannel);
			}
			else
			{
				int fdnChannel = -1;
				if (data->IsFeedingFDN() && imageSources.at(id).GetFDNChannel() < 0)
					fdnChannel = AssignFDNChannel(dspConfig->GetData().fdnSize);

				bool remove = imageSources.at(id).Update(*data, fdnChannel);

				assert(!(data->IsFeedingFDN() && imageSources.at(id).GetFDNChannel() == -1));

				if (fdnChannel >= 0) // Add vSource old fdnChannel to freeFDNChannels (Also prevents leaking FDN channels if !data->visible and the channel is not assigned to vSource)
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

		int Source::AssignFDNChannel(const int fdnSize)
		{
			if (freeFDNChannels.back() >= fdnSize)
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