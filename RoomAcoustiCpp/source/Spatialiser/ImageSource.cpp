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

		ImageSource::ImageSource(Binaural::CCore* core, const std::shared_ptr<Config>& config, const ImageSourceData& data, int fdnChannel) : mCore(core), mSource(nullptr), mFDNChannel(fdnChannel),
			gain(0.0), mFilter(data.GetAbsorption(), config->frequencyBands, config->Q, config->fs), mAirAbsorption(data.GetDistance(), config->fs), feedsFDN(data.IsFeedingFDN()), bStore(config->numFrames), bDiffStore(config->numFrames), mDiffractionPath(data.GetDiffractionPath()),
			reflection(data.IsReflection()), diffraction(data.IsDiffraction()), transform(data.GetTransform()), isCrossFading(false),
			currentDiffractionModel(config->GetDiffractionModel()), diffractionGain(0.0)
		{
			diffractionGain.Reset(1.0);			
			switch (currentDiffractionModel)
			{
			case DiffractionModel::attenuate:
			{
				activeModel = std::make_shared<Diffraction::Attenuate>(mDiffractionPath);
				break;
			}
			case DiffractionModel::lowPass:
			{
				activeModel = std::make_shared<Diffraction::LPF>(mDiffractionPath, config->fs);
				break;
			}
			case DiffractionModel::udfa:
			{
				activeModel = std::make_shared<Diffraction::UDFA>(mDiffractionPath, config->fs);
				break;
			}
			case DiffractionModel::udfai:
			{
				activeModel = std::make_shared<Diffraction::UDFAI>(mDiffractionPath, config->fs);
				break;
			}
			case DiffractionModel::nnSmall:
			{
				activeModel = std::make_shared<Diffraction::NNSmall>(mDiffractionPath);
				break;
			}
			case DiffractionModel::nnBest:
			{
				activeModel = std::make_shared<Diffraction::NNBest>(mDiffractionPath);
				break;
			}
			case DiffractionModel::utd:
			{
				activeModel = std::make_shared<Diffraction::UTD>(mDiffractionPath, config->fs);
				break;
			}
			case DiffractionModel::btm:
			{
				activeModel = std::make_shared<Diffraction::BTM>(mDiffractionPath, config->fs);
				break;
			}
			}
			releasePool.Add(activeModel);

			bInput = CMonoBuffer<float>(config->numFrames);
			bOutput.left = CMonoBuffer<float>(config->numFrames);
			bOutput.right = CMonoBuffer<float>(config->numFrames);
			bMonoOutput = CMonoBuffer<float>(config->numFrames);
			bDiffStore = Buffer(config->numFrames);

			Init(data, config->GetImpulseResponseMode(), config->GetSpatialisationMode());
			Update(data, fdnChannel);
		}

		////////////////////////////////////////

		ImageSource::~ImageSource()
		{
			if (mSource != nullptr)
				mCore->RemoveSingleSourceDSP(mSource);
		}

		////////////////////////////////////////

		void ImageSource::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			if (isInitialised.load())
			{
				unique_lock<shared_mutex> lock(tuneInMutex);
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
		}

		////////////////////////////////////////

		void ImageSource::UpdateImpulseResponseMode(const bool mode)
		{
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
			}
		}

		////////////////////////////////////////

		void ImageSource::UpdateDiffractionModel(const DiffractionModel model, const int fs)
		{
			if (currentDiffractionModel == model)
				return;
			currentDiffractionModel = model;

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
					if (isInitialised.load())
						Remove();
					return true;
				}
			}
			return false;
		}

		////////////////////////////////////////

		void ImageSource::Init(const ImageSourceData& data, const bool impulseReponseMode, const SpatialisationMode spatialisationMode)
		{
			// audioMutex already locked
#ifdef DEBUG_IMAGE_SOURCE
			Debug::Log("Init virtual source", Colour::Green);
#endif
			{
				shared_lock<shared_mutex> lock(tuneInMutex);

				// Initialise source to core
				mSource = mCore->CreateSingleSourceDSP();
				mSource->EnablePropagationDelay();
				mSource->DisableFarDistanceEffect();
				mSource->DisableNearFieldEffect();

				if (impulseReponseMode)
				{
					mSource->DisableDistanceAttenuationSmoothingAnechoic();
					mSource->DisableInterpolation();
				}

				mSource->SetSourceTransform(data.GetTransform());
			}
			//updateTransform = true;
			isInitialised.store(true);

			UpdateSpatialisationMode(spatialisationMode);
		}

		////////////////////////////////////////

		void ImageSource::UpdateParameters(const ImageSourceData& data, int& fdnChannel)
		{
			// audioMutex already locked
			// Update reflection/directivity filter
			mFilter.SetTargetGains(data.GetAbsorption());

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

			mAirAbsorption.SetTargetDistance(data.GetDistance());

			if (data.IsFeedingFDN() != feedsFDN)
			{
				feedsFDN = !feedsFDN;
				std::swap(mFDNChannel, fdnChannel);
			}

			transform = data.GetTransform();
		}

		////////////////////////////////////////

		void ImageSource::Remove()
		{
			// audioMutex already locked
#ifdef DEBUG_IMAGE_SOURCE
			Debug::Log("Remove virtual source", Colour::Red);
#endif
			{
				unique_lock<shared_mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
				mSource.reset();
			}
			isInitialised.store(false);
		}

		////////////////////////////////////////

		int ImageSource::ProcessAudio(const Buffer& data, Buffer& reverbInput, Buffer& outputBuffer, const Real lerpFactor)
		{
			if (!isInitialised.load())
			{
				std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0);
				return -1;
			}

			if (gain.IsZero())
			{
				std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0);
				return -1;
			}
			const int numFrames = data.Length();

#ifdef PROFILE_AUDIO_THREAD
			BeginVirtualSource();
#endif
#ifdef PROFILE_AUDIO_THREAD
			BeginReflection();	// Always process as also includes directivity
#endif
			mFilter.ProcessAudio(data, bStore, lerpFactor);
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
			mAirAbsorption.ProcessAudio(bStore, bStore, lerpFactor);
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
				mSource->SetSourceTransform(transform);
				mSource->SetBuffer(bInput);
			
				// lock_guard<mutex> lock(tuneInMutex);
				if (feedsFDN)
				{
					{
						mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
						std::transform(bMonoOutput.begin(), bMonoOutput.end(), reverbInput.begin(),
							[](auto value) { return static_cast<Real>(value); });
					}
				}
				else
				{
					std::fill(reverbInput.begin(), reverbInput.end(), 0.0);
					mSource->ProcessAnechoic(bOutput.left, bOutput.right);
				}
			}
#ifdef PROFILE_AUDIO_THREAD
			End3DTI();
#endif
			int j = 0;
			for (int i = 0; i < numFrames; i++)
			{
				outputBuffer[j++] = static_cast<Real>(bOutput.left[i]);
				outputBuffer[j++] = static_cast<Real>(bOutput.right[i]);
			}
#ifdef PROFILE_AUDIO_THREAD
			EndVirtualSource();
#endif
			return mFDNChannel;
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