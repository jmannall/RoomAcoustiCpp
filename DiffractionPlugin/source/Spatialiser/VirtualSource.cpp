
#include "Spatialiser/VirtualSource.h"

using namespace Spatialiser;

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

void VirtualSourceData::UpdateTransform(const vec3& vEdgeSourcePosition)
{
	transform.SetPosition(Common::CVector3(vEdgeSourcePosition.x, vEdgeSourcePosition.y, vEdgeSourcePosition.z));
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
	mRPositions = data.mRPositions;
	if (!reflection)
		reflection = data.reflection;

	Edge edge = mDiffractionPath.GetEdge();
	Diffraction::Path oldPath = mDiffractionPath;
	mDiffractionPath = Diffraction::Path(GetPosition(), data.GetRPosition(), edge);

	if (mDiffractionPath.valid) // Should be valid as previous paths were valid (unless apex has moved)
	{
		valid = true;
		if (mDiffractionPath.GetApex() == oldPath.GetApex() && mDiffractionPath.GetApex() == data.mDiffractionPath.GetApex())
		{
			vec3 vPosition = listenerPosition + (mDiffractionPath.sData.d + mDiffractionPath.rData.d) * UnitVector(data.GetTransformPosition() - listenerPosition);
			transform.SetPosition(Common::CVector3(vPosition.x, vPosition.y, vPosition.z));
			if (visible && data.visible)
				return false;
		}
		else
		{
			vec3 vPosition = listenerPosition + (mDiffractionPath.sData.d + mDiffractionPath.rData.d) * UnitVector(data.GetTransformPosition() - listenerPosition);
			transform.SetPosition(Common::CVector3(vPosition.x, vPosition.y, vPosition.z));
		}
	}
	else
		valid = false;
	return true;
}

VirtualSourceData VirtualSourceData::Trim(const int i)
{
	order = i + 1;

	if (!mPositions.empty())
		mPositions.erase(mPositions.begin() + order, mPositions.end());
	if (!mRPositions.empty())
		mRPositions.erase(mRPositions.begin() + order, mRPositions.end());
	if (!isReflection.empty())
		isReflection.erase(isReflection.begin() + order, isReflection.end());
	if (!IDs.empty())
		IDs.erase(IDs.begin() + order, IDs.end());

	feedsFDN = false;
	mFDNChannel = -1;

	Reset();

	reflection = false;
	diffraction = false;
	for (int i = 0; i < order; i++)
	{
		if (isReflection[i])
			reflection = true;
		else
			diffraction = true;
	}

	key = key.substr(0, 2 * order);

	return *this;
}


VirtualSource::VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs, const VirtualSourceData& data, const int& fdnChannel) : mCore(core), mSource(NULL), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(4, fs), isInitialised(false), mHRTFMode(hrtfMode), feedsFDN(false), mFDNChannel(fdnChannel), mDiffractionPath(data.mDiffractionPath), btm(&mDiffractionPath, fs), udfa(&mDiffractionPath, fs), reflection(false), diffraction(false)
{
	UpdateVirtualSource(data);
	// Set local bool for use at audio processing?
}

VirtualSource::VirtualSource(const VirtualSource& vS) : mCore(vS.mCore), mSource(vS.mSource), mPosition(vS.mPosition), mFilter(vS.mFilter), isInitialised(vS.isInitialised), mHRTFMode(vS.mHRTFMode), feedsFDN(vS.feedsFDN), mFDNChannel(vS.mFDNChannel), mDiffractionPath(vS.mDiffractionPath), btm(vS.btm), udfa(vS.udfa), mTargetGain(vS.mTargetGain), mCurrentGain(vS.mCurrentGain), reflection(vS.reflection), diffraction(vS.diffraction), mVirtualSources(vS.mVirtualSources), mVirtualEdgeSources(vS.mVirtualEdgeSources)
{
	btm.UpdatePath(&mDiffractionPath);
	udfa.UpdatePath(&mDiffractionPath);
}

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

bool VirtualSource::UpdateVirtualSource(const VirtualSourceData& data)
{
	if (data.visible) // Process virtual source - Init if doesn't exist
	{
		lock_guard<mutex> lock(audioMutex);
		mTargetGain = 1.0f;
		if (!isInitialised)
			Init(data);
		Update(data);
	}
	else
	{
		Debug::Log("vSource invisible, gain: " + FloatToStr(mCurrentGain), Color::Yellow);
		//lock_guard<mutex> lock(audioMutex);
		std::unique_lock<mutex> lock(audioMutex);
		mTargetGain = 0.0f;
		if (mCurrentGain < 0.0001)
		{
			if (isInitialised)
				Remove();
			lock.unlock();
			return true;
		}
	}
	return false;
}

// Obselete? now sources removed automatically
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

int VirtualSource::ProcessAudio(const float* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer, const float lerpFactor)
{
	int ret = 0;
	{
		lock_guard<mutex> lock(vWallMutex);
		// for (auto it : mVirtualSources)
		for (auto it = mVirtualSources.begin(); it != mVirtualSources.end(); it++)
		{
			ret += it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer, lerpFactor);
		}
	}
	{
		lock_guard<mutex> lock(vEdgeMutex);
		for (auto it = mVirtualEdgeSources.begin(); it != mVirtualEdgeSources.end(); it++)
		{
			ret += it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer, lerpFactor);
		}
	}

	ret++;
	if (isInitialised)
	{
		// Copy input into internal storage and apply wall absorption
		CMonoBuffer<float> bInput(numFrames);
		const float* inputPtr = data;

		if (diffraction)
		{
			{
				lock_guard<mutex> lock(audioMutex);
				btm.ProcessAudio(inputPtr, &bInput[0], numFrames, lerpFactor);
				// udfa.ProcessAudio(inputPtr, &bInput[0], numFrames, lerpFactor);
				if (reflection)
				{
					for (int i = 0; i < numFrames; i++)
					{
						bInput[i] = mFilter.GetOutput(bInput[i]);
					}
				}
			}
		}
		else if (reflection)
		{
			lock_guard<mutex> lock(audioMutex);
			for (int i = 0; i < numFrames; i++)
			{
				bInput[i] = mFilter.GetOutput(*inputPtr++);
			}
		}

		for (int i = 0; i < numFrames; i++)
		{
			bInput[i] *= mCurrentGain;
			if (mCurrentGain != mTargetGain)
				mCurrentGain = LERP_FLOAT(mCurrentGain, mTargetGain, lerpFactor);
		}
		
		mSource->SetBuffer(bInput);

		Common::CEarPair<CMonoBuffer<float>> bOutput;
		if (feedsFDN)
		{
			Debug::Log("Feeds FDN", Color::Yellow);

			CMonoBuffer<float> bMonoOutput;
			{
				lock_guard<mutex> lock(audioMutex);
				mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
			}
			for (int i = 0; i < numFrames; i++)
			{
				reverbInput.IncreaseEntry(bMonoOutput[i], i, mFDNChannel);
			}
		}
		else
		{
			lock_guard<mutex> lock(audioMutex);
			mSource->ProcessAnechoic(bOutput.left, bOutput.right);
		}
		int j = 0;
		for (int i = 0; i < numFrames; i++)
		{
			outputBuffer[j++] += bOutput.left[i];
			outputBuffer[j++] += bOutput.right[i];
		}
	}
	return ret;
}

void VirtualSource::Init(const VirtualSourceData& data)
{
	if (data.reflection) // Init reflection filter
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
	reflection = data.reflection;
	diffraction = data.diffraction;

	if (diffraction)
	{
		mDiffractionPath = data.mDiffractionPath;
		btm.UpdateParameters();
		udfa.UpdateParameters();
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