/*
* @class Source, SourceData
*
* @brief Declaration of Source and SourceData classes
*
*/

// Common headers
#include "Common/Types.h" 
#include "Common/Vec3.h"
#include "Common/Vec4.h"

// Spatialiser headers
#include "Spatialiser/Source.h"
#include "Spatialiser/Mutexes.h"

// Unity headers
#include "Unity/Debug.h"
#include "Unity/UnityInterface.h"

// 3DTI headers
#include "BinauralSpatializer/Core.h"
#include "3dti_Toolkit/Common/CommonDefinitions.h"

// DSP headers
#include "DSP/Interpolate.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	// using namespace Unity;
	using namespace DSP;
	namespace Spatialiser
	{

		//////////////////// Source class ////////////////////

		Source::Source(Binaural::CCore* core, const Config& config) : mCore(core), mConfig(config), targetGain(0.0f), currentGain(0.0f), mAirAbsorption(mConfig.fs), mDirectivity(SourceDirectivity::omni), feedsFDN(false), hasChanged(true)
		{
			vWallMutex = std::make_shared<std::mutex>();
			vEdgeMutex = std::make_shared<std::mutex>();
			dataMutex = std::make_shared<std::mutex>();
			vSourcesMutex = std::make_shared<std::mutex>();
			vSourceDataMutex = std::make_shared<std::mutex>();
			currentDataMutex = std::make_shared<std::mutex>();

			// Initialise source to core
			{
				lock_guard<mutex> lock(tuneInMutex);
				mSource = mCore->CreateSingleSourceDSP();
				mSource->EnablePropagationDelay();
				mSource->DisableInterpolation();

				//Select spatialisation mode
				UpdateSpatialisationMode(config.spatMode);
			}

			ResetFDNSlots();
			bStore.ResizeBuffer(mConfig.numFrames);
			bInput = CMonoBuffer<float>(config.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
			bMonoOutput = CMonoBuffer<float>(mConfig.numFrames);
		}

		Source::~Source()
		{
			{
				lock_guard<mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
			}
			Reset();
		}

		void Source::UpdateSpatialisationMode(const SpatMode mode)
		{
			mConfig.spatMode = mode;
			switch (mode)
			{
			case SpatMode::quality:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
				break;
			}
			case SpatMode::performance:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			case SpatMode::none:
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
				lock_guard<mutex> lock(*vSourcesMutex);
				for (auto& it : mVSources)
					it.second.UpdateSpatialisationMode(mode);
			}
		}

		void Source::UpdateDiffractionModel(const DiffractionModel model)
		{
			mConfig.diffractionModel = model;
			{
				lock_guard<mutex> lock(*vSourcesMutex);
				for (auto& it : mVSources)
					it.second.UpdateDiffractionModel(model);
			}
		}

		void Source::ResetFDNSlots()
		{
			freeFDNChannels.clear();
			freeFDNChannels.push_back(0);
		}

		void Source::ProcessAudio(const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer)
		{
			{
				lock_guard<mutex> lock(*vSourcesMutex);
				for (auto& it : mVSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer, reverbEnergy);
			}

#ifdef DEBUG_AUDIO_THREAD
	// Debug::Log("Total audio vSources: " + std::to_string(counter), Colour::Orange);
#endif
			lock_guard<std::mutex> lock(*dataMutex);
			if (currentGain == 0.0 && targetGain == 0.0)
				return;

#ifdef PROFILE_AUDIO_THREAD
			BeginSource();
			BeginAirAbsorption();
#endif
			mAirAbsorption.ProcessAudio(data, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
			EndAirAbsorption();
#endif
			if (currentGain == targetGain)
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
						Real factor = 1.1 * mAirAbsorption.GetDistance() * reverbEnergy / currentGain; // Adjusted gain
						factor /= static_cast<Real>(mConfig.numFDNChannels);

						mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
						for (int i = 0; i < mConfig.numFrames; i++)
						{
							Real in = static_cast<Real>(bMonoOutput[i]) * factor;
							for (int j = 0; j < mConfig.numFDNChannels; j++)
								reverbInput[i][j] += in;
						}
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

		void Source::Update(const Vec3& position, const Vec4& orientation, const Real distance)
		{

			/*UpdateVirtualSourceDataMap();
			UpdateVirtualSources();*/

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

		void Source::UpdateVirtualSourceDataMap()
		{
			// lock_guard<mutex> lock(*vSourceDataMutex);
			for (auto& [key, vSource] : currentVSources)
			{
				auto it = targetVSources.find(key);
				if (it == targetVSources.end()) // case: old vSource
					vSource.Invisible();
			}

			for (auto& [key, vSource] : targetVSources)
			{
				auto it = currentVSources.find(key);
				if (it == currentVSources.end()) // case: add new vSource
					currentVSources.emplace(key, vSource);
				else // case: update exist vSource
					it->second = vSource;
			}
		}

		void Source::UpdateVirtualSources()
		{
			std::vector<std::string> keys;
			for (auto& [key, vSource] : currentVSources)
			{
				if (UpdateVirtualSource(vSource))
					keys.push_back(key);
			}
			for (const auto& key : keys)
				currentVSources.erase(key);
		}

		bool Source::UpdateVirtualSource(const VirtualSourceData& data)
		{
			auto it = mVSources.find(data.GetKey());
			if (it == mVSources.end())		// case: virtual source does not exist
			{
				int fdnChannel = -1;
				if (data.feedsFDN)
					fdnChannel = AssignFDNChannel();
				{
					unique_lock<mutex> lock(*vSourcesMutex, defer_lock);
					if (lock.try_lock())
						mVSources.try_emplace(data.GetKey(), mCore, mConfig, data, fdnChannel);
				}
			}
			else
			{
				int fdnChannel = -1;
				if (data.feedsFDN && it->second.GetFDNChannel() < 0)
					fdnChannel = AssignFDNChannel();

				bool remove = it->second.UpdateVirtualSource(data, fdnChannel);

				assert(!(data.feedsFDN && it->second.GetFDNChannel() == -1));

				if (fdnChannel >= 0) // Add vSource old fdnChannel to freeFDNChannels (Also prevents leaking FDN channels if !data.visible and the channel is not assigned to vSource)
					freeFDNChannels.push_back(fdnChannel);
				if (remove)
				{
					if (it->second.GetFDNChannel() >= 0)
						freeFDNChannels.push_back(it->second.GetFDNChannel());

					size_t n = 0;
					{
						unique_lock<mutex> lock(*vSourcesMutex, defer_lock);
						if (lock.try_lock())
							n = mVSources.erase(data.GetKey());
					}

					if (n == 1) // Check vSource has been successfully erased
						return true;
				}
			}
			return false;
		}

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