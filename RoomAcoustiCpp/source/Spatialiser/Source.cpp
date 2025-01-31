/*
* @class Source
*
* @brief Declaration of Source class
*
*/

// Spatialiser headers
#include "Spatialiser/Source.h"
#include "Spatialiser/Mutexes.h"

// DSP headers
#include "DSP/Interpolate.h"

// Unity headers
#include "Unity/Debug.h"
#include "Unity/UnityInterface.h"

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

		Source::Source(Binaural::CCore* core, const Config& config) : mCore(core), mConfig(config), targetGain(0.0f), currentGain(0.0f), mAirAbsorption(mConfig.fs), mDirectivity(SourceDirectivity::omni), directivityFilter(config.frequencyBands, config.Q, config.fs), feedsFDN(false), hasChanged(true)
		{
			dataMutex = std::make_shared<std::mutex>();
			imageSourcesMutex = std::make_shared<std::mutex>();
			imageSourceDataMutex = std::make_shared<std::mutex>();
			currentDataMutex = std::make_shared<std::mutex>();

			// Initialise source to core
			{
				lock_guard<mutex> lock(tuneInMutex);
				mSource = mCore->CreateSingleSourceDSP();
				mSource->EnablePropagationDelay();
				mSource->DisableInterpolation();

				//Select spatialisation mode
				UpdateSpatialisationMode(config.spatialisationMode);
			}

			ResetFDNSlots();
			bStore.ResizeBuffer(mConfig.numFrames);
			bInput = CMonoBuffer<float>(config.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
			bMonoOutput = CMonoBuffer<float>(mConfig.numFrames);
		}

		////////////////////////////////////////

		Source::~Source()
		{
			{
				lock_guard<mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
			}
			Reset();
		}

		////////////////////////////////////////

		void Source::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			mConfig.spatialisationMode = mode;
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
				for (auto& it : mImageSources)
					it.second.UpdateSpatialisationMode(mode);
			}
		}

		////////////////////////////////////////

		void Source::UpdateDiffractionModel(const DiffractionModel model)
		{
			mConfig.diffractionModel = model;
			{
				lock_guard<mutex> lock(*imageSourcesMutex);
				for (auto& it : mImageSources)
					it.second.UpdateDiffractionModel(model);
			}
		}

		////////////////////////////////////////

		void Source::UpdateDirectivity(const SourceDirectivity directivity)
		{
			{
				lock_guard<mutex> lock(*dataMutex);
				mDirectivity = directivity;
				switch (mDirectivity)
				{
				case SourceDirectivity::omni:
					reverbEnergy = 1.0;
					break;
				case SourceDirectivity::cardioid:
					reverbEnergy = 0.5;
					break;
				case SourceDirectivity::genelec8020c:
					reverbEnergy = 0.5;
					break;
				default:
					reverbEnergy = 1.0;
					break;
				}
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

		void Source::ProcessAudio(const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer)
		{
			{
				lock_guard<mutex> lock(*imageSourcesMutex);
				for (auto& it : mImageSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer);
			}

			lock_guard<std::mutex> lock(*dataMutex);
			/*if (currentGain == 0.0 && targetGain == 0.0)
				return;*/
			if (directivityFilter.Invalid())
				return;

			Real gain;
			if (feedsFDN)
				gain = directivityFilter.GetDCGain(); // Adjusted gain

#ifdef PROFILE_AUDIO_THREAD
			BeginSource();
			BeginReflection();
#endif
			directivityFilter.ProcessAudio(data, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
			EndReflection();
			BeginAirAbsorption();
#endif
			mAirAbsorption.ProcessAudio(bStore, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
			EndAirAbsorption();
#endif
			if (feedsFDN && !directivityFilter.Invalid())
			{
				if (gain == 0.0)
					gain = directivityFilter.GetDCGain(); // Adjusted gain
				else
					gain = (gain + directivityFilter.GetDCGain()) / 2.0; // Adjusted gain
			}
			/*if (currentGain == targetGain)
			{
				for (int i = 0; i < mConfig.numFrames; i++)
					bInput[i] = static_cast<float>(currentGain * bStore[i]);
			}
			else if (Equals(currentGain, targetGain))
			{
				currentGain = targetGain;
				for (int i = 0; i < mConfig.numFrames; i++)
					bInput[i] = static_cast<float>(currentGain * bStore[i]);
			}
			else
			{
				for (int i = 0; i < mConfig.numFrames; i++)
				{
					bInput[i] = static_cast<float>(currentGain * bStore[i]);
					currentGain = Lerp(currentGain, targetGain, mConfig.lerpFactor);
				}
			}*/

			for (int i = 0; i < mConfig.numFrames; i++)
				bInput[i] = static_cast<float>(bStore[i]);

#ifdef PROFILE_AUDIO_THREAD
			Begin3DTI();
#endif
			{
				lock_guard<mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(transform);
				mSource->SetBuffer(bInput);

				if (feedsFDN)
				{
					Real factor = 1.1 * mAirAbsorption.GetDistance() * reverbEnergy / gain; // Adjusted gain
					factor /= static_cast<Real>(mConfig.numFDNChannels);

					mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
					for (int i = 0; i < mConfig.numFrames; i++)
					{
						Real in = static_cast<Real>(bMonoOutput[i]) * factor;
						for (int j = 0; j < mConfig.numFDNChannels; j++)
							reverbInput[i][j] += in;
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
			EndSource();
#endif
		}

		////////////////////////////////////////

		void Source::Update(const Vec3& position, const Vec4& orientation, const Real distance)
		{
			{
				lock_guard<std::mutex>lock(*dataMutex);
				mAirAbsorption.SetDistance(distance);
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

		void Source::UpdateImageSources()
		{
			std::vector<std::string> keys;
			for (auto& [key, vSource] : currentImageSources)
			{
				if (UpdateImageSource(vSource))
					keys.push_back(key);
			}
			for (const auto& key : keys)
				currentImageSources.erase(key);
		}

		////////////////////////////////////////

		bool Source::UpdateImageSource(const ImageSourceData& data)
		{
			auto it = mImageSources.find(data.GetKey());
			if (it == mImageSources.end())		// case: virtual source does not exist
			{
				int fdnChannel = -1;
				if (data.IsFeedingFDN())
					fdnChannel = AssignFDNChannel();
				{
					unique_lock<mutex> lock(*imageSourcesMutex, defer_lock);
					if (lock.try_lock())
						mImageSources.try_emplace(data.GetKey(), mCore, mConfig, data, fdnChannel);
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
					fdnChannel = AssignFDNChannel();

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
							n = mImageSources.erase(data.GetKey());
					}

					if (n == 1) // Check vSource has been successfully erased
						return true;
				}
			}
			return false;
		}

		////////////////////////////////////////

		int Source::AssignFDNChannel()
		{
			if (freeFDNChannels.back() >= mConfig.numFDNChannels)
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