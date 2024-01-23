/*
*
*  \Source class
*
*/

// Common headers
#include "Common/Types.h" 
#include "Common/Vec3.h"

// Spatialiser headers
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

		Source::Source(Binaural::CCore* core, const size_t& numFDNChannels, HRTFMode hrtfMode, int fs) : mCore(core), mNumFDNChannels(numFDNChannels), targetGain(0.0f), currentGain(0.0f), isVisible(false), mHRTFMode(hrtfMode), sampleRate(fs)
		{
			// Initialise source to core
			mSource = mCore->CreateSingleSourceDSP();

			//Select spatialisation mode
			switch (mHRTFMode)
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

		void Source::ProcessAudio(const Real* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer, const Real lerpFactor)
		{
			int counter = 0;
			{
				lock_guard<mutex> lock(vWallMutex);
				// for (auto it : mVirtualSources)
				for (auto it = mVirtualSources.begin(); it != mVirtualSources.end(); it++)
				{
					counter += it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer, lerpFactor);
				}
			}
			{
				lock_guard<mutex> lock(vEdgeMutex);
				for (auto it = mVirtualEdgeSources.begin(); it != mVirtualEdgeSources.end(); it++)
				{
					counter += it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer, lerpFactor);
				}
			}

#if DEBUG_AUDIO_THREAD
	Debug::Log("Total audio vSources: " + IntToStr(counter), Colour::Orange);
#endif

			if (isVisible || currentGain != 0.0f)
			{
				CEarPair<CMonoBuffer<float>> bOutput;

				// Copy input into internal storage
				CMonoBuffer<float> bInput(numFrames);
				const Real* inputPtr = data;

				{
					lock_guard<mutex> lock(audioMutex);
					if (currentGain == targetGain)
					{
						for (int i = 0; i < numFrames; i++)
							bInput[i] = static_cast<float>(*inputPtr++ * currentGain);
					}
					else
					{
						for (int i = 0; i < numFrames; i++)
						{
							bInput[i] = static_cast<float>(*inputPtr++ * currentGain);
							currentGain = Lerp(currentGain, targetGain, lerpFactor);
						}
					}
					mSource->SetBuffer(bInput);
					mSource->ProcessAnechoic(bOutput.left, bOutput.right);
				}

				int j = 0;
				for (int i = 0; i < numFrames; i++)
				{
					outputBuffer[j++] += bOutput.left[i];
					outputBuffer[j++] += bOutput.right[i];
				}
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

#if DEBUG_VIRTUAL_SOURCE
	Debug::Log("vSource: " + it->first, Colour::Yellow);
#endif
			}

			for (auto vSource : newVSources)
				oldData.insert({ vSource.GetKey(), vSource });

#if DEBUG_VIRTUAL_SOURCE
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
					lock_guard<mutex> lock(*m);
					auto newIt = tempStore->insert_or_assign(data.GetID(i), VirtualSource(mCore, mHRTFMode, sampleRate)); // feedsFDN always the highest order ism
					it = newIt.first;
					VirtualSourceData vSource = data;
					newVSources.push_back(vSource.Trim(i));
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
					fdnChannel = (int)(freeFDNChannels.back() % mNumFDNChannels);
					if (freeFDNChannels.size() > 1)
						freeFDNChannels.pop_back();
					else
						freeFDNChannels[0]++;
				}
				VirtualSource virtualSource = VirtualSource(mCore, mHRTFMode, sampleRate, data, fdnChannel);
				{
					lock_guard<mutex> lock(*m);
					tempStore->insert_or_assign(data.GetID(orderIdx), virtualSource);
				}
				virtualSource.Deactivate();
			}
			else
			{
				int fdnChannel = -1;
				if (data.feedsFDN && it->second.GetFDNChannel() < 0)
				{
					fdnChannel = (int)(freeFDNChannels.back() % mNumFDNChannels);
					if (freeFDNChannels.size() > 1)
						freeFDNChannels.pop_back();
					else
						freeFDNChannels[0]++;
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
							lock_guard<mutex> lock(*m);
							n = tempStore->erase(data.GetID(orderIdx));
						}

						if (n == 1) // Check vSource has been successfully erased
							return true;
					}
					else if (!(it->second.mVirtualSources.size() > 0 || it->second.mVirtualEdgeSources.size() > 0))
					{
						int n = 0;
						{
							lock_guard<mutex> lock(*m);
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