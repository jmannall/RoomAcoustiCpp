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
#include "Spatialiser/Mutexes.h"

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
	namespace Spatialiser
	{

		//////////////////// ImageSourceData class ////////////////////

		////////////////////////////////////////

		void ImageSourceData::AddPlaneID(const size_t id)
		{
			pathParts.emplace_back(id, true);
			order++;
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
			pathParts.emplace_back(id, false);
			order++;
			diffraction = true;
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

		void ImageSourceData::Clear()
		{
			Reset();
			pathParts.clear();
			mPositions.clear();
			reflection = false;
			diffraction = false;
			order = 0;
			key = "";
		}

		////////////////////////////////////////

		void ImageSourceData::Update(const ImageSourceData& imageSource)
		{
			pathParts = imageSource.pathParts;
			mPositions = imageSource.mPositions;
			reflection = imageSource.reflection;
			diffraction = imageSource.diffraction;
			if (diffraction)
				mDiffractionPath = imageSource.mDiffractionPath;
			order = imageSource.order;
			key = imageSource.key;
		}

		//////////////////// ImageSource class ////////////////////

		////////////////////////////////////////

		ImageSource::ImageSource(Binaural::CCore* core, const Config& config, const ImageSourceData& data, int fdnChannel) : mCore(core), mSource(nullptr), mConfig(config), mFDNChannel(fdnChannel),
			mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(data.GetAbsorption(), config.frequencyBands, config.Q, config.fs), mAirAbsorption(data.GetDistance(), mConfig.fs), feedsFDN(data.IsFeedingFDN()), bStore(config.numFrames), bDiffStore(config.numFrames),
			attenuate(&mDiffractionPath), lowPass(&mDiffractionPath, config.fs), udfa(&mDiffractionPath, config.fs), udfai(&mDiffractionPath, config.fs), nnSmall(&mDiffractionPath), nnBest(&mDiffractionPath), utd(&mDiffractionPath, config.fs), btm(&mDiffractionPath, config.fs),
			reflection(data.IsReflection()), diffraction(data.IsDiffraction()), transform(data.GetTransform()), isInitialised(false), isCrossFading(false), mDiffractionModel(nullptr), mOldDiffractionModel(nullptr),
			crossfadeCounter(0), crossfadeLengthSamples(static_cast<int>(round(mConfig.fs * 0.01)))
		{
			UpdateDiffractionModel(config.diffractionModel);
			
			audioMutex = std::make_shared<std::mutex>();

			bInput = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
			bMonoOutput = CMonoBuffer<float>(mConfig.numFrames);
			bDiffStore.ResizeBuffer(std::min(mConfig.numFrames, crossfadeLengthSamples));

			Update(data, fdnChannel);
		}

		////////////////////////////////////////

		ImageSource::~ImageSource()
		{
			mCore->RemoveSingleSourceDSP(mSource);
		}

		////////////////////////////////////////

		void ImageSource::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			mConfig.spatialisationMode = mode;
			if (isInitialised)
			{
				lock_guard<mutex> lock(tuneInMutex);
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

		void ImageSource::UpdateDiffractionModel(const DiffractionModel model)
		{
			mOldDiffractionModel = mDiffractionModel;
			mConfig.diffractionModel = model;
			switch (model)
			{
			case DiffractionModel::attenuate:
			{ mDiffractionModel = &attenuate; break; }
			case DiffractionModel::lowPass:
			{ mDiffractionModel = &lowPass; break; }
			case DiffractionModel::udfa:
			{ mDiffractionModel = &udfa; break; }
			case DiffractionModel::udfai:
			{ mDiffractionModel = &udfai; break; }
			case DiffractionModel::nnBest:
			{ mDiffractionModel = &nnBest; break; }
			case DiffractionModel::nnSmall:
			{ mDiffractionModel = &nnSmall; break; }
			case DiffractionModel::utd:
			{ mDiffractionModel = &utd; break; }
			case DiffractionModel::btm:
			{ mDiffractionModel = &btm; break; }
			default:
				mDiffractionModel = &btm;
			}
			UpdateDiffraction();

			if (mOldDiffractionModel == mDiffractionModel)
				return;

			if (mOldDiffractionModel == nullptr)
				return;

			isCrossFading = true;
			crossfadeCounter = 0;
		}

		////////////////////////////////////////

		bool ImageSource::Update(const ImageSourceData& data, int& fdnChannel)
		{
			if (data.IsVisible()) // Process virtual source - Init if doesn't exist
			{
				unique_lock<mutex> lck(*audioMutex, std::defer_lock);
				if (lck.try_lock())
				{
					mTargetGain = data.GetDirectivity();
					if (!isInitialised)
						Init(data);
					UpdateParameters(data, fdnChannel);
				}
			}
			else
			{
				unique_lock<mutex> lck(*audioMutex, std::defer_lock);
				if (lck.try_lock())
				{
					mTargetGain = 0.0f;
					// mDirectivityStore = mCurrentGain;
					if (mCurrentGain < 0.0001)
					{
						if (isInitialised)
							Remove();
						lck.unlock();
						return true;
					}
				}
			}
			return false;
		}

		////////////////////////////////////////

		void ImageSource::Init(const ImageSourceData& data)
		{
			// audioMutex already locked
#ifdef DEBUG_VIRTUAL_SOURCE
			Debug::Log("Init virtual source", Colour::Green);
#endif
			reflection = data.IsReflection();
			diffraction = data.IsDiffraction();

			if (reflection) // Set reflection filter
				mFilter.InitParameters(data.GetAbsorption());

			// Set btm currentIr
			if (diffraction)
			{
				mDiffractionPath = data.GetDiffractionPath();
				btm.UpdatePath(&mDiffractionPath);
				btm.InitParameters();
			}

			order = data.GetOrder();
			{
				lock_guard<mutex> lock(tuneInMutex);

				// Initialise source to core
				mSource = mCore->CreateSingleSourceDSP();
				mSource->EnablePropagationDelay();
				mSource->SetSourceTransform(data.GetTransform());
				mSource->DisableInterpolation();
			}
			//updateTransform = true;
			isInitialised = true;

			//Select spatialisation mode
			UpdateSpatialisationMode(mConfig.spatialisationMode);
		}

		////////////////////////////////////////

		void ImageSource::UpdateParameters(const ImageSourceData& data, int& fdnChannel)
		{
			// audioMutex already locked
			if (reflection) // Update reflection filter
				mFilter.SetGain(data.GetAbsorption());

			if (diffraction)
			{
				mDiffractionPath = data.GetDiffractionPath();
				UpdateDiffraction();
			}

			mAirAbsorption.SetDistance(data.GetDistance());

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
#ifdef DEBUG_VIRTUAL_SOURCE
			Debug::Log("Remove virtual source", Colour::Red);
#endif
			{
				lock_guard<mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
			}
			isInitialised = false;
		}

		////////////////////////////////////////

		void ImageSource::ProcessAudio(const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer)
		{
			lock_guard<mutex> lock(*audioMutex);

			if (!isInitialised)
				return;

			if (mCurrentGain == 0.0 && mTargetGain == 0.0)
				return;

#ifdef PROFILE_AUDIO_THREAD
			BeginVirtualSource();
#endif
			if (diffraction)
			{
				{
#ifdef PROFILE_AUDIO_THREAD
					BeginDiffraction();
#endif
					ProcessDiffraction(data, bStore);
#ifdef PROFILE_AUDIO_THREAD
					EndDiffraction();
#endif
					if (reflection)
					{
#ifdef PROFILE_AUDIO_THREAD
						BeginReflection();
#endif
						mFilter.ProcessAudio(bStore, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
						EndReflection();
#endif
					}
				}
			}
			else if (reflection)
			{
#ifdef PROFILE_AUDIO_THREAD
				BeginReflection();
#endif
				mFilter.ProcessAudio(data, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
				EndReflection();
#endif
			}
#ifdef PROFILE_AUDIO_THREAD
			BeginAirAbsorption();
#endif
			mAirAbsorption.ProcessAudio(bStore, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
			EndAirAbsorption();
#endif
			if (mCurrentGain == mTargetGain)
			{
				for (int i = 0; i < mConfig.numFrames; i++)
					bInput[i] = static_cast<float>(bStore[i] * mCurrentGain);
			}
			else if (Equals(mCurrentGain, mTargetGain))
			{
				mCurrentGain = mTargetGain;
				for (int i = 0; i < mConfig.numFrames; i++)
					bInput[i] = static_cast<float>(bStore[i] * mCurrentGain);
			}
			else
			{
				for (int i = 0; i < mConfig.numFrames; i++)
				{
					bInput[i] = static_cast<float>(bStore[i] * mCurrentGain);
					mCurrentGain = Lerp(mCurrentGain, mTargetGain, mConfig.lerpFactor);
				}
			}
		
#ifdef PROFILE_AUDIO_THREAD
			Begin3DTI();
#endif
			{
				lock_guard<mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(transform);
				mSource->SetBuffer(bInput);

				if (feedsFDN)
				{
					{
						mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
						for (int i = 0; i < mConfig.numFrames; i++)
							reverbInput[i][mFDNChannel] += static_cast<Real>(bMonoOutput[i]);
					}
				}
				else
					mSource->ProcessAnechoic(bOutput.left, bOutput.right);
			}
#ifdef PROFILE_AUDIO_THREAD
			End3DTI();
#endif
			int j = 0;
			for (int i = 0; i < mConfig.numFrames; i++)
			{
				outputBuffer[j++] += static_cast<Real>(bOutput.left[i]);
				outputBuffer[j++] += static_cast<Real>(bOutput.right[i]);
			}
#ifdef PROFILE_AUDIO_THREAD
			EndVirtualSource();
#endif
		}

		////////////////////////////////////////

		void ImageSource::ProcessDiffraction(const Buffer& inBuffer, Buffer& outBuffer)
		{
			if (isCrossFading)
			{
				assert(mOldDiffractionModel != nullptr);

				mOldDiffractionModel->ProcessAudio(inBuffer, bDiffStore, bDiffStore.Length(), mConfig.lerpFactor);
				mDiffractionModel->ProcessAudio(inBuffer, outBuffer, mConfig.numFrames, mConfig.lerpFactor);

				for (int i = 0; i < bDiffStore.Length(); ++i)
				{
					float crossfadeFactor = static_cast<float>(crossfadeCounter) / crossfadeLengthSamples;
					outBuffer[i] *= crossfadeFactor;
					outBuffer[i] += bDiffStore[i] * (1.0f - crossfadeFactor);
					++crossfadeCounter;

					if (crossfadeCounter >= crossfadeLengthSamples)
					{
						isCrossFading = false;
						mOldDiffractionModel = nullptr;
						return;
					}
				}
			}
			else
				mDiffractionModel->ProcessAudio(inBuffer, outBuffer, mConfig.numFrames, mConfig.lerpFactor);
		}
	}
}