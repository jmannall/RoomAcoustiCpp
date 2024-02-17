/*
*
*  \Source class
*
*/

// Common headers
#include "Common/Types.h" 
#include "Common/Vec3.h"

// Spatialiser headers
#include "Spatialiser/Main.h"
#include "Spatialiser/Source.h"

// Unity headers
#include "Unity/Debug.h"

// 3DTI headers
#include "BinauralSpatializer/Core.h"

namespace UIE
{
	using namespace Common;
	// using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// Source class ////////////////////

		Source::Source(Binaural::CCore* core, const Config& config) : mCore(core), mConfig(config), targetGain(0.0f), currentGain(0.0f), isVisible(false)
		{
			// Initialise source to core
			mSource = mCore->CreateSingleSourceDSP();

			//Select spatialisation mode
			switch (mConfig.hrtfMode)
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
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			}
			mSource->EnablePropagationDelay();

			ResetFDNSlots();
			bInput = CMonoBuffer<float>(config.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
		}

		Source::~Source()
		{
			mCore->RemoveSingleSourceDSP(mSource);
			Reset();
		}

		void Source::ResetFDNSlots()
		{
			freeFDNChannels.reserve(1);
			std::fill_n(std::back_inserter(freeFDNChannels), 1, 0);
		}

		void Source::ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer)
		{
			{
				lock_guard<mutex> lock(vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer);
				/*for (auto it = mVirtualSources.begin(); it != mVirtualSources.end(); it++)
				{
					counter += it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer, lerpFactor);
				}*/
			}
			{
				lock_guard<mutex> lock(vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer);
				/*for (auto it = mVirtualEdgeSources.begin(); it != mVirtualEdgeSources.end(); it++)
				{
					counter += it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer, lerpFactor);
				}*/
			}

#ifdef DEBUG_AUDIO_THREAD
	// Debug::Log("Total audio vSources: " + IntToStr(counter), Colour::Orange);
#endif

			if (isVisible || currentGain != 0.0f)
			{
#ifdef PROFILE_AUDIO_THREAD
				BeginSource();
#endif
				{
					lock_guard<mutex> lock(audioMutex);
					if (currentGain == targetGain)
					{
						for (int i = 0; i < mConfig.numFrames; i++)
							bInput[i] = static_cast<float>(data[i] * currentGain);
					}
					else
					{
						for (int i = 0; i < mConfig.numFrames; i++)
						{
							bInput[i] = static_cast<float>(data[i] * currentGain);
							currentGain = Lerp(currentGain, targetGain, mConfig.lerpFactor);
						}
					}
					mSource->SetBuffer(bInput);
					mSource->ProcessAnechoic(bOutput.left, bOutput.right);
				}

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

		void Source::Update(const CTransform& transform, const SourceData& data)
		{
			{
				lock_guard<mutex> lock(audioMutex);
				mSource->SetSourceTransform(transform); // Could lock the source while being updated to prevent background thread changing the data?
				isVisible = data.visible;
				if (data.visible)
					targetGain = 1.0f;
				else
					targetGain = 0.0f;
			}

			UpdateVirtualSources(data.vSources);
		}

		void Source::UpdateVirtualSources(const VirtualSourceDataMap& data)
		{
			std::vector<std::string> keys;
			std::vector<VirtualSourceData> newVSources;

			/*std::vector<size_t>::iterator iter;
			for (iter = mOldWallSlots.begin(); iter != mOldWallSlots.end(); ) {
				if (std::find(mWallsInUse.begin(), mWallsInUse.end(), *iter) == mWallsInUse.end())
				{
					mEmptyWallSlots.push_back(*iter);
					iter = mOldWallSlots.erase(iter);
				}
				else
					++iter;
			}*/
			// for (auto it : oldData)
			int j = 0;
			for (auto it = oldData.begin(); it != oldData.end(); it++, j++)
			{
				auto out = data.find(it->first);
				if (out == data.end()) // case: old vSource
				{
					it->second.Invisible();
					bool remove = UpdateVirtualSource(it->second, newVSources);
					if (remove)
					{
						keys.push_back(it->first);
					}
				}
			}

			for (auto key : keys)
			{
				oldData.erase(key);
			}

			// new or existing vSources
			for (auto it = data.begin(); it != data.end(); it++)
			{
				UpdateVirtualSource(it->second, newVSources);	// newVSources are new placeholders in the ISM tree
				oldData.insert_or_assign(it->first, it->second);

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
			std::mutex* m;

			if (data.IsReflection(0))
			{
				tempStore = &mVirtualSources;
				m = &vWallMutex;
			}
			else
			{
				tempStore = &mVirtualEdgeSources;
				m = &vEdgeMutex;
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
					m = &it->second.vWallMutex;
				}
				else
				{
					tempStore = &it->second.mVirtualEdgeSources;
					m = &it->second.vEdgeMutex;
				}
			}

			auto it = tempStore->find(data.GetID(orderIdx));
			if (it == tempStore->end())		// case: virtual source does not exist
			{
				int fdnChannel = -1;
				if (data.feedsFDN)
				{
					fdnChannel = (int)(freeFDNChannels.back() % mConfig.numFDNChannels);
					if (freeFDNChannels.size() > 1)
						freeFDNChannels.pop_back();
					else
						freeFDNChannels[0]++;
				}
				VirtualSource virtualSource = VirtualSource(mCore, mConfig, data, fdnChannel);
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
					fdnChannel = freeFDNChannels.back();
					if (freeFDNChannels.size() > 1)
						freeFDNChannels.pop_back();
					else
					{
						freeFDNChannels[0]++;
						if (freeFDNChannels[0] >= mConfig.numFDNChannels)
							freeFDNChannels[0] = 0;
					}
				}

				bool remove = it->second.UpdateVirtualSource(data, fdnChannel);

				if (fdnChannel >= 0) // Add vSource old fdnChannel to freeFDNChannels (Also prevents leaking FDN channels if !data.visible and the channel is not assigned to vSource)
					freeFDNChannels.push_back(it->second.GetFDNChannel());
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