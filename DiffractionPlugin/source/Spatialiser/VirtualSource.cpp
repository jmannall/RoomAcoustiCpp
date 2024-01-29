/*
*
*  \VirtualSource class
*  \Currently can only handle one edge per path/virtual source
*
*/

#include "Spatialiser/VirtualSource.h"

using namespace Common;
namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// VirtualSourceData class ////////////////////

		std::vector<size_t> VirtualSourceData::GetWallIDs() const
		{
			std::vector<size_t> ret;
			for (Part part : parts)
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

		// Append EdSp to a SpEd source
		//bool VirtualSourceData::AppendVSource(VirtualSourceData& data, const vec3& listenerPosition)
		//{
		//	assert((IsReflection(order - 1) && data.IsReflection(0)) == false);
		//	assert(GetID() == data.GetID(0)); // last ref or diffraction was the same

		//	if (data.reflection)
		//	{
		//		Absorption absorption;
		//		data.GetAbsorption(absorption);
		//		AddWallIDs(data.GetWallIDs(), absorption);
		//	}
		//	mRPositions = data.mRPositions;
		//	if (!reflection)
		//		reflection = data.reflection;

		//	Edge edge = mDiffractionPath.GetEdge();
		//	Diffraction::Path oldPath = mDiffractionPath;
		//	mDiffractionPath = Diffraction::Path(GetPosition(), data.GetRPosition(), edge);

		//	if (mDiffractionPath.valid) // Should be valid as previous paths were valid (unless apex has moved)
		//	{
		//		valid = true;
		//		if (mDiffractionPath.GetApex() == oldPath.GetApex() && mDiffractionPath.GetApex() == data.mDiffractionPath.GetApex())
		//		{
		//			vec3 vPosition = listenerPosition + (mDiffractionPath.sData.d + mDiffractionPath.rData.d) * UnitVector(data.GetTransformPosition() - listenerPosition);
		//			transform.SetPosition(Common::CVector3(vPosition.x, vPosition.y, vPosition.z));
		//			if (visible && data.visible)
		//				return false;
		//		}
		//		else
		//		{
		//			vec3 vPosition = listenerPosition + (mDiffractionPath.sData.d + mDiffractionPath.rData.d) * UnitVector(data.GetTransformPosition() - listenerPosition);
		//			transform.SetPosition(Common::CVector3(vPosition.x, vPosition.y, vPosition.z));
		//		}
		//	}
		//	else
		//		valid = false;
		//	return true;
		//}

		VirtualSourceData VirtualSourceData::Trim(const int i)
		{
			order = i + 1;

			if (!mPositions.empty())
				mPositions.erase(mPositions.begin() + order, mPositions.end());
			if (!mRPositions.empty())
				mRPositions.erase(mRPositions.begin() + order, mRPositions.end());
			if (!parts.empty())
				parts.erase(parts.begin() + order, parts.end());

			feedsFDN = false;
			mFDNChannel = -1;

			Reset();

			key = "";
			reflection = false;
			diffraction = false;
			int j = 0;
			for (Part part : parts)
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


			/*reflection = false;
			diffraction = false;
			for (int i = 0; i < order; i++)
			{
				if (isReflection[i])
				{
					reflection = true;
					key = key + IntToStr(IDs[i]) + "r";
				}
				else
				{
					diffraction = true;
					key = key + IntToStr(IDs[i]) + "d";
				}
			}*/

			return *this;
		}

		//////////////////// VirtualSource class ////////////////////

		VirtualSource::VirtualSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs, const VirtualSourceData& data, int fdnChannel) : mCore(core), mSource(NULL), mCurrentGain(0.0f), mTargetGain(0.0f), mFilter(4, fs), isInitialised(false), mHRTFMode(hrtfMode), feedsFDN(data.feedsFDN), mFDNChannel(fdnChannel), mDiffractionPath(data.mDiffractionPath), btm(&mDiffractionPath, fs), reflection(data.reflection), diffraction(data.diffraction)
		{
			UpdateVirtualSource(data, fdnChannel);
		}

		VirtualSource::VirtualSource(const VirtualSource& vS) : mCore(vS.mCore), mSource(vS.mSource), mPosition(vS.mPosition), mFilter(vS.mFilter), isInitialised(vS.isInitialised), mHRTFMode(vS.mHRTFMode), feedsFDN(vS.feedsFDN), mFDNChannel(vS.mFDNChannel), mDiffractionPath(vS.mDiffractionPath), btm(vS.btm), mTargetGain(vS.mTargetGain), mCurrentGain(vS.mCurrentGain), reflection(vS.reflection), diffraction(vS.diffraction), mVirtualSources(vS.mVirtualSources), mVirtualEdgeSources(vS.mVirtualEdgeSources)
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

		// Obselete? now sources removed automatically
		/*void VirtualSource::RemoveVirtualSources(const size_t& id)
		{
			if (!mVirtualSources.empty())
			{
				mVirtualSources.erase(id);
				for (auto it = mVirtualSources.begin(); it != mVirtualSources.end(); it++)
				{
					it->second.RemoveVirtualSources(id);
				}
			}
		}*/

		int VirtualSource::ProcessAudio(const Real* data, const size_t& numFrames, matrix& reverbInput, Buffer& outputBuffer, const Real lerpFactor)
		{
			int ret = 0;
			{
				lock_guard<mutex> lock(vWallMutex);
				// for (auto it : mVirtualSources)
				for (auto it = mVirtualSources.begin(); it != mVirtualSources.end(); it++)
				{
					ret += it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer, lerpFactor);
				}
			}
			{
				lock_guard<mutex> lock(vEdgeMutex);
				for (auto it = mVirtualEdgeSources.begin(); it != mVirtualEdgeSources.end(); it++)
				{
					ret += it->second.ProcessAudio(data, numFrames, reverbInput, outputBuffer, lerpFactor);
				}
			}

			ret++;
			lock_guard<mutex> lock(audioMutex);
			if (isInitialised)
			{
				// Copy input into internal storage and apply wall absorption
				CMonoBuffer<Real> bStore(numFrames);
				CMonoBuffer<float> bInput(numFrames);
				const Real* inputPtr = data;

				if (diffraction)
				{
					{
						//lock_guard<mutex> lock(audioMutex);
						btm.ProcessAudio(inputPtr, &bStore[0], numFrames, lerpFactor);
						if (reflection)
						{
							for (int i = 0; i < numFrames; i++)
								bStore[i] = mFilter.GetOutput(bStore[i]);
						}
					}
				}
				else if (reflection)
				{
					//lock_guard<mutex> lock(audioMutex);
					for (int i = 0; i < numFrames; i++)
						bStore[i] = mFilter.GetOutput(*inputPtr++);
				}

				if (mCurrentGain == mTargetGain)
				{
					for (int i = 0; i < numFrames; i++)
						bInput[i] = static_cast<float>(bStore[i] * mCurrentGain);
				}
				else
				{
					for (int i = 0; i < numFrames; i++)
					{
						bInput[i] = static_cast<float>(bStore[i] * mCurrentGain);
						mCurrentGain = Lerp(mCurrentGain, mTargetGain, lerpFactor);
					}
				}
		
				mSource->SetBuffer(bInput);

				CEarPair<CMonoBuffer<float>> bOutput;
				if (feedsFDN)
				{
					CMonoBuffer<float> bMonoOutput;
					{
						//lock_guard<mutex> lock(audioMutex);
						mSource->ProcessAnechoic(bMonoOutput, bOutput.left, bOutput.right);
						for (int i = 0; i < numFrames; i++)
							reverbInput.IncreaseEntry(bMonoOutput[i], i, mFDNChannel);
					}
				}
				else
				{
					//lock_guard<mutex> lock(audioMutex);
					mSource->ProcessAnechoic(bOutput.left, bOutput.right);
				}
				int j = 0;
				for (int i = 0; i < numFrames; i++)
				{
					outputBuffer[j++] += bOutput.left[i];
					outputBuffer[j++] += bOutput.right[i];
				}
			}
			return ret;
		}

		void VirtualSource::Init(const VirtualSourceData& data)
		{
#ifdef DEBUG_VIRTUAL_SOURCE
	Debug::Log("Init virtual source", Colour::Green);
#endif

			if (reflection) // Init reflection filter
			{
				Real fc[] = { 250, 500, 1000, 2000, 4000 };
				Real g[5];
				data.GetAbsorption(g);
				mFilter.UpdateParameters(fc, g);
			}

			// Set btm currentIr
			if (diffraction)
				btm.InitParameters();

			// Initialise source to core
			mSource = mCore->CreateSingleSourceDSP();

			//Select spatialisation mode
			switch (mHRTFMode)
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
			mPosition = data.GetPosition();
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