/*
* @class VirtualSource, VirtualsourceData
*
* @brief Declaration of VirtualSource and VirtualsourceData classes
* 
* @remarks Currently, it can only handle one diffracting edge per path/virtual source
*
*/

// Spatialiser headers
#include "Spatialiser/VirtualSource.h"
#include "Spatialiser/Mutexes.h"
#include "Spatialiser/Types.h"

// Unity headers
#include "Unity/UnityInterface.h"

// DSP headers
#include "DSP/Interpolate.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{

		//////////////////// VirtualSourceData class ////////////////////

		vec3 VirtualSourceData::GetPosition(const int i) const
		{
			assert(i < order);
			return mPositions[i];
		}

		void VirtualSourceData::SetTransform(const vec3& vSourcePosition)
		{
			transform.SetPosition(CVector3(static_cast<float>(vSourcePosition.x), static_cast<float>(vSourcePosition.y), static_cast<float>(vSourcePosition.z)));
			mPositions.emplace_back(vSourcePosition);
		}

		void VirtualSourceData::SetTransform(const vec3& vSourcePosition, const vec3& vEdgeSourcePosition)
		{
			transform.SetPosition(CVector3(static_cast<float>(vEdgeSourcePosition.x), static_cast<float>(vEdgeSourcePosition.y), static_cast<float>(vEdgeSourcePosition.z)));
			mPositions.emplace_back(vSourcePosition);
		}

		VirtualSourceData VirtualSourceData::Trim(const int i)
		{
			order = i + 1;

			if (mPositions.size() > i)
				mPositions.erase(mPositions.begin() + order, mPositions.end());
			if (pathParts.size() > i)
				pathParts.erase(pathParts.begin() + order, pathParts.end());

			feedsFDN = false;
			mFDNChannel = -1;

			Reset();

			key = "";
			reflection = false;
			diffraction = false;
			int j = 0;
			for (Part part : pathParts)
			{
				if (part.isReflection)
				{
					reflection = true;
					key = key + std::to_string(part.id) + "r";
				}
				else
				{
					diffraction = true;
					key = key + std::to_string(part.id) + "d";
				}
				j++;
			}
			return *this;
		}

		void VirtualSourceData::SetDistance(const vec3& listenerPosition)
		{
			if (diffraction)
				distance = mDiffractionPath.rData.d + mDiffractionPath.sData.d;
			else
				distance = (listenerPosition - GetPosition()).Length();
		}

		//////////////////// VirtualSource class ////////////////////

		VirtualSource::VirtualSource(Binaural::CCore* core, const Config& config) : mCore(core), mSource(NULL), order(0), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(config.frequencyBands, config.Q, config.fs), isInitialised(false), mConfig(config), feedsFDN(false), mFDNChannel(-1),
			attenuate(&mDiffractionPath), lowPass(&mDiffractionPath, config.fs), udfa(&mDiffractionPath, config.fs), udfai(&mDiffractionPath, config.fs), nnSmall(&mDiffractionPath), nnBest(&mDiffractionPath), utd(&mDiffractionPath, config.fs), btm(&mDiffractionPath, config.fs),
			reflection(false), diffraction(false), mAirAbsorption(config.fs), bStore(config.numFrames)
		{
			vWallMutex = std::make_shared<std::mutex>();
			vEdgeMutex = std::make_shared<std::mutex>();
			audioMutex = std::make_shared<std::mutex>();
			bInput = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
			bMonoOutput = CMonoBuffer<float>(mConfig.numFrames);
			UpdateDiffractionModel(config.diffractionModel);
		}

		VirtualSource::VirtualSource(Binaural::CCore* core, const Config& config, const VirtualSourceData& data, int fdnChannel) : mCore(core), mSource(NULL), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(data.GetAbsorption(), config.frequencyBands, config.Q, config.fs), isInitialised(false), mConfig(config), feedsFDN(data.feedsFDN), mFDNChannel(fdnChannel),
			attenuate(&mDiffractionPath), lowPass(&mDiffractionPath, config.fs), udfa(&mDiffractionPath, config.fs), udfai(&mDiffractionPath, config.fs), nnSmall(&mDiffractionPath), nnBest(&mDiffractionPath), utd(&mDiffractionPath, config.fs), btm(&mDiffractionPath, config.fs),
			reflection(data.reflection), diffraction(data.diffraction), mAirAbsorption(data.distance, mConfig.fs), bStore(config.numFrames)
		{
			vWallMutex = std::make_shared<std::mutex>();
			vEdgeMutex = std::make_shared<std::mutex>();
			audioMutex = std::make_shared<std::mutex>();
			bInput = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
			bMonoOutput = CMonoBuffer<float>(mConfig.numFrames);
			UpdateDiffractionModel(config.diffractionModel);
			UpdateVirtualSource(data, fdnChannel);
		}

		VirtualSource::~VirtualSource()
		{
			mCore->RemoveSingleSourceDSP(mSource);
			Reset();
		};

		bool VirtualSource::UpdateVirtualSource(const VirtualSourceData& data, int& fdnChannel)
		{
			if (data.visible) // Process virtual source - Init if doesn't exist
			{
				lock_guard<mutex> lock(*audioMutex);
				mTargetGain = 1.0f;
				if (!isInitialised)
					Init(data);
				Update(data, fdnChannel);
			}
			else
			{
				std::unique_lock<mutex> lock(*audioMutex);
				mTargetGain = 0.0f;
				if (mCurrentGain < 0.0001)
				{
					if (isInitialised)
						Remove();
					lock.unlock();
					return true;
				}
			}
			return false;
		}

		void VirtualSource::UpdateSpatialisationMode(const HRTFMode mode)
		{
			lock_guard<mutex> lock(tuneInMutex);
			switch (mode)
			{
			case HRTFMode::quality:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
				break;
			}
			case HRTFMode::performance:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			case HRTFMode::none:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::NoSpatialization);
				break;
			}
			default:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::NoSpatialization);
				break;
			}
			}
		}

		void VirtualSource::UpdateSpatialisationMode(const SPATConfig config)
		{
			mConfig.spatConfig = config;
			if (isInitialised)
				UpdateSpatialisationMode(config.GetMode(order));
			{
				lock_guard<mutex> lock(*vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.UpdateSpatialisationMode(config);
			}
			{
				lock_guard<mutex> lock(*vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.UpdateSpatialisationMode(config);
			}
		}

		void VirtualSource::UpdateDiffractionModel(const DiffractionModel model)
		{
			mConfig.diffractionModel = model;
			switch (model)
			{
			case DiffractionModel::attenuate:
			{
				mDiffractionModel = &attenuate;
				break;
			}
			case DiffractionModel::lowPass:
			{
				mDiffractionModel = &lowPass;
				break;
			}
			case DiffractionModel::udfa:
			{
				mDiffractionModel = &udfa;
				break;
			}
			case DiffractionModel::udfai:
			{
				mDiffractionModel = &udfai;
				break;
			}
			case DiffractionModel::nnBest:
			{
				mDiffractionModel = &nnBest;
				break;
			}
			case DiffractionModel::nnSmall:
			{
				mDiffractionModel = &nnSmall;
				break;
			}
			case DiffractionModel::utd:
			{
				mDiffractionModel = &utd;
				break;
			}
			case DiffractionModel::btm:
			{
				mDiffractionModel = &btm;
				break;
			}
			default:
				mDiffractionModel = &btm;
			}
			UpdateDiffraction();
		}

		void VirtualSource::ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer)
		{
			{
				lock_guard<mutex> lock(*vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer);
			}
			{
				lock_guard<mutex> lock(*vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer);
			}

			lock_guard<mutex> lock(*audioMutex);
			if (isInitialised)
			{
#ifdef PROFILE_AUDIO_THREAD
				BeginVirtualSource();
#endif
				if (diffraction)
				{
					{
#ifdef PROFILE_AUDIO_THREAD
						BeginDiffraction();
#endif
						ProcessDiffraction(data, bStore);
#ifdef PROFILE_AUDIO_THREAD
						EndDiffraction();
#endif
						if (reflection)
						{
#ifdef PROFILE_AUDIO_THREAD
							BeginReflection();
#endif
							mFilter.ProcessAudio(bStore, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
							EndReflection();
#endif
						}
					}
				}
				else if (reflection)
				{
#ifdef PROFILE_AUDIO_THREAD
					BeginReflection();
#endif
					mFilter.ProcessAudio(data, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
					EndReflection();
#endif
				}
#ifdef PROFILE_AUDIO_THREAD
				BeginAirAbsorption();
#endif
				mAirAbsorption.ProcessAudio(bStore, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
				EndAirAbsorption();
#endif
				if (mCurrentGain == mTargetGain)
				{
					for (int i = 0; i < mConfig.numFrames; i++)
						bInput[i] = static_cast<float>(bStore[i] * mCurrentGain);
				}
				else
				{
					for (int i = 0; i < mConfig.numFrames; i++)
					{
						bInput[i] = static_cast<float>(bStore[i] * mCurrentGain);
						mCurrentGain = Lerp(mCurrentGain, mTargetGain, mConfig.lerpFactor);
					}
				}
		
#ifdef PROFILE_AUDIO_THREAD
				Begin3DTI();
#endif
				{
					lock_guard<mutex> lock(tuneInMutex);
					mSource->SetBuffer(bInput);

					if (feedsFDN)
					{
						{
							mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
							for (int i = 0; i < mConfig.numFrames; i++)
								reverbInput[i][mFDNChannel] += static_cast<Real>(bMonoOutput[i]);
						}
					}
					else
						mSource->ProcessAnechoic(bOutput.left, bOutput.right);
				}
#ifdef PROFILE_AUDIO_THREAD
				End3DTI();
#endif
				int j = 0;
				for (int i = 0; i < mConfig.numFrames; i++)
				{
					outputBuffer[j++] += static_cast<Real>(bOutput.left[i]);
					outputBuffer[j++] += static_cast<Real>(bOutput.right[i]);
				}
#ifdef PROFILE_AUDIO_THREAD
				EndVirtualSource();
#endif
			}
		}

		void VirtualSource::Init(const VirtualSourceData& data)
		{
			// audioMutex already locked
#ifdef DEBUG_VIRTUAL_SOURCE
	Debug::Log("Init virtual source", Colour::Green);
#endif
			reflection = data.reflection;
			diffraction = data.diffraction;

			if (reflection) // Set reflection filter
				mFilter.InitParameters(data.GetAbsorption());

			// Set btm currentIr
			if (diffraction)
			{
				mDiffractionPath = data.mDiffractionPath;
				btm.UpdatePath(&mDiffractionPath);
				btm.InitParameters();
			}

			order = data.GetOrder();
			{
				lock_guard<mutex> lock(tuneInMutex);

				// Initialise source to core
				mSource = mCore->CreateSingleSourceDSP();
				mSource->EnablePropagationDelay();
			}

			//Select spatialisation mode
			HRTFMode mode = mConfig.spatConfig.GetMode(order);
			UpdateSpatialisationMode(mode);

			isInitialised = true;
		}

		void VirtualSource::Update(const VirtualSourceData& data, int& fdnChannel)
		{
			// audioMutex already locked
			if (reflection) // Update reflection filter
				mFilter.SetGain(data.GetAbsorption());

			if (diffraction)
			{
				mDiffractionPath = data.mDiffractionPath;
				UpdateDiffraction();
			}

			mAirAbsorption.SetDistance(data.distance);

			if (data.feedsFDN != feedsFDN)
			{
				feedsFDN = data.feedsFDN;
				int oldChannel = mFDNChannel;
				mFDNChannel = fdnChannel;
				fdnChannel = oldChannel;
			}
			{
				lock_guard<mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(data.transform);
			}
		}

		void VirtualSource::Remove()
		{
#ifdef DEBUG_VIRTUAL_SOURCE
	Debug::Log("Remove virtual source", Colour::Red);
#endif
			{
				lock_guard<mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
			}
			isInitialised = false;
		}
	}
}