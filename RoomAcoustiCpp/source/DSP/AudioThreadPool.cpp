/*
* @class AudioThreadPool
*
* @brief Declaration of AudioThreadPool class
*
*/

// Common headers
#include "Unity/UnityInterface.h"

// DSP headers
#include "DSP/AudioThreadPool.h"

// Common headers
#include "Common/RACProfiler.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace RAC
{
	namespace DSP
	{
        //////////////////// AudioThreadPool Class ////////////////////

        ////////////////////////////////////////

        AudioThreadPool::AudioThreadPool(size_t numThreads, const std::shared_ptr<DSPConfig>& dspConfig)
            : stop(false), threadCount(numThreads), tasks(MAX_IMAGESOURCES + MAX_SOURCES)
        {
			int numFrames = dspConfig->GetData().numFrames;

            // we allow 0 thread for debugging, so make sure we always have at least one buffer
            const auto numOutputBuffers = std::max(threadCount, static_cast<size_t>(1));
            threadOutputBuffers.resize(numOutputBuffers, Buffer<>(2 * numFrames));
            threadReverbOutputs.resize(numOutputBuffers, std::vector<Buffer<>>(dspConfig->GetData().numReverbSources, Buffer<>(numFrames)));

#if USE_BLOCKING_TASKS
            tasksAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);
            stopRequested = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif

#ifdef _WIN32
            static int audioThreadConstructionIndex = 0;
            const int currentAudioThreadConstructionIndex = audioThreadConstructionIndex++;
#else
            const int currentAudioThreadConstructionIndex = 0;
#endif

            for (size_t i = 0; i < threadCount; ++i)
            {
                workers.emplace_back([this, i, currentAudioThreadConstructionIndex] {
#ifdef USE_UNITY_PROFILER
                    int id = RegisterAudioThread();
#endif

#ifdef _WIN32
                    WCHAR description[64];
                    swprintf_s(description, L"Audio thread %d:%d", currentAudioThreadConstructionIndex, static_cast<int>(i));
                    SetThreadDescription(GetCurrentThread(), description); 
#endif

                    //FlushDenormals();
                    std::shared_ptr<AudioTaskBase> task;
                    while (!stop.load(std::memory_order_acquire))
                    {
                        while (tasks.try_dequeue(task))
                            task->Run(threadOutputBuffers[i], threadReverbOutputs[i]);

#if USE_BLOCKING_TASKS
                        // wait for either data to come in or the stop request (which doesn't reset so we will always catch it)
                        HANDLE handles[] = { tasksAvailable, stopRequested };
                        WaitForMultipleObjects(2, handles, FALSE, INFINITE);
#else
						// Once the queue is empty, often a large wait until next used. _mm_pause() or SpinLock cause performance issues here causes 
						std::this_thread::yield();
#endif
                    }
#ifdef USE_UNITY_PROFILER
                    UnregisterAudioThread(id);
#endif
                    //NoFlushDenormals();
                });
            }
        }

        AudioThreadPool::~AudioThreadPool()
        {
	        Stop();

#if USE_BLOCKING_TASKS
            // close our event
            CloseHandle(tasksAvailable);
            tasksAvailable = NULL;
            CloseHandle(stopRequested);
            stopRequested = NULL;
#endif
        }

        ////////////////////////////////////////

        void AudioThreadPool::Stop()
        {
	        if (stop.exchange(true, std::memory_order_acq_rel))
		        return;

#if USE_BLOCKING_TASKS
            // release waiting tasks
            SetEvent(stopRequested);
#endif

	        for (auto& worker : workers)
	        {
		        if (worker.joinable())
			        worker.join();
	        }
        }

        void AudioThreadPool::ProcessAllSources(std::array<std::optional<Source>, MAX_SOURCES>& sources, ImageSourceManager& imageSources, Buffer<>& outputBuffer, const AudioData& audioData)
        {
            if (stop.load(std::memory_order_acquire))
                return;

			int maxNumTasks = audioData.earlyReverbEnabled ? MAX_SOURCES + MAX_IMAGESOURCES : MAX_SOURCES;
            SpinLock tasksRemaining(maxNumTasks);

            for (size_t t = 0; t < threadCount; ++t)
                threadOutputBuffers[t].Reset();

            for (int i = 0; i < MAX_SOURCES; ++i)
            {
                if (sources[i]->CanEdit())
                {
                    tasksRemaining.Subtract();
                    continue;
                }
                Enqueue(&sources[i].value(), &tasksRemaining, audioData);
            }

            if (audioData.earlyReverbEnabled)
            {
                for (int i = 0; i < MAX_IMAGESOURCES; ++i)
                {
                    if (imageSources.at(i).CanEdit())
                    {
                        tasksRemaining.Subtract();
                        continue;
                    }
                    Enqueue(&imageSources.at(i), &tasksRemaining, audioData);
                }
            }

            tasksRemaining.Lock();

            PROFILE_Diffraction
            for (size_t t = 0; t < threadCount; ++t)
                outputBuffer += threadOutputBuffers[t];
        }

        ////////////////////////////////////////

        void AudioThreadPool::ProcessReverbSources(std::vector<std::unique_ptr<ReverbSource>>& reverbSources, Buffer<>& outputBuffer, const AudioData& audioData)
        {
            if (stop.load(std::memory_order_acquire))
                return;

            SpinLock tasksRemaining(reverbSources.size());

            for (size_t t = 0; t < threadCount; ++t)
                threadOutputBuffers[t].Reset();

            for (size_t i = 0; i < reverbSources.size(); ++i)
                Enqueue(reverbSources[i].get(), &tasksRemaining, audioData);

            tasksRemaining.Lock();

            PROFILE_Diffraction
            for (size_t t = 0; t < threadCount; ++t)
                outputBuffer += threadOutputBuffers[t];
        }

        void AudioThreadPool::ProcessFDNs(std::vector<std::unique_ptr<FDN<Complex>>>& FDNs, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData)
        {
            if (stop.load(std::memory_order_acquire))
                return;

            SpinLock tasksRemaining(FDNs.size());

            for (size_t t = 0; t < threadCount; ++t)
            {
                for (size_t i = 0; i < outputBuffers.size(); ++i)
                    threadReverbOutputs[t][i].Reset();
            }

            for (size_t i = 0; i < FDNs.size(); ++i)
                Enqueue(FDNs[i].get(), &tasksRemaining, audioData);

            tasksRemaining.Lock();

            PROFILE_Diffraction
            for (size_t t = 0; t < threadCount; ++t)
            {
                for (size_t i = 0; i < outputBuffers.size(); ++i)
                    outputBuffers[i] += threadReverbOutputs[t][i];
            }
        }

	}
}