/*
* @brief Defines the RoomAcoustiCpp global context
*
*/

// Spatialiser headers
#include "Spatialiser/Context.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/Mutexes.h"

// Unity headers
#include "Unity/Debug.h"
#include "Unity/UnityInterface.h"

// 3DTI headers
#include "HRTF/HRTFFactory.h"
#include "HRTF/HRTFCereal.h"
#include "ILD/ILDCereal.h"

namespace RAC
{
	using namespace Unity;
	namespace Spatialiser
	{

		// Global mutex
		std::mutex tuneInMutex;

#ifndef DISABLE_SOFA_SUPPORT
#if defined(UNITY_WIN) || (defined(TARGET_OS_OSX) && !defined(TARGET_OS_IOS))
#define ENABLE_SOFA_SUPPORT
#endif
#endif

		//////////////////// Background Thread ////////////////////

		void BackgroundProcessor(Context* context)
		{
#ifdef DEBUG_IEM_THREAD
	Debug::Log("Begin background thread", Colour::Green);
#endif
#ifdef PROFILE_BACKGROUND_THREAD
			RegisterBackgroundThread();
#endif
			bool isRunning = context->IsRunning();
			std::shared_ptr<Room> room = context->GetRoom();
			std::shared_ptr<ImageEdge> imageEdgeModel = context->GetImageEdgeModel();

			const int loop_interval_ms = 10;

			while (isRunning)
			{
				auto start_time = std::chrono::steady_clock::now();

#ifdef PROFILE_BACKGROUND_THREAD
				BeginBackgroundLoop();
#endif
				// Update IEM Config
				// imageEdgeModel->UpdateIEMConfig(context->GetIEMConfig());

				// Update IEM
				bool hasChanged = imageEdgeModel->RunIEM();

				// Update FDN reflection filters
				// if (hasChanged)
				imageEdgeModel->UpdateLateReverbFilters();

				isRunning = context->IsRunning();
#ifdef PROFILE_BACKGROUND_THREAD
				EndBackgroundLoop();
#endif
				auto end_time = std::chrono::steady_clock::now();
				auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
				if (elapsed_time < loop_interval_ms) {
					std::this_thread::sleep_for(std::chrono::milliseconds(loop_interval_ms - elapsed_time));
				}
			}

#ifdef PROFILE_BACKGROUND_THREAD
			UnregisterBackgroundThread();
#endif
#ifdef DEBUG_IEM_THREAD
	Debug::Log("End background thread", Colour::Red);
#endif
		}

		//////////////////// Context ////////////////////

		////////////////////////////////////////

		Context::Context(const Config& config) : mConfig(config), mIsRunning(true), IEMThread()
		{
#ifdef DEBUG_INIT
			Debug::Log("Init Context", Colour::Green);
#endif

			// Set dsp settings
			mCore.SetAudioState({ mConfig.fs, mConfig.numFrames });

			// Create listener
			mListener = mCore.CreateListener();
			headRadius = mListener->GetHeadRadius();

			mReverb = std::make_shared<Reverb>(&mCore, mConfig);
			mRoom = std::make_shared<Room>(mConfig.frequencyBands.Length());
			mSources = std::make_shared<SourceManager>(&mCore, mConfig);
			mImageEdgeModel = std::make_shared<ImageEdge>(mRoom, mSources, mReverb, mConfig.frequencyBands.Length());
			
			// Initialize NNs
			myNN_initialize();

			// Start background thread after all systems are initialized
			IEMThread = std::thread(BackgroundProcessor, this);

			mInputBuffer = Buffer(mConfig.numFrames);
			mOutputBuffer = Buffer(2 * mConfig.numFrames); // Stereo output buffer
			mSendBuffer = BufferF(2 * mConfig.numFrames);
			mReverbInput = Matrix(mConfig.numFrames, mConfig.numFDNChannels);
		}

		////////////////////////////////////////

		Context::~Context()
		{
#ifdef DEBUG_REMOVE
	Debug::Log("Exit Context", Colour::Red);
#endif

			StopRunning();
			IEMThread.join();

			mImageEdgeModel.reset();
			mSources.reset();
			mRoom.reset();
			mReverb.reset();

			// Terminate NNs
			myNN_terminate();

			lock_guard<mutex> audioLock(tuneInMutex);
			mCore.RemoveListener();
		}

		////////////////////////////////////////

		bool Context::LoadSpatialisationFiles(const int hrtfResamplingStep, const std::vector<std::string>& filePaths)
		{
			lock_guard<mutex> lock(tuneInMutex);

#ifdef DEBUG_HRTF
			Debug::Log("HRTF resampling step: " + mCore.GetHRTFResamplingStep(), Colour::Blue);
			Debug::Log("HRTF file path: " + filePaths[0], Colour::Blue);
			Debug::Log("Near field file path: " + filePaths[1], Colour::Blue);
			Debug::Log("ILD file path: " + filePaths[2], Colour::Blue);
#endif

			// Set HRTF resampling step
			mCore.SetHRTFResamplingStep(hrtfResamplingStep);

			// Load high quality files
			bool result = HRTF::CreateFrom3dti(filePaths[0], mListener);
			if (result)
				result = ILD::CreateFrom3dti_ILDNearFieldEffectTable(filePaths[1], mListener);
			else
			{
#ifdef DEBUG_HRTF
				Debug::Log("Failed to load HRTF files", Colour::Red);
#endif
			}
			// Load high performance files
			if (result)
				result = ILD::CreateFrom3dti_ILDSpatializationTable(filePaths[2], mListener);
			else
			{
#ifdef DEBUG_HRTF
				Debug::Log("Failed to load near field files", Colour::Red);
#endif
			}

			if (!result)
			{
#ifdef DEBUG_HRTF
				Debug::Log("Failed to load ILD field files", Colour::Red);
#endif
			}

			return result;
		}

		////////////////////////////////////////

		void Context::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			mConfig.spatialisationMode = mode;
			mReverb->UpdateSpatialisationMode(mode);
			mSources->UpdateSpatialisationMode(mode);
		}

		////////////////////////////////////////

		void Context::UpdateReverbTimeModel(const ReverbFormula model)
		{
			Coefficients T60 = mRoom->UpdateReverbTimeModel(model);
			UpdateReverbTime(T60);
		}

		////////////////////////////////////////

		void Context::UpdateDiffractionModel(const DiffractionModel model)
		{
			mConfig.diffractionModel = model;
			mSources->UpdateDiffractionModel(model);
		}

		////////////////////////////////////////

		void Context::UpdateRoom(const Real volume, const Vec& dimensions)
		{
			Coefficients T60 = mRoom->GetReverbTime(volume);
			if (dimensions.Rows() == 0)
			{
				Debug::Log("No dimensions provided for room", Colour::Red);
				Vec defaultDimensions = Vec(3); // Assume a shoebox
				defaultDimensions[0] = 2.5; // Assume height
				defaultDimensions[1] = 4.0; // Assume width
				defaultDimensions[2] = volume / 10.0; // Calculate depth
				mReverb->UpdateFDNParameters(T60, defaultDimensions);
			}
			else
				mReverb->UpdateFDNParameters(T60, dimensions);
		}

		////////////////////////////////////////

		void Context::UpdateListener(const Vec3& position, const Vec4& orientation)
		{
#if DEBUG_UPDATE
	Debug::Log("Update Listener", Colour::Yellow);
#endif
			listenerPosition = position;

			// Set listener position and orientation
			CTransform transform;
			transform.SetOrientation(CQuaternion(static_cast<float>(orientation.w), static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)));
			transform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));
			
			{
				lock_guard<mutex> lock(tuneInMutex);
				mListener->SetListenerTransform(transform);
				mReverb->UpdateReverbSourcePositions(position);
			}
			mImageEdgeModel->SetListenerPosition(position);
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
#ifdef DEBUG_UPDATE
			Debug::Log("Update Source", Colour::Yellow);
#endif
			Real distance = (position - listenerPosition).Length();

			// Ensure source is outside listener head radius
			if (distance < headRadius)
			{
				Vec3 newPosition = position;
				if (distance == 0.0)
					newPosition = mSources->GetSourcePosition(id);

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

		size_t Context::InitWall(const Vertices& vData, const Absorption& absorption)
		{
#ifdef DEBUG_INIT
	Debug::Log("Init Wall and Edges", Colour::Green);
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

		void Context::UpdateWallAbsorption(size_t id, const Absorption& absorption)
		{
			mRoom->UpdateWallAbsorption(id, absorption);
			UpdateReverbTime(mRoom->GetReverbTime());
		}

		////////////////////////////////////////

		void Context::RemoveWall(size_t id)
		{
#ifdef DEBUG_REMOVE
	Debug::Log("Remove Wall and Edges", Colour::Red);
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

		void Context::SubmitAudio(size_t id, const float* data)
		{
			unique_lock<mutex> lock(audioMutex, std::defer_lock);
			if (lock.try_lock())
			{
				for (int i = 0; i < mConfig.numFrames; i++)
					mInputBuffer[i] = static_cast<Real>(data[i]);
				mSources->ProcessAudio(id, mInputBuffer, mReverbInput, mOutputBuffer);
			}
		}

		////////////////////////////////////////

		void Context::GetOutput(float** bufferPtr)
		{
			// Process reverb
			unique_lock<mutex> lock(audioMutex, std::defer_lock);
			if (lock.try_lock())
			{
				mReverb->ProcessAudio(mReverbInput, mOutputBuffer);
				mReverbInput.Reset();
			}
			if (!lock.owns_lock())
			{
				while (lock.try_lock() == false) { std::this_thread::yield(); }
			}
			// Copy output to send and set pointer
			mSendBuffer = mOutputBuffer;
			*bufferPtr = &mSendBuffer[0];

			// Reset output buffer
			mOutputBuffer.ResetBuffer();
		}
	}
}