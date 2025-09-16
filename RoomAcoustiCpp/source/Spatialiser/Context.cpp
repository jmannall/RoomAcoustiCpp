/*
* @brief Defines the RoomAcoustiCpp global context
*
*/

#include <random>

//Common headers
#include "Common/RACProfiler.h"
#include "Common/FileReader.h"

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

		Context::Context(const DSPData& data) : dspConfig(std::make_shared<DSPConfig>(data)), mIsRunning(true), IEMThread(), rayTracingThread(), applyHeadphoneEQ(false), headphoneEQ(2048)
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
			mCore.SetAudioState({ dspConfig->GetData().fs, dspConfig->GetData().numFrames });

			// Create listener
			mListener = mCore.CreateListener();
			headRadius = mListener->GetHeadRadius();

			CreateAudioThreadPool();
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
			IEMThread.join();
			rayTracingThread.join();
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

		void Context::UpdateMoDARTDelay(const Real delay)
		{
			if (!lateReverbInitialised.load(std::memory_order_acquire))
				return;

			while (audioFlag.exchange(true, std::memory_order_acquire))
				std::this_thread::yield();

			mReverb->SetPrecedingDelay(delay, dspConfig->GetData().fs);
			ResetLateReverb();

			audioFlag.store(false, std::memory_order_release);
		}

		////////////////////////////////////////

		void Context::UpdateMoDARTMinimumReverbTime(const Real T60)
		{
			if (!lateReverbInitialised.load(std::memory_order_acquire))
				return;

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
			size_t numThreads = std::min((unsigned int)8, std::thread::hardware_concurrency());
			audioThreadPool = std::make_unique<AudioThreadPool>(numThreads, dspConfig);
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
			mReverbInput = Matrix<>(dspConfig->GetReverbInputDimensions());

			CreateAudioThreadPool();

			mRayTracing = std::make_shared<TracingThread>(mRoom, mSources, mReverb, data, dspConfig);

			// Start background thread after all systems are initialized
			rayTracingThread = std::thread(RayTracerProcessor, this);
			EnableLateReverb(data.enabled);
		}

		////////////////////////////////////////

		bool Context::InitSingleFDN(const RoomData& roomData, const LateReverbData& data)
		{
			if (lateReverbInitialised.load(std::memory_order_acquire))
				return false;

			while (audioFlag.exchange(true, std::memory_order_acquire))
				std::this_thread::yield();

			mRoom->UpdateRoomData(roomData);
			dspConfig->UpdateLateReverbModel(LateReverbModel::fdn, 1);
			mReverb = std::make_shared<SingleFDN>(&mCore, mRoom->GetReverbTime(), mRoom->GetDimensions(), data, dspConfig);
			InitLateReverb(data);

			lateReverbInitialised.store(true, std::memory_order_release);
			audioFlag.store(false, std::memory_order_release);
			return true;
		}

		////////////////////////////////////////

		bool Context::InitMoDART(const MoDARTData& data)
		{
			if (lateReverbInitialised.load(std::memory_order_acquire))
				return false;

			while (audioFlag.exchange(true, std::memory_order_acquire))
				std::this_thread::yield();

			dspConfig->UpdateLateReverbModel(LateReverbModel::raves, data.t60s.Rows());
			mReverb = std::make_shared<RAVES>(&mCore, data, dspConfig);
			InitLateReverb(data);

			mSources->UpdateMoDARTParameters(data.frequencyIndexing, dspConfig->GetData().numFrames);
			mRoom->CreateTriangleMeshSoA();

			int numPaths = data.rightEigenvectors[0].Rows();
			mRayTracing->InitRoom(numPaths, data.indexing, data.energyDecay);

			lateReverbInitialised.store(true, std::memory_order_release);
			audioFlag.store(false, std::memory_order_release);
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
			if (lateReverbInitialised.load(std::memory_order_acquire))
				mReverb->UpdateReverbSourcePositions(position);
			mImageEdgeModel->SetListenerPosition(position);
			mRayTracing->SetListenerPosition(position);
		}

		////////////////////////////////////////

		size_t Context::InitSource()
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
				Vec3 newPosition = position;
				if (distance == 0.0)
					newPosition = mSources->GetSourcePosition(id);
				distance = (newPosition - listenerPosition).Length();
				if (distance == 0.0)
					newPosition = listenerPosition + Vec3(1.0,0.0,0.0);
				newPosition = listenerPosition + UnitVector(newPosition - listenerPosition) * headRadius;

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

		size_t Context::InitWall(const Vertices& vData, const Absorption<>& absorption, int polygonId)
		{
#ifdef DEBUG_INIT
	Debug::Log("Init Wall", Colour::Green);
#endif

			Wall wall = Wall(vData, absorption, polygonId);
			size_t id = mRoom->AddWall(wall);
			mRoom->InitEdges(id);
			return id;
		}

		////////////////////////////////////////

		void Context::UpdateWall(size_t id, const Vertices& vData)
		{
			mRoom->UpdateWall(id, vData);
		}

		////////////////////////////////////////

		void Context::UpdateWallAbsorption(size_t id, const Absorption<>& absorption)
		{
			mRoom->UpdateWallAbsorption(id, absorption);
			if (lateReverbInitialised.load(std::memory_order_acquire))
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

		void Context::GetOutput(Buffer<>& outputBuffer)
		{
			outputBuffer.Reset();
			if (audioFlag.exchange(true, std::memory_order_acquire))
				return;

			PROFILE_AudioThread;
			if (outputBuffer.Length() != 2 * dspConfig->GetData().numFrames)
			{
				Debug::Log("Incorrect buffer size", Colour::Red);
				outputBuffer.ResizeBuffer(2 * dspConfig->GetData().numFrames);
			}

			// Reset buffers
			mReverbInput.Reset();

			const AudioData audioData(dspConfig);

			if (earlyReverbInitialised.load(std::memory_order_acquire) && (audioData.earlyReverbEnabled || audioData.lateReverbEnabled))
				mSources->ProcessAudio(outputBuffer, audioData);

			if (lateReverbInitialised.load(std::memory_order_acquire) && audioData.lateReverbEnabled)
			{
				mSources->ProcessLateReverbSend(mReverbInput, audioData);
				mReverb->ProcessAudio(mReverbInput, outputBuffer, audioData);
			}

			if (applyHeadphoneEQ)
				headphoneEQ.ProcessAudio(outputBuffer, outputBuffer, audioData);

			audioFlag.store(false, std::memory_order_release);
		}
	}
}