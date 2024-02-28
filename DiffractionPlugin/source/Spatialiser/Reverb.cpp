
#include "Spatialiser/Reverb.h"

// Unity headers
#include "Unity/Debug.h"
#include "Unity/UnityInterface.h"

using namespace Common;
namespace UIE
{
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// ReverbSource class ////////////////////

		ReverbSource::ReverbSource(Binaural::CCore* core, const Config& config) : mCore(core), mConfig(config), mReflectionFilter(REFLECTION_FILTER_ORDER, config.frequencyBands, config.fs), mAbsorption(config.frequencyBands.Length())
		{
			bInput = CMonoBuffer<float>(mConfig.numFrames);
			/*bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);*/
			UpdateReflectionFilter();
			Init();
		}

		ReverbSource::~ReverbSource()
		{
#ifdef DEBUG_REMOVE
	Debug::Log("Remove reverb source", Colour::Red);
#endif

		mCore->RemoveSingleSourceDSP(mSource);
		}

		void ReverbSource::Init()
		{
#ifdef DEBUG_INIT
	Debug::Log("Init reverb source", Colour::Green);
#endif

			// Initialise source to core
			mSource = mCore->CreateSingleSourceDSP();
			mSource->DisablePropagationDelay();
			mSource->DisableDistanceAttenuationAnechoic();
			mSource->DisableDistanceAttenuationSmoothingAnechoic();
			mSource->DisableNearFieldEffect();
			mSource->DisableFarDistanceEffect();

			//Select spatialisation mode
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
		}

		void ReverbSource::Update(const vec3& position)
		{
			CTransform transform;
			transform.SetPosition(CVector3(static_cast<float>(position.x + mShift.x), static_cast<float>(position.y + mShift.y), static_cast<float>(position.z + mShift.z)));
			mSource->SetSourceTransform(transform);
		}

		void ReverbSource::UpdateReflectionFilter()
		{
			mReflectionFilter.UpdateParameters(mAbsorption);
		}

		void ReverbSource::UpdateReflectionFilter(const Absorption& absorption)
		{
			assert(absorption.area != 0);
			if (mAbsorption.area > 0)
			{
				mAbsorption *= mAbsorption.area;
				mAbsorption += absorption * absorption.area;
				// mAbsorption = mAbsorption * mAbsorption.area + absorption * absorption.area;
				if (mAbsorption.area != 0)
					mAbsorption /= mAbsorption.area;
			}
			else
			{
				mAbsorption = absorption;
			}
			UpdateReflectionFilter();
			valid = mAbsorption > EPS;
		}

		void ReverbSource::ProcessAudio(const vec& data, Buffer& outputBuffer)
		{
			if (valid)
			{
#ifdef PROFILE_AUDIO_THREAD
				BeginReverbSource();
#endif
				// Copy input into internal storage and apply wall absorption
				//CMonoBuffer<float> bInput(numFrames);
				//const Real* inputPtr = data;
#ifdef PROFILE_AUDIO_THREAD
				BeginReflection();
#endif
				for (int i = 0; i < mConfig.numFrames; i++)
					bInput[i] = static_cast<float>(mReflectionFilter.GetOutput(data.GetEntry(i)));

#ifdef PROFILE_AUDIO_THREAD
				EndReflection();
				Begin3DTI();
#endif

				mSource->SetBuffer(bInput);

				CEarPair<CMonoBuffer<float>> bOutput;
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
				EndReverbSource();
#endif
			}
		}

		//////////////////// Reverb class ////////////////////

		Reverb::Reverb(Binaural::CCore* core, const Config& config, const vec& dimensions) : mConfig(config), mFDN(config), mCore(core), valid(false), runFDN(false), mTargetGain(0.0), mCurrentGain (0.0)
		{
			input = matrix(mConfig.numFrames, mConfig.numFDNChannels);
			col = new Real[mConfig.numFrames];
			InitSources();
		}

		Reverb::Reverb(Binaural::CCore* core, const Config& config, const vec& dimensions, const Coefficients& T60) : mFDN(T60, dimensions, config), mCore(core), mConfig(config), valid(false), runFDN(false), mTargetGain(0.0), mCurrentGain(0.0)
		{
			input = matrix(mConfig.numFrames, mConfig.numFDNChannels);
			col = new Real[mConfig.numFDNChannels];
			InitSources();
		}

		void Reverb::InitSources()
		{
			lock_guard<mutex> lock(mCoreMutex);

			mReverbSources.reserve(mConfig.numFDNChannels);

			for (int i = 0; i < mConfig.numFDNChannels; i++)
			{
				ReverbSource temp = ReverbSource(mCore, mConfig);
				mReverbSources.push_back(temp);
				temp.Deactivate();
			}

			mReverbSources[0].SetShift(vec3(1.0, 1.0, 3.0));		// +z
			mReverbSources[1].SetShift(vec3(-1.0, -1.0, 3.0));	// +z
			mReverbSources[2].SetShift(vec3(-1.0, 1.0, -3.0));	// -z
			mReverbSources[3].SetShift(vec3(1.0, -1.0, -3.0));	// -z
			mReverbSources[4].SetShift(vec3(3.0, 1.0, 1.0));		// +x
			mReverbSources[5].SetShift(vec3(3.0, -1.0, -1.0));	// +x
			mReverbSources[6].SetShift(vec3(-3.0, -1.0, 1.0));	// -x
			mReverbSources[7].SetShift(vec3(-3.0, 1.0, -1.0));	// -x
			mReverbSources[8].SetShift(vec3(1.0, 3.0, 1.0));		// +y
			mReverbSources[9].SetShift(vec3(-1.0, 3.0, -1.0));	// +y
			mReverbSources[10].SetShift(vec3(-1.0, -3.0, 1.0));	// -y
			mReverbSources[11].SetShift(vec3(1.0, -3.0, -1.0));	// -y
		}

		void Reverb::UpdateReverb(const vec3& position, const bool on)
		{
			{
				lock_guard<mutex> lock(mCoreMutex);
				for (int i = 0; i < mConfig.numFDNChannels; i++)
					mReverbSources[i].Update(position);
			}

			lock_guard<mutex> lock(mFDNMutex);

			if (valid && on)
				mTargetGain = 1.0;
			else
			{
				mTargetGain = 0.0;
				if (mCurrentGain < 0.0001)
				{
					runFDN = false;
					mFDN.Reset();
					for (ReverbSource& source : mReverbSources)
						source.Reset();
					return;
				}
			}
			runFDN = true;
		}

		void Reverb::UpdateReflectionFilters(const ReverbWall& id, const Absorption& absorption, const Absorption& oldAbsorption)
		{
			UpdateReflectionFilters(id, absorption * absorption.area - oldAbsorption * oldAbsorption.area);
		}

		void Reverb::UpdateReflectionFilters(const ReverbWall& id, const Absorption& absorption)
		{
			switch (id)
			{
			case ReverbWall::posZ: // +z
			{
				mReverbSources[0].UpdateReflectionFilter(absorption);
				mReverbSources[1].UpdateReflectionFilter(absorption);
				break;
			}
			case ReverbWall::negZ: // -z
			{
				mReverbSources[2].UpdateReflectionFilter(absorption);
				mReverbSources[3].UpdateReflectionFilter(absorption);
				break;
			}
			case ReverbWall::posX: // +x
			{
				mReverbSources[4].UpdateReflectionFilter(absorption);
				mReverbSources[5].UpdateReflectionFilter(absorption);
				break;
			}
			case ReverbWall::negX: // -x
			{
				mReverbSources[6].UpdateReflectionFilter(absorption);
				mReverbSources[7].UpdateReflectionFilter(absorption);
				break;
			}
			case ReverbWall::posY: // +y
			{
				mReverbSources[8].UpdateReflectionFilter(absorption);
				mReverbSources[9].UpdateReflectionFilter(absorption);
				break;
			}
			case ReverbWall::negY: // -y
			{
				mReverbSources[10].UpdateReflectionFilter(absorption);
				mReverbSources[11].UpdateReflectionFilter(absorption);
				break;
			}
			case ReverbWall::none:
			{
				break;
			}
			}
		}

		void Reverb::ProcessAudio(const matrix& data, Buffer& outputBuffer)
		{
			if (runFDN)
			{
#ifdef PROFILE_AUDIO_THREAD
				BeginReverb();
#endif
				// Process FDN and save to buffer
				{
					lock_guard <mutex> lock(mFDNMutex);

#ifdef PROFILE_AUDIO_THREAD
					BeginFDN();
#endif

					if (mCurrentGain == mTargetGain)
					{
						for (int i = 0; i < data.Rows(); i++)
						{
							rowvec out = mFDN.GetOutput(data.GetRow(i), mCurrentGain, valid);
							for (int j = 0; j < mConfig.numFDNChannels; j++)
								input.AddEntry(out.GetEntry(j), i, j);
						}
					}
					else
					{
						for (int i = 0; i < data.Rows(); i++)
						{
							rowvec out = mFDN.GetOutput(data.GetRow(i), mCurrentGain, valid);
							for (int j = 0; j < mConfig.numFDNChannels; j++)
								input.AddEntry(out.GetEntry(j), i, j);
							Lerp(mCurrentGain, mTargetGain, mConfig.lerpFactor);
						}
					}
					
#ifdef PROFILE_AUDIO_THREAD
					EndFDN();
#endif
				}
				// Process buffer of each channel
				lock_guard<mutex> lock(mCoreMutex);
				int j = 0;
				for (ReverbSource& source : mReverbSources)
				{
					source.ProcessAudio(input.GetColumn(j), outputBuffer);
					j++;
				}
#ifdef PROFILE_AUDIO_THREAD
				EndReverb();
#endif
			}
		}

		void Reverb::SetFDNParameters(const Coefficients& T60, const vec& dimensions)
		{
			{
				lock_guard <mutex> lock(mFDNMutex);
				mFDN.SetParameters(T60, dimensions);
			}
			if (T60 < 20.0)
			{
				if (T60 > 0.0)
				{
#ifdef DEBUG_INIT
					Real t60[5]; T60.GetValues(&t60[0]);
					Debug::Log("Init FDN", Colour::Green);
					Debug::Log("Reverb T60: [" + RealToStr(t60[0]) + ", " + RealToStr(t60[1]) + ", " +
						RealToStr(t60[2]) + ", " + RealToStr(t60[3]) + ", " + RealToStr(t60[4]) + "]", Colour::Orange);
#endif
					valid = true;
				}
				else
				{
					valid = false;
#ifdef DEBUG_INIT
					Debug::Log("FDN reverb failed to initialise. T60 equal to or less than 0s.", Colour::Red);
#endif
				}
			}
			else
			{
				valid = false;
#ifdef DEBUG_INIT
				Debug::Log("FDN reverb failed to initialise. T60 over 20s.", Colour::Red);
#endif
			}
		}
	}
}
