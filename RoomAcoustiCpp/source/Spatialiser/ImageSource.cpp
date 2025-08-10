/*
* @class ImageSource, ImageSourceData
*
* @brief Declaration of ImageSource and ImageSourceData classes
*
* @remarks Currently, it can only handle one diffracting edge per path/image source
*
*/

//Common headers
#include "Common/RACProfiler.h"

// Spatialiser headers
#include "Spatialiser/ImageSource.h"
#include "Spatialiser/Globals.h"

// Unity headers
#include "Unity/Debug.h"

// DSP headers
#include "DSP/Interpolate.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	using namespace DSP;
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// ImageSourceData class ////////////////////

		////////////////////////////////////////

		void ImageSourceData::CreateKey(int sourceID)
		{
			AddSourceIDToKey(sourceID);
			for (const auto& part : pathParts)
			{
				if (part.isReflection)
					AddPlaneIDToKey(part.id);
				else
					AddEdgeIDToKey(part.id);
			}
		}

		////////////////////////////////////////

		void ImageSourceData::AddEdgeID(size_t id)
		{
			pathParts.back().id = id;
			pathParts.back().isReflection = false;
			diffraction = true;
			diffractionIndex = static_cast<int>(pathParts.size()) - 1;
		}

		////////////////////////////////////////

		void ImageSourceData::AddSourceIDToKey(int sourceID)
		{
			key = "";
			auto [ptr, ec] = std::to_chars(idKey.data(), idKey.data() + idKey.size(), sourceID);
			if (ec == std::errc()) // Null-terminate manually if conversion is successful
				*ptr = '\0';
			else// Failed to convert id to char array
			{
				key += sourceKey[0];
				return;
			}

			key += idKey.data();
			key += sourceKey[0];
		}

		////////////////////////////////////////

		void ImageSourceData::AddPlaneIDToKey(const size_t id)
		{
			auto [ptr, ec] = std::to_chars(idKey.data(), idKey.data() + idKey.size(), id);
			if (ec == std::errc()) // Null-terminate manually if conversion is successful
				*ptr = '\0';
			else// Failed to convert id to char array
			{
				key += reflectionKey[0];
				return;
			}

			key += idKey.data();
			key += reflectionKey[0];
		}

		////////////////////////////////////////

		void ImageSourceData::AddEdgeIDToKey(const size_t id)
		{
			auto [ptr, ec] = std::to_chars(idKey.data(), idKey.data() + idKey.size(), id);
			if (ec == std::errc()) // Null-terminate manually if conversion is successful
				*ptr = '\0';
			else// Failed to convert id to char array
			{
				key += diffractionKey[0];
				return;
			}
			key += idKey.data();
			key += diffractionKey[0];
		}

		////////////////////////////////////////

		void ImageSourceData::SetTransform(const Vec3& position)
		{
			transform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));
			mPositions.back() = position;
		}

		////////////////////////////////////////

		void ImageSourceData::SetTransform(const Vec3& position, const Vec3& rotatedEdgePosition)
		{
			transform.SetPosition(CVector3(static_cast<float>(rotatedEdgePosition.x), static_cast<float>(rotatedEdgePosition.y), static_cast<float>(rotatedEdgePosition.z)));
			mPositions.back() = position;
		}

		////////////////////////////////////////

		void ImageSourceData::SetDistance(const Vec3& listenerPosition)
		{
			if (diffraction)
				distance = mDiffractionPath.rData.d + mDiffractionPath.sData.d;
			else
				distance = (listenerPosition - GetPosition()).Length();
		}

		////////////////////////////////////////

		Vec3 ImageSourceData::GetPosition(int i) const
		{
			if (diffraction)
			{
				if (i >= diffractionIndex)
				{
					assert(i < mEdges.size());
					return mEdges[i].GetEdgeCoordinate(mDiffractionPath.GetApexZ());
				}
			}
			assert(i < mPositions.size());
			return mPositions[i];
		}

		////////////////////////////////////////

		void ImageSourceData::Clear()
		{
			Reset();
			reflection = false;
			diffraction = false;
			key.clear();
		}

		////////////////////////////////////////

		void ImageSourceData::IncreaseImageSourceOrder()
		{
			pathParts.emplace_back(0, true);
			mPositions.emplace_back(Vec3());
			mEdges.emplace_back(Vec3(), Vec3());
		}

		////////////////////////////////////////

		void ImageSourceData::Update(const ImageSourceData& imageSource)
		{
			for (int i = 0; i < imageSource.pathParts.size(); i++)
			{
				pathParts[i] = imageSource.pathParts[i];
				mPositions[i] = imageSource.mPositions[i];
			}
			reflection = imageSource.reflection;
			diffraction = imageSource.diffraction;
			if (diffraction)
			{
				for (int i = 0; i < imageSource.mEdges.size(); i++)
					mEdges[i] = imageSource.mEdges[i];
				diffractionIndex = imageSource.diffractionIndex;
				mDiffractionPath = imageSource.mDiffractionPath;
			}
			key.clear();
		}

		//////////////////// ImageSource class ////////////////////

		ReleasePool ImageSource::releasePool;		

		////////////////////////////////////////

		void ImageSource::InitSource()
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			mSource = mCore->CreateSingleSourceDSP();
			mSource->EnablePropagationDelay();
			mSource->DisableFarDistanceEffect();
			mSource->DisableNearFieldEffect();
			SetSpatialisationMode(spatialisationMode.load(std::memory_order_acquire));
		}

		////////////////////////////////////////

		void ImageSource::RemoveSource()
		{
			currentImpulseResponseMode = false;
			currentSpatialisationMode = SpatialisationMode::none;

			unique_lock<shared_mutex> lock(tuneInMutex);
			mCore->RemoveSingleSourceDSP(mSource);
			mSource.reset();
		}

		////////////////////////////////////////

		void ImageSource::Init(const Buffer<>* sourceBuffer, const std::shared_ptr<Config>& config, const ImageSourceData& data, int fdnChannel)
		{
			InitSource();
			InitBuffers(config->numFrames);

			inputBuffer = sourceBuffer;
			mFilter = make_unique<GraphicEQ>(data.GetAbsorption(), config->frequencyBands, config->Q, config->fs);
			mAirAbsorption = make_unique<AirAbsorption>(data.GetDistance(), config->fs);

			diffraction = data.IsDiffraction();
			reflection = data.IsReflection();

			InitDiffractionModel(config->GetDiffractionModel(), data.GetDiffractionPath(), config->fs);

			feedsFDN.store(data.IsFeedingFDN(), std::memory_order_release);
			mFDNChannel.store(fdnChannel, std::memory_order_release);

			UpdateTransform(data.GetTransform());

			if (data.IsVisible())
				gain.SetTarget(1.0);

			AllowAccess();
			isReset.store(false, std::memory_order_release);
			LateInit(config);
		}

		////////////////////////////////////////

		void ImageSource::LateInit(const std::shared_ptr<Config>& config)
		{
			DiffractionModel model = config->GetDiffractionModel();
			UpdateDiffractionModel(model, config->fs);
			while (model != config->GetDiffractionModel())
			{
				model = config->GetDiffractionModel();
				UpdateDiffractionModel(model, config->fs);
			}
		}

		////////////////////////////////////////

		bool ImageSource::Update(const ImageSourceData& data, int& fdnChannel)
		{
			if (data.IsVisible())
			{
				gain.SetTarget(1.0);
				UpdateParameters(data, fdnChannel);
			}
			else
			{
				gain.SetTarget(0.0);
				if (gain.IsZero())
				{
					Remove();
					return true;
				}
			}
			return false;
		}

		////////////////////////////////////////

		void ImageSource::UpdateParameters(const ImageSourceData& data, int& fdnChannel)
		{
			mFilter->SetTargetGains(data.GetAbsorption());

			if (diffraction)
			{
				mDiffractionPath = data.GetDiffractionPath();
				activeModel->SetTargetParameters(mDiffractionPath);
#ifdef __ANDROID__
				std::shared_ptr<Diffraction::Model> temp = std::atomic_load(&incomingModel);
#else
				std::shared_ptr<Diffraction::Model> temp = incomingModel.load(std::memory_order_acquire);
#endif
				if (temp)
					temp->SetTargetParameters(mDiffractionPath);
#ifdef __ANDROID__
				temp = std::atomic_load(&nextModel);
#else
				temp = nextModel.load(std::memory_order_acquire);
#endif
				if (temp)
					temp->SetTargetParameters(mDiffractionPath);
				if (isCrossFading.load(std::memory_order_acquire))
				{
					temp = fadeModel;
					if (temp)
						temp->SetTargetParameters(mDiffractionPath);
				}
			}

			mAirAbsorption->SetTargetDistance(data.GetDistance());

			if (feedsFDN.load(std::memory_order_acquire) != data.IsFeedingFDN())
			{
				feedsFDN.store(data.IsFeedingFDN(), std::memory_order_release);
				fdnChannel = mFDNChannel.exchange(fdnChannel, std::memory_order_acq_rel);
			}

			UpdateTransform(data.GetTransform());
		}

		////////////////////////////////////////

		void ImageSource::Reset()
		{
			if (isReset.load(std::memory_order_acquire))
				return;
			if (!CanEdit())
				return;
			if (!mSource)
				return;
			ClearBuffers();
			RemoveSource();
			ClearPointers();
			isReset.store(true, std::memory_order_release);
		}
		
		////////////////////////////////////////

		void ImageSource::InitBuffers(int numFrames)
		{
			bInput = CMonoBuffer<float>(numFrames);
			bOutput.left = CMonoBuffer<float>(numFrames);
			bOutput.right = CMonoBuffer<float>(numFrames);
			bMonoOutput = CMonoBuffer<float>(numFrames);
			bStore = Buffer(numFrames);
			bDiffStore = Buffer(numFrames);
		}

		////////////////////////////////////////

		void ImageSource::ClearBuffers()
		{
			bInput.clear();
			bOutput.left.clear();
			bOutput.right.clear();
			bMonoOutput.clear();
			bStore.ResizeBuffer(1);
			bDiffStore.ResizeBuffer(1);
		}

		////////////////////////////////////////

		void ImageSource::ClearPointers()
		{
			mFilter.reset();
			mAirAbsorption.reset();
			activeModel.reset();
			fadeModel.reset();
#ifdef __ANDROID__
			std::atomic_load(&incomingModel).reset();
			std::atomic_load(&nextModel).reset();
			std::atomic_load(&transform).reset();
#else
			incomingModel.load(std::memory_order_acquire).reset();
			nextModel.load(std::memory_order_acquire).reset();
			transform.load(std::memory_order_acquire).reset();
#endif
		}

		////////////////////////////////////////

		void ImageSource::UpdateTransform(const CTransform& newTransform)
		{
			std::shared_ptr<CTransform> transformCopy = std::make_shared<CTransform>(newTransform);
#ifdef __ANDROID__
			std::atomic_store(&transform, transformCopy);
#else
			transform.store(transformCopy, std::memory_order_release);
#endif
			releasePool.Add(transformCopy);
		}

		////////////////////////////////////////

		void ImageSource::SetSpatialisationMode(const SpatialisationMode mode)
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

		void ImageSource::SetImpulseResponseMode(const bool mode)
		{
			if (mode)
			{
				mSource->DisableDistanceAttenuationSmoothingAnechoic();
				mSource->DisableInterpolation();
			}
			else
			{
				mSource->EnableDistanceAttenuationSmoothingAnechoic();
				mSource->EnableInterpolation();
			}
			currentImpulseResponseMode = mode;
		}

		////////////////////////////////////////

		void ImageSource::InitDiffractionModel(const DiffractionModel model, const Diffraction::Path& path, const int fs)
		{
			diffractionGain.Reset(1.0);
			isCrossFading.store(false, std::memory_order_release);
			
			if (!diffraction)
			{
				activeModel.reset();
				fadeModel.reset();
#ifdef __ANDROID__
				std::atomic_load(&incomingModel).reset();
				std::atomic_load(&nextModel).reset();
#else
				incomingModel.load(std::memory_order_acquire).reset();
				nextModel.load(std::memory_order_acquire).reset();
#endif
				return;
			}

			currentDiffractionModel.store(model, std::memory_order_release);
			mDiffractionPath = path;
			switch (model)
			{
			case DiffractionModel::attenuate:
			{
				activeModel = std::make_shared<Diffraction::Attenuate>(path);
				break;
			}
			case DiffractionModel::lowPass:
			{
				activeModel = std::make_shared<Diffraction::LPF>(path, fs);
				break;
			}
			case DiffractionModel::udfa:
			{
				activeModel = std::make_shared<Diffraction::UDFA>(path, fs);
				break;
			}
			case DiffractionModel::udfai:
			{
				activeModel = std::make_shared<Diffraction::UDFAI>(path, fs);
				break;
			}
			case DiffractionModel::nnSmall:
			{
				activeModel = std::make_shared<Diffraction::NNSmall>(path);
				break;
			}
			case DiffractionModel::nnBest:
			{
				activeModel = std::make_shared<Diffraction::NNBest>(path);
				break;
			}
			case DiffractionModel::utd:
			{
				activeModel = std::make_shared<Diffraction::UTD>(path, fs);
				break;
			}
			case DiffractionModel::btm:
			{
				activeModel = std::make_shared<Diffraction::BTM>(path, fs);
				break;
			}
			}
			releasePool.Add(activeModel);
		}

		////////////////////////////////////////

		void ImageSource::UpdateDiffractionModel(const DiffractionModel model, const int fs)
		{
			if (!GetAccess())
				return;

			if (!diffraction)
			{
				FreeAccess();
				return;
			}

			if (!diffraction ||currentDiffractionModel.exchange(model, std::memory_order_acq_rel) == model)
			{
				FreeAccess();
				return;
			}

			switch (model)
			{
			case DiffractionModel::attenuate:
			{ 
#ifdef __ANDROID__
				std::atomic_store(&nextModel, std::static_pointer_cast<Diffraction::Model>(std::make_shared<Diffraction::Attenuate>(mDiffractionPath)));
#else
				nextModel.store(std::make_shared<Diffraction::Attenuate>(mDiffractionPath), std::memory_order_release);
#endif
				break;
			}
			case DiffractionModel::lowPass:
			{
#ifdef __ANDROID__
				std::atomic_store(&nextModel, std::static_pointer_cast<Diffraction::Model>(std::make_shared<Diffraction::LPF>(mDiffractionPath, fs)));
#else
				nextModel.store(std::make_shared<Diffraction::LPF>(mDiffractionPath, fs), std::memory_order_release);
#endif
				break;
			}
			case DiffractionModel::udfa:
			{
#ifdef __ANDROID__
				std::atomic_store(&nextModel, std::static_pointer_cast<Diffraction::Model>(std::make_shared<Diffraction::UDFA>(mDiffractionPath, fs)));
#else
				nextModel.store(std::make_shared<Diffraction::UDFA>(mDiffractionPath, fs), std::memory_order_release);
#endif
				break;
			}
			case DiffractionModel::udfai:
			{
#ifdef __ANDROID__
				std::atomic_store(&nextModel, std::static_pointer_cast<Diffraction::Model>(std::make_shared<Diffraction::UDFAI>(mDiffractionPath, fs)));
#else
				nextModel.store(std::make_shared<Diffraction::UDFAI>(mDiffractionPath, fs), std::memory_order_release);
#endif
				break;
			}
			case DiffractionModel::nnSmall:
			{
#ifdef __ANDROID__
				std::atomic_store(&nextModel, std::static_pointer_cast<Diffraction::Model>(std::make_shared<Diffraction::NNSmall>(mDiffractionPath)));
#else
				nextModel.store(std::make_shared<Diffraction::NNSmall>(mDiffractionPath), std::memory_order_release);
#endif
				break;
			}
			case DiffractionModel::nnBest:
			{
#ifdef __ANDROID__
				std::atomic_store(&nextModel, std::static_pointer_cast<Diffraction::Model>(std::make_shared<Diffraction::NNBest>(mDiffractionPath)));
#else
				nextModel.store(std::make_shared<Diffraction::NNBest>(mDiffractionPath), std::memory_order_release);
#endif
				break;
			}
			case DiffractionModel::utd:
			{
#ifdef __ANDROID__
				std::atomic_store(&nextModel, std::static_pointer_cast<Diffraction::Model>(std::make_shared<Diffraction::UTD>(mDiffractionPath, fs)));
#else
				nextModel.store(std::make_shared<Diffraction::UTD>(mDiffractionPath, fs), std::memory_order_release);
#endif
				break;
			}
			case DiffractionModel::btm:
			{
#ifdef __ANDROID__
				std::atomic_store(&nextModel, std::static_pointer_cast<Diffraction::Model>(std::make_shared<Diffraction::BTM>(mDiffractionPath, fs)));
#else
				nextModel.store(std::make_shared<Diffraction::BTM>(mDiffractionPath, fs), std::memory_order_release);
#endif
				break;
			}
			}
#ifdef __ANDROID__
			releasePool.Add(std::atomic_load(&nextModel));
#else
			releasePool.Add(nextModel.load(std::memory_order_acquire));
#endif
			if (!isCrossFading.load(std::memory_order_acquire))
			{
#ifdef __ANDROID__
				std::atomic_exchange(&incomingModel, std::shared_ptr<Diffraction::Model>{});
#else
				incomingModel = nextModel.exchange(nullptr, std::memory_order_acq_rel);
#endif
				isCrossFading.store(true, std::memory_order_release);
			}
			FreeAccess();
		}		

		////////////////////////////////////////

		void ImageSource::ProcessAudio(Buffer<>& outputBuffer, Matrix& reverbInput, const Real lerpFactor)
		{
			if (!GetAccess())
				return;

#ifdef __ANDROID__
			if (!std::atomic_load(&transform))  // Check if the source position has been updated before using
			{
				FreeAccess();
				return;
			}
#else
			if (!transform.load(std::memory_order_acquire))  // Check if the source position has been updated before using
			{
				FreeAccess();
				return;
			}
#endif
			PROFILE_ImageSource
			if (gain.IsZero())
			{
				FreeAccess();
				return;
			}

			if (bool mode = impulseResponseMode.load(std::memory_order_acquire); mode != currentImpulseResponseMode)
				SetImpulseResponseMode(mode);

			if (SpatialisationMode mode = spatialisationMode.load(std::memory_order_acquire); mode != currentSpatialisationMode)
				SetSpatialisationMode(mode);

			const int numFrames = inputBuffer->Length();

			{
				PROFILE_Reflection
				mFilter->ProcessAudio(*inputBuffer, bStore, lerpFactor);
			}

			if (diffraction)
				ProcessDiffraction(bStore, bStore, lerpFactor);

			mAirAbsorption->ProcessAudio(bStore, bStore, lerpFactor);

			for (int i = 0; i < numFrames; i++)
				bInput[i] = static_cast<float>(bStore[i] * gain.Use(lerpFactor));

			{
				PROFILE_Spatialisation
				shared_lock<shared_mutex> lock(tuneInMutex);
#ifdef __ANDROID__
				mSource->SetSourceTransform(*std::atomic_load(&transform));
#else
				mSource->SetSourceTransform(*transform.load(std::memory_order_acquire));
#endif
				mSource->SetBuffer(bInput);
			
				int fdnChannel = mFDNChannel.load(std::memory_order_acquire);
				if (fdnChannel > -1)
				{
					mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
					std::transform(bMonoOutput.begin(), bMonoOutput.begin() + numFrames, reverbInput[fdnChannel].begin(), reverbInput[fdnChannel].begin(),
						[](auto input, auto current) { return current + input; });
				}
				else
					mSource->ProcessAnechoic(bOutput.left, bOutput.right);
			}
			int j = 0;
			for (int i = 0; i < numFrames; i++)
			{
				outputBuffer[j++] += static_cast<Real>(bOutput.left[i]);
				outputBuffer[j++] += static_cast<Real>(bOutput.right[i]);
			}

			FreeAccess();
			return;
		}

		////////////////////////////////////////

		void ImageSource::ProcessDiffraction(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor)
		{
			PROFILE_Diffraction
			activeModel->ProcessAudio(inBuffer, outBuffer, lerpFactor);
			
			if (!isCrossFading.load(std::memory_order_acquire))
				return;

#ifdef __ANDROID__
			if (auto newModel = std::atomic_exchange(&incomingModel, std::shared_ptr<Diffraction::Model>{}))
				fadeModel = newModel;
#else
			if (auto newModel = incomingModel.exchange(nullptr, std::memory_order_acq_rel))
				fadeModel = newModel;
#endif
			fadeModel->ProcessAudio(inBuffer, bDiffStore, lerpFactor);

			Real factor = 0.0;
			for (int i = 0; i < inBuffer.Length(); i++)
			{
				factor = diffractionGain.Use(0.03);	// Interpolation from one to zero takes 454 samples at 48kHz
				outBuffer[i] *= 1.0f - factor;
				outBuffer[i] += bDiffStore[i] * factor;
			}

			if (diffractionGain.IsZero())
			{
				diffractionGain.Reset(1.0);

				fadeModel.swap(activeModel);

#ifdef __ANDROID__
				if (auto queued = std::atomic_exchange(&nextModel, std::shared_ptr<Diffraction::Model>{}))
					std::atomic_store(&incomingModel, queued);
#else
				if (auto queued = nextModel.exchange(nullptr, std::memory_order_acq_rel))
					incomingModel.store(queued, std::memory_order_release);
#endif
				else
					isCrossFading.store(false, std::memory_order_release);
			}
		}
	}
}