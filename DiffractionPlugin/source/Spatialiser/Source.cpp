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
				Debug::Log("High quality mode", Colour::Green);
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
				break;
			}
			case HRTFMode::performance:
			{
				Debug::Log("High performance mode", Colour::Green);
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			case HRTFMode::none:
			{
				Debug::Log("No spatialisation mode", Colour::Green);
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::NoSpatialization);
				break;
			}
			default:
			{
				Debug::Log("High performance mode (default)", Colour::Green);
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			}
			mSource->EnablePropagationDelay();

			ResetFDNSlots();
		}

		Source::~Source()
		{
			if (mSource)
			{
				Debug::Log("Remove source", Colour::Red);
			}
			Debug::Log("Source destructor", Colour::White);
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

			Debug::Log("Total audio vSources: " + IntToStr(counter), Colour::Yellow);

			if (isVisible || currentGain != 0.0f)
			{
				CEarPair<CMonoBuffer<float>> bOutput;

				// Copy input into internal storage
				CMonoBuffer<float> bInput(numFrames);
				const Real* inputPtr = data;

				{
					lock_guard<mutex> lock(audioMutex);
					for (int i = 0; i < numFrames; i++)
					{
						bInput[i] = static_cast<float>(*inputPtr++ * currentGain);
						if (currentGain != targetGain)
							currentGain = Lerp(currentGain, targetGain, lerpFactor);
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

			Debug::Log("Source visible: " + BoolToStr(data.visible), Colour::Yellow);

			// RemoveVirtualSources();
			UpdateVirtualSources(data.vSources);
		}

		void Source::UpdateVirtualSources(const VirtualSourceDataMap& data)
		{
			Debug::Log("Old data: " + IntToStr(oldData.size()));
			Debug::Log("Data: " + IntToStr(data.size()));

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
					Debug::Log("Remove " + it->first + ": " + BoolToStr(remove), Colour::White);
				}
			}

			for (auto key : keys)
			{
				oldData.erase(key);
			}

			Debug::Log("Erased old data: " + IntToStr(oldData.size()));

			// new or existing vSources
			for (auto it = data.begin(); it != data.end(); it++)
			{
				UpdateVirtualSource(it->second, newVSources);
				oldData.insert_or_assign(it->first, it->second);
				Debug::Log("Visible: " + it->first, Colour::Blue);
			}

			for (auto vSource : newVSources)
			{
				oldData.insert({ vSource.GetKey(), vSource });
				Debug::Log("Extra: " + vSource.GetKey(), Colour::Orange);
			}

			Debug::Log("Total vSources: " + IntToStr(oldData.size()));
		}

		// obselete? as vSources removed automatically is no longer exist.
		//void Source::RemoveVirtualSources() // Add remove edge sources
		//{
		//	if (!removedWalls.empty())
		//	{
		//		for (int i = 0; i < removedWalls.size(); i++)
		//		{
		//			mVirtualSources.erase(removedWalls[i]);
		//			for (auto it = mVirtualSources.begin(); it != mVirtualSources.end(); it++)
		//			{
		//				it->second.RemoveVirtualSources(removedWalls[i]);
		//			}
		//		}
		//		removedWalls.clear();
		//	}
		//}

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
				if (it == tempStore->end())		// case: source higher in the tree does not exist
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
			if (it == tempStore->end())		// case: source does not exist
			{
				// Virtual source does not exist in tree
				Debug::Log("Is visible: " + BoolToStr(data.visible), Colour::Orange);
				Debug::Log("Is valid: " + BoolToStr(data.valid), Colour::Orange);

				VirtualSource virtualSource = VirtualSource(mCore, mHRTFMode, sampleRate, data, (int)(freeFDNChannels.back() % mNumFDNChannels));
				if (freeFDNChannels.size() > 1)
					freeFDNChannels.pop_back();
				else
					freeFDNChannels[0]++;
				{
					lock_guard<mutex> lock(*m);
					tempStore->insert_or_assign(data.GetID(orderIdx), virtualSource);
				}
				virtualSource.Deactivate();
			}
			else
			{
				//Debug::Log("Update virtual source", Colour::Orange);
				//Debug::Log("Is visible: " + BoolToStr(data.visible), Colour::Orange);
				bool remove = it->second.UpdateVirtualSource(data);

				Debug::Log("To be removed : " + BoolToStr(remove), Colour::Yellow);
				if (remove)
				{
					if (it->second.GetFDNChannel() >= 0)
					{
						Debug::Log("FDN Channel : " + IntToStr(it->second.GetFDNChannel()), Colour::Yellow);

						freeFDNChannels.push_back(it->second.GetFDNChannel());
						int n = 0;
						{
							lock_guard<mutex> lock(*m);
							Debug::Log("Get mutex", Colour::White);
							n = tempStore->erase(data.GetID(orderIdx));
						}
						Debug::Log("Release mutex", Colour::White);

						if (n == 1)
							return true;
					}
					else if (!(it->second.mVirtualSources.size() > 0 || it->second.mVirtualEdgeSources.size() > 0))
					{
						Debug::Log("No vSources", Colour::Yellow);

						int n = 0;
						{
							lock_guard<mutex> lock(*m);
							Debug::Log("Get mutex", Colour::White);
							n = tempStore->erase(data.GetID(orderIdx));
						}
						Debug::Log("Release mutex", Colour::White);

						if (n == 1)
							return true;
					}
					else
						Debug::Log("Not removed: " + IntToStr(it->second.mVirtualSources.size() + it->second.mVirtualEdgeSources.size()) + " children", Colour::Yellow);
				}
			}
			return false;
		}
	}
}