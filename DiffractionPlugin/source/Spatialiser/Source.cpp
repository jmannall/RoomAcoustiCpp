/*
*
*  \Source class
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

// DSP headers
#include "DSP/Interpolate.h"

namespace UIE
{
	using namespace Common;
	// using namespace Unity;
	using namespace DSP;
	namespace Spatialiser
	{

		//////////////////// Source class ////////////////////

		Source::Source(Binaural::CCore* core, const Config& config) : mCore(core), mConfig(config), targetGain(0.0f), currentGain(0.0f), mAirAbsorption(mConfig.fs)
		{
			vWallMutex = std::make_shared<std::mutex>();
			vEdgeMutex = std::make_shared<std::mutex>();
			dataMutex = std::make_shared<std::mutex>();

			// Initialise source to core
			{
				lock_guard<mutex> lock(tuneInMutex);
				mSource = mCore->CreateSingleSourceDSP();
				mSource->EnablePropagationDelay();

				//Select spatialisation mode
				UpdateSpatialisationMode(config.spatConfig.GetMode(0));
			}

			ResetFDNSlots();
			bStore.ResizeBuffer(mConfig.numFrames);
			bInput = CMonoBuffer<float>(config.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
		}

		Source::~Source()
		{
			{
				lock_guard<mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
			}
			Reset();
		}

		void Source::UpdateSpatialisationMode(const HRTFMode& mode)
		{
			switch (mode)
			{
			case HRTFMode::quality:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
				break;
			}
			case HRTFMode::performance:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			case HRTFMode::none:
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

		void Source::UpdateSpatialisationMode(const SPATConfig& config)
		{
			mConfig.spatConfig = config;
			UpdateSpatialisationMode(config.GetMode(0));
			{
				lock_guard<mutex> lock(*vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.UpdateSpatialisationMode(config);
			}
			{
				lock_guard<mutex> lock(*vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.UpdateSpatialisationMode(config);
			}
		}

		void Source::ResetFDNSlots()
		{
			freeFDNChannels.clear();
			freeFDNChannels.push_back(0);
		}

		void Source::ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer)
		{
			{
				lock_guard<mutex> lock(*vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer);
			}
			{
				lock_guard<mutex> lock(*vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer);
			}
#ifdef DEBUG_AUDIO_THREAD
	// Debug::Log("Total audio vSources: " + IntToStr(counter), Colour::Orange);
#endif
			lock_guard<std::mutex> lock(*dataMutex);
			if (mData.visible || currentGain != 0.0f)
			{
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
				else
				{
					for (int i = 0; i < mConfig.numFrames; i++)
					{
						bInput[i] = static_cast<float>(currentGain * bStore[i]);
						Lerp(currentGain, targetGain, mConfig.lerpFactor);
					}
				}

#ifdef PROFILE_AUDIO_THREAD
				Begin3DTI();
#endif
				{
					lock_guard<mutex> lock(tuneInMutex);
					mSource->SetBuffer(bInput);
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
		}

		void Source::Update(const vec3& position, const vec4& orientation, const Real& distance)
		{
			CTransform transform;
			transform.SetOrientation(CQuaternion(static_cast<float>(orientation.w), static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)));
			transform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));

			{
				lock_guard<mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(transform);
			}

			VirtualSourceDataMap vSources;	
			{
				lock_guard<std::mutex>lock(*dataMutex);
				mAirAbsorption.SetDistance(distance);
				mData.mPosition = position;
				if (mData.visible)
					targetGain = 1.0f;
				else
					targetGain = 0.0f;
				vSources = mData.vSources;
			}
			UpdateVirtualSources(vSources);
		}

		void Source::UpdateVirtualSources(const VirtualSourceDataMap& data)
		{
			std::vector<std::string> keys;
			std::vector<VirtualSourceData> newVSources;

			for (auto& oData : oldData)
			{
				auto out = data.find(oData.first);
				if (out == data.end()) // case: old vSource
				{
					oData.second.Invisible();
					bool remove = UpdateVirtualSource(oData.second, newVSources);
					if (remove)
					{
						keys.push_back(oData.first);
					}
				}
			}

			for (auto key : keys)
				oldData.erase(key);

			for (auto& in : data)
			{
				UpdateVirtualSource(in.second, newVSources);	// newVSources are new placeholders in the ISM tree
				oldData.insert_or_assign(in.first, in.second);

#ifdef DEBUG_VIRTUAL_SOURCE
				Debug::Log("vSource: " + it->first, Colour::Yellow);
#endif
			}

			for (auto vSource : newVSources)
				oldData.insert({ vSource.GetKey(), vSource });

#ifdef DEBUG_VIRTUAL_SOURCE
	Debug::Log("Total vSources: " + IntToStr(oldData.size()), Colour::Yellow);
#endif
		}

		bool Source::UpdateVirtualSource(const VirtualSourceData& data, std::vector<VirtualSourceData>& newVSources)
		{
			int orderIdx = data.GetOrder() - 1;

			VirtualSourceMap* tempStore;
			std::shared_ptr<std::mutex> m;

			if (data.IsReflection(0))
			{
				tempStore = &mVirtualSources;
				m = vWallMutex;
			}
			else
			{
				tempStore = &mVirtualEdgeSources;
				m = vEdgeMutex;
			}

			for (int i = 0; i < orderIdx; i++)
			{
				auto it = tempStore->find(data.GetID(i));
				if (it == tempStore->end())		// case: virtual source lower in the tree does not exist
				{
					unique_lock<mutex> lck(*m, std::defer_lock);
					if (lck.try_lock())
					{
						auto newIt = tempStore->insert_or_assign(data.GetID(i), VirtualSource(mCore, mConfig)); // feedsFDN always the highest order ism
						it = newIt.first;
						VirtualSourceData vSource = data;
						newVSources.push_back(vSource.Trim(i));
					}
					else
						return false;
				}

				if (data.IsReflection(i + 1))
				{
					tempStore = &it->second.mVirtualSources;
					m = it->second.vWallMutex;
				}
				else
				{
					tempStore = &it->second.mVirtualEdgeSources;
					m = it->second.vEdgeMutex;
				}
			}

			auto it = tempStore->find(data.GetID(orderIdx));
			if (it == tempStore->end())		// case: virtual source does not exist
			{
				int fdnChannel = -1;
				if (data.feedsFDN)
				{
					if (freeFDNChannels.back() >= mConfig.numFDNChannels)
						freeFDNChannels.back() = 0;
					fdnChannel = static_cast<int>(freeFDNChannels.back());
					if (freeFDNChannels.size() > 1)
						freeFDNChannels.pop_back();
					else
						freeFDNChannels[0]++;
				}
				VirtualSource virtualSource = VirtualSource(mCore, mConfig, data, fdnChannel);

				assert(!(data.feedsFDN && virtualSource.GetFDNChannel() == -1));
				{
					unique_lock<mutex> lck(*m, std::defer_lock);
					if (lck.try_lock())
						tempStore->insert_or_assign(data.GetID(orderIdx), virtualSource);
				}
				virtualSource.Deactivate();
			}
			else
			{
				int fdnChannel = -1;
				if (data.feedsFDN && it->second.GetFDNChannel() < 0)
				{
					if (freeFDNChannels.back() >= mConfig.numFDNChannels)
						freeFDNChannels.back() = 0;
					fdnChannel = static_cast<int>(freeFDNChannels.back());
					if (freeFDNChannels.size() > 1)
						freeFDNChannels.pop_back();
					else
						freeFDNChannels[0]++;
				}

				bool remove = it->second.UpdateVirtualSource(data, fdnChannel);

				assert(!(data.feedsFDN && it->second.GetFDNChannel() == -1));

				if (fdnChannel >= 0) // Add vSource old fdnChannel to freeFDNChannels (Also prevents leaking FDN channels if !data.visible and the channel is not assigned to vSource)
					freeFDNChannels.push_back(fdnChannel);
				if (remove)
				{
					if (it->second.GetFDNChannel() >= 0)
					{
						freeFDNChannels.push_back(it->second.GetFDNChannel());
						int n = 0;
						{
							unique_lock<mutex> lck(*m, std::defer_lock);
							if (lck.try_lock())
								n = tempStore->erase(data.GetID(orderIdx));
						}

						if (n == 1) // Check vSource has been successfully erased
							return true;
					}
					else if (!(it->second.mVirtualSources.size() > 0 || it->second.mVirtualEdgeSources.size() > 0))
					{
						int n = 0;
						{
							unique_lock<mutex> lck(*m, std::defer_lock);
							if (lck.try_lock())
								n = tempStore->erase(data.GetID(orderIdx));
						}

						if (n == 1) // Check vSource has been successfully erased
							return true;
					}
				}
			}
			return false;
		}
	}
}