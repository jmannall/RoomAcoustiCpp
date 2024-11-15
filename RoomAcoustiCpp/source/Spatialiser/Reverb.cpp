/*
*
*  \Reverb class
*
*/

#if defined(_ANDROID)
// Common headers
#include "Common/Definitions.h"
#endif

// Spatialiser headers
#include "Spatialiser/Reverb.h"
#include "Spatialiser/Mutexes.h"

// Unity headers
#include "Unity/Debug.h"
#include "Unity/UnityInterface.h"

// DSP headers
#include "DSP/Interpolate.h"

// Common headers
#include "Common/SphericalGeometries.h"

using namespace Common;
namespace RAC
{
	using namespace Unity;
	using namespace DSP;
	namespace Spatialiser
	{

		//////////////////// ReverbSource class ////////////////////

		ReverbSource::ReverbSource(Binaural::CCore* core, const Config& config) : mCore(core), mConfig(config), mReflectionFilter(config.frequencyBands, config.Q, config.fs),
			mAbsorption(config.frequencyBands.Length()), filterInitialised(false), targetGain(0.0), currentGain(0.0), inputBuffer(mConfig.numFrames)
		{
			mMutex = std::make_shared<std::mutex>();
			bInput = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
			Init();
		}

		ReverbSource::ReverbSource(Binaural::CCore* core, const Config& config, const Vec3& shift) : mCore(core), mConfig(config), mReflectionFilter(config.frequencyBands, config.Q, config.fs),
			mAbsorption(config.frequencyBands.Length()), mShift(shift), filterInitialised(false), targetGain(0.0), currentGain(0.0), inputBuffer(mConfig.numFrames)
		{
			mMutex = std::make_shared<std::mutex>();
			bInput = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.left = CMonoBuffer<float>(mConfig.numFrames);
			bOutput.right = CMonoBuffer<float>(mConfig.numFrames);
			Init();
		}

		ReverbSource::~ReverbSource()
		{
			// delete mMutex;
#ifdef DEBUG_REMOVE
	Debug::Log("Remove reverb source", Colour::Red);
#endif
			{
				lock_guard<mutex> lock(tuneInMutex);
				mCore->RemoveSingleSourceDSP(mSource);
			}
		}

		void ReverbSource::Init()
		{
#ifdef DEBUG_INIT
			Debug::Log("Init reverb source", Colour::Green);
#endif
			{
				lock_guard<mutex> lock(tuneInMutex);

				// Initialise source to core
				mSource = mCore->CreateSingleSourceDSP();
				mSource->DisablePropagationDelay();
				mSource->DisableDistanceAttenuationSmoothingAnechoic();
				mSource->DisableDistanceAttenuationAnechoic();
				mSource->DisableNearFieldEffect();
				mSource->DisableFarDistanceEffect();
				mSource->DisableInterpolation();

				//Select spatialisation mode
				UpdateSpatialisationMode(SpatialisationMode::none);
			}
			
		}

		void ReverbSource::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			mConfig.spatialisationMode = mode;
			switch (mode)
			{
			case SpatialisationMode::quality:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
				break;
			}
			case SpatialisationMode::performance:
			{
				mSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
				break;
			}
			case SpatialisationMode::none:
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

		void ReverbSource::UpdatePosition(const Vec3& position)
		{
			// tuneInMutex already locked by context
			CTransform transform;
			transform.SetPosition(CVector3(static_cast<float>(position.x + mShift.x), static_cast<float>(position.y + mShift.y), static_cast<float>(position.z + mShift.z)));
			mSource->SetSourceTransform(transform);
		}

		void ReverbSource::UpdateReflectionFilter(const Absorption& absorption)
		{
			lock_guard<mutex> lock(*mMutex);
			mAbsorption = absorption;

			if (mAbsorption > EPS)
			{
				if (filterInitialised)
					mReflectionFilter.SetGain(mAbsorption);
				else
				{
					mReflectionFilter.InitParameters(mAbsorption);
					filterInitialised = true;
					targetGain = 1.0;
				}
			}
			else
			{
				filterInitialised = false;
				targetGain = 0.0;
			}
		}

		void ReverbSource::ProcessAudio(Buffer& outputBuffer)
		{
#ifdef PROFILE_AUDIO_THREAD
			BeginReverbSource();
#endif
			{
				lock_guard<mutex> lock(*mMutex);
#ifdef PROFILE_AUDIO_THREAD
				BeginReflection();
#endif
				mReflectionFilter.ProcessAudio(inputBuffer, inputBuffer, mConfig.numFrames, mConfig.lerpFactor);

#ifdef PROFILE_AUDIO_THREAD
				EndReflection();
#endif
				if (currentGain > targetGain + EPS || currentGain < targetGain - EPS)
				{
					FlushDenormals();
					for (int i = 0; i < mConfig.numFrames; i++)
					{
						bInput[i] = static_cast<float>(currentGain * inputBuffer[i]);
						currentGain = Lerp(currentGain, targetGain, mConfig.lerpFactor);
					}
					NoFlushDenormals();
				}
				else
				{
					for (int i = 0; i < mConfig.numFrames; i++)
						bInput[i] = static_cast<float>(targetGain * inputBuffer[i]);
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
			EndReverbSource();
#endif
		}

		//////////////////// Reverb class ////////////////////

		Reverb::Reverb(Binaural::CCore* core, const Config& config) : mConfig(config), mFDN(config), mCore(core), valid(false), runFDN(false), mTargetGain(0.0), mCurrentGain (0.0), mT60(config.frequencyBands.Length())
		{
			input = Matrix(mConfig.numFDNChannels, mConfig.numFrames);
			out = Rowvec(mConfig.numFDNChannels);
			InitSources();
		}

		Reverb::Reverb(Binaural::CCore* core, const Config& config, const Vec& dimensions, const Coefficients& T60) : mFDN(T60, dimensions, config), mCore(core), mConfig(config), valid(false), runFDN(false), mTargetGain(0.0), mCurrentGain(0.0), mT60(T60)
		{
			input = Matrix(mConfig.numFDNChannels, mConfig.numFrames);
			InitSources();
		}

		void Reverb::UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			mConfig.spatialisationMode = mode;
			lock_guard<mutex> lock(tuneInMutex);
			for (ReverbSource& source : mReverbSources)
				source.UpdateSpatialisationMode(mode);
		}

		void Reverb::InitSources()
		{
			lock_guard<mutex> lock(mCoreMutex);

			std::vector<Vec3> points;
			switch (mConfig.numFDNChannels)
			{
			case 4:
			{ Tetrahedron(points, true); break; }
			case 6:
			{ Octahedron(points); break; }	
			case 8:
			{ Cube(points); break; }
			case 12:
			{ Icosahedron(points, true); break; }
			case 16:
			{ Cube(points); Octahedron(points); break; }
			case 20:
			{ Dodecahedron(points, true); break; }
			case 24:
			{ Icosahedron(points, true); Icosahedron(points, false); break; }
			case 32:
			{ Icosahedron(points, true); Dodecahedron(points, true); break; }
			}

			// BREAKS update reverb source system!!!
			// Remove and replace with ray traced wall absorption intersection.
			mReverbSources.reserve(mConfig.numFDNChannels);
			for (int i = 0; i < mConfig.numFDNChannels; i++)
			{
				ReverbSource temp = ReverbSource(mCore, mConfig, points[i]);
				mReverbSources.push_back(temp);
				temp.Deactivate();
			}
		}

		void Reverb::UpdateReverb(const Vec3& position)
		{
			// tuneInMutex already locked by context
			for (int i = 0; i < mConfig.numFDNChannels; i++)
				mReverbSources[i].UpdatePosition(position);
		}

		void Reverb::UpdateReflectionFilters(const std::vector<Absorption>& absorptions, const bool running)
		{
			{
				if (valid && running)
					mTargetGain = 1.0;
				else
				{
					mTargetGain = 0.0;
					if (mCurrentGain < EPS)
					{
						runFDN = false;
						{
							std::lock_guard<std::mutex> lock(mFDNMutex);
							mFDN.Reset();
						}
						for (ReverbSource& source : mReverbSources)
							source.Reset();
						return;
					}
				}
				runFDN = true;
			}

			for (int i = 0; i < mConfig.numFDNChannels; i++)
				mReverbSources[i].UpdateReflectionFilter(absorptions[i]);
		}

		void Reverb::ProcessAudio(const Matrix& data, Buffer& outputBuffer)
		{
			if (runFDN)
			{
#ifdef PROFILE_AUDIO_THREAD
				BeginReverb();
#endif
				// Process FDN and save to buffer
				
#ifdef PROFILE_AUDIO_THREAD
				BeginFDN();
#endif					
				FlushDenormals();

				if (mCurrentGain > mTargetGain + EPS || mCurrentGain < mTargetGain - EPS)
				{
					int j = 0;
					lock_guard <mutex> lock(mFDNMutex);
					for (int i = 0; i < mConfig.numFrames; i++)
					{
						mFDN.ProcessOutput(data.GetRow(i), mCurrentGain);
						j = 0;
						for (auto& source : mReverbSources)
						{
							source.AddInput(mFDN.GetOutput(j), i);
							j++;
						}
						mCurrentGain = Lerp(mCurrentGain, mTargetGain, mConfig.lerpFactor);
					}
				}
				else
				{
					int  j = 0;
					lock_guard <mutex> lock(mFDNMutex);
					for (int i = 0; i < mConfig.numFrames; i++)
					{
						mFDN.ProcessOutput(data.GetRow(i), mTargetGain);
						j = 0;
						for (auto& source : mReverbSources)
						{
							source.AddInput(mFDN.GetOutput(j), i);
							j++;
						}
					}
				}

				NoFlushDenormals();
#ifdef PROFILE_AUDIO_THREAD
				EndFDN();
#endif

				// Process buffer of each channel
				for (auto& source : mReverbSources)
					source.ProcessAudio(outputBuffer);
#ifdef PROFILE_AUDIO_THREAD
				EndReverb();
#endif
			}
		}

		void Reverb::UpdateReverbTime(const Coefficients& T60)
		{
#ifdef DEBUG_INIT
			Debug::Log("Reverb T60: [" + RealToStr(T60[0]) + ", " + RealToStr(T60[1]) + ", " +
				RealToStr(T60[2]) + ", " + RealToStr(T60[3]) + ", " + RealToStr(T60[4]) + "]", Colour::Orange);
#endif
			lock_guard <mutex> lock(mFDNMutex);
			mFDN.UpdateT60(T60);
		}

		void Reverb::SetFDNParameters(const Coefficients& T60, const Vec& dimensions)
		{
			{
				lock_guard <mutex> lock(mFDNMutex);
				mFDN.Reset();
				mFDN.SetParameters(T60, dimensions);
			}
			if (T60 < 20.0)
			{
				if (T60 > 0.0)
				{
#ifdef DEBUG_INIT
					Debug::Log("Init FDN", Colour::Green);
					Debug::Log("Reverb T60: [" + RealToStr(T60[0]) + ", " + RealToStr(T60[1]) + ", " +
						RealToStr(T60[2]) + ", " + RealToStr(T60[3]) + ", " + RealToStr(T60[4]) + "]", Colour::Orange);
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
