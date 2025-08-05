/*
* @brief Defines the RoomAcoustiCpp global context
*
*/

//Common headers
#include "Common/RACProfiler.h"

// Spatialiser headers
#include "Spatialiser/Globals.h"
#include "Spatialiser/Context.h"
#include "Spatialiser/Types.h"

// Unity headers
#include "Unity/Debug.h"

// 3DTI headers
#include "HRTF/HRTFFactory.h"
#include "HRTF/HRTFCereal.h"
#include "ILD/ILDCereal.h"
#include "Common/ErrorHandler.h"

// Globals
std::shared_mutex RAC::DSP::tuneInMutex;
std::unique_ptr<RAC::DSP::AudioThreadPool> RAC::DSP::audioThreadPool;

namespace RAC
{
	using namespace Unity;
	using namespace DSP;
	using namespace Common;
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

#ifdef DEBUG_INIT
			Debug::Log("Begin background thread", Colour::Green);
#endif
#ifdef USE_UNITY_PROFILER
			RegisterBackgroundThread();
#endif
			std::shared_ptr<Room> room = context->GetRoom();
			std::shared_ptr<ImageEdge> imageEdgeModel = context->GetImageEdgeModel();

			const int loopInterval_ms = 10;
			while (context->IsRunning())
			{
				auto startTime = std::chrono::steady_clock::now();

				// Update IEM
				imageEdgeModel->RunIEM();

				auto endTime = std::chrono::steady_clock::now();
				auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
				if (elapsedTime < loopInterval_ms)
					std::this_thread::sleep_for(std::chrono::milliseconds(loopInterval_ms - elapsedTime));
			}

#ifdef USE_UNITY_PROFILER
			UnregisterBackgroundThread();
#endif
#ifdef DEBUG_REMOVE
			Debug::Log("End background thread", Colour::Red);
#endif
		}

		//////////////////// Context ////////////////////

		////////////////////////////////////////

		Context::Context(const std::shared_ptr<Config> config) : mConfig(config), mIsRunning(true), IEMThread(), applyHeadphoneEQ(false), headphoneEQ(2048)
		{
#ifdef DEBUG_INIT
			Debug::Log("Init Context", Colour::Green);
#endif
			CErrorHandler::Instance().SetAssertMode(ASSERT_MODE_CONTINUE);
			CErrorHandler::Instance().SetVerbosityMode(VERBOSITYMODE_ERRORSANDWARNINGS);
			std::string timestamp = GetTimestamp();
			logFile = GetLogPath(timestamp);
			CErrorHandler::Instance().SetErrorLogFile(logFile, true);

#ifdef PROFILE_BACKGROUND_THREAD || PROFILE_AUDIO_THREAD
			profileFile = GetProfilePath(timestamp);
			Profiler::Instance().SetOutputFile(profileFile, true);
#endif

			// Set dsp settings
			mCore.SetAudioState({ mConfig->fs, mConfig->numFrames });

			// Create listener
			mListener = mCore.CreateListener();
			headRadius = mListener->GetHeadRadius();

			mReverb = std::make_shared<Reverb>(&mCore, mConfig);
			mRoom = std::make_shared<Room>(mConfig->frequencyBands.Length());
			mSources = std::make_shared<SourceManager>(&mCore, mConfig);
			mImageEdgeModel = std::make_shared<ImageEdge>(mRoom, mSources, mReverb, mConfig->frequencyBands);
			
			// Initialize NNs
			myNN_initialize();

			// Start background thread after all systems are initialized
			IEMThread = std::thread(BackgroundProcessor, this);
			audioThreadPool = std::make_unique<AudioThreadPool>(std::min((unsigned int)8, std::thread::hardware_concurrency()), mConfig->numFrames, mConfig->numReverbSources);

			mInputBuffer = Buffer(mConfig->numFrames);
			mOutputBuffer = Buffer(2 * mConfig->numFrames); // Stereo output buffer
			mSendBuffer = std::vector<float>(2 * mConfig->numFrames, 0.0);
			mReverbInput = Matrix(mConfig->numReverbSources, mConfig->numFrames);
		}

		////////////////////////////////////////

		Context::~Context()
		{
#ifdef DEBUG_REMOVE
			Debug::Log("Exit Context", Colour::Red);
#endif
			StopRunning();
			IEMThread.join();
			if (audioThreadPool)
				audioThreadPool->Stop();

			mImageEdgeModel.reset();
			mSources.reset();
			mRoom.reset();
			mReverb.reset();
			audioThreadPool.reset();

			// Terminate NNs
			myNN_terminate();

			unique_lock<shared_mutex> lock(tuneInMutex);
			mCore.RemoveListener();

			CErrorHandler::Instance().SetErrorLogFile(logFile, false); // Disable logging to file
			if (!logFile.empty())
			{
				std::ifstream f(logFile);	// Delete file if it is empty
				if (f.good() && f.peek() == std::ifstream::traits_type::eof())
				{
					f.close();
					std::remove(logFile.c_str());
				}
			}
#ifdef PROFILE_BACKGROUND_THREAD || PROFILE_AUDIO_THREAD
			Profiler::Instance().SetOutputFile(profileFile, false);
#endif
		}

		////////////////////////////////////////

		bool Context::LoadSpatialisationFiles(const int hrtfResamplingStep, const std::vector<std::string>& filePaths)
		{
			unique_lock<shared_mutex> lock(tuneInMutex);

			// Set HRTF resampling step
			mCore.SetHRTFResamplingStep(hrtfResamplingStep);

			// Load high quality files
			bool result = HRTF::CreateFrom3dti(filePaths[0], mListener);
			if (result)
				result = ILD::CreateFrom3dti_ILDNearFieldEffectTable(filePaths[1], mListener);
			// Load high performance files
			if (result)
				result = ILD::CreateFrom3dti_ILDSpatializationTable(filePaths[2], mListener);
			return result;
		}

		////////////////////////////////////////

		void Context::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			mConfig->spatialisationMode.store(mode, std::memory_order_release);
			mReverb->UpdateSpatialisationMode(mode);
			mSources->UpdateSpatialisationMode(mode);
		}

		////////////////////////////////////////

		void Context::UpdateReverbTime(const ReverbFormula model)
		{
			mRoom->UpdateReverbTimeFormula(model);
			mReverb->SetTargetT60(mRoom->GetReverbTime());
		}

		////////////////////////////////////////

		void Context::UpdateReverbTime(const Coefficients<>& T60)
		{
			mRoom->UpdateReverbTime(T60);
			mReverb->SetTargetT60(mRoom->GetReverbTime());
		}

		////////////////////////////////////////

		void Context::UpdateDiffractionModel(const DiffractionModel model)
		{
			mConfig->diffractionModel.store(model, std::memory_order_release);
			mImageEdgeModel->UpdateDiffractionModel(model);
			mSources->UpdateDiffractionModel(model);
		}

		////////////////////////////////////////

		bool Context::InitLateReverb(const Real volume, const Vec& dimensions, const FDNMatrix matrix)
		{
			if (dimensions.Rows() == 0)
			{
				Debug::Log("No dimensions provided for room", Colour::Red);
				return false;
			}

			Coefficients T60 = mRoom->GetReverbTime(volume);
			mReverb->InitLateReverb(T60, dimensions, matrix, mConfig);
			return true;
		}

		////////////////////////////////////////

		void Context::UpdateListener(const Vec3& position, const Vec4& orientation)
		{
			listenerPosition = position;

			// Set listener position and orientation
			CTransform transform;
			transform.SetOrientation(CQuaternion(static_cast<float>(orientation.w), static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)));
			transform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));
			
			{
				unique_lock<shared_mutex> lock(tuneInMutex);
				mListener->SetListenerTransform(transform);
			}
			mReverb->UpdateReverbSourcePositions(position);
			mImageEdgeModel->SetListenerPosition(position);
		}

		////////////////////////////////////////

		int Context::InitSource()
		{
#ifdef DEBUG_INIT
	Debug::Log("Init Source", Colour::Green);
#endif
			return mSources->Init();
		}

		////////////////////////////////////////

		void Context::UpdateSource(size_t id, const Vec3& position, const Vec4& orientation)
		{
			Real distance = (position - listenerPosition).Length();

			// Ensure source is outside listener head radius
			if (distance < headRadius)
			{
				std::optional<Vec3> newPosition = position;
				if (distance == 0.0)
				{
					newPosition = mSources->GetSourcePosition(id);
					if (!newPosition.has_value())
						return; // Exit if source position is not found
				}

				distance = (newPosition.value() - listenerPosition).Length();
				if (distance == 0.0)
					newPosition = listenerPosition + Vec3(1.0,0.0,0.0);
				newPosition = listenerPosition + UnitVector(newPosition.value() - listenerPosition) * headRadius;

				// Update source position, orientation and virtual sources
				mSources->Update(id, newPosition.value(), orientation, headRadius);
			}
			else
				// Update source position, orientation and virtual sources
				mSources->Update(id, position, orientation, distance);
		}

		////////////////////////////////////////

		void Context::RemoveSource(size_t id)
		{

#ifdef DEBUG_REMOVE
	Debug::Log("Remove Source", Colour::Red);
#endif
			mSources->Remove(id);
		}

		////////////////////////////////////////

		int Context::InitWall(const Vertices& vertices, const Absorption<>& absorption)
		{
#ifdef DEBUG_INIT
			Debug::Log("Init Wall", Colour::Green);
#endif
			if (absorption.Length() != mConfig->frequencyBands.Length())
			{
				Debug::Log("Absorption coefficients length does not match frequency bands length", Colour::Red);
				return -1; // Return -1 if absorption coefficients length is incorrect
			}
			size_t id = mRoom->InitWall(vertices, absorption);
			mRoom->InitEdges(id);
			return static_cast<int>(id);
		}

		////////////////////////////////////////

		void Context::UpdateWall(size_t id, const Vertices& vData)
		{
			mRoom->UpdateWall(id, vData);
		}

		////////////////////////////////////////

		void Context::UpdateWallAbsorption(size_t id, const Absorption<>& absorption)
		{
			if (absorption.Length() != mConfig->frequencyBands.Length())
			{
				Debug::Log("Absorption coefficients length does not match frequency bands length", Colour::Red);
				return;
			}
			mRoom->UpdateWallAbsorption(id, absorption);
			mReverb->SetTargetT60(mRoom->GetReverbTime());
		}

		////////////////////////////////////////

		void Context::RemoveWall(size_t id)
		{
#ifdef DEBUG_REMOVE
	Debug::Log("Remove Wall", Colour::Red);
#endif
			mRoom->RemoveWall(id);
		}

		////////////////////////////////////////

		void Context::UpdatePlanesAndEdges()
		{
			mRoom->UpdatePlanes();
			mRoom->UpdateEdges();
		}

		////////////////////////////////////////

		void Context::GetOutput(float** bufferPtr)
		{
			PROFILE_AudioThread;
			const Real lerpFactor = mConfig->GetLerpFactor();
			mSources->ProcessAudio(mOutputBuffer, mReverbInput, lerpFactor);

			mReverb->ProcessAudio(mReverbInput, mOutputBuffer, lerpFactor);
			mReverbInput.Reset();

			if (applyHeadphoneEQ)
				headphoneEQ.ProcessAudio(mOutputBuffer, mOutputBuffer, lerpFactor);
			
			// Copy output to send and set pointer
			std::transform(mOutputBuffer.begin(), mOutputBuffer.end(), mSendBuffer.begin(),
				[&](auto value) { return static_cast<float>(value); });
			*bufferPtr = &mSendBuffer[0];

			// Reset output buffer
			mOutputBuffer.Reset();
			mReverbInput.Reset();
		}

		////////////////////////////////////////

		void Context::UpdateImpulseResponseMode(const bool mode)
		{
			mConfig->impulseResponseMode.store(mode, std::memory_order_release);
			mSources->UpdateImpulseResponseMode(mode);
			if (mode)
				headphoneEQ.Reset();		// TO DO: Should this be here?
		}
	}
}