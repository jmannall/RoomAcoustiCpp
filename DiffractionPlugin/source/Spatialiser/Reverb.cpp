
#include "Spatialiser/Reverb.h"

using namespace Spatialiser;

Reverb::Reverb(Binaural::CCore* core, HRTFMode hrtfMode, const vec& dimensions, int fs) : mNumChannels(12), mFDN(mNumChannels, fs), mCore(core), mHRTFMode(hrtfMode), valid(false)
{
	InitSources(fs);
}

Reverb::Reverb(Binaural::CCore* core, HRTFMode hrtfMode, const vec& dimensions, const FrequencyDependence& T60, int fs) : mNumChannels(12), mFDN(T60, dimensions, mNumChannels, fs), mCore(core), mHRTFMode(hrtfMode), valid(false)
{
	InitSources(fs);
}

void Reverb::InitSources(int fs)
{
	lock_guard<mutex> lock(mCoreMutex);

	mReverbSources.reserve(mNumChannels);
	for (int i = 0; i < mNumChannels; i++)
	{
		ReverbSource temp = ReverbSource(mCore, mHRTFMode, fs);
		mReverbSources.push_back(temp);
		temp.Deactivate();
	}

	mReverbSources[0].SetShift(vec3(1, 1, 3));		// +z
	mReverbSources[1].SetShift(vec3(-1, -1, 3));	// +z
	mReverbSources[2].SetShift(vec3(-1, 1, -3));	// -z
	mReverbSources[3].SetShift(vec3(1, -1, -3));	// -z
	mReverbSources[4].SetShift(vec3(3, 1, 1));		// +x
	mReverbSources[5].SetShift(vec3(3, -1, -1));	// +x
	mReverbSources[6].SetShift(vec3(-3, -1, 1));	// -x
	mReverbSources[7].SetShift(vec3(-3, 1, -1));	// -x
	mReverbSources[8].SetShift(vec3(1, 3, 1));		// +y
	mReverbSources[9].SetShift(vec3(-1, 3, -1));	// +y
	mReverbSources[10].SetShift(vec3(-1, -3, 1));	// -y
	mReverbSources[11].SetShift(vec3(1, -3, -1));	// -y
}

void Reverb::UpdateReverbSources(const vec3& position)
{
	for (int i = 0; i < mNumChannels; i++)
	{
		lock_guard<mutex> lock(mCoreMutex);

		mReverbSources[i].Update(position);
	}
}

void Reverb::UpdateReflectionFilters(const ReverbWall& id, const Absorption& absorption, const Absorption& oldAbsorption)
{
	UpdateReflectionFilters(id, absorption * absorption.area - oldAbsorption * oldAbsorption.area);
}

void Reverb::UpdateReflectionFilters(const ReverbWall& id, const Absorption& absorption)
{
	switch (id)
	{
		case ReverbWall::posZ: // +z
		{
			mReverbSources[0].UpdateReflectionFilter(absorption);
			mReverbSources[1].UpdateReflectionFilter(absorption);
			break;
		}
		case ReverbWall::negZ: // -z
		{
			mReverbSources[2].UpdateReflectionFilter(absorption);
			mReverbSources[3].UpdateReflectionFilter(absorption);
			break;
		}
		case ReverbWall::posX: // +x
		{
			mReverbSources[4].UpdateReflectionFilter(absorption);
			mReverbSources[5].UpdateReflectionFilter(absorption);
			break;
		}
		case ReverbWall::negX: // -x
		{
			mReverbSources[6].UpdateReflectionFilter(absorption);
			mReverbSources[7].UpdateReflectionFilter(absorption);
			break;
		}
		case ReverbWall::posY: // +y
		{
			mReverbSources[8].UpdateReflectionFilter(absorption);
			mReverbSources[9].UpdateReflectionFilter(absorption);
			break;
		}
		case ReverbWall::negY: // -y
		{
			mReverbSources[10].UpdateReflectionFilter(absorption);
			mReverbSources[11].UpdateReflectionFilter(absorption);
			break;
		}
		case ReverbWall::none:
		{
			break;
		}
	}
}

void Reverb::ProcessAudio(const matrix& data, Buffer& outputBuffer)
{
	if (valid)
	{
		// Process FDN and save to buffer
		size_t numFrames = data.Rows();
		matrix input = matrix(numFrames, mNumChannels);
		for (int i = 0; i < numFrames; i++)
		{
			rowvec out = mFDN.GetOutput(data.GetRow(i));
			for (int j = 0; j < mNumChannels; j++)
			{
				float test = out[j];
				input.AddEntry(out[j], i, j);
			}
		}
		// Process buffer of each channel
		for (int j = 0; j < mNumChannels; j++)
		{
			lock_guard<mutex> lock(mCoreMutex);

			mReverbSources[j].ProcessAudio(input.GetColumn(j), numFrames, outputBuffer);
		}
	}
}

void Reverb::SetFDNParameters(const FrequencyDependence& T60, const vec& dimensions)
{
	mFDN.SetParameters(T60, dimensions);
	if (T60 < 20)
	{
		valid = true;
	}
	Debug::Log("Reverb: " + BoolToStr(valid), Color::Yellow);
}


ReverbSource::ReverbSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs) : mCore(core), mHRTFMode(hrtfMode), mReflectionFilter(4, fs), mAbsorption(1.0f, 1.0f, 1.0f, 1.0f, 1.0f)
{
	UpdateReflectionFilter();
	Init();
}

ReverbSource::~ReverbSource()
{
	if (mSource)
	{
		Debug::Log("Remove reverb source", Color::Red);
		mCore->RemoveSingleSourceDSP(mSource);
	}
	Debug::Log("Reverb source destructor", Color::White);
}

void ReverbSource::Init()
{
	Debug::Log("Init reverb source", Color::Green);

	// Initialise source to core
	mSource = mCore->CreateSingleSourceDSP();
	mSource->DisablePropagationDelay();
	mSource->DisableDistanceAttenuationAnechoic();
	mSource->DisableDistanceAttenuationSmoothingAnechoic();
	mSource->DisableNearFieldEffect();
	mSource->DisableFarDistanceEffect();

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
}

void ReverbSource::Update(const vec3& position)
{
	Common::CTransform transform;
	transform.SetPosition(Common::CVector3(position.x + mShift.x, position.y + mShift.y, position.z + mShift.z));
	mSource->SetSourceTransform(transform);
}

void ReverbSource::UpdateReflectionFilter()
{
	float g[5];
	mAbsorption.GetValues(&g[0]);
	for( int i = 0; i < 5; i++)
	{
		Debug::Log("Reflection filter absorption values: " + FloatToStr(g[i]), Color::White);
	}
	mReflectionFilter.UpdateParameters(ABSORPTION_FREQUENCIES, g);
}

void ReverbSource::UpdateReflectionFilter(const Absorption& absorption)
{
	assert(absorption.area != 0);
	if (mAbsorption.area > 0)
	{
		mAbsorption = mAbsorption * mAbsorption.area + absorption * absorption.area;
		if (mAbsorption.area != 0)
		{
			mAbsorption /= mAbsorption.area;
		}
	}
	else
	{
		mAbsorption = absorption;
	}
	UpdateReflectionFilter();
}

void ReverbSource::ProcessAudio(const float* data, const size_t& numFrames, Buffer& outputBuffer)
{
	// Copy input into internal storage and apply wall absorption
	CMonoBuffer<float> bInput(numFrames);
	const float* inputPtr = data;
	for (int i = 0; i < numFrames; i++)
	{
		bInput[i] = mReflectionFilter.GetOutput(*inputPtr++);
	}

	Common::CEarPair<CMonoBuffer<float>> bOutput;

	mSource->SetBuffer(bInput);

	mSource->ProcessAnechoic(bOutput.left, bOutput.right);

	int j = 0;
	for (int i = 0; i < numFrames; i++)
	{
		outputBuffer[j++] += bOutput.left[i];
		outputBuffer[j++] += bOutput.right[i];
	}
}
