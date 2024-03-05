/*
*
*  \VirtualSource class
*  \Currently can only handle one edge per path/virtual source
*
*/

// Spatialiser headers
#include "Spatialiser/VirtualSource.h"

// Unity headers
#include "Unity/UnityInterface.h"

// DSP headers
#include "DSP/Interpolate.h"

using namespace Common;
namespace UIE
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{

		//////////////////// VirtualSourceData class ////////////////////

		std::vector<size_t> VirtualSourceData::GetWallIDs() const
		{
			std::vector<size_t> ret;
			for (Part part : pathParts)
			{
				if (part.isReflection)
					ret.push_back(part.id);
			}
			return ret;
		}

		vec3 VirtualSourceData::GetPosition(int i) const
		{
			assert(i < order);
			return mPositions[i];
		}

		vec3 VirtualSourceData::GetTransformPosition()
		{
			CVector3 position = transform.GetPosition();
			return vec3(position.x, position.y, position.z);
		}

		void VirtualSourceData::SetTransform(const vec3& vSourcePosition)
		{
			transform.SetPosition(CVector3(static_cast<float>(vSourcePosition.x), static_cast<float>(vSourcePosition.y), static_cast<float>(vSourcePosition.z)));
			mPositions.push_back(vSourcePosition);
		}

		void VirtualSourceData::SetTransform(const vec3& vSourcePosition, const vec3& vEdgeSourcePosition)
		{
			transform.SetPosition(CVector3(static_cast<float>(vEdgeSourcePosition.x), static_cast<float>(vEdgeSourcePosition.y), static_cast<float>(vEdgeSourcePosition.z)));
			mPositions.push_back(vSourcePosition);
		}

		void VirtualSourceData::UpdateTransform(const vec3& vEdgeSourcePosition)
		{
			transform.SetPosition(CVector3(static_cast<float>(vEdgeSourcePosition.x), static_cast<float>(vEdgeSourcePosition.y), static_cast<float>(vEdgeSourcePosition.z)));
		}


		vec3 VirtualSourceData::GetRPosition(int i) const
		{
			assert(i < order);
			return mRPositions[i];
		}

		void VirtualSourceData::SetRPosition(const vec3& vReceiverPosition)
		{
			mRPositions.push_back(vReceiverPosition);
		}

		void VirtualSourceData::SetRTransform(const vec3& vReceiverPosition, const vec3& vEdgeSourcePosition)
		{
			transform.SetPosition(CVector3(static_cast<float>(vEdgeSourcePosition.x), static_cast<float>(vEdgeSourcePosition.y), static_cast<float>(vEdgeSourcePosition.z)));
			mRPositions.push_back(vReceiverPosition);
		}

		VirtualSourceData VirtualSourceData::Trim(const int i)
		{
			order = i + 1;

			if (!mPositions.empty())
				mPositions.erase(mPositions.begin() + order, mPositions.end());
			if (!mRPositions.empty())
				mRPositions.erase(mRPositions.begin() + order, mRPositions.end());
			if (!pathParts.empty())
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
					key = key + IntToStr(part.id) + "r";
				}
				else
				{
					diffraction = true;
					key = key + IntToStr(part.id) + "d";
				}
				j++;
			}
			return *this;
		}

		//////////////////// VirtualSource class ////////////////////

		VirtualSource::VirtualSource(Binaural::CCore* core, const Config& config) : mCore(core), mSource(NULL), order(0), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(4, config.frequencyBands, config.fs), isInitialised(false), mConfig(config), feedsFDN(false), mFDNChannel(-1), btm(&mDiffractionPath, config.fs), reflection(false), diffraction(false)
		{
			bInput = CMonoBuffer<float>(mConfig.numFrames);
			bStore.ResizeBuffer(mConfig.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
			bMonoOutput = CMonoBuffer<float>(mConfig.numFrames);
		}

		VirtualSource::VirtualSource(Binaural::CCore* core, const Config& config, const VirtualSourceData& data, int fdnChannel) : mCore(core), mSource(NULL), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(4, config.frequencyBands, config.fs), isInitialised(false), mConfig(config), feedsFDN(data.feedsFDN), mFDNChannel(fdnChannel), mDiffractionPath(data.mDiffractionPath), btm(&mDiffractionPath, config.fs), reflection(data.reflection), diffraction(data.diffraction)
		{
			bInput = CMonoBuffer<float>(mConfig.numFrames);
			bStore.ResizeBuffer(mConfig.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
			bMonoOutput = CMonoBuffer<float>(mConfig.numFrames);
			UpdateVirtualSource(data, fdnChannel);
		}

		VirtualSource::VirtualSource(const VirtualSource& vS) : mCore(vS.mCore), mSource(vS.mSource), mFilter(vS.mFilter), isInitialised(vS.isInitialised), mConfig(vS.mConfig), feedsFDN(vS.feedsFDN), mFDNChannel(vS.mFDNChannel), mDiffractionPath(vS.mDiffractionPath), btm(vS.btm), mTargetGain(vS.mTargetGain), mCurrentGain(vS.mCurrentGain), reflection(vS.reflection), diffraction(vS.diffraction), mVirtualSources(vS.mVirtualSources), mVirtualEdgeSources(vS.mVirtualEdgeSources), bInput(vS.bInput), bStore(vS.bStore), bOutput(vS.bOutput), bMonoOutput(vS.bMonoOutput)
		{
			btm.UpdatePath(&mDiffractionPath);
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
				lock_guard<mutex> lock(audioMutex);
				mTargetGain = 1.0f;
				if (!isInitialised)
					Init(data);
				Update(data, fdnChannel);
			}
			else
			{
				std::unique_lock<mutex> lock(audioMutex);
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

		void VirtualSource::UpdateSpatialisationMode(const HRTFMode& mode)
		{
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

		void VirtualSource::UpdateSpatialisationMode(const SPATConfig& config)
		{
			mConfig.spatConfig = config;
			UpdateSpatialisationMode(config.GetMode(order));
			{
				lock_guard<mutex> lock(vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.UpdateSpatialisationMode(config);
			}
			{
				lock_guard<mutex> lock(vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.UpdateSpatialisationMode(config);
			}
		}

		void VirtualSource::ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer)
		{
			{
				lock_guard<mutex> lock(vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer);
			}
			{
				lock_guard<mutex> lock(vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.ProcessAudio(data, reverbInput, outputBuffer);
			}

			lock_guard<mutex> lock(audioMutex);
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
						btm.ProcessAudio(data, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
						EndDiffraction();
#endif
						if (reflection)
						{
#ifdef PROFILE_AUDIO_THREAD
							BeginReflection();
#endif
							for (int i = 0; i < mConfig.numFrames; i++)
								bStore[i] = mFilter.GetOutput(bStore[i]);
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
					for (int i = 0; i < mConfig.numFrames; i++)
						bStore[i] = mFilter.GetOutput(data[i]);
#ifdef PROFILE_AUDIO_THREAD
					EndReflection();
#endif
				}

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
						Lerp(mCurrentGain, mTargetGain, mConfig.lerpFactor);
					}
				}
		
#ifdef PROFILE_AUDIO_THREAD
				Begin3DTI();
#endif
				mSource->SetBuffer(bInput);

				if (feedsFDN)
				{
					{
						mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
						for (int i = 0; i < mConfig.numFrames; i++)
							reverbInput.IncreaseEntry(static_cast<Real>(bMonoOutput[i]), i, mFDNChannel);
					}
				}
				else
					mSource->ProcessAnechoic(bOutput.left, bOutput.right);

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
#ifdef DEBUG_VIRTUAL_SOURCE
	Debug::Log("Init virtual source", Colour::Green);
#endif

			if (reflection) // Init reflection filter
			{
				Absorption g = data.GetAbsorption();
				mFilter.UpdateParameters(g);
			}

			// Set btm currentIr
			if (diffraction)
				btm.InitParameters();

			// Initialise source to core
			mSource = mCore->CreateSingleSourceDSP();
			
			order = data.GetOrder();

			//Select spatialisation mode
			UpdateSpatialisationMode(mConfig.spatConfig.GetMode(order));

			switch (mConfig.hrtfMode)
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
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			}
			mSource->EnablePropagationDelay();

			isInitialised = true;
		}

		void VirtualSource::Update(const VirtualSourceData& data, int& fdnChannel)
		{
			if (diffraction)
			{
				mDiffractionPath = data.mDiffractionPath;
				btm.UpdateParameters();
			}

			if (data.feedsFDN != feedsFDN)
			{
				feedsFDN = data.feedsFDN;
				int oldChannel = mFDNChannel;
				mFDNChannel = fdnChannel;
				fdnChannel = oldChannel;
			}
			mSource->SetSourceTransform(data.transform);
		}

		void VirtualSource::Remove()
		{
#ifdef DEBUG_VIRTUAL_SOURCE
	Debug::Log("Remove virtual source", Colour::Red);
#endif
			mCore->RemoveSingleSourceDSP(mSource);
			isInitialised = false;
		}
	}
}