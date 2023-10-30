
#include "Spatialiser/VirtualSource.h"

using namespace Spatialiser;

VirtualSource::~VirtualSource()
{
	if (mSource)
	{
		Debug::Log("Remove virtual source", Color::Red);
	}
	Debug::Log("Virtual source destructor", Color::White);
	mCore->RemoveSingleSourceDSP(mSource);
	Reset();
};

void VirtualSourceData::SetTransform(const vec3& pos)
{
	if (visible)
	{
		transform.SetPosition(Common::CVector3(pos.x, pos.y, pos.z));
	}
	mPositions.push_back(pos);
}

vec3 VirtualSourceData::GetPosition(int i) const
{
	if (i < reflectionOrder)
	{
		return mPositions[i];
	}
	else
	{
		return vec3();
	}
}

VirtualSource::VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs, const VirtualSourceData& data, const int& fdnChannel) : mCore(core), mSource(NULL), mFilter(4, fs), isInitialised(false), mHRTFMode(hrtfMode), feedsFDN(false), mFDNChannel(fdnChannel)
{
	// mReflectionOrder = data.GetReflectionOrder();
	/*for (int i = 0; i < mReflectionOrder; i++)
	{
		wallIDs.push_back(data.GetWallID(i));
	}*/
	//InitFilter(data.GetAbsorption());
	float fc[] = { 250, 500, 1000, 2000, 4000 };
	float g[5];
	data.GetAbsorption(g);
	mFilter.UpdateParameters(fc, g);
	UpdateVirtualSource(data);
}

void VirtualSource::UpdateVirtualSource(const VirtualSourceData& data)
{
	if (data.valid)
	{
		if (data.visible)
		{
			// Process virtual source - Init if doesn't exist
			if (!isInitialised)
			{
				Init();
			}
			Update(data);
		}
		else
		{
			// Check if in core - remove if is
			if (isInitialised)
			{
				Remove();
			}
		}
	}
	else
	{
		// Check if init - remove if is
		if (isInitialised)
		{
			Remove();
		}
		// Delete any children
		if (!mChildren.empty())
		{
			Reset();
		}
	}
}

void VirtualSource::RemoveVirtualSources(const size_t& id)
{
	if (!mChildren.empty())
	{
		mChildren.erase(id);
		for (auto it = mChildren.begin(); it != mChildren.end(); it++)
		{
			it->second.RemoveVirtualSources(id);
		}
	}
}

void VirtualSource::ProcessAudio(const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer)
{
	for (int i = 0; i < mChildren.size(); i++)
	{
		mChildren[i].ProcessAudio(data, numFrames, reverbInput, outputBuffer);
	}

	if (isInitialised)
	{
		// Copy input into internal storage and apply wall absorption
		CMonoBuffer<float> bInput(numFrames);
		const float* inputPtr = data;
		for (int i = 0; i < numFrames; i++)
		{
			bInput[i] = mFilter.GetOutput(*inputPtr++);
		}
		mSource->SetBuffer(bInput);

		Common::CEarPair<CMonoBuffer<float>> bOutput;
		if (feedsFDN)
		{
			CMonoBuffer<float> bMonoOutput;
			mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
			for (int i = 0; i < numFrames; i++)
			{
				reverbInput.IncreaseEntry(bMonoOutput[i], i, mFDNChannel);
			}
		}
		else
		{
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

void VirtualSource::Init()
{
	Debug::Log("Init virtual source", Color::Green);

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

	isInitialised = true;
}

void VirtualSource::Update(const VirtualSourceData& data)
{ 
	feedsFDN = data.feedsFDN;
	mPosition = data.GetPosition();
	mSource->SetSourceTransform(data.transform);
}


void VirtualSource::Remove()
{
	Debug::Log("Remove virtual source", Color::Red);

	mCore->RemoveSingleSourceDSP(mSource);

	isInitialised = false;
}