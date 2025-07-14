
#ifndef RoomAcoustiCpp_Profiler_h
#define RoomAcoustiCpp_Profiler_h

#include <atomic>
#include <fstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <string_view>
#include <string>

#include "Unity/UnityInterface.h"

#include "moodycamel/concurrentqueue.h"

//#define PROFILE_BACKGROUND_THREAD
//#define PROFILE_BACKGROUND_THREAD_DETAILED
//#define PROFILE_AUDIO_THREAD
//#define PROFILE_AUDIO_THREAD_DETAILED


#ifdef PROFILE_AUDIO_THREAD_DETAILED
#ifndef PROFILE_AUDIO_THREAD
#define PROFILE_AUDIO_THREAD
#endif
#endif

#ifdef PROFILE_BACKGROUND_THREAD_DETAILED
#ifndef PROFILE_BACKGROUND_THREAD
#define PROFILE_BACKGROUND_THREAD
#endif
#endif

#define PROFILER_CATEGORY(category) \
    ProfileSection section(ProfilerCategories::category); \
    (void)section; // Prevent unused variable warning

#ifdef PROFILE_BACKGROUND_THREAD
#define PROFILE_BackgroundThread \
    PROFILER_CATEGORY(BackgroundThread)
#else
#define PROFILE_BackgroundThread
#endif

#ifdef PROFILE_AUDIO_THREAD
#define PROFILE_AudioThread \
    PROFILER_CATEGORY(AudioThread)
#define PROFILE_SubmitAudio \
    PROFILER_CATEGORY(SubmitAudio)
#else
#define PROFILE_AudioThread
#define PROFILE_SubmitAudio
#endif

#ifdef PROFILE_BACKGROUND_THREAD_DETAILED
#define PROFILE_ImageEdgeModel \
    PROFILER_CATEGORY(ImageEdgeModel)
#define PROFILE_Direct \
    PROFILER_CATEGORY(Direct)
#define PROFILE_FirstOrderReflections \
    PROFILER_CATEGORY(FirstOrderReflections)
#define PROFILE_FirstOrderDiffraction \
    PROFILER_CATEGORY(FirstOrderDiffraction)
#define PROFILE_ReverbRayTracing \
    PROFILER_CATEGORY(ReverbRayTracing)
#define PROFILE_UpdateAudioData \
    PROFILER_CATEGORY(UpdateAudioData)
#else
#define PROFILE_ImageEdgeModel
#define PROFILE_Direct
#define PROFILE_FirstOrderReflections
#define PROFILE_FirstOrderDiffraction
#define PROFILE_ReverbRayTracing
#define PROFILE_UpdateAudioData
#endif

#ifdef PROFILE_AUDIO_THREAD_DETAILED
#define PROFILE_EarlyReflections \
    PROFILER_CATEGORY(EarlyReflections)
#define PROFILE_LateReverb \
    PROFILER_CATEGORY(LateReverb)
#define PROFILE_Source \
    PROFILER_CATEGORY(Source)
#define PROFILE_ImageSource \
    PROFILER_CATEGORY(ImageSource)
#define PROFILE_ReverbSource \
    PROFILER_CATEGORY(ReverbSource)
#define PROFILE_FDN \
    PROFILER_CATEGORY(FDN)
#define PROFILE_Reflection \
    PROFILER_CATEGORY(Reflection)
#define PROFILE_Diffraction \
    PROFILER_CATEGORY(Diffraction)
#define PROFILE_AirAbsorption \
    PROFILER_CATEGORY(AirAbsorption)
#define PROFILE_Spatialisation \
    PROFILER_CATEGORY(Spatialisation)
#else
#define PROFILE_EarlyReflections
#define PROFILE_LateReverb
#define PROFILE_Source
#define PROFILE_ImageSource
#define PROFILE_ReverbSource
#define PROFILE_FDN
#define PROFILE_Reflection
#define PROFILE_Diffraction
#define PROFILE_AirAbsorption
#define PROFILE_Spatialisation
#endif

enum ProfilerCategories
{
    BackgroundThread,
    ImageEdgeModel,
    Direct,
    FirstOrderReflections,
    FirstOrderDiffraction,
    SecondOrderReflections,
    SecondOrderDiffraction,
    ThirdOrderReflections,
    ThirdOrderDiffraction,
    HigherOrderReflection,
    HigherOrderDiffraction,
    ReverbRayTracing,
    UpdateAudioData,
    AudioThread,
    SubmitAudio,
    EarlyReflections,
    LateReverb,
    FDN,
    Source,
    ImageSource,
    ReverbSource,
    Reflection,
    Diffraction,
    AirAbsorption,
    Spatialisation
};

inline std::ostream& operator<<(std::ostream& os, const ProfilerCategories& category)
{
    switch (category)
    {
    case ProfilerCategories::BackgroundThread:
        os << "BackgroundThread";
        break;
	case ProfilerCategories::ImageEdgeModel:
		os << "ImageEdgeModel";
		break;
	case ProfilerCategories::Direct:
		os << "Direct";
		break;
	case ProfilerCategories::FirstOrderReflections:
		os << "FirstOrderReflections";
		break;
	case ProfilerCategories::FirstOrderDiffraction:
		os << "FirstOrderDiffraction";
		break;
	case ProfilerCategories::SecondOrderReflections:
		os << "SecondOrderReflections";
		break;
	case ProfilerCategories::SecondOrderDiffraction:
		os << "SecondOrderDiffraction";
		break;
	case ProfilerCategories::ThirdOrderReflections:
		os << "ThirdOrderReflections";
		break;
	case ProfilerCategories::ThirdOrderDiffraction:
		os << "ThirdOrderDiffraction";
		break;
	case ProfilerCategories::HigherOrderReflection:
		os << "HigherOrderReflection";
		break;
	case ProfilerCategories::HigherOrderDiffraction:
		os << "HigherOrderDiffraction";
		break;
	case ProfilerCategories::ReverbRayTracing:
		os << "ReverbRayTracing";
		break;
	case ProfilerCategories::UpdateAudioData:
		os << "UpdateAudioData";
		break;
    case ProfilerCategories::AudioThread:
		os << "AudioThread";
		break;
	case ProfilerCategories::SubmitAudio:
		os << "SubmitAudio";
		break;
	case ProfilerCategories::EarlyReflections:
		os << "EarlyReflections";
		break;
	case ProfilerCategories::LateReverb:
		os << "LateReverb";
		break;
	case ProfilerCategories::FDN:
		os << "FDN";
		break;
	case ProfilerCategories::Source:
		os << "Source";
		break;
	case ProfilerCategories::ImageSource:
		os << "ImageSource";
		break;
	case ProfilerCategories::ReverbSource:
		os << "ReverbSource";
		break;
	case ProfilerCategories::Reflection:
		os << "Reflection";
		break;
	case ProfilerCategories::Diffraction:
		os << "Diffraction";
		break;
	case ProfilerCategories::AirAbsorption:
		os << "AirAbsorption";
		break;
	case ProfilerCategories::Spatialisation:
		os << "Spatialisation";
		break;
    default:
        break;
    }
    return os;
}

namespace RAC
{
    namespace Common
    {
        class Profiler
        {
            struct ProfileEvent
            {
                ProfilerCategories category;
                uint64_t duration_ns;
                uint64_t timestamp_ns;

                ProfileEvent() : category(ProfilerCategories::BackgroundThread), duration_ns(0), timestamp_ns(0) {}
                ProfileEvent(ProfilerCategories category, uint64_t duration_ns, uint64_t timestamp_ns) : category(category), duration_ns(duration_ns), timestamp_ns(timestamp_ns) {}
            };

        public:
            using Clock = std::chrono::steady_clock;
            using TimePoint = Clock::time_point;
            using Duration = std::chrono::nanoseconds;

            inline static Profiler& Instance()
            {
                static Profiler instance;
                return instance;
            }

            inline void AddSample(ProfilerCategories category, Duration duration, TimePoint start)
            {
                queue.enqueue({ category, static_cast<uint64_t>(duration.count()), static_cast<uint64_t>(start.time_since_epoch().count()) });
	        }

            inline void SetOutputFile(const std::string& filename, bool logOn)
            {
                if (logOn)
                    output.open(filename);
                else
                    Shutdown(filename);
            }

            inline void Shutdown(const std::string& filename)
            {
                {
                    std::lock_guard<std::mutex> lock(fileMutex);
                    if (!output.is_open())
                        return;
                }

                running.store(false, std::memory_order_release);
                if (logThread.joinable())
                    logThread.join();

                // Flush any remaining events after the thread has stopped
                ProfileEvent event;
                while (queue.try_dequeue(event))
                    output << event.category << "," << event.duration_ns << "," << event.timestamp_ns << "\n";

                output.close();
                std::ifstream f(filename);  // Delete file if it is empty
                if (f.good() && f.peek() == std::ifstream::traits_type::eof())
                {
                    f.close();
                    std::remove(filename.c_str());
                }

				running.store(true, std::memory_order_release);
				logThread = std::thread([this]() { this->ThreadFunc(); });
            }

        private:
            Profiler() : queue(4096), logThread([this]() { this->ThreadFunc(); }) {}

            ~Profiler()
            {
                running.store(false, std::memory_order_release);
                if (logThread.joinable())
                    logThread.join();
            }

            inline void ThreadFunc() {
                ProfileEvent event;
                while (running)
                {
                    while (queue.try_dequeue(event))
                    {
                        std::lock_guard<std::mutex> lock(fileMutex);
                        if (output.is_open())
                            output << event.category << "," << event.duration_ns << "," << event.timestamp_ns << "\n";
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }

            std::atomic<bool> running{ true };
            moodycamel::ConcurrentQueue<ProfileEvent> queue;
            std::thread logThread;
            std::ofstream output;
            std::mutex fileMutex;
        };

        class ProfileSection {
        public:
            ProfileSection(ProfilerCategories category) : category(category), start(Profiler::Clock::now())
            {
#ifdef USE_UNITY_PROFILER
                switch (category)
                {
                case ProfilerCategories::BackgroundThread:
                    BeginBackgroundLoop();
                    break;
                case ProfilerCategories::ImageEdgeModel:
                    BeginImageEdgeModel();
                    break;
                case ProfilerCategories::Direct:
                    BeginDirect();
                    break;
                case ProfilerCategories::FirstOrderReflections:
                    BeginFirstOrderRef();
                    break;
                case ProfilerCategories::FirstOrderDiffraction:
                    BeginFirstOrderDiff();
                    break;
                case ProfilerCategories::SecondOrderReflections:
                    BeginSecondOrderRef();
                    break;
                case ProfilerCategories::SecondOrderDiffraction:
                    BeginSecondOrderRefDiff();
                    break;
                case ProfilerCategories::ThirdOrderReflections:
                    BeginThirdOrderRef();
                    break;
                case ProfilerCategories::ThirdOrderDiffraction:
                    BeginThirdOrderRefDiff();
                    break;
                case ProfilerCategories::HigherOrderReflection:
                    BeginHigherOrderRef();
                    break;
                case ProfilerCategories::HigherOrderDiffraction:
                    BeginHigherOrderRefDiff();
                    break;
                case ProfilerCategories::ReverbRayTracing:
                    BeginReverbRayTracing();
                    break;
                case ProfilerCategories::UpdateAudioData:
                    BeginUpdateAudioData();
                    break;
                case ProfilerCategories::AudioThread:
                    BeginAudioThread();
                    break;
                case ProfilerCategories::SubmitAudio:
                    BeginSubmitAudio();
                    break;
                case ProfilerCategories::EarlyReflections:
                    BeginEarlyReflections();
                    break;
                case ProfilerCategories::LateReverb:
                    BeginLateReverb();
                    break;
                case ProfilerCategories::FDN:
                    BeginFDN();
                    break;
                case ProfilerCategories::Source:
                    BeginSource();
                    break;
                case ProfilerCategories::ImageSource:
                    BeginImageSource();
                    break;
                case ProfilerCategories::ReverbSource:
                    BeginReverbSource();
                    break;
                case ProfilerCategories::Reflection:
                    BeginReflection();
                    break;
                case ProfilerCategories::Diffraction:
                    BeginDiffraction();
                    break;
                case ProfilerCategories::AirAbsorption:
                    BeginAirAbsorption();
                    break;
                case ProfilerCategories::Spatialisation:
                    Begin3DTI();
                    break;
                default:
                    break;
                }
#endif
            }

            ~ProfileSection()
            {
                auto end = Profiler::Clock::now();
                Profiler::Duration duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                Profiler::Instance().AddSample(category, duration, start);

#ifdef USE_UNITY_PROFILER
                switch (category)
                {
                case ProfilerCategories::BackgroundThread:
                    EndBackgroundLoop();
                    break;
                case ProfilerCategories::ImageEdgeModel:
                    EndImageEdgeModel();
                    break;
                case ProfilerCategories::Direct:
                    EndDirect();
                    break;
                case ProfilerCategories::FirstOrderReflections:
                    EndFirstOrderRef();
                    break;
                case ProfilerCategories::FirstOrderDiffraction:
                    EndFirstOrderDiff();
                    break;
                case ProfilerCategories::SecondOrderReflections:
                    EndSecondOrderRef();
                    break;
                case ProfilerCategories::SecondOrderDiffraction:
                    EndSecondOrderRefDiff();
                    break;
                case ProfilerCategories::ThirdOrderReflections:
                    EndThirdOrderRef();
                    break;
                case ProfilerCategories::ThirdOrderDiffraction:
                    EndThirdOrderRefDiff();
                    break;
                case ProfilerCategories::HigherOrderReflection:
                    EndHigherOrderRef();
                    break;
                case ProfilerCategories::HigherOrderDiffraction:
                    EndHigherOrderRefDiff();
                    break;
                case ProfilerCategories::ReverbRayTracing:
                    EndReverbRayTracing();
                    break;
                case ProfilerCategories::UpdateAudioData:
                    EndUpdateAudioData();
                    break;
                case ProfilerCategories::AudioThread:
                    EndAudioThread();
                    break;
                case ProfilerCategories::SubmitAudio:
                    EndSubmitAudio();
                    break;
                case ProfilerCategories::EarlyReflections:
                    EndEarlyReflections();
                    break;
                case ProfilerCategories::LateReverb:
                    EndLateReverb();
                    break;
                case ProfilerCategories::FDN:
                    EndFDN();
                    break;
                case ProfilerCategories::Source:
                    EndSource();
                    break;
                case ProfilerCategories::ImageSource:
                    EndImageSource();
                    break;
                case ProfilerCategories::ReverbSource:
                    EndReverbSource();
                    break;
                case ProfilerCategories::Reflection:
                    EndReflection();
                    break;
                case ProfilerCategories::Diffraction:
                    EndDiffraction();
                    break;
                case ProfilerCategories::AirAbsorption:
                    EndAirAbsorption();
                    break;
                case ProfilerCategories::Spatialisation:
                    End3DTI();
                    break;
                default:
                    break;
                }
#endif
            }

        private:
            ProfilerCategories category;
            Profiler::TimePoint start;
        };

    }
}
#endif