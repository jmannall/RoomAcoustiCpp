/*
*
*  \Spatialiser context
*
*/

// Spatialiser headers
#include "Spatialiser/Context.h"
#include "Spatialiser/Types.h"

// Unity headers
#include "Unity/Debug.h"
#include "Unity/Profiler.h"

namespace UIE
{
	using namespace Unity;
	namespace Spatialiser
	{

#ifndef DISABLE_SOFA_SUPPORT
#if defined(UNITY_WIN) || (defined(TARGET_OS_OSX) && !defined(TARGET_OS_IOS))
#define ENABLE_SOFA_SUPPORT
#endif
#endif

		//////////////////// Background Thread ////////////////////

		void BackgroundProcessor(Context* context)
		{
#ifdef DEBUG_ISM_THREAD
	Debug::Log("Begin background thread", Colour::Green);
#endif

			bool isRunning = context->IsRunning();
			Room* room = context->GetRoom();

			while (isRunning)
			{
				// Update ISM Config
				room->UpdateISMConfig(context->GetISMConfig());

				// Update ISM
				room->UpdateISM();

				isRunning = context->IsRunning();
			}

#ifdef DEBUG_ISM_THREAD
	Debug::Log("End background thread", Colour::Red);
#endif
		}

		//////////////////// Context ////////////////////

		// Load and Destroy

		Context::Context(const Config* config, const std::vector<std::string>& filePaths) : mIsRunning(true), ISMThread()
		{
#ifdef DEBUG_INIT
	Debug::Log("Init Context", Colour::Green);
#endif

			// Copy config
			std::memcpy(&mConfig, config, sizeof(Config));

			// Set dsp settings
			mCore.SetAudioState({ mConfig.fs, mConfig.numFrames });
			mCore.SetHRTFResamplingStep(mConfig.hrtfResamplingStep);

			// Create listener
			mListener = mCore.CreateListener();

#ifdef DEBUG_HRTF
	Debug::Log("HRTF file path: " + filePaths[0] + filePaths[1]);
	Debug::Log("ILD file path: " + filePaths[0] + filePaths[2]);
#endif

			// Load HRTF files
			bool hrtfLoaded = HRTF::CreateFrom3dti(filePaths[0] + filePaths[1], mListener);
			bool ildLoaded = false;

			string mode;
			if (hrtfLoaded)
			{
				switch (mConfig.hrtfMode)
				{
				case HRTFMode::quality:
				{ 
					ildLoaded = ILD::CreateFrom3dti_ILDSpatializationTable(filePaths[0] + filePaths[2], mListener);
					mode = "quality"; break;
				}
				case HRTFMode::performance:
				{
					ildLoaded = ILD::CreateFrom3dti_ILDNearFieldEffectTable(filePaths[0] + filePaths[2], mListener);
					mode = "performance"; break;
				}
				case HRTFMode::none:
					break;
				default:
				{ 
					mConfig.hrtfMode = HRTFMode::none; break;
				}
				}
			}
			else
				mConfig.hrtfMode = HRTFMode::none;

#ifdef DEBUG_HRTF
	if (mConfig.hrtfMode == HRTFMode::none)
		Debug::Log("Spatialisation set to none", Colour::Green);
	else if (ildLoaded)
	{
		Debug::Log("HRTF files loaded successfully", Colour::Green);
		Debug::Log("Spatialisation set to " + mode, Colour::Green);
	}
	else
	{
		Debug::Log("Failed to load HRTF files", Colour::Red);
		Debug::Log("Spatialisation set to none", Colour::Green);
	}
#endif

			// TO DO: Add reverb to the init process then test the audio processing and FDN all working

			// Allocate memory all at once
			unsigned size =
				sizeof(Room) +			// room
				sizeof(Reverb) +		// reverb
				sizeof(SourceManager);	// hrtf manager

			mMem = new char[size];
			if (!mMem)
			{
				//throw 0;
			}
			std::memset(mMem, 0, size);

			// Place memory locations
			char* temp = mMem;
			mRoom = reinterpret_cast<Room*>(temp); temp += sizeof(Room);
			mReverb = reinterpret_cast<Reverb*>(temp); temp += sizeof(Reverb);
			mSources = reinterpret_cast<SourceManager*>(temp); temp += sizeof(SourceManager);

			mRoom = new Room();
			mReverb = new Reverb(&mCore, mConfig.hrtfMode, vec(mConfig.numFDNChannels), mConfig.fs);
			mSources = new SourceManager(&mCore, mConfig.numFDNChannels, mConfig.hrtfMode, mConfig.fs);

			// Start background thread after all systems are initialized
			ISMThread = std::thread(BackgroundProcessor, this);

			mReverbInput = matrix(mConfig.numFrames, mConfig.numFDNChannels);	// Move these to initialise based on unity settings at context init();
			mOutputBuffer = Buffer(mConfig.numFrames * mConfig.numChannels);
		}

		Context::~Context()
		{
#ifdef DEBUG_REMOVE
	Debug::Log("Exit Context", Colour::Red);
#endif

			StopRunning();
			ISMThread.join();

			mSources->~SourceManager();
			mReverb->~Reverb();
			mRoom->~Room();
			mCore.RemoveListener();

			// Deallocate buffers
			DSP_SAFE_ARRAY_DELETE(mMem);
		}

		// Reverb

		void Context::SetFDNParameters(const Real& volume, const vec& dimensions)
		{
			FrequencyDependence T60 = mRoom->GetReverbTime(volume);
			mReverb->SetFDNParameters(T60, dimensions);
		}

		// Listener

		void Context::UpdateListener(const vec3& position, const vec4& orientation)
		{
#if DEBUG_UPDATE
	Debug::Log("Update Listener", Colour::Yellow);
#endif

			mRoom->UpdateListenerPosition(position);

			// Set listener position and orientation
			CTransform transform;
			transform.SetOrientation(CQuaternion(static_cast<float>(orientation.w), static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)));
			transform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));
			{
				lock_guard <mutex> lock(audioMutex);
				mListener->SetListenerTransform(transform);
			}
			mReverb->UpdateReverbSources(position);
		}

		// Source

		size_t Context::InitSource()
		{
#ifdef DEBUG_INIT
	Debug::Log("Init Source", Colour::Green);
#endif

			return mSources->Init();
		}

		void Context::UpdateSource(size_t id, const vec3& position, const vec4& orientation)
		{
#ifdef DEBUG_UPDATE
	Debug::Log("Update Source", Colour::Yellow);
#endif

			// Update source position in background thread and
			// return a copy of all virtual sources
			SourceData data = mRoom->UpdateSourcePosition(id, position);

			// Update source position, orientation and virtual sources
			CTransform transform;
			transform.SetOrientation(CQuaternion(static_cast<float>(orientation.w), static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)));
			transform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));
			
			mSources->Update(id, transform, data);

			mReverb->UpdateValid(mISMConfig.lateReverb);
		}

		void Context::RemoveSource(size_t id)
		{

#ifdef DEBUG_REMOVE
	Debug::Log("Remove Source", Colour::Red);
#endif

			mRoom->RemoveSourcePosition(id);
			mSources->Remove(id);
		}

		// Wall

		size_t Context::InitWall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
		{
#ifdef DEBUG_INIT
	Debug::Log("Init Wall and Edges", Colour::Green);
#endif

			Wall wall = Wall(normal, vData, numVertices, absorption);
			mReverb->UpdateReflectionFilters(reverbWall, absorption);
			return mRoom->AddWall(wall);
		}

		// Assumes reverbWall never changes
		void Context::UpdateWall(size_t id, const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
		{
			Absorption oldAbsorption = mRoom->UpdateWall(id, normal, vData, numVertices, absorption);
			mReverb->UpdateReflectionFilters(reverbWall, absorption, oldAbsorption);
			// TO DO: Update edges?
		}

		// Assumes reverbWall never changes
		void Context::RemoveWall(size_t id, const ReverbWall& reverbWall)
		{
#ifdef DEBUG_REMOVE
	Debug::Log("Remove Wall and Edges", Colour::Red);
#endif

			Absorption absorption = mRoom->RemoveWall(id);
			if (absorption.area != 0)
				mReverb->UpdateReflectionFilters(reverbWall, absorption);
		}

		// Audio

		void Context::SubmitAudio(size_t id, const float* data)
		{
			Buffer in = Buffer(mConfig.numFrames);
			for (int i = 0; i < mConfig.numFrames; i++)
				in[i] = static_cast<Real>(data[i]);

			//memcpy(&in[0], data, mConfig.numFrames);

			mSources->ProcessAudio(id, &in[0], mConfig.numFrames, mReverbInput, mOutputBuffer, mConfig.lerpFactor);
		}

		void Context::GetOutput(float** bufferPtr)
		{
			// Process reverb
			mReverb->ProcessAudio(mReverbInput, mOutputBuffer);


			// Copy output to send and set pointer
			// TO DO: Chek unity can't call ProcessAudio and GetOutput at the same time
			mSendBuffer = mOutputBuffer;
			*bufferPtr = &mSendBuffer[0];

			// Reset output and reverb buffers
			mOutputBuffer.ResetBuffer();
			mReverbInput.Reset();
		}
	}
}