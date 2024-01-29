
#include "Spatialiser/Reverb.h"

// Unity headers
#include "Unity/Debug.h"

using namespace Common;
namespace UIE
{
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// ReverbSource class ////////////////////

		ReverbSource::ReverbSource(Binaural::CCore* core, HRTFMode hrtfMode, int fs) : mCore(core), mHRTFMode(hrtfMode), mReflectionFilter(4, fs), mAbsorption(1.0f, 1.0f, 1.0f, 1.0f, 1.0f)
		{
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
		}

		void ReverbSource::Update(const vec3& position)
		{
			CTransform transform;
			transform.SetPosition(CVector3(static_cast<float>(position.x + mShift.x), static_cast<float>(position.y + mShift.y), static_cast<float>(position.z + mShift.z)));
			mSource->SetSourceTransform(transform);
		}

		void ReverbSource::UpdateReflectionFilter()
		{
			Real g[NUM_ABSORPTION_FREQ];
			mAbsorption.GetValues(&g[0]);
			mReflectionFilter.UpdateParameters(ABSORPTION_FREQ, g);
		}

		void ReverbSource::UpdateReflectionFilter(const Absorption& absorption)
		{
			assert(absorption.area != 0);
			if (mAbsorption.area > 0)
			{
				mAbsorption = mAbsorption * mAbsorption.area + absorption * absorption.area;
				if (mAbsorption.area != 0)
				{
					mAbsorption /= mAbsorption.area;
				}
			}
			else
			{
				mAbsorption = absorption;
			}
			UpdateReflectionFilter();
			valid = mAbsorption > EPS;
		}

		void ReverbSource::ProcessAudio(const Real* data, const size_t& numFrames, Buffer& outputBuffer)
		{
			if (valid)
			{
				// Copy input into internal storage and apply wall absorption
				CMonoBuffer<float> bInput(numFrames);
				const Real* inputPtr = data;
				for (int i = 0; i < numFrames; i++)
					bInput[i] = static_cast<float>(mReflectionFilter.GetOutput(*inputPtr++));

				CEarPair<CMonoBuffer<float>> bOutput;

				mSource->SetBuffer(bInput);

				mSource->ProcessAnechoic(bOutput.left, bOutput.right);

				int j = 0;
				for (int i = 0; i < numFrames; i++)
				{
					outputBuffer[j++] += bOutput.left[i];
					outputBuffer[j++] += bOutput.right[i];
				}
			}
		}

		//////////////////// Reverb class ////////////////////

		Reverb::Reverb(Binaural::CCore* core, HRTFMode hrtfMode, const vec& dimensions, int fs) : mNumChannels(12), mFDN(mNumChannels, fs), mCore(core), mHRTFMode(hrtfMode), valid(false)
		{
			InitSources(fs);
		}

		Reverb::Reverb(Binaural::CCore* core, HRTFMode hrtfMode, const vec& dimensions, const FrequencyDependence& T60, int fs) : mNumChannels(12), mFDN(T60, dimensions, mNumChannels, fs), mCore(core), mHRTFMode(hrtfMode), valid(false)
		{
			InitSources(fs);
		}

		void Reverb::InitSources(int fs)
		{
			lock_guard<mutex> lock(mCoreMutex);

			mReverbSources.reserve(mNumChannels);
			for (int i = 0; i < mNumChannels; i++)
			{
				ReverbSource temp = ReverbSource(mCore, mHRTFMode, fs);
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

		void Reverb::UpdateReverbSources(const vec3& position)
		{
			for (int i = 0; i < mNumChannels; i++)
			{
				lock_guard<mutex> lock(mCoreMutex);

				mReverbSources[i].Update(position);
			}
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
			if (valid)
			{
				// Process FDN and save to buffer
				matrix input = matrix(data.Rows(), mNumChannels);

				{
					lock_guard <mutex> lock(mFDNMutex);

					for (int i = 0; i < data.Rows(); i++)
					{
						rowvec out = mFDN.GetOutput(data.GetRow(i), valid);
						for (int j = 0; j < mNumChannels; j++)
							input.AddEntry(out[j], i, j);
					}
				}
				// Process buffer of each channel
				for (int j = 0; j < mNumChannels; j++)
				{
					lock_guard<mutex> lock(mCoreMutex);

					mReverbSources[j].ProcessAudio(input.GetColumn(j), data.Rows(), outputBuffer);
				}
			}
		}

		void Reverb::SetFDNParameters(const FrequencyDependence& T60, const vec& dimensions)
		{
			{
				lock_guard <mutex> lock(mFDNMutex);
				mFDN.SetParameters(T60, dimensions);
				/*mFDN.Reset();
				for (ReverbSource& source : mReverbSources)
					source.Reset();*/
			}
			if (T60 < 20)
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
				Debug::Log("FDN reverb failed to initialise. Excessively long T60.", Colour::Red);
#endif
			}
		}
	}
}
