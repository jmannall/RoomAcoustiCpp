
#include "Spatialiser/Context.h"
#include "Definitions.h";

using namespace Spatialiser;
using namespace Common;

#pragma region Background Thread

void BackgroundProcessor(Context* context)
{
	bool isRunning = context->IsRunning();

	HRTFManager* sources = context->GetHRTFManager();
	Room* room = context->GetRoom();

	Debug::Log("Begin background thread", Color::Green);
	while (isRunning)
	{
		//UPDATE_PROFILE_SECTION(
		//	{
		//		// Update ISM
		//		context->UpdateISM();

		//		isRunning = context->IsRunning();
		//	},
		//	"Time for Background Thread to Process");

		// Update ISM
		room->UpdateISM();

		isRunning = context->IsRunning();
		//Debug::Log("Running background thread", Color::Blue);
	}
	Debug::Log("End background thread", Color::Red);
}

#pragma endregion

#pragma region Context Init and Exit

Context::Context(const Config* config) : mIsRunning(true), mBackgroundProcessor(), count(0)
{
	Debug::Log("Init Context", Color::Green);
	// Copy config
	std::memcpy(&mConfig, config, sizeof(Config));

	// Set dsp settings
	mCore.SetAudioState({ mConfig.sampleRate, mConfig.bufferSize });
	mCore.SetHRTFResamplingStep(mConfig.hrtfResamplingStep);

	// Create listener
	mListener = mCore.CreateListener();

	string resourcePath = "D:\\Joshua Mannall\\GitHub\\3dti_AudioToolkit\\resources";
	string hrtfPath = "\\HRTF\\3DTI\\3DTI_HRTF_IRC1008_128s_48000Hz.3dti-hrtf";

	// Load binaural resources
	//hrtfLoaded = HRTF::CreateFrom3dti(mConfig.resourcePath + mConfig.hrtfPath, mListener);
	//ildLoaded = ILD::CreateFrom3dti_ILDNearFieldEffectTable(mConfig.resourcePath + mConfig.ildPath, mListener);

	hrtfLoaded = HRTF::CreateFrom3dti(resourcePath + hrtfPath, mListener);
	switch (mConfig.hrtfMode)
	{
		case HRTFMode::quality:
		{
			string ildPath = "\\ILD\\HRTF_ILD_48000.3dti-ild";
			ildLoaded = ILD::CreateFrom3dti_ILDSpatializationTable(resourcePath + ildPath, mListener);
			break;
		}
		case HRTFMode::performance:
		{
			string ildNearFieldPath = "\\ILD\\NearFieldCompensation_ILD_48000.3dti-ild";
			ildLoaded = ILD::CreateFrom3dti_ILDNearFieldEffectTable(resourcePath + ildNearFieldPath, mListener);
			break;
		}
		case HRTFMode::none:
			break;
	}

	// Add reverb to the init process then test tge audio processing and FDN all working
	// allocate memory all at once
	unsigned size =
		sizeof(Room) +			// room
		sizeof(Reverb) +		// reverb
		sizeof(HRTFManager);	// hrtf manager

	mMem = new char[size];
	if (!mMem)
	{
		//throw 0;
	}
	std::memset(mMem, 0, size);

	// place memory locations
	char* temp = mMem;
	mRoom = reinterpret_cast<Room*>(temp); temp += sizeof(Room);
	mReverb = reinterpret_cast<Reverb*>(temp); temp += sizeof(Reverb);
	mSources = reinterpret_cast<HRTFManager*>(temp); temp += sizeof(HRTFManager);

	vec dimensions = vec(12);		// Change to be determined by room dimensions.
	for (int i = 0; i < 12; i++)
	{
		dimensions[i] = 0.5f * (i + 1);
	}
	mRoom = new Room(config->maxRefOrder);
	mReverb = new Reverb(&mCore, mConfig.hrtfMode, dimensions, mConfig.sampleRate);
	mSources = new HRTFManager(&mCore, mReverb->NumChannels(), mConfig.hrtfMode, mConfig.sampleRate);

	// start background thread after all systems are initialized
	mBackgroundProcessor = std::thread(BackgroundProcessor, this);
}

Context::~Context()
{
	Debug::Log("Exit Context", Color::Red);

	StopRunning();
	mBackgroundProcessor.join();

	mSources->~HRTFManager();
	mReverb->~Reverb();
	mRoom->~Room();
	mCore.RemoveListener();

	// deallocate buffers
	DSP_SAFE_ARRAY_DELETE(mMem);
}

#pragma endregion

bool Context::FilesLoaded()
{
	if (hrtfLoaded && ildLoaded)
		return true;
	return false;
}

#pragma region Walls

size_t Context::InitWall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
{
	Debug::Log("Init Wall and Edges", Color::Green);
	Wall wall = Wall(normal, vData, numVertices, absorption);
	mReverb->UpdateReflectionFilters(reverbWall, absorption);
	return mRoom->AddWall(wall);
}

// assumes reverbWall never changes
void Context::UpdateWall(size_t id, const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
{
	Absorption oldAbsorption = mRoom->UpdateWall(id, normal, vData, numVertices, absorption);
	mReverb->UpdateReflectionFilters(reverbWall, absorption, oldAbsorption);
	// Update edges?
}

// assumes reverbWall never changes
void Context::RemoveWall(size_t id, const ReverbWall& reverbWall)
{
	Debug::Log("Remove Wall and Edges", Color::Red);
	Absorption absorption = mRoom->RemoveWall(id);
	absorption.area = -absorption.area;
	mReverb->UpdateReflectionFilters(reverbWall, absorption);
	// mSources->LogEdgeRemoval(ids);
	mSources->LogWallRemoval(id); // To remove virtual sources later - Include relative edges?
}

void Context::SetFDNParameters(const float& volume, const vec& dimensions)
{
	Debug::Log("Set FDN Reverb Time", Color::Green);
	FrequencyDependence T60 = mRoom->GetReverbTime(volume);
	float temp[5];
	T60.GetValues(&temp[0]);
	for (int i = 0; i < 5; i++)
	{
		Debug::Log("FDN T60: " + FloatToStr(temp[i]) + " I: " + IntToStr(i), Color::White);
	}
	mReverb->SetFDNParameters(T60, dimensions);
}

#pragma endregion

#pragma region Listener

void Context::UpdateListener(const vec3& position, const quaternion& orientation)
{
	mRoom->UpdateListenerPosition(position);

	// Set the listener position and orientation
	CTransform transform;
	transform.SetOrientation(CQuaternion(orientation.w, orientation.x, orientation.y, orientation.z));
	transform.SetPosition(CVector3(position.x, position.y, position.z));
	{
		lock_guard <mutex> lock(audioMutex);
		mListener->SetListenerTransform(transform);
	}
	mReverb->UpdateReverbSources(position);
	// Debug::Log("Listener position: " + VecToStr(position), Color::Yellow);
}

#pragma endregion

#pragma region Sources

size_t Context::InitSource()
{
	Debug::Log("Init Source", Color::Green);
	return mSources->Init();
}

void Context::UpdateSource(size_t id, const vec3& position, const quaternion& orientation)
{
	CTransform transform;
	transform.SetOrientation(CQuaternion(orientation.w, orientation.x, orientation.y, orientation.z));
	transform.SetPosition(CVector3(position.x, position.y, position.z));

	SourceData& data = mRoom->UpdateSourcePosition(id, position); // Also returns latest data
	mSources->Update(id, transform, data);
	// Debug::Log("Source position: " + VecToStr(position), Color::Yellow);
}

void Context::RemoveSource(size_t id)
{
	Debug::Log("Remove Source", Color::Red);
	mRoom->RemoveSourcePosition(id);
	mSources->Remove(id);
}

#pragma endregion

#pragma region Audio Proccessing

void Context::SubmitAudio(size_t id, const float* data, size_t numFrames)
{
	//mNumChannels = (int)CHANNEL_COUNT;
	mNumChannels = 2;
	size_t numSamples = numFrames * mNumChannels;
																// Currently reverbInput gets overwritten for each source.
	mReverbInput = matrix(numFrames, mReverb->NumChannels());	// Move these to initialise based on unity settings at context init();
	mOutputBuffer.ResizeBuffer(numSamples);

	// don't do anything if input is invalid
		
	mSources->ProcessAudio(id, data, numFrames, mReverbInput, mOutputBuffer);
}

void Context::GetOutput(float** bufferPtr)
{
	mReverb->ProcessAudio(mReverbInput, mOutputBuffer);
	mSendBuffer = mOutputBuffer;	// Streamline this, Audio system needs over haul to process at the source to apply absorption.
	*bufferPtr = &mSendBuffer[0];	// Add mutex to prevent outputBuffer readwrite issues? Change in unity to happen every source?

	mOutputBuffer.ResetBuffer();
}

#pragma endregion