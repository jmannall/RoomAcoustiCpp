/*
* @class ImageSource, ImageSourceData
*
* @brief Declaration of ImageSource and ImageSourceData classes
*
* @remarks Currently, it can only handle one diffracting edge per path/image source
*
*/

// Spatialiser headers
#include "Spatialiser/ImageSource.h"
#include "Spatialiser/Globals.h"

// Unity headers
#include "Unity/UnityInterface.h"
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

		void ImageSourceData::AddSourceID(const size_t id)
		{
			key = "";
			auto [ptr, ec] = std::to_chars(idKey.data(), idKey.data() + idKey.size(), id);
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

		void ImageSourceData::AddPlaneID(const size_t id)
		{
			pathParts.emplace_back(id, true);
			reflection = true;
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

		void ImageSourceData::AddEdgeID(const size_t id)
		{
			if (!diffraction)
				mEdges.clear();

			pathParts.emplace_back(id, false);
			diffraction = true;
			diffractionIndex = static_cast<int>(pathParts.size()) - 1;
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
			mPositions.emplace_back(position);
		}

		////////////////////////////////////////

		void ImageSourceData::SetTransform(const Vec3& position, const Vec3& rotatedEdgePosition)
		{
			transform.SetPosition(CVector3(static_cast<float>(rotatedEdgePosition.x), static_cast<float>(rotatedEdgePosition.y), static_cast<float>(rotatedEdgePosition.z)));
			mPositions.emplace_back(position);
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
					assert(i - diffractionIndex < mEdges.size());
					return mEdges[i - diffractionIndex].GetEdgeCoordinate(mDiffractionPath.GetApexZ());
				}
			}
			assert(i < mPositions.size());
			return mPositions[i];
		}

		////////////////////////////////////////

		void ImageSourceData::Clear(int sourceID)
		{
			Reset();
			pathParts.clear();
			mPositions.clear();
			mEdges.clear();
			reflection = false;
			diffraction = false;
			AddSourceID(sourceID);
		}

		////////////////////////////////////////

		void ImageSourceData::Update(const ImageSourceData& imageSource)
		{
			pathParts = imageSource.pathParts;
			mPositions = imageSource.mPositions;
			reflection = imageSource.reflection;
			diffraction = imageSource.diffraction;
			if (diffraction)
			{
				mEdges = imageSource.mEdges;
				diffractionIndex = imageSource.diffractionIndex;
				mDiffractionPath = imageSource.mDiffractionPath;
			}
			key = imageSource.key;
		}

		//////////////////// ImageSource class ////////////////////

		ReleasePool ImageSource::releasePool;		

		////////////////////////////////////////

		void ImageSource::InitSource()
		{
#ifdef DEBUG_IMAGE_SOURCE
			Debug::Log("Init virtual source", Colour::Green);
#endif
			unique_lock<shared_mutex> lock(tuneInMutex);
			mSource = mCore->CreateSingleSourceDSP();
			mSource->EnablePropagationDelay();
			mSource->DisableFarDistanceEffect();
			mSource->DisableNearFieldEffect();
		}

		////////////////////////////////////////

		void ImageSource::RemoveSource()
		{
#ifdef DEBUG_IMAGE_SOURCE
			Debug::Log("Remove virtual source", Colour::Red);
#endif
			currentImpulseResponseMode = false;
			currentSpatialisationMode = SpatialisationMode::none;

			unique_lock<shared_mutex> lock(tuneInMutex);
			mCore->RemoveSingleSourceDSP(mSource);
			mSource.reset();
		}

		////////////////////////////////////////

		void ImageSource::Init(const Buffer* sourceBuffer, const std::shared_ptr<Config>& config, const ImageSourceData& data, int fdnChannel)
		{
			if (mSource == nullptr)
			{
				InitBuffers(config->numFrames);
				InitSource();
			}

			inputBuffer = sourceBuffer;
			mFilter = make_unique<GraphicEQ>(data.GetAbsorption(), config->frequencyBands, config->Q, config->fs);
			mAirAbsorption = make_unique<AirAbsorption>(data.GetDistance(), config->fs);

			diffraction = data.IsDiffraction();
			reflection = data.IsReflection();

			InitDiffractionModel(config->GetDiffractionModel(), data.GetDiffractionPath(), config->fs);

			feedsFDN.store(data.IsFeedingFDN());
			mFDNChannel.store(fdnChannel);

			UpdateTransform(data.GetTransform());

			if (data.IsVisible())
				gain.SetTarget(1.0);

			AllowAccess();
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
				std::shared_ptr<Diffraction::Model> temp = incomingModel.load();
				if (temp)
					temp->SetTargetParameters(mDiffractionPath);
				temp = nextModel.load();
				if (temp)
					temp->SetTargetParameters(mDiffractionPath);
				if (isCrossFading.load())
				{
					temp = fadeModel;
					if (temp)
						temp->SetTargetParameters(mDiffractionPath);
				}
			}

			mAirAbsorption->SetTargetDistance(data.GetDistance());

			if (feedsFDN.load() != data.IsFeedingFDN())
			{
				feedsFDN.store(data.IsFeedingFDN());
				fdnChannel = mFDNChannel.exchange(fdnChannel);
			}

			UpdateTransform(data.GetTransform());
		}

		////////////////////////////////////////

		void ImageSource::Reset()
		{
			if (!CanEdit())
				return;
			if (mSource == nullptr)
				return;
			ClearBuffers();
			RemoveSource();
			ClearPointers();
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
			incomingModel.load().reset();
			nextModel.load().reset();
			transform.load().reset();
		}

		////////////////////////////////////////

		void ImageSource::UpdateTransform(const CTransform& newTransform)
		{
			std::shared_ptr<CTransform> transformCopy = std::make_shared<CTransform>(newTransform);
			transform.store(transformCopy);
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
			isCrossFading.store(false);
			
			if (!diffraction)
			{
				activeModel.reset();
				fadeModel.reset();
				incomingModel.load().reset();
				nextModel.load().reset();
				return;
			}

			currentDiffractionModel.store(model);
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

			if (!diffraction ||currentDiffractionModel.exchange(model) == model)
			{
				FreeAccess();
				return;
			}

			switch (model)
			{
			case DiffractionModel::attenuate:
			{ 
				nextModel.store(std::make_shared<Diffraction::Attenuate>(mDiffractionPath));
				break;
			}
			case DiffractionModel::lowPass:
			{
				nextModel.store(std::make_shared<Diffraction::LPF>(mDiffractionPath, fs));
				break;
			}
			case DiffractionModel::udfa:
			{
				nextModel.store(std::make_shared<Diffraction::UDFA>(mDiffractionPath, fs));
				break;
			}
			case DiffractionModel::udfai:
			{
				nextModel.store(std::make_shared<Diffraction::UDFAI>(mDiffractionPath, fs));
				break;
			}
			case DiffractionModel::nnSmall:
			{
				nextModel.store(std::make_shared<Diffraction::NNSmall>(mDiffractionPath));
				break;
			}
			case DiffractionModel::nnBest:
			{
				nextModel.store(std::make_shared<Diffraction::NNBest>(mDiffractionPath));
				break;
			}
			case DiffractionModel::utd:
			{
				nextModel.store(std::make_shared<Diffraction::UTD>(mDiffractionPath, fs));
				break;
			}
			case DiffractionModel::btm:
			{
				nextModel.store(std::make_shared<Diffraction::BTM>(mDiffractionPath, fs));
				break;
			}
			}
			releasePool.Add(nextModel.load());
			if (!isCrossFading.load())
			{
				incomingModel = nextModel.exchange(nullptr);
				isCrossFading.store(true);
			}
			FreeAccess();
		}		

		////////////////////////////////////////

		void ImageSource::ProcessAudio(Buffer& outputBuffer, Matrix& reverbInput, const Real lerpFactor)
		{
			if (!GetAccess())
				return;

			if (gain.IsZero())
			{
				FreeAccess();
				return;
			}

			if (currentImpulseResponseMode != impulseResponseMode.load())
				SetImpulseResponseMode(impulseResponseMode.load());

			if (currentSpatialisationMode != spatialisationMode.load())
				SetSpatialisationMode(spatialisationMode.load());

			const int numFrames = inputBuffer->Length();

#ifdef PROFILE_AUDIO_THREAD
			BeginVirtualSource();
#endif
#ifdef PROFILE_AUDIO_THREAD
			BeginReflection();	// Always process as also includes directivity
#endif
			mFilter->ProcessAudio(*inputBuffer, bStore, lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
			EndReflection();
#endif

			if (diffraction)
			{

#ifdef PROFILE_AUDIO_THREAD
				BeginDiffraction();
#endif
				ProcessDiffraction(bStore, bStore, lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
				EndDiffraction();
#endif
			}

#ifdef PROFILE_AUDIO_THREAD
			BeginAirAbsorption();
#endif
			mAirAbsorption->ProcessAudio(bStore, bStore, lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
			EndAirAbsorption();
#endif

			for (int i = 0; i < numFrames; i++)
				bInput[i] = static_cast<float>(bStore[i] * gain.Use(lerpFactor));

#ifdef PROFILE_AUDIO_THREAD
			Begin3DTI();
#endif
			{
				shared_lock<shared_mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(*std::atomic_load(&transform));
				mSource->SetBuffer(bInput);
			
				int fdnChannel = mFDNChannel.load();
				if (fdnChannel > -1)
				{
					mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
					std::transform(bMonoOutput.begin(), bMonoOutput.begin() + numFrames, reverbInput[fdnChannel].begin(), reverbInput[fdnChannel].begin(),
						[](auto input, auto current) { return current + input; });
				}
				else
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
			EndVirtualSource();
#endif
			FreeAccess();
			return;
		}

		////////////////////////////////////////

		void ImageSource::ProcessDiffraction(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor)
		{
			activeModel->ProcessAudio(inBuffer, outBuffer, lerpFactor);
			
			if (!isCrossFading.load())
				return;

			if (auto newModel = incomingModel.exchange(nullptr))
				fadeModel = newModel;

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

				if (auto queued = nextModel.exchange(nullptr))
					incomingModel.store(queued);
				else
					isCrossFading.store(false);
			}
		}
	}
}