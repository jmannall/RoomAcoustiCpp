/*
*
*  \Spatialiser context
*
*/

#include "Spatialiser/Context.h"

namespace UIE
{
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// Background Thread ////////////////////

		void BackgroundProcessor(Context* context)
		{
			Debug::Log("Begin background thread", Colour::Green);

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
			Debug::Log("End background thread", Colour::Red);
		}

		//////////////////// Context ////////////////////

		// Load and Destroy

		Context::Context(const Config* config) : mIsRunning(true), ISMThread()
		{
			Debug::Log("Init Context", Colour::Green);

			// Copy config
			std::memcpy(&mConfig, config, sizeof(Config));

			// Set dsp settings
			mCore.SetAudioState({ mConfig.fs, mConfig.numFrames });
			mCore.SetHRTFResamplingStep(mConfig.hrtfResamplingStep);

			// Create listener
			mListener = mCore.CreateListener();

			// Load HRTF files
			// TO DO: Move file locations to config
			string resourcePath = "D:\\Joshua Mannall\\GitHub\\3dti_AudioToolkit\\resources";
			bool hrtfLoaded = HRTF::CreateFrom3dti(resourcePath + "\\HRTF\\3DTI\\3DTI_HRTF_IRC1008_128s_48000Hz.3dti-hrtf", mListener);
			bool ildLoaded = false;

			string mode;
			if (hrtfLoaded)
			{
				switch (mConfig.hrtfMode)
				{
				case HRTFMode::quality:
				{ ildLoaded = ILD::CreateFrom3dti_ILDSpatializationTable(resourcePath + "\\ILD\\HRTF_ILD_48000.3dti-ild", mListener);
				mode = "quality"; break; }
				case HRTFMode::performance:
				{ ildLoaded = ILD::CreateFrom3dti_ILDNearFieldEffectTable(resourcePath + "\\ILD\\NearFieldCompensation_ILD_48000.3dti-ild", mListener);
				mode = "performance"; break; }
				case HRTFMode::none:
					break;
				default:
				{ mConfig.hrtfMode = HRTFMode::none; break; }
				}
			}

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
				mConfig.hrtfMode = HRTFMode::none;
				Debug::Log("Spatialisation set to none", Colour::Green);
			}

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
			Debug::Log("Exit Context", Colour::Red);

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

			Debug::Log("Listener position: " + VecToStr(position), Colour::Yellow);
		}

		// Source

		size_t Context::InitSource()
		{
			Debug::Log("Init Source", Colour::Green);

			return mSources->Init();
		}

		void Context::UpdateSource(size_t id, const vec3& position, const vec4& orientation)
		{
			// Update source position in background thread and
			// return a copy of all virtual sources
			SourceData data = mRoom->UpdateSourcePosition(id, position);

			// Update source position, orientation and virtual sources
			CTransform transform;
			transform.SetOrientation(CQuaternion(static_cast<float>(orientation.w), static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)));
			transform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));
			mSources->Update(id, transform, data);

			mReverb->UpdateValid(mISMConfig.lateReverb);

			Debug::Log("Source position: " + VecToStr(position), Colour::Yellow);
		}

		void Context::RemoveSource(size_t id)
		{
			Debug::Log("Remove Source", Colour::Red);

			mRoom->RemoveSourcePosition(id);
			mSources->Remove(id);
		}

		// Wall

		size_t Context::InitWall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
		{
			Debug::Log("Init Wall and Edges", Colour::Green);

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
			Debug::Log("Remove Wall and Edges", Colour::Red);

			Absorption absorption = mRoom->RemoveWall(id);
			if (absorption.area != 0)
				mReverb->UpdateReflectionFilters(reverbWall, absorption);
		}

		// Audio

		void Context::SubmitAudio(size_t id, const Real* data)
		{
			mSources->ProcessAudio(id, data, mConfig.numFrames, mReverbInput, mOutputBuffer, mConfig.lerpFactor);
		}

		void Context::GetOutput(Real** bufferPtr)
		{
			// Process reverb
			mReverb->ProcessAudio(mReverbInput, mOutputBuffer);

			// Debug::Log("Output: " + RealToStr(mOutputBuffer[0]));
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