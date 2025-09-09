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
            threadOutputBuffers.resize(threadCount, Buffer<>(2 * numFrames));

            threadReverbOutputs.resize(threadCount, std::vector<Buffer<>>(dspConfig->GetData().numReverbSources, Buffer<>(numFrames)));

            for (size_t i = 0; i < threadCount; ++i)
            {
                workers.emplace_back([this, i] {
#ifdef USE_UNITY_PROFILER
                    int id = RegisterAudioThread();
#endif
                    //FlushDenormals();
                    std::shared_ptr<AudioTaskBase> task;
                    while (!stop.load(std::memory_order_acquire))
                    {
                        while (tasks.try_dequeue(task))
                            task->Run(threadOutputBuffers[i], threadReverbOutputs[i]);
                        // Once the queue is empty, often a large wait until next used. _mm_pause() or SpinLock cause performance issues here causes 
                        std::this_thread::yield();
                    }
#ifdef USE_UNITY_PROFILER
                    UnregisterAudioThread(id);
#endif
                    //NoFlushDenormals();
                    });
            }
        }

        ////////////////////////////////////////

        void AudioThreadPool::ProcessAllSources(std::array<std::optional<Source>, MAX_SOURCES>& sources, ImageSourceManager& imageSources, Buffer<>& outputBuffer, const AudioData& audioData)
        {
            if (stop.load(std::memory_order_acquire))
                return;

            SpinLock tasksRemaining(MAX_SOURCES + MAX_IMAGESOURCES);

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

            for (int i = 0; i < MAX_IMAGESOURCES; ++i)
            {
                if (imageSources.at(i).CanEdit())
                {
                    tasksRemaining.Subtract();
                    continue;
                }
                Enqueue(&imageSources.at(i), &tasksRemaining, audioData);
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