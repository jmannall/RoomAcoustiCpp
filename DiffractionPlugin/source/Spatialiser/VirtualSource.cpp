
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

std::vector<size_t> VirtualSourceData::GetWallIDs() const
{
	std::vector<size_t> ret;
	for (int i = 0; i < order; i++)
	{
		if (isReflection[i])
		{
			ret.push_back(IDs[i]);
		}
	}
	return ret;
}

vec3 VirtualSourceData::GetPosition(int i) const
{
	assert(i < order);
	return mPositions[i];
}

vec3 VirtualSourceData::GetTransformPosition()
{
	Common::CVector3 position = transform.GetPosition();
	return vec3(position.x, position.y, position.z);
}

void VirtualSourceData::SetTransform(const vec3& vSourcePosition)
{
	transform.SetPosition(Common::CVector3(vSourcePosition.x, vSourcePosition.y, vSourcePosition.z));
	mPositions.push_back(vSourcePosition);
}

void VirtualSourceData::SetTransform(const vec3& vSourcePosition, const vec3& vEdgeSourcePosition)
{
	transform.SetPosition(Common::CVector3(vEdgeSourcePosition.x, vEdgeSourcePosition.y, vEdgeSourcePosition.z));
	mPositions.push_back(vSourcePosition);
}

vec3 VirtualSourceData::GetRPosition(int i) const
{
	assert(i < order);
	return mRPositions[i];
}

void VirtualSourceData::SetRPosition(const vec3& vReceiverPosition)
{
	mRPositions.push_back(vReceiverPosition);
}

void VirtualSourceData::SetRTransform(const vec3& vReceiverPosition, const vec3& vEdgeSourcePosition)
{
	transform.SetPosition(Common::CVector3(vEdgeSourcePosition.x, vEdgeSourcePosition.y, vEdgeSourcePosition.z));
	mRPositions.push_back(vReceiverPosition);
}

// Append EdSp to a SpEd source
bool VirtualSourceData::AppendVSource(VirtualSourceData& data, const vec3& listenerPosition)
{
	assert((IsReflection(order - 1) && data.IsReflection(0)) == false);
	assert(GetID() == data.GetID(0)); // last ref or diffraction was the same

	if (data.reflection)
	{
		Absorption absorption;
		data.GetAbsorption(absorption);
		AddWallIDs(data.GetWallIDs(), absorption);
	}
	order += data.GetOrder();
	mRPositions = data.mRPositions;
	if (!reflection)
		reflection = data.reflection;

	Edge edge = mDiffractionPath.GetEdge();
	Diffraction::Path path = Diffraction::Path(GetPosition(), data.GetRPosition(), edge);

	vec3 vPosition = listenerPosition + (path.sData.d + path.rData.d) * UnitVector(data.GetTransformPosition() - listenerPosition);
	transform.SetPosition(Common::CVector3(vPosition.x, vPosition.y, vPosition.z));

	if (path.valid) // Should be valid as previous paths were valid (unless apex has moved)
	{
		valid = true;
		if (path.GetApex() == mDiffractionPath.GetApex() && path.GetApex() == data.mDiffractionPath.GetApex())
		{
			if (visible)
				visible = data.visible;
			return false;
		}
		return true;
	}
}

VirtualSource::VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs, const VirtualSourceData& data, const int& fdnChannel) : mCore(core), mSource(NULL), mFilter(4, fs), mHPF(20.0f, FilterShape::hpf, fs), isInitialised(false), mHRTFMode(hrtfMode), feedsFDN(false), mFDNChannel(fdnChannel), mDiffractionPath(data.mDiffractionPath), btm(&mDiffractionPath, fs), reflection(false), diffraction(false)
{
	if (data.reflection)
	{
		float fc[] = { 250, 500, 1000, 2000, 4000 };
		float g[5];
		data.GetAbsorption(g);
		mFilter.UpdateParameters(fc, g);
		reflection = true;
	}
	if (data.diffraction)
	{
		// BTM filter already init
		diffraction = true;
	}
	feedsFDN = data.feedsFDN;
	UpdateVirtualSource(data);
	// Set local bool for use at audio processing?
}

VirtualSource::VirtualSource(const VirtualSource& vS) : mCore(vS.mCore), mSource(vS.mSource), mPosition(vS.mPosition), mFilter(vS.mFilter), mHPF(vS.mHPF), isInitialised(vS.isInitialised), mHRTFMode(vS.mHRTFMode), feedsFDN(vS.feedsFDN), mFDNChannel(vS.mFDNChannel), mDiffractionPath(vS.mDiffractionPath), btm(vS.btm), reflection(vS.reflection), diffraction(vS.diffraction), mVirtualSources(vS.mVirtualSources), mVirtualEdgeSources(vS.mVirtualEdgeSources)
{
	btm.UpdatePath(&mDiffractionPath);
}

void VirtualSource::UpdateVirtualSource(const VirtualSourceData& data)
{
	//if (data.valid)
	//{
	//	if (data.visible) // Process virtual source - Init if doesn't exist
	//	{
	//		if (!isInitialised)
	//			Init();
	//		Update(data);
	//	}
	//	else // Check if in core - remove if is
	//	{
	//		if (isInitialised)
	//			Remove();
	//	}
	//}
	//else // Check if init - remove if is
	//{
	//	if (isInitialised)
	//		Remove();
	//	if (!mVirtualSources.empty()) // Delete any children
	//		Reset();
	//	if (!mVirtualEdgeSources.empty()) // Delete any children
	//		Reset();
	//}

	if (data.visible) // Process virtual source - Init if doesn't exist
	{
		if (!isInitialised)
			Init();
		Update(data);
	}
	else // Check if in core - remove if is
	{
		if (isInitialised)
			Remove();
		if (!data.valid)
			Reset();	// Delete any child virtual sources
	}
}

void VirtualSource::RemoveVirtualSources(const size_t& id)
{
	if (!mVirtualSources.empty())
	{
		mVirtualSources.erase(id);
		for (auto it = mVirtualSources.begin(); it != mVirtualSources.end(); it++)
		{
			it->second.RemoveVirtualSources(id);
		}
	}
}

void VirtualSource::ProcessAudio(const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer)
{
	for (int i = 0; i < mVirtualSources.size(); i++)
	{
		mVirtualSources[i].ProcessAudio(data, numFrames, reverbInput, outputBuffer);
	}
	for (int i = 0; i < mVirtualEdgeSources.size(); i++)
	{
		mVirtualEdgeSources[i].ProcessAudio(data, numFrames, reverbInput, outputBuffer);
	}

	if (isInitialised)
	{
		// Copy input into internal storage and apply wall absorption
		CMonoBuffer<float> bInput(numFrames);
		const float* inputPtr = data;
		if (diffraction)
		{
			btm.ProcessAudio(inputPtr, &bInput[0], numFrames, 1.0f / (numFrames * 2.0f));
			if (reflection)
			{
				for (int i = 0; i < numFrames; i++)
				{
					bInput[i] = mFilter.GetOutput(bInput[i]);
				}
			}
		}
		else if (reflection)
		{
			for (int i = 0; i < numFrames; i++)
			{
				bInput[i] = mFilter.GetOutput(*inputPtr++);
			}
		}
		
		/*for (int i = 0; i < numFrames; i++)
		{
			bInput[i] = mHPF.GetOutput(bInput[i]);
		}*/
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
	mSource->DisableFarDistanceEffect();

	isInitialised = true;
}

void VirtualSource::Update(const VirtualSourceData& data)
{
	if (diffraction)
	{
		mDiffractionPath = data.mDiffractionPath;
		btm.UpdateParameters();
	}
	mPosition = data.GetPosition();
	mSource->SetSourceTransform(data.transform);
}


void VirtualSource::Remove()
{
	Debug::Log("Remove virtual source", Color::Red);

	mCore->RemoveSingleSourceDSP(mSource);

	isInitialised = false;
}