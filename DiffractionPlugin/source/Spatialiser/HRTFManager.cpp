
#include "Spatialiser/HRTFManager.h"

namespace Spatialiser
{
	size_t HRTFManager::Init()
	{
		lock_guard <mutex> lock(mCoreMutex);
		SourceNew source = SourceNew(mCore, mNumFDNChannels, mHRTFMode, sampleRate);
		if (!mEmptySlots.empty()) // Assign source to an existing ID
		{
			size_t next = mEmptySlots.back();
			mEmptySlots.pop_back();
			mSources.insert_or_assign(next, source);
			source.Deactivate();
			return next;
		}
		else // Create a new ID
		{
			size_t next = mSources.size();
			mSources.insert_or_assign(next, source);
			source.Deactivate();
			return next;
		}
	}

	/*vec3 HRTFManager::GetSourcePosition(shared_ptr<Binaural::CSingleSourceDSP> source)
	{
		Common::CTransform transform = source->GetCurrentSourceTransform();
		Common::CVector3 position = transform.GetPosition();
		return vec3(position.x, position.y, position.z);
	}*/

	void HRTFManager::ProcessAudio(const size_t& id, const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer)
	{
		lock_guard<mutex> lock(mCoreMutex);
		auto it = mSources.find(id);
		if (it == mSources.end())
		{
			// Source doesn't exist
		}
		else
		{
			it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer);
		}
	}

	SourceNew::SourceNew(Binaural::CCore* core, const size_t& numFDNChannels, HRTFMode hrtfMode, int fs) : mCore(core), mNumFDNChannels(numFDNChannels), isVisible(false), mHRTFMode(hrtfMode), sampleRate(fs)
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
		}
		mSource->EnablePropagationDelay();
		mSource->DisableFarDistanceEffect();

		ResetFDNSlots();
	}

	void SourceNew::ResetFDNSlots()
	{
		freeFDNChannels.reserve(1);
		std::fill_n(std::back_inserter(freeFDNChannels), 1, 0);
	}

	SourceNew::~SourceNew()
	{
		if (mSource)
		{
			Debug::Log("Remove source", Color::Red);
		}
		Debug::Log("Source destructor", Color::White);
		mCore->RemoveSingleSourceDSP(mSource);
		Reset();
	}

	void SourceNew::ProcessAudio(const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer)
	{
		for (int i = 0; i < mVirtualSources.size(); i++)
		{
			mVirtualSources[i].ProcessAudio(data, numFrames, reverbInput, outputBuffer);
		}
		for (int i = 0; i < mVirtualEdgeSources.size(); i++)
		{
			mVirtualEdgeSources[i].ProcessAudio(data, numFrames, reverbInput, outputBuffer);
		}

		if (isVisible)
		{
			// Copy input into internal storage
			CMonoBuffer<float> bInput(numFrames);
			const float* inputPtr = data;
			for (int i = 0; i < numFrames; i++)
			{
				bInput[i] = *inputPtr++;
			}
			mSource->SetBuffer(bInput);

			Common::CEarPair<CMonoBuffer<float>> bOutput;
			mSource->ProcessAnechoic(bOutput.left, bOutput.right);
			int j = 0;
			for (int i = 0; i < numFrames; i++)
			{
				outputBuffer[j++] += bOutput.left[i];
				outputBuffer[j++] += bOutput.right[i];
			}
		}
	}

	void SourceNew::Update(const Common::CTransform& transform, const SourceData& data)
	{
		mSource->SetSourceTransform(transform); // Could lock the source while being updated to prevent background thread changing the data?
		isVisible = data.visible;
		Debug::Log("Source visible: " + BoolToStr(isVisible), Color::Yellow);
		// A source cannot be removed while being updated. However, we do not want virtual 
		// sources changed by the background thread (due to the newly updated position)
		// as we traverse the virtual source tree. Therefore, make a copy.
		std::vector<VirtualSourceData> vData(data.vSources);
		RemoveVirtualSources();
		UpdateVirtualSources(vData);
	}

	void SourceNew::UpdateVirtualSources(const std::vector<VirtualSourceData>& data)
	{
		for (int i = 0; i < data.size(); i++)
		{
			UpdateVirtualSource(data[i]);
		}
	}

	void SourceNew::RemoveVirtualSources() // Add remove edge sources
	{
		if (!removedWalls.empty())
		{
			for (int i = 0; i < removedWalls.size(); i++)
			{
				mVirtualSources.erase(removedWalls[i]);
				for (auto it = mVirtualSources.begin(); it != mVirtualSources.end(); it++)
				{
					it->second.RemoveVirtualSources(removedWalls[i]);
				}
			}
			removedWalls.clear();
		}
	}

	void SourceNew::UpdateVirtualSource(const VirtualSourceData& data)
	{
		int orderIdx = data.GetOrder() - 1;

		VirtualSourceMap* tempStore;
		if (data.IsReflection(0))
			tempStore = &mVirtualSources;
		else
			tempStore = &mVirtualEdgeSources;

		for (int i = 0; i < orderIdx; i++)
		{
			auto it = tempStore->find(data.GetID(i));
			if (it == tempStore->end())		// case: source does not exist
			{
				// Source higher in the tree does not exist
				return;
			}
			else
			{
				if (data.IsReflection(i + 1))
					tempStore = &it->second.mVirtualSources;
				else
					tempStore = &it->second.mVirtualEdgeSources;
			}
		}
		auto it = tempStore->find(data.GetID(orderIdx));
		if (it == tempStore->end())		// case: source does not exist
		{
			// Virtual source does not exist in tree
			Debug::Log("Is visible: " + BoolToStr(data.visible), Color::Orange);
			Debug::Log("Is valid: " + BoolToStr(data.valid), Color::Orange);

			VirtualSource virtualSource = VirtualSource(mCore, mHRTFMode, sampleRate, data, freeFDNChannels.back() % mNumFDNChannels);
			if (freeFDNChannels.size() > 1)
				freeFDNChannels.pop_back();
			else
				freeFDNChannels[0]++;
			tempStore->insert_or_assign(data.GetID(orderIdx), virtualSource);
			virtualSource.Deactivate();
			return;
		}
		else
		{
			//Debug::Log("Update virtual source", Color::Orange);
			//Debug::Log("Is visible: " + BoolToStr(data.visible), Color::Orange);
			it->second.UpdateVirtualSource(data);
		}
	}
}