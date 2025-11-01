/*
* @brief Defines the RoomAcoustiCpp global context
*
*/

#include <random>

//Common headers
#include "Common/RACProfiler.h"
#include "Common/FileReader.h"
#include "Common/Access.h"

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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

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

		//////////////////// IEM Thread ////////////////////

		////////////////////////////////////////

		void IEMProcessor(Context* context)
		{

#ifdef DEBUG_INIT
			Debug::Log("Begin image edge model thread", Colour::Green);
#endif
#ifdef USE_UNITY_PROFILER
			RegisterIEMThread();
#endif
#ifdef _WIN32
			SetThreadDescription(GetCurrentThread(), L"IEMProcessor");
#endif

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
			UnregisterIEMThread();
#endif
#ifdef DEBUG_REMOVE
			Debug::Log("End image edge model thread", Colour::Red);
#endif
		}

		//////////////////// Ray Tracing Thread ////////////////////

		////////////////////////////////////////

		void RayTracerProcessor(Context* context)
		{

#ifdef DEBUG_INIT
			Debug::Log("Begin racy tracing thread", Colour::Green);
#endif
#ifdef USE_UNITY_PROFILER
			RegisterRayTracingThread();
#endif
			std::shared_ptr<TracingThread> rayTracing = context->GetRayTracing();

			const int loopInterval_ms = 50;
			while (context->IsRunning())
			{
				auto startTime = std::chrono::steady_clock::now();

				// Update RTM
				rayTracing->RunTracing();

				auto endTime = std::chrono::steady_clock::now();
				auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
				if (elapsedTime < loopInterval_ms)
					std::this_thread::sleep_for(std::chrono::milliseconds(loopInterval_ms - elapsedTime));
			}

#ifdef USE_UNITY_PROFILER
			UnregisterRayTracingThread();
#endif
#ifdef DEBUG_REMOVE
			Debug::Log("End ray tracing thread", Colour::Red);
#endif
		}

		//////////////////// Context ////////////////////

		////////////////////////////////////////

		Context::Context(const DSPData& data,const ContextOptionalArguments& optionalArguments)
		: dspConfig(std::make_shared<DSPConfig>(data)), mIsRunning(true), IEMThread(), rayTracingThread(), applyHeadphoneEQ(false), headphoneEQ(2048)
		{
#ifdef DEBUG_INIT
			Debug::Log("Init Context", Colour::Green);
#endif

			if (!optionalArguments.logPrefix.empty())
			{
				CErrorHandler::Instance().SetErrorLogFile(optionalArguments.logPrefix + "_log.txt", true);
#if defined(PROFILE_BACKGROUND_THREAD) || defined(PROFILE_AUDIO_THREAD)
				Profiler::Instance().SetOutputFile(optionalArguments.logPrefix + "_profile.txt", true);
#endif
			}
			CErrorHandler::Instance().SetAssertMode(ASSERT_MODE_CONTINUE);
			CErrorHandler::Instance().SetVerbosityMode(VERBOSITYMODE_ERRORSANDWARNINGS);

			// Set dsp settings
			mCore.SetAudioState({ dspConfig->GetData().fs, dspConfig->GetData().numFrames });

			// Create listener
			mListener = mCore.CreateListener();
			headRadius = mListener->GetHeadRadius();

			if (optionalArguments.desiredAudioThreads.has_value())
				numDesiredWorkerThreads = optionalArguments.desiredAudioThreads.value();
			else
				numDesiredWorkerThreads = std::min((unsigned int)8, std::thread::hardware_concurrency());

			mSources = std::make_shared<SourceManager>(&mCore, dspConfig);
			mRoom = std::make_shared<Room>(dspConfig->GetData().numFrequencyBands);

			// Initialize NNs
			myNN_initialize();
		}

		////////////////////////////////////////

		Context::~Context()
		{
#ifdef DEBUG_REMOVE
			Debug::Log("Exit Context", Colour::Red);
#endif
			StopRunning();
			if (IEMThread.joinable())
				IEMThread.join();
			if (rayTracingThread.joinable())
				rayTracingThread.join();

			mImageEdgeModel.reset();
			mRayTracing.reset();

			AtomicFlagGuard guard(audioFlag);

			mSources.reset();
			mRoom.reset();
			mReverb.reset();

			if (audioThreadPool)
				audioThreadPool->Stop();
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
#if defined(PROFILE_BACKGROUND_THREAD) || defined(PROFILE_AUDIO_THREAD)
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

		void Context::UpdateMoDARTDelay(const Real delay)
		{
			if (!lateReverbInitialised.load(std::memory_order_acquire))
				return;

			AtomicFlagGuard guard(audioFlag);

			mReverb->SetPrecedingDelay(delay, dspConfig->GetData().fs);
			ResetLateReverb();
		}

		////////////////////////////////////////

		void Context::UpdateMoDARTMinimumReverbTime(const Real T60)
		{
			if (lateReverbInitialised.load(std::memory_order_acquire))
				mReverb->SetMinimumT60(T60);
		}

		////////////////////////////////////////

		void Context::UpdateSingleFDNReverbTime(const ReverbFormula model)
		{
			if (lateReverbInitialised.load(std::memory_order_acquire))
				mReverb->SetTargetT60(mRoom->GetReverbTime(model));
		}

		////////////////////////////////////////

		void Context::UpdateSingleFDNReverbTime(const Coefficients<>& T60)
		{
			mRoom->UpdateReverbTime(T60);
			if (lateReverbInitialised.load(std::memory_order_acquire))
				mReverb->SetTargetT60(T60);
		}

		////////////////////////////////////////

		void Context::UpdateDiffractionModel(const DiffractionModel model)
		{
			dspConfig->UpdateDiffractionModel(model);
			mSources->UpdateDiffractionModel(model);

			if (earlyReverbInitialised.load(std::memory_order_acquire))
				mImageEdgeModel->UpdateDiffractionModel(model);
		}

		////////////////////////////////////////

		void Context::CreateAudioThreadPool()
		{
			assert(!audioThreadPool);
			audioThreadPool = std::make_unique<AudioThreadPool>(numDesiredWorkerThreads, dspConfig);
		}


		////////////////////////////////////////

		bool Context::InitEarlyReverb(const bool enabled, const EarlyReverbData& data, const DiffractionModel model)
		{
			if (earlyReverbInitialised.load(std::memory_order_acquire))
				return false;

			UpdateDiffractionModel(model);
			mImageEdgeModel = std::make_shared<ImageEdge>(mRoom, mSources, data, dspConfig);

			// Start background thread after all systems are initialized
			IEMThread = std::thread(IEMProcessor, this);

			EnableEarlyReverb(enabled);
			earlyReverbInitialised.store(true, std::memory_order_release);

			return true;
		}

		////////////////////////////////////////

		void Context::InitLateReverb(const LateReverbData& data)
		{
			auto dimensions = dspConfig->GetReverbInputDimensions();
			mReverbInput = Matrix<>::Zero(dimensions.first, dimensions.second);

			// Start background thread after all systems are initialized
			rayTracingThread = std::thread(RayTracerProcessor, this);
			EnableLateReverb(data.enabled);
		}

		////////////////////////////////////////

		bool Context::InitSingleFDN(const RoomData& roomData, const LateReverbData& data)
		{
			if (lateReverbInitialised.load(std::memory_order_acquire))
				return false;

			AtomicFlagGuard guard(audioFlag);

			mRoom->UpdateRoomData(roomData);
			dspConfig->UpdateLateReverbModel(LateReverbModel::fdn, 1);
			mReverb = std::make_shared<SingleFDN>(&mCore, mRoom->GetReverbTime(), mRoom->GetDimensions(), data, dspConfig);
			mRayTracing = std::make_shared<SingleFDNTracing>(mRoom, mSources, mReverb, data, dspConfig);

			InitLateReverb(data);

			mRoom->CreateTriangleMeshSoA();

			lateReverbInitialised.store(true, std::memory_order_release);
			return true;
		}

		////////////////////////////////////////

		bool Context::InitMoDART(const MoDARTData& data)
		{
			if (lateReverbInitialised.load(std::memory_order_acquire))
				return false;

			AtomicFlagGuard guard(audioFlag);

			dspConfig->UpdateLateReverbModel(LateReverbModel::raves, ToInt(data.t60s.Length()));
			mReverb = std::make_shared<RAVES>(&mCore, data, dspConfig);
			mRayTracing = std::make_shared<MoDARTTracing>(mRoom, mSources, mReverb, data, dspConfig);

			InitLateReverb(data);

			mSources->UpdateMoDARTParameters(data.frequencyIndexing, dspConfig->GetData().numFrames);
			mRoom->CreateTriangleMeshSoA();

			lateReverbInitialised.store(true, std::memory_order_release);
			return true;
		}

		////////////////////////////////////////

		void Context::UpdateListener(const Vec3& position, const Vec4& orientation)
		{
			listenerPosition = position;

			// Set listener position and orientation
			CTransform transform;
			transform.SetOrientation(CQuaternion(static_cast<float>(orientation.w()), static_cast<float>(orientation.x()), static_cast<float>(orientation.y()), static_cast<float>(orientation.z())));
			transform.SetPosition(CVector3(static_cast<float>(position.x()), static_cast<float>(position.y()), static_cast<float>(position.z())));
			
			{
				unique_lock<shared_mutex> lock(tuneInMutex);
				mListener->SetListenerTransform(transform);
			}
			if (lateReverbInitialised.load(std::memory_order_acquire))
			{
				mReverb->UpdateReverbSourcePositions(position);
				mRayTracing->SetListenerPosition(position);
			}
			if (earlyReverbInitialised.load(std::memory_order_acquire))
				mImageEdgeModel->SetListenerPosition(position);

			listenerInitialised = true;
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
			if (!listenerInitialised)
				return;

			Real distance = (position - listenerPosition).Normal();

			// Ensure source is outside listener head radius
			if (distance < headRadius)
			{
				Vec3 newPosition = position;
				if (distance == 0.0)
					newPosition = mSources->GetSourcePosition(id);
				distance = (newPosition - listenerPosition).Normal();
				if (distance == 0.0)
					newPosition = listenerPosition + Vec3(1.0,0.0,0.0);
				newPosition = listenerPosition + (newPosition - listenerPosition).Normalised() * headRadius;

				// Update source position, orientation and virtual sources
				mSources->Update(id, newPosition, orientation, headRadius);
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

		void Context::UpdateMaterial(size_t id, const Coefficients<>& material)
		{
			mRoom->UpdateMaterial(id, material);
			if (lateReverbInitialised.load(std::memory_order_acquire))
				mReverb->SetTargetT60(mRoom->GetReverbTime());
		}

		////////////////////////////////////////

		size_t Context::InitWall(const Vertices& vData, size_t materialID)
		{
#ifdef DEBUG_INIT
	Debug::Log("Init Wall", Colour::Green);
#endif

			Wall wall = Wall(vData, materialID);
			size_t id = mRoom->AddWall(wall);
			mRoom->InitEdges(id);
			return id;
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

		void Context::GetOutput(Buffer<>& outputBuffer)
		{
			outputBuffer.Reset();
			AtomicFlagGuard guard(audioFlag, true); // Try once

			if (!guard.Acquired())
				return; // someone else has the flag, exit early

			PROFILE_AudioThread;
			if (outputBuffer.Length() != 2 * dspConfig->GetData().numFrames)
			{
				Debug::Log("Incorrect buffer size", Colour::Red);
				outputBuffer.Resize(2 * dspConfig->GetData().numFrames);
			}

			// make sure our threads are initialized
			EnsureAudioThreadPoolInitialized();

			// Reset buffers
			mReverbInput.Reset();

			const AudioData audioData(dspConfig);

			mSources->ResetInputBuffers();
			if (earlyReverbInitialised.load(std::memory_order_acquire) && (audioData.earlyReverbEnabled || audioData.lateReverbEnabled))
				mSources->ProcessAudio(outputBuffer, audioData);

			if (lateReverbInitialised.load(std::memory_order_acquire) && audioData.lateReverbEnabled)
			{
				mSources->ProcessLateReverbSend(mReverbInput, audioData);
				mReverb->ProcessAudio(mReverbInput, outputBuffer, audioData);
			}

			if (applyHeadphoneEQ)
				headphoneEQ.ProcessAudio(outputBuffer, outputBuffer, audioData);
		}

		////////////////////////////////////////

		void Context::RecordImpulseResponse(const Vec3& position, const Vec4& orientation, Buffer<>& outputBuffer)
		{
			int id = InitSource();
			if (id < 0)
				return;

			// TODO: Allow different directivities
			UpdateSourceDirectivity(static_cast<size_t>(id), SourceDirectivity::omni);
			UpdateSource(static_cast<size_t>(id), position, orientation);

			mImageEdgeModel->ResetEndFlag();
			mRayTracing->ResetEndFlag();
			UpdateImpulseResponseMode(true);
			ResetLateReverb();

			int numFrames = dspConfig->GetData().numFrames;
			Buffer<> input = Buffer<>::Zero(numFrames);
			Buffer<> output = Buffer<>::Zero(2 * numFrames);

			while (!mImageEdgeModel->HasCompleted())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));

			while (!mRayTracing->HasCompleted())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));

			// Run once with empty input (ensures all interpolation is updated)
			mSources->SetInputBuffer(id, input);
			GetOutput(output);

			int irLength = ToInt(outputBuffer.Length());
			int outputBufferLength = ToInt(output.Length());
			int numBuffers = irLength / outputBufferLength;
			int remainder = irLength - numBuffers * outputBufferLength;

			// Process buffers
			int count = 0;
			input[0] = 1.0;
			for (int i = 0; i < numBuffers; i++)
			{
				mSources->SetInputBuffer(id, input);
				GetOutput(output);
				for (int j = 0; j < outputBufferLength; j++)
					outputBuffer[count++] = output[j];
				input[0] = 0.0;
			}
			// Process remaining samples
			mSources->SetInputBuffer(id, input);
			GetOutput(output);
			for (int i = 0; i < remainder; i++)
				outputBuffer[count++] = output[i];

			RemoveSource(id);
			UpdateImpulseResponseMode(false);
		}
	}
}