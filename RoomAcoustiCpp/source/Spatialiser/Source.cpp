/*
* @class Source, SourceData
*
* @brief Declaration of Source and SourceData classes
*
*/

// Common headers
#include "Common/Types.h" 
#include "Common/Vec3.h"
#include "Common/Vec4.h"

// Spatialiser headers
#include "Spatialiser/Source.h"
#include "Spatialiser/Mutexes.h"

// Unity headers
#include "Unity/Debug.h"
#include "Unity/UnityInterface.h"

// 3DTI headers
#include "BinauralSpatializer/Core.h"
#include "3dti_Toolkit/Common/CommonDefinitions.h"

// DSP headers
#include "DSP/Interpolate.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	// using namespace Unity;
	using namespace DSP;
	namespace Spatialiser
	{

		//////////////////// Source class ////////////////////

		Source::Source(Binaural::CCore* core, const Config& config) : mCore(core), mConfig(config), targetGain(0.0f), currentGain(0.0f), mAirAbsorption(mConfig.fs)
		{
			vWallMutex = std::make_shared<std::mutex>();
			vEdgeMutex = std::make_shared<std::mutex>();
			dataMutex = std::make_shared<std::mutex>();
			vSourcesMutex = std::make_shared<std::mutex>();

			// Initialise source to core
			{
				lock_guard<mutex> lock(tuneInMutex);
				mSource = mCore->CreateSingleSourceDSP();
				mSource->EnablePropagationDelay();
				// mSource->DisableInterpolation();

				//Select spatialisation mode
				UpdateSpatialisationMode(config.spatMode);
			}

			ResetFDNSlots();
			bStore.ResizeBuffer(mConfig.numFrames);
			bInput = CMonoBuffer<float>(config.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
		}

		Source::~Source()
		{
			{
				lock_guard<mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
			}
			Reset();
		}

		void Source::UpdateSpatialisationMode(const SpatMode mode)
		{
			mConfig.spatMode = mode;
			switch (mode)
			{
			case SpatMode::quality:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
				break;
			}
			case SpatMode::performance:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			case SpatMode::none:
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

			{
				lock_guard<mutex> lock(*vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.UpdateSpatialisationMode(mode);
			}
			{
				lock_guard<mutex> lock(*vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.UpdateSpatialisationMode(mode);
			}
		}

		void Source::UpdateDiffractionModel(const DiffractionModel model)
		{
			mConfig.diffractionModel = model;
			{
				lock_guard<mutex> lock(*vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.UpdateDiffractionModel(model);
			}
			{
				lock_guard<mutex> lock(*vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.UpdateDiffractionModel(model);
			}
		}

		void Source::ResetFDNSlots()
		{
			freeFDNChannels.clear();
			freeFDNChannels.push_back(0);
		}

		void ProcessVirtualSource(VirtualSource& vS, const Buffer& data, matrix& reverbInput, Buffer& outputBuffer)
		{
			vS.ProcessAudio(data, reverbInput, outputBuffer);
		}

		void Source::ProcessAudioParallel(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer)
		{
			std::vector<VirtualSource> vSources;
			{
				lock_guard<mutex> lock(*vWallMutex);
				for (auto& it : mVirtualSources)
					it.second.GetVirtualSources(vSources);
			}
			{
				lock_guard<mutex> lock(*vEdgeMutex);
				for (auto& it : mVirtualEdgeSources)
					it.second.GetVirtualSources(vSources);
			}

			const int N = 100'000'000;
			vector<double> vec(N);

//#pragma omp parallel for
//			for (int i = 0; i < vec.size(); ++i) {
//				vec[i] = sin(M_PI * float(i) / N);
//			}
//
//			double sum = 0.;
//#pragma omp barrier
//
//#pragma omp parallel for reduction(+:sum)
//			for (int i = 0; i < vec.size(); ++i) {
//				sum += vec[i];
//			}
//
//#pragma omp barrier
//			std::cout << "Result: " << M_PI * sum / N << '\n';
//			
#pragma omp parallel for
				for (int i = 0; i < 10; i++)
				{
					//int thread_id = omp_get_thread_num();
					//std::cout << "Hello from thread " << thread_id << std::endl;
					int num = vSources[i].mVirtualSources.size();
					//std::cout << "vSources " << num << thread_id << std::endl;
				}

//
//
//			int sum1 = 0;
//			//omp_set_num_threads(1);
//#pragma omp parallel
//			{
//				int localSum1(sum1);
//				// Create thread-local copies
//				//matrix localReverbInput(reverbInput);
//				//Buffer localOutputBuffer(outputBuffer);
//				//Buffer localData(data);
//
//#pragma omp for
//				for (int i = 0; i < vSources.size(); i++)
//				{
//					localSum1++;
//					//vSources[i].ProcessAudioParallel(localData, localReverbInput, localOutputBuffer);
//				}
//
//				// Combine the local results into the shared variables
//#pragma omp critical
//				{
//					sum1 += localSum1;
////					reverbInput += localReverbInput;  // Assuming operator+= is defined for matrix
////					outputBuffer += localOutputBuffer;  // Assuming operator+= is defined for Buffer
//				}
//			}
//
//#pragma omp barrier

			/*matrix rI = reverbInput;
			Buffer oB = outputBuffer;
			int sum1 = 0;
			#pragma omp parallel for num_threads(omp_get_max_threads())
			for (int i = 0; i < vSources.size(); i++)
			{
				vSources[i].ProcessAudioParallel(data, rI, oB);
			}*/
			/*for (auto& vSource : vSources)
			{				
				vSource.ProcessAudioParallel(data, reverbInput, outputBuffer);
			}*/

			/*reverbInput = rI;
			outputBuffer = oB;*/
#ifdef DEBUG_AUDIO_THREAD
			// Debug::Log("Total audio vSources: " + std::to_string(counter), Colour::Orange);
#endif
			lock_guard<std::mutex> lock(*dataMutex);
			if (isVisible || currentGain != 0.0f)
			{
#ifdef PROFILE_AUDIO_THREAD
				BeginSource();
				BeginAirAbsorption();
#endif
				mAirAbsorption.ProcessAudio(data, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
				EndAirAbsorption();
#endif
				if (currentGain == targetGain)
				{
					for (int i = 0; i < mConfig.numFrames; i++)
						bInput[i] = static_cast<float>(currentGain * bStore[i]);
				}
				else
				{
					for (int i = 0; i < mConfig.numFrames; i++)
					{
						bInput[i] = static_cast<float>(currentGain * bStore[i]);
						currentGain = Lerp(currentGain, targetGain, mConfig.lerpFactor);
					}
				}

#ifdef PROFILE_AUDIO_THREAD
				Begin3DTI();
#endif
				{
					lock_guard<mutex> lock(tuneInMutex);
					mSource->SetBuffer(bInput);
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
				EndSource();
#endif
			}
		}

		void Source::ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer)
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
#ifdef DEBUG_AUDIO_THREAD
	// Debug::Log("Total audio vSources: " + std::to_string(counter), Colour::Orange);
#endif
			lock_guard<std::mutex> lock(*dataMutex);
			if (isVisible || currentGain != 0.0f)
			{
#ifdef PROFILE_AUDIO_THREAD
				BeginSource();
				BeginAirAbsorption();
#endif
				mAirAbsorption.ProcessAudio(data, bStore, mConfig.numFrames, mConfig.lerpFactor);
#ifdef PROFILE_AUDIO_THREAD
				EndAirAbsorption();
#endif
				if (currentGain == targetGain)
				{
					for (int i = 0; i < mConfig.numFrames; i++)
						bInput[i] = static_cast<float>(currentGain * bStore[i]);
				}
				else
				{
					for (int i = 0; i < mConfig.numFrames; i++)
					{
						bInput[i] = static_cast<float>(currentGain * bStore[i]);
						currentGain = Lerp(currentGain, targetGain, mConfig.lerpFactor);
					}
				}

#ifdef PROFILE_AUDIO_THREAD
				Begin3DTI();
#endif
				{
					lock_guard<mutex> lock(tuneInMutex);
					mSource->SetBuffer(bInput);
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
				EndSource();
#endif
			}
		}

		void Source::Update(const vec3& position, const vec4& orientation, const Real distance)
		{
			CTransform transform;
			transform.SetOrientation(CQuaternion(static_cast<float>(orientation.w), static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)));
			transform.SetPosition(CVector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));

			{
				lock_guard<mutex> lock(tuneInMutex);
				mSource->SetSourceTransform(transform);
#ifdef DEBUG_HRTF
				Debug::Log("Azimuth: " + FloatToStr(mSource->GetCurrentEarAzimuth(T_ear::LEFT)));
				Debug::Log("Elevation: " + FloatToStr(mSource->GetCurrentEarElevation(T_ear::LEFT)));
#endif
			}

			{
				lock_guard<std::mutex>lock(*dataMutex);
				mAirAbsorption.SetDistance(distance);
				if (isVisible)
					targetGain = 1.0f;
				else
					targetGain = 0.0f;
			}
			{
				lock_guard<std::mutex>lock(*vSourcesMutex);
				UpdateVirtualSources(mVSources);
			}

		}

		void Source::UpdateVirtualSources(const VirtualSourceDataMap& data)
		{
			std::vector<std::string> keys;
			std::vector<VirtualSourceData> newVSources;

			for (auto& oData : oldData)
			{
				auto out = data.find(oData.first);
				if (out == data.end()) // case: old vSource
				{
					oData.second.Invisible();
					bool remove = UpdateVirtualSource(oData.second, newVSources);
					if (remove)
					{
						keys.push_back(oData.first);
					}
				}
			}

			for (auto key : keys)
				oldData.erase(key);

			for (auto& in : data)
			{
				UpdateVirtualSource(in.second, newVSources);	// newVSources are new placeholders in the IEM tree
				oldData.insert_or_assign(in.first, in.second);

#ifdef DEBUG_VIRTUAL_SOURCE
				Debug::Log("vSource: " + it->first, Colour::Yellow);
#endif
			}

			for (auto vSource : newVSources)
				oldData.insert({ vSource.GetKey(), vSource });

#ifdef DEBUG_VIRTUAL_SOURCE
	Debug::Log("Total vSources: " + std::to_string(oldData.size()), Colour::Yellow);
#endif
		}

		bool Source::UpdateVirtualSource(const VirtualSourceData& data, std::vector<VirtualSourceData>& newVSources)
		{
			int orderIdx = data.GetOrder() - 1;

			VirtualSourceMap* tempStore;
			std::shared_ptr<std::mutex> m;

			if (data.IsReflection(0))
			{
				tempStore = &mVirtualSources;
				m = vWallMutex;
			}
			else
			{
				tempStore = &mVirtualEdgeSources;
				m = vEdgeMutex;
			}

			for (int i = 0; i < orderIdx; i++)
			{
				auto it = tempStore->find(data.GetID(i));
				if (it == tempStore->end())		// case: virtual source lower in the tree does not exist
				{
					unique_lock<mutex> lck(*m, std::defer_lock);
					if (lck.try_lock())
					{
						auto newIt = tempStore->try_emplace(data.GetID(i), mCore, mConfig); // feedsFDN always the highest order ism
						it = newIt.first;
						VirtualSourceData vSource = data;
						newVSources.push_back(vSource.Trim(i));
					}
					else
						return false;
				}

				if (data.IsReflection(i + 1))
				{
					tempStore = &it->second.mVirtualSources;
					m = it->second.vWallMutex;
				}
				else
				{
					tempStore = &it->second.mVirtualEdgeSources;
					m = it->second.vEdgeMutex;
				}
			}

			auto it = tempStore->find(data.GetID(orderIdx));
			if (it == tempStore->end())		// case: virtual source does not exist
			{
				int fdnChannel = -1;
				if (data.feedsFDN)
				{
					if (freeFDNChannels.back() >= mConfig.numFDNChannels)
						freeFDNChannels.back() = 0;
					fdnChannel = static_cast<int>(freeFDNChannels.back());
					if (freeFDNChannels.size() > 1)
						freeFDNChannels.pop_back();
					else
						freeFDNChannels[0]++;
				}
				//VirtualSource virtualSource = VirtualSource(mCore, mConfig, data, fdnChannel);

				//assert(!(data.feedsFDN && virtualSource.GetFDNChannel() == -1));
				{
					unique_lock<mutex> lck(*m, std::defer_lock);
					if (lck.try_lock())
						tempStore->try_emplace(data.GetID(orderIdx), mCore, mConfig, data, fdnChannel);
						// tempStore->insert_or_assign(data.GetID(orderIdx), virtualSource);
				}
				//virtualSource.Deactivate();
			}
			else
			{
				int fdnChannel = -1;
				if (data.feedsFDN && it->second.GetFDNChannel() < 0)
				{
					if (freeFDNChannels.back() >= mConfig.numFDNChannels)
						freeFDNChannels.back() = 0;
					fdnChannel = static_cast<int>(freeFDNChannels.back());
					if (freeFDNChannels.size() > 1)
						freeFDNChannels.pop_back();
					else
						freeFDNChannels[0]++;
				}

				bool remove = it->second.UpdateVirtualSource(data, fdnChannel);

				assert(!(data.feedsFDN && it->second.GetFDNChannel() == -1));

				if (fdnChannel >= 0) // Add vSource old fdnChannel to freeFDNChannels (Also prevents leaking FDN channels if !data.visible and the channel is not assigned to vSource)
					freeFDNChannels.push_back(fdnChannel);
				if (remove)
				{
					if (it->second.GetFDNChannel() >= 0)
					{
						freeFDNChannels.push_back(it->second.GetFDNChannel());
						int n = 0;
						{
							unique_lock<mutex> lck(*m, std::defer_lock);
							if (lck.try_lock())
								n = tempStore->erase(data.GetID(orderIdx));
						}

						if (n == 1) // Check vSource has been successfully erased
							return true;
					}
					else if (!(it->second.mVirtualSources.size() > 0 || it->second.mVirtualEdgeSources.size() > 0))
					{
						int n = 0;
						{
							unique_lock<mutex> lck(*m, std::defer_lock);
							if (lck.try_lock())
								n = tempStore->erase(data.GetID(orderIdx));
						}

						if (n == 1) // Check vSource has been successfully erased
							return true;
					}
				}
			}
			return false;
		}

		vec3 Source::GetPosition()
		{
			lock_guard<mutex> lock(tuneInMutex);
			CTransform transform = mSource->GetCurrentSourceTransform();
			return vec3(transform.GetPosition().x, transform.GetPosition().y, transform.GetPosition().z);
		}
	}
}