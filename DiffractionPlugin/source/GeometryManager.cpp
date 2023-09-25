
#include "GeometryManager.h"
#include <chrono>

// Geometry singleton ptr
static Geometry* geometry = nullptr;

Geometry* GetGeometry()
{
	return geometry;
}

#pragma region ClientInterface
namespace GA
{
	#pragma region Global Geometry
	void InitGeometry(const DSPConfig* config) {
		if (geometry)
		{
			ExitGeometry();
		}
		geometry = new Geometry(config); 
	}

	void ExitGeometry()
	{ 
		if (geometry)
		{
			delete geometry;
			geometry = nullptr;
		}
	}

	// sets global listener position
	void SetListenerPosition(const vec3& listenerPosition)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			geometry->SetListenerPosition(listenerPosition);
	}

	void SetModel(const Model& model)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			geometry->SetModel(model);
	}
	#pragma endregion


	//void UpdateEmissionData(size_t id)
	//{
	//	auto* geometry = GetGeometry();
	//	if (geometry)
	//		geometry->UpdateEmissionData(id);
	//}

	
	#pragma region Sources
	size_t InitSource(const vec3& position)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			return geometry->InitSource(position);
		return (size_t)(-1);
	}

	void RemoveSource(size_t id)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			geometry->RemoveSource(id);
	}

	void UpdateSourceData(size_t id, const vec3& position)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			geometry->UpdateSourceData(id, position);
	}
	#pragma endregion

	#pragma region Wedges
	size_t InitWedge(const Wedge& wedge)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			return geometry->InitWedge(wedge);
		return (size_t)(-1);
	}

	void RemoveWedge(size_t id)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			geometry->RemoveWedge(id);
	}

	void UpdateWedgeData(size_t id, const Wedge& wedge)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			geometry->UpdateWedgeData(id, wedge);
	}

	void UpdatePaths()
	{
		auto* geometry = GetGeometry();
		if (geometry)
			geometry->UpdatePaths();
	}

	float GetSd(size_t sID, size_t wID)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			return geometry->GetSd(sID, wID);
		else
			return -1;
	}

	float GetRd(size_t sID, size_t wID)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			return geometry->GetRd(sID, wID);
		else
			return -1;
	}

	float GetZ(size_t sID, size_t wID)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			return geometry->GetZ(sID, wID);
		else
			return -1;
	}
	#pragma endregion

	#pragma region Audio
	void SendAudio(size_t sID, size_t wID, const float* in, unsigned numFrames)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			geometry->SubmitAudio(sID, wID, in, numFrames);
	}

	void GetOutput(float** buffer)
	{
		auto* geometry = GetGeometry();
		if (geometry)
			geometry->GetOutput(buffer);
		else
			*buffer = nullptr;
	}
	#pragma endregion
}
#pragma endregion

void BackgroundProcessor(Geometry* geometry)
{
	bool isRunning = geometry->IsRunning();
	
	WedgeManager* wedges = geometry->GetWedgeManager();
	SourceManager* sources = geometry->GetSourceManager();
	EmissionManager* emissions = geometry->GetEmissionManager();
	PathManager* paths = geometry->GetPathManager();

	while (isRunning)
	{
		//PROFILE_SECTION(
		//	{
		//		PROFILE_TIME(paths->UpdatePaths(), "Paths");

		//		// Update geometry changes

		//		isRunning = geometry->IsRunning();
		//	},
		//	"Time for Background Thread to Process");

		paths->UpdatePaths();
		isRunning = geometry->IsRunning();
	}
}

#pragma region GeometryClass

Geometry::Geometry(const DSPConfig* config) : mModel(Model::attenuate), mIsRunning(true), mListener(0, 0, 0), mBackgroundProcessor()
{
	// copy the config
	std::memcpy(&mConfig, config, sizeof(DSPConfig));
	std::cout << "samplingRate: " << mConfig.samplingRate << "\n";
	std::cout << "dspSmoothingFactor: " << mConfig.dspSmoothingFactor << "\n";

	// find buffer size in bytes
	mBufferSize = sizeof(Buffer);

	// allocate memory all at once
	unsigned size =
		sizeof(SourceManager) +		// emissions manager
		sizeof(WedgeManager) +		// wedge manager
		sizeof(EmissionManager) +	// emission manager
		sizeof(PathManager);		// path manager

	mMem = new char[size];
	if (!mMem)
	{
		//throw 0;
	}
	std::memset(mMem, 0, size);

	// place memory locations
	char* temp = mMem;
	mSources = reinterpret_cast<SourceManager*>(temp); temp += sizeof(SourceManager);
	mWedges = reinterpret_cast<WedgeManager*>(temp); temp += sizeof(WedgeManager);
	mEmissions = reinterpret_cast<EmissionManager*>(temp); temp += sizeof(EmissionManager);
	mPaths = reinterpret_cast<PathManager*>(temp); temp += sizeof(PathManager);

	mSources = new (mSources) SourceManager();
	mWedges = new (mWedges) WedgeManager();
	mEmissions = new (mEmissions) EmissionManager(mConfig.samplingRate);
	mPaths = new (mPaths) PathManager(mSources, &mListener, mWedges, mEmissions);

	mEmissions->SetPathManager(mPaths);

	myNN_initialize();

	// start background thread after all systems are initialized
	mBackgroundProcessor = std::thread(BackgroundProcessor, this);
}

Geometry::~Geometry()
{
	StopRunning();
	mBackgroundProcessor.join();

	// call dtor on all systems in reverse order
	mPaths->~PathManager();
	mEmissions->~EmissionManager();
	mWedges->~WedgeManager();
	mSources->~SourceManager();

	myNN_terminate();

	// deallocate buffers
	DSP_SAFE_ARRAY_DELETE(mMem);
}

#pragma region Audio
void Geometry::SubmitAudio(size_t sID, size_t wID, const float* data, unsigned numFrames)
{
	mNumChannels = (int)CHANNEL_COUNT;
	int numSamples = numFrames * mNumChannels;

	if ((int)numFrames > mNumFrames)
	{
		mNumFrames = (int)numFrames;
		mInputBuffer.ResizeBuffer((size_t)numFrames);
		mAttenuateBuffer.ResizeBuffer((size_t)numFrames);
		mOffBuffer.ResizeBuffer((size_t)numFrames);
		mLpfBuffer.ResizeBuffer((size_t)numFrames);
		mUdfaBuffer.ResizeBuffer((size_t)numFrames);
		mUdfaiBuffer.ResizeBuffer((size_t)numFrames);
		mNNBestBuffer.ResizeBuffer((size_t)numFrames);
		mNNSmallBuffer.ResizeBuffer((size_t)numFrames);
		mUtdBuffer.ResizeBuffer((size_t)numFrames);
		mBtmBuffer.ResizeBuffer((size_t)numFrames);
		mSendBuffer.ResizeBuffer((size_t)numSamples);
	}
	mOutputBuffer.ResizeBuffer((size_t)numSamples);
	
	// don't do anything if input is invalid
	
	float lerpFactor = 1.f / ((float)mNumFrames * (float)mConfig.dspSmoothingFactor);
	
	//auto& emissionData = mEmissions.GetDataTarget(id);
	//emissionData.attenuation = dspParams->attenuation;
	//emissionData.directionX = dspParams->directionX;
	//emissionData.directionY = dspParams->directionY;
	//emissionData.directionZ = dspParams->directionZ;
	//float targetGain = emissionData.attenuation;

	//auto& currentData = mEmissions.GetDataCurrent(id);
	//float currentGain = currentData.attenuation;

	// Copy input into internal storage
	const float* inputPtr = data;
	for (int i = 0; i < mNumFrames; i++)
	{
		mInputBuffer[i] = *inputPtr++;
	}

	// Apply gain
	//for (int i = 0; i < mNumFrames; i++)
	//{
	//	float nextGain = currentGain;
	//	mBuffer[i] *= nextGain;

	//	currentGain = LERP_FLOAT(currentGain, targetGain, lerpFactor);
	//}

	// Apply stereo processing

	auto& emission = mEmissions->GetDSPParameters(sID, wID);
	
	AUDIO_PROFILE_SECTION(
		{
			AUDIO_PROFILE_TIME(emission.attenuate.ProcessAudio(&mInputBuffer[0], &mAttenuateBuffer[0], numFrames, lerpFactor), "Attenuate");
			AUDIO_PROFILE_TIME(emission.lpf.ProcessAudio(&mInputBuffer[0],  &mLpfBuffer[0], numFrames, lerpFactor), "LPF");
			AUDIO_PROFILE_TIME(emission.udfa.ProcessAudio(&mInputBuffer[0], &mUdfaBuffer[0], numFrames, lerpFactor), "UDFA");
			AUDIO_PROFILE_TIME(emission.udfai.ProcessAudio(&mInputBuffer[0], &mUdfaiBuffer[0], numFrames, lerpFactor), "UDFAI");
			AUDIO_PROFILE_TIME(emission.nnBest.ProcessAudio(&mInputBuffer[0], &mNNBestBuffer[0], numFrames, lerpFactor), "NNBest");
			AUDIO_PROFILE_TIME(emission.nnSmall.ProcessAudio(&mInputBuffer[0], &mNNSmallBuffer[0], numFrames, lerpFactor), "NNSmall");
			AUDIO_PROFILE_TIME(emission.utd.ProcessAudio(&mInputBuffer[0], &mUtdBuffer[0], numFrames, lerpFactor), "UTD");
			AUDIO_PROFILE_TIME(emission.btm.ProcessAudio(&mInputBuffer[0], &mBtmBuffer[0], numFrames, lerpFactor), "BTM");
		},
		"Time for Processing Audio");

	switch (mModel) {
	case Model::attenuate:
			SetOutputBuffer(&mAttenuateBuffer[0]);
			break;
		case Model::off:
			SetOutputBuffer(&mOffBuffer[0]);
			break;
		case Model::lowPass:
			SetOutputBuffer(&mLpfBuffer[0]);
			break;
		case Model::udfa:
			SetOutputBuffer(&mUdfaBuffer[0]);
			break;
		case Model::udfai:
			SetOutputBuffer(&mUdfaiBuffer[0]);
			break;
		case Model::nnBest:
			SetOutputBuffer(&mNNBestBuffer[0]);
			break;
		case Model::nnSmall:
			SetOutputBuffer(&mNNSmallBuffer[0]);
			break;
		case Model::utd:
			SetOutputBuffer(&mUtdBuffer[0]);
			break;
		case Model::btm:
			SetOutputBuffer(&mBtmBuffer[0]);
			break;
	};
}

void Geometry::SetOutputBuffer(float* buffer)
{
	for (int i = 0; i < mNumFrames; i++)
	{
		for (int j = 0; j < mNumChannels; j++)
		{
			mOutputBuffer[i * mNumChannels + j] += buffer[i];
		}
	}
}

void Geometry::GetOutput(float** bufferPtr)
{
	mSendBuffer = mOutputBuffer;
	*bufferPtr = &mSendBuffer[0];

	mOutputBuffer.ResetBuffer();
}
#pragma endregion

#pragma region Sources
void Geometry::UpdateSourceData(size_t id, const vec3& position)
{
	Source& source = mSources->GetData(id);
	source.position = position;
}
#pragma endregion

#pragma region Wedges
void Geometry::UpdateWedgeData(size_t id, const Wedge& newWedge)
{
	Wedge& wedge = mWedges->GetData(id);
	wedge = newWedge;
}

float Geometry::GetZ(size_t sID, size_t wID)
{
	return mPaths->GetData(sID, wID).zA / mWedges->GetData(wID).zW;
}
#pragma endregion

void Geometry::UpdatePaths()
{
	mPaths->UpdatePaths();
}

float Geometry::GetSd(size_t sID, size_t wID)
{
	return mPaths->GetData(sID, wID).sData.d;
}

float Geometry::GetRd(size_t sID, size_t wID)
{
	return mPaths->GetData(sID, wID).rData.d;
}

#pragma endregion

