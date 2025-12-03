/*
* @brief Defines the RoomAcoustiCpp global context
*
*/

#include <random>

//Common headers
#include "Common/RACProfiler.h"
#include "Common/FileReader.h"
#include "Common/Access.h"
#include "Common/Debug.h"

// Spatialiser headers
#include "Spatialiser/Globals.h"
#include "Spatialiser/Context.h"
#include "Spatialiser/Types.h"

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

			Debug::Log("Begin image edge model thread", DebugType::Init);

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

			Debug::Log("End image edge model thread", DebugType::Remove);
		}

		//////////////////// Ray Tracing Thread ////////////////////

		////////////////////////////////////////

		void RayTracerProcessor(Context* context)
		{

			Debug::Log("Begin racy tracing thread", DebugType::Init);

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

			Debug::Log("End ray tracing thread", DebugType::Remove);
		}

		//////////////////// Context ////////////////////

		static DebugLogStreamBuffer logBuffer;
		static std::ostream logStream(&logBuffer);

		////////////////////////////////////////

		Context::Context(const DSPData& data,const ContextOptionalArguments& optionalArguments)
		: dspConfig(std::make_shared<DSPConfig>(data)), mIsRunning(true), IEMThread(), rayTracingThread(), applyHeadphoneEQ(false), headphoneEQ(2048), dcBlocker(data.fs)
		{
			Debug::Log("Init Context", DebugType::Init);

			CErrorHandler::Instance().SetErrorLogStream(&logStream, true);
			if (!optionalArguments.logPrefix.empty())
			{
				// CErrorHandler::Instance().SetErrorLogFile(optionalArguments.logPrefix + "_log.txt", true);
#if defined(PROFILE_BACKGROUND_THREAD) || defined(PROFILE_AUDIO_THREAD)
				Profiler::Instance().SetOutputFile(optionalArguments.logPrefix + "_profile.txt", true);
#endif
			}
			CErrorHandler::Instance().SetAssertMode(ASSERT_MODE_CONTINUE);
			CErrorHandler::Instance().SetVerbosityMode(VERBOSITYMODE_ERRORSANDWARNINGS);

			// TODO: Comment frame rate doesn't change at runtime
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
			Debug::Log("Exit Context", DebugType::Remove);

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
			Debug::Assert(hrtfResamplingStep > 0, "Invalid HRTF resampling step: " + ToString(hrtfResamplingStep));
			Debug::Assert(filePaths.size() == 3, "Invalid number of file paths");

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
			Debug::Assert(delay >= 0, "Invalid MoD-ART delay: " + ToString(delay));

			if (!lateReverbInitialised.load(std::memory_order_acquire))
				return;

			AtomicFlagGuard guard(audioFlag);

			mReverb->SetPrecedingDelay(delay, dspConfig->GetData().fs);
			ResetLateReverb();
		}

		////////////////////////////////////////

		void Context::UpdateMoDARTMinimumReverbTime(const Real T60)
		{
			Debug::Assert(T60 > 0, "Invalid MoD-ART minimum reverb time: " + ToString(T60));

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
			Debug::Assert(T60.IsGreaterThan(0), "Invalid Single FDN reverb time: " + ToString(T60));

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
			Debug::Assert(!audioThreadPool, "Audio thread pool already created");
			audioThreadPool = std::make_unique<AudioThreadPool>(numDesiredWorkerThreads, dspConfig);
		}


		////////////////////////////////////////

		bool Context::InitEarlyReverb(const bool enabled, const EarlyReverbData& data, const DiffractionModel model)
		{
			if (earlyReverbInitialised.load(std::memory_order_acquire))
			{
				Debug::Log("Early reverb already initialized", DebugType::Warning);
				return false;
			}

			Debug::Assert(data.reflOrder >= 0 , "Invalid reflection order: " + ToString(data.reflOrder));
			Debug::Assert(data.shadowDiffOrder >= 0, "Invalid shadow diffraction order: " + ToString(data.shadowDiffOrder));
			Debug::Assert(data.specularDiffOrder >= 0, "Invalid specular diffraction order: " + ToString(data.specularDiffOrder));
			Debug::Assert(data.minEdgeLength >= 0, "Invalid minimum edge length: " + ToString(data.minEdgeLength));
			Debug::Assert(data.maxPathLength >= 0, "Invalid maximum path length: " + ToString(data.maxPathLength));

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
			Debug::Assert(data.numRays >= 0, "Invalid number of rays: " + ToString(data.numRays));

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
			{
				Debug::Log("Late reverb already initialized", DebugType::Warning);
				return false;
			}

			Debug::Assert(data.numRays >= 0, "Invalid number of rays: " + ToString(data.numRays));
			Debug::Assert(roomData.volume > 0, "Invalid room volume: " + ToString(roomData.volume));
			Debug::Assert(roomData.dimensions.Length() > 0, "No room dimensions provided");
			Debug::Assert(roomData.dimensions.IsGreaterThan(0), "Invalid room dimensions: " + ToString(roomData.dimensions));
			Debug::Assert(roomData.customT60.IsGreaterThan(0), "Invalid custom room reverb time: " + ToString(roomData.customT60));

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

			Debug::Assert(data.numRays >= 0, "Invalid number of rays: " + ToString(data.numRays));
			Debug::Assert(data.delay >= 0, "Invalid delay: " + ToString(data.delay));
			Debug::Assert(data.minimumT60 >= 0, "Invalid minimum reverb time: " + ToString(data.minimumT60));
			Debug::Assert(data.t60s.IsGreaterThan(0), "Invalid reverb times: " + ToString(data.t60s));
			Debug::Assert(data.energyDecay.IsGreaterThan(0), "Invalid energy decays: " + ToString(data.energyDecay));
			Debug::Assert(data.frequencyIndexing.IsGreaterThan(-1), "Invalid frequency indexing: " + ToString(data.frequencyIndexing));
			// Debug::Assert(data.frequencyIndexing.IsLessThan(), "Invalid frequency indexing: " + ToString(data.frequencyIndexing));
			// TODO: Further validation of indexing and eigenvectors?

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
			else
				Debug::Log("Late reverb not initialised when updating listener position", DebugType::Warning);

			if (earlyReverbInitialised.load(std::memory_order_acquire))
				mImageEdgeModel->SetListenerPosition(position);
			else
				Debug::Log("Early reverb not initialised when updating listener position", DebugType::Warning);

			listenerInitialised = true;
		}

		////////////////////////////////////////

		int Context::InitSource()
		{
			Debug::Log("Init Source", DebugType::Init);
			int id = mSources->Init();
			Debug::Assert(id >= 0, "Failed to initialise source");
			return id;
		}

		////////////////////////////////////////

		void Context::UpdateSource(size_t id, const Vec3& position, const Vec4& orientation)
		{
			if (!listenerInitialised)
			{
				Debug::Log("Update Source called before listener initialised", DebugType::Warning);
				return;
			}

			if (id >= MAX_SOURCES)
			{
				Debug::Log("Invalid source ID", DebugType::Error);
				return;
			}

			Real distance = (position - listenerPosition).Normal();

			// Ensure source is outside listener head radius
			if (distance < headRadius)
			{
				Vec3 newPosition = position;
				if (distance == 0.0)
				{
					newPosition = mSources->GetSourcePosition(id);
					Debug::Log("Source position coincides with listener position. Using previous source position", DebugType::Warning);
				}
				distance = (newPosition - listenerPosition).Normal();
				if (distance == 0.0)
				{
					newPosition = listenerPosition + Vec3(1.0, 0.0, 0.0);
					Debug::Log("Previous source position coincides with listener position. Defaulting source position to in front of the listener", DebugType::Warning);
				}
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
			Debug::Log("Remove Source", DebugType::Remove);
			if (id >= MAX_SOURCES)
			{
				Debug::Log("Invalid source ID", DebugType::Error);
				return;
			}
			mSources->Remove(id);
		}

		////////////////////////////////////////

		void Context::UpdateMaterial(size_t id, const Coefficients<>& material)
		{
			Debug::Assert(material.IsGreaterEqThan(0) && material.IsLessEqThan(1), "Invalid material coefficients: " + ToString(material));
			Debug::Assert(material.Length() == dspConfig->GetData().numFrequencyBands, "Invalid material coefficients length: " + ToString(material.Length()));
			
			mRoom->UpdateMaterial(id, material);
			if (lateReverbInitialised.load(std::memory_order_acquire))
				mReverb->SetTargetT60(mRoom->GetReverbTime());
		}

		////////////////////////////////////////

		size_t Context::InitWall(const Vertices& vData, size_t materialID)
		{
			Debug::Log("Init Wall", DebugType::Init);
			Debug::Assert(mRoom->MaterialExists(materialID), "Material ID does not exist: " + ToString(materialID));

			Wall wall = Wall(vData, materialID);
			size_t id = mRoom->AddWall(wall);
			mRoom->InitEdges(id);
			return id;
		}

		////////////////////////////////////////

		void Context::RemoveWall(size_t id)
		{
			Debug::Log("Remove Wall", DebugType::Remove);
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
			AtomicFlagGuard guard(audioFlag, true); // Try once

			if (!guard.Acquired())
				return; // someone else has the flag, exit early

			PROFILE_AudioThread;
			outputBuffer.Reset();
			Debug::Assert(outputBuffer.Length() == 2 * dspConfig->GetData().numFrames, "Output buffer has incorrect length");

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

			dcBlocker.ProcessAudio(outputBuffer);
		}

		////////////////////////////////////////

		void Context::RecordImpulseResponse(const Vec3& position, const Vec4& orientation, Buffer<>& outputBuffer)
		{
			int id = InitSource();
			if (id < 0)
			{
				Debug::Log("Failed to initialise source", DebugType::Error);
				return;
			}

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
			mSources->SetInputBuffer(static_cast<size_t>(id), input);
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
				mSources->SetInputBuffer(static_cast<size_t>(id), input);
				GetOutput(output);
				for (int j = 0; j < outputBufferLength; j++)
					outputBuffer[count++] = output[j];
				input[0] = 0.0;
			}
			// Process remaining samples
			mSources->SetInputBuffer(static_cast<size_t>(id), input);
			GetOutput(output);
			for (int i = 0; i < remainder; i++)
				outputBuffer[count++] = output[i];

			RemoveSource(static_cast<size_t>(id));
			UpdateImpulseResponseMode(false);
		}
	}
}