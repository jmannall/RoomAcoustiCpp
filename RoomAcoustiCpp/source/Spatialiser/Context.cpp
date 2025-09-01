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

				// Update IEM
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

		Context::Context(const std::shared_ptr<DSPConfig> config) : mConfig(config), mIsRunning(true), IEMThread(), rayTracingThread(), applyHeadphoneEQ(false), headphoneEQ(2048)
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

			switch (mConfig->lateReverbModel.load(std::memory_order_acquire))
			{
			case LateReverbModel::fdn:
				mReverb = std::make_shared<SingleFDN>(&mCore, mConfig);
				break;
			case LateReverbModel::raves:
				mReverb = std::make_shared<RAVES>(&mCore, mConfig);
				break;
			default:
				// Unknown late reverb model, using default SingleFDN
				mReverb = std::make_shared<SingleFDN>(&mCore, mConfig);
				break;
			}

			mSources = std::make_shared<SourceManager>(&mCore, mConfig);
			InitialiseAudio();
			mRoom = std::make_shared<Room>(mConfig->frequencyBands.Length());

			mImageEdgeModel = std::make_shared<ImageEdge>(mRoom, mSources, mReverb, mConfig);
			mRayTracing = std::make_shared<TracingThread>(mRoom, mSources, mReverb, mConfig);
			
			// Initialize NNs
			myNN_initialize();

			// Start background thread after all systems are initialized
			IEMThread = std::thread(IEMProcessor, this);
			rayTracingThread = std::thread(RayTracerProcessor, this);
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

		void Context::InitialiseAudio()
		{
			while (audioFlag.exchange(true, std::memory_order_acquire))
				std::this_thread::yield();

			size_t numThreads = std::min((unsigned int)8, std::thread::hardware_concurrency());
			switch (mConfig->lateReverbModel.load(std::memory_order_acquire))
			{
			default:
			case LateReverbModel::fdn:
				audioThreadPool = std::make_unique<AudioThreadPool>(numThreads, mConfig->numFrames, mConfig->numReverbSources, mConfig->numFrames, mConfig->numReverbSources);
				// mReverb = std::make_shared<SingleFDN>(&mCore, mConfig);
				mReverbInput = Matrix<>(mConfig->numReverbSources, mConfig->numFrames);
				break;
			case LateReverbModel::raves:
				audioThreadPool = std::make_unique<AudioThreadPool>(numThreads, mConfig->numFrames, mConfig->GetNumRavesFDNs(), 2 * mConfig->numFrames, mConfig->numReverbSources);
				// mReverb = std::make_shared<RAVES>(&mCore, mConfig);
				mSources->UpdateNumRavesFDNs(mConfig->GetNumRavesFDNs());
				mReverbInput = Matrix<>(mConfig->GetNumRavesFDNs(), 2 * mConfig->numFrames);
				break;
			}
			audioFlag.store(false, std::memory_order_release);
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
			Coefficients<> T60 = mRoom->GetReverbTime();
			mReverb->SetTargetT60(T60);
		}

		////////////////////////////////////////

		void Context::UpdateReverbTime(const Coefficients<>& T60)
		{
			mRoom->UpdateReverbTime(T60);
			mReverb->SetTargetT60(T60);
		}

		////////////////////////////////////////

		void Context::UpdateDiffractionModel(const DiffractionModel model)
		{
			mConfig->diffractionModel.store(model, std::memory_order_release);
			mImageEdgeModel->UpdateDiffractionModel(model);
			mSources->UpdateDiffractionModel(model);
		}

		////////////////////////////////////////

		// How to stop ImageSources and Sources trying the write in old format to new
		// reverbSend matrix (in case IE model slow to update)?
		void Context::UpdateLateReverbModel(const LateReverbModel model)
		{
			//while (audioFlag.exchange(true, std::memory_order_acquire))
			//	std::this_thread::yield();

			//// SingleFDN currently not working - background thread keeps running. Tries to access Reverb when being changed/before initialised
			//mConfig->lateReverbModel.store(model, std::memory_order_release);
			//mImageEdgeModel->UpdateLateReverbModel(model);
			//audioThreadPool->Stop(); // Stop the audio thread pool to reinitialize the reverb
			//InitialiseAudio();

			//if (model == LateReverbModel::raves)
			//{
			//	std::vector<Coefficients<>> T60s(mConfig->numRavesFDNs, Coefficients<>(1));
			//	for (int i = 0; i < mConfig->numRavesFDNs; ++i)
			//		T60s[i] = (0.5 * i + 0.5) * mRoom->GetReverbTime();
			//	Vec delayLineLengths({1.0, 1.7, 3.2});
			//	mReverb->InitLateReverb(T60s, delayLineLengths, FDNMatrix::randomOrthogonal, mConfig);
			//	std::vector<Absorption<>> listenerResidues(mConfig->numRavesFDNs, Absorption<>(mConfig->numReverbSources));
			//	for (int i = 0; i < mConfig->numRavesFDNs; ++i)
			//	{
			//		for (int j = 0; j < mConfig->numReverbSources; ++j)
			//			listenerResidues[i][j] = (0.5 * i - 0.5) + (0.2 * j - 0.6);
			//	}
			//	mReverb->SetTargetOutputFilters(listenerResidues);
			//}

			//audioFlag.store(false, std::memory_order_release);
		}

		////////////////////////////////////////

		void Context::InitLateReverb(const Real volume, const Vec<>& dimensions, const FDNMatrix matrix)
		{
			Coefficients T60 = mRoom->GetReverbTime(volume);
			if (dimensions.Rows() == 0)
			{
				Debug::Log("No dimensions provided for room", Colour::Red);
				mReverb->InitLateReverb(T60, matrix, mConfig);
			}
			else
				mReverb->InitLateReverb(T60, dimensions, matrix, mConfig);
		}

		void Context::InitRAVES(const std::string& folderPath, const FDNMatrix matrix)
		{
			// Folder path should end with file separator
			auto info = Parse1Dcsv<int>(folderPath + "info.csv");
			int numTriangles = info[0];
			int numPaths = info[1];
			int numModes = info[2];

			// TODO: Ensure info always has 4 entries
			int numFrequencyBands = 1;
			if (info.size() > 3)
				numFrequencyBands = info[3];

			assert(mRoom->GetNumberOfWalls() == numTriangles);

			auto indexing = Parse2Dcsv<int>(folderPath + "indexing.csv");

			assert(indexing.size() == numTriangles);
			for (int i = 0; i < numTriangles; i++)
				assert(indexing[i].size() == numTriangles);
			for (int i = 0; i < numTriangles; i++) {
				for (int j = 0; j < numTriangles; j++) {
					assert(indexing[i][j] >= -1);
					assert(indexing[i][j] < numPaths);
				}
			}
			
			// Store mode_1_freq_1, mode_1_freq_2, mode_2_freq_1, mode_2_freq_2...
			std::vector<Real> t60s(numModes * numFrequencyBands);
			Vec<> energyDecay(numModes * numFrequencyBands);
			std::vector<Vec<>> rightEigenvectors(numModes * numFrequencyBands);
			std::vector<Vec<>> leftEigenvectors(numModes * numFrequencyBands);
			int count = 0;
			for (int i = 0; i < numModes; i++)
			{
				auto mode = Parse2Dcsv<Real>(folderPath + "mode_" + std::to_string(i + 1) + ".csv");

				assert(mode.size() == 3 * numFrequencyBands);
				for (int j = 0; j < numFrequencyBands; j++)
				{
					int idx = 3 * j;
					assert(mode[idx].size() == 2);
					assert(mode[idx + 1].size() == numPaths);
					assert(mode[idx + 2].size() == numPaths);

					t60s[count] = mode[idx][0];
					energyDecay[count] = mode[idx][1];

					rightEigenvectors[count] = mode[idx + 1];
					leftEigenvectors[count] = mode[idx + 2];
					count++;
				}
			}
			// TODO: Manage how number of modes is set and controlled by user
			mConfig->numRavesFDNs.store(numModes * numFrequencyBands, std::memory_order_release);
			InitialiseAudio();

			mReverb->InitLateReverb(t60s, matrix, mConfig);
			mReverb->SetEigenvectors(rightEigenvectors, leftEigenvectors);

			mRayTracing->InitRoom(numPaths, indexing, energyDecay);
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

		size_t Context::InitWall(const Vertices& vData, const Absorption<>& absorption)
		{
#ifdef DEBUG_INIT
	Debug::Log("Init Wall", Colour::Green);
#endif

			Wall wall = Wall(vData, absorption);
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
			if (audioFlag.exchange(true, std::memory_order_acquire))
				return;

			PROFILE_AudioThread;
			if (outputBuffer.Length() != 2 * mConfig->numFrames)
			{
				Debug::Log("Incorrect buffer size", Colour::Red);
				outputBuffer.ResizeBuffer(2 * mConfig->numFrames);
			}

			// Reset buffers
			outputBuffer.Reset();
			mReverbInput.Reset();
			const Real lerpFactor = mConfig->GetLerpFactor();

			mSources->ProcessAudio(outputBuffer, mReverbInput, lerpFactor);
			mReverb->ProcessAudio(mReverbInput, outputBuffer, lerpFactor);

			if (applyHeadphoneEQ)
				headphoneEQ.ProcessAudio(outputBuffer, outputBuffer, lerpFactor);

			audioFlag.store(false, std::memory_order_release);
		}

		////////////////////////////////////////

		void Context::UpdateImpulseResponseMode(const bool mode)
		{
			mConfig->impulseResponseMode.store(mode, std::memory_order_release);
			mSources->UpdateImpulseResponseMode(mode);
			if (mode)
				headphoneEQ.Reset();		// TODO: Should this be here?
		}
	}
}