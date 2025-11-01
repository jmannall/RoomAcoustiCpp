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

		std::shared_ptr<ImageSourceData> ImageSourceData::CreateShallowCopy() const
		{
			// TODO: only copy data we need
			return std::make_shared<ImageSourceData>(*this);
		}

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
			pathParts.back().id = static_cast<partid_t>(id);
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
			transform.SetPosition(CVector3(static_cast<float>(position.x()), static_cast<float>(position.y()), static_cast<float>(position.z())));
			mPositions.back() = position;
		}

		////////////////////////////////////////

		void ImageSourceData::SetTransform(const Vec3& position, const Vec3& rotatedEdgePosition)
		{
			transform.SetPosition(CVector3(static_cast<float>(rotatedEdgePosition.x()), static_cast<float>(rotatedEdgePosition.y()), static_cast<float>(rotatedEdgePosition.z())));
			mPositions.back() = position;
		}

		////////////////////////////////////////

		void ImageSourceData::SetDistance(const Vec3& listenerPosition)
		{
			if (diffraction)
				distance = mDiffractionPath.rData.d + mDiffractionPath.sData.d;
			else
				distance = (listenerPosition - GetPosition()).Normal();
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
			assert(pathParts.size() >= imageSource.pathParts.size());
			for (int i = 0; i < imageSource.pathParts.size(); i++)
			{
				pathParts[i] = imageSource.pathParts[i];
				mPositions[i] = imageSource.mPositions[i];
			}

			reflection = imageSource.reflection;
			diffraction = imageSource.diffraction;
			if (diffraction)
			{
				assert(mEdges.size() >= imageSource.mEdges.size());
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

		void ImageSource::InitSource(const std::shared_ptr<DSPConfig>& dspConfig)
		{
			unique_lock<shared_mutex> lock(tuneInMutex);
			mSource = mCore->CreateSingleSourceDSP();
			mSource->EnablePropagationDelay();
			mSource->DisableFarDistanceEffect();
			mSource->DisableNearFieldEffect();
			SetSpatialisationMode(dspConfig->GetSpatialisationMode());
			SetImpulseResponseMode(dspConfig->GetImpulseResponseMode());
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

		void ImageSource::Init(const Buffer<>* sourceBuffer, const std::shared_ptr<DSPConfig>& dspConfig, const std::shared_ptr<ImageSourceData>& data, int fdnChannel)
		{
			InitSource(dspConfig);
			const DSPData& dspData = dspConfig->GetData();
			InitBuffers(dspData.numFrames);

			inputBuffer = sourceBuffer;
			mFilter = make_unique<GraphicEQ<>>(data->GetAbsorption(), dspData.frequencyBands, dspData.Q, dspData.fs);
			mAirAbsorption = make_unique<AirAbsorption>(data->GetDistance(), dspConfig->GetData().fs);

			diffraction = data->IsDiffraction();
			reflection = data->IsReflection();

			if (diffraction)
				InitDiffractionModel(dspConfig->GetDiffractionModel(), data->GetDiffractionPath(), dspData.fs);

			feedsFDN.store(data->IsFeedingFDN(), std::memory_order_release);
			mFDNChannel.store(fdnChannel, std::memory_order_release);

			UpdateTransform(data->GetTransform());

			if (data->IsVisible())
				gain.SetTarget((Real)1.0);

			AllowAccess();
			isReset.store(false, std::memory_order_release);
			LateInit(dspConfig);
		}

		////////////////////////////////////////

		void ImageSource::LateInit(const std::shared_ptr<DSPConfig>& dspConfig)
		{
			DiffractionModel model = dspConfig->GetDiffractionModel();
			UpdateDiffractionModel(model, dspConfig->GetData().fs);
			while (model != dspConfig->GetDiffractionModel())
			{
				model = dspConfig->GetDiffractionModel();
				UpdateDiffractionModel(model, dspConfig->GetData().fs);
			}
		}

		////////////////////////////////////////

		bool ImageSource::Update(const ImageSourceData& data, int& fdnChannel)
		{
			if (data.IsVisible())
			{
				gain.SetTarget((Real)1.0);
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
				std::shared_ptr<Diffraction::Model> temp = incomingModel.load(std::memory_order_acquire);
				if (temp)
					temp->SetTargetParameters(mDiffractionPath);
				temp = nextModel.load(std::memory_order_acquire);
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
			if (!mSource) // TODO: Is this check necessary?
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
		}

		////////////////////////////////////////

		void ImageSource::ClearBuffers()
		{
			bInput.clear();
			bOutput.left.clear();
			bOutput.right.clear();
			bMonoOutput.clear();
		}

		////////////////////////////////////////

		void ImageSource::ClearPointers()
		{
			mFilter.reset();
			mAirAbsorption.reset();
			activeModel.reset();
			fadeModel.reset();
			incomingModel.load(std::memory_order_acquire).reset();
			nextModel.load(std::memory_order_acquire).reset();
			transform.load(std::memory_order_acquire).reset();
		}

		////////////////////////////////////////

		void ImageSource::UpdateTransform(const CTransform& newTransform)
		{
			std::shared_ptr<CTransform> transformCopy = std::make_shared<CTransform>(newTransform);
			transform.store(transformCopy, std::memory_order_release);
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
			currentSpatialisationMode = mode;
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
			assert(diffraction);

			diffractionGain.Reset((Real)1.0);
			isCrossFading.store(false, std::memory_order_release);

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

			if (currentDiffractionModel.exchange(model, std::memory_order_acq_rel) == model)
			{
				FreeAccess();
				return;
			}

			switch (model)
			{
			case DiffractionModel::attenuate:
			{ 
				nextModel.store(std::make_shared<Diffraction::Attenuate>(mDiffractionPath), std::memory_order_release);
				break;
			}
			case DiffractionModel::lowPass:
			{
				nextModel.store(std::make_shared<Diffraction::LPF>(mDiffractionPath, fs), std::memory_order_release);
				break;
			}
			case DiffractionModel::udfa:
			{
				nextModel.store(std::make_shared<Diffraction::UDFA>(mDiffractionPath, fs), std::memory_order_release);
				break;
			}
			case DiffractionModel::udfai:
			{
				nextModel.store(std::make_shared<Diffraction::UDFAI>(mDiffractionPath, fs), std::memory_order_release);
				break;
			}
			case DiffractionModel::nnSmall:
			{
				nextModel.store(std::make_shared<Diffraction::NNSmall>(mDiffractionPath), std::memory_order_release);
				break;
			}
			case DiffractionModel::nnBest:
			{
				nextModel.store(std::make_shared<Diffraction::NNBest>(mDiffractionPath), std::memory_order_release);
				break;
			}
			case DiffractionModel::utd:
			{
				nextModel.store(std::make_shared<Diffraction::UTD>(mDiffractionPath, fs), std::memory_order_release);
				break;
			}
			case DiffractionModel::btm:
			{
				nextModel.store(std::make_shared<Diffraction::BTM>(mDiffractionPath, fs), std::memory_order_release);
				break;
			}
			}
			releasePool.Add(nextModel.load(std::memory_order_acquire));
			if (!isCrossFading.load(std::memory_order_acquire))
			{
				incomingModel = nextModel.exchange(nullptr, std::memory_order_acq_rel);
				isCrossFading.store(true, std::memory_order_release);
			}
			FreeAccess();
		}		

		////////////////////////////////////////

		void ImageSource::ProcessAudio(Buffer<>& outputBuffer, const AudioData& audioData)
		{
			if (!GetAccess())
				return;

			if (!transform.load(std::memory_order_acquire))  // Check if the source position has been updated before using
			{
				FreeAccess();
				return;
			}

			PROFILE_ImageSource
			if (gain.IsZero())
			{
				FreeAccess();
				return;
			}

			if (audioData.impulseResponseMode != currentImpulseResponseMode)
				SetImpulseResponseMode(audioData.impulseResponseMode);

			if (audioData.spatialisationMode != currentSpatialisationMode)
				SetSpatialisationMode(audioData.spatialisationMode);

			const int numFrames = ToInt(inputBuffer->Length());

			{
				PROFILE_Reflection
				mFilter->ProcessAudio(*inputBuffer, bStore, audioData.lerpFactor);
			}

			if (diffraction)
				ProcessDiffraction(bStore, bStore, audioData.lerpFactor);

			mAirAbsorption->ProcessAudio(bStore, bStore, audioData.lerpFactor);

			for (int i = 0; i < numFrames; i++)
				bInput[i] = static_cast<float>(bStore[i] * gain.Use(audioData.lerpFactor));

			{
				PROFILE_Spatialisation
				shared_lock<shared_mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(*transform.load(std::memory_order_acquire));
				mSource->SetBuffer(bInput);
			
				if (audioData.lateReverbModel == LateReverbModel::fdn && mFDNChannel.load(std::memory_order_acquire) > -1)
					mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
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
		}

		////////////////////////////////////////

		void ImageSource::ProcessSingleFDNSend(Matrix<>& reverbInput, const Real lerpFactor)
		{
			if (!GetAccess())
				return;

			PROFILE_ImageSource
			const int numFrames = ToInt(inputBuffer->Length());

			int fdnChannel = mFDNChannel.load(std::memory_order_acquire);
			if (fdnChannel > -1)
			{
				for (int i = 0; i < numFrames; i++)
					reverbInput(fdnChannel, i) += static_cast<Real>(bMonoOutput[i]);
			}

			FreeAccess();
		}

		////////////////////////////////////////

		void ImageSource::ProcessDiffraction(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor)
		{
			PROFILE_Diffraction
			activeModel->ProcessAudio(inBuffer, outBuffer, lerpFactor);
			
			if (!isCrossFading.load(std::memory_order_acquire))
				return;

			if (auto newModel = incomingModel.exchange(nullptr, std::memory_order_acq_rel))
				fadeModel = newModel;

			fadeModel->ProcessAudio(inBuffer, bDiffStore, lerpFactor);

			Real factor = 0.0;
			for (int i = 0; i < inBuffer.Length(); i++)
			{
				factor = diffractionGain.Use((Real)0.03);	// Interpolation from one to zero takes 454 samples at 48kHz
				outBuffer[i] *= (Real)1.0 - factor;
				outBuffer[i] += bDiffStore[i] * factor;
			}

			if (diffractionGain.IsZero())
			{
				diffractionGain.Reset((Real)1.0);

				fadeModel.swap(activeModel);

				if (auto queued = nextModel.exchange(nullptr, std::memory_order_acq_rel))
					incomingModel.store(queued, std::memory_order_release);
				else
					isCrossFading.store(false, std::memory_order_release);
			}
		}
	}
}