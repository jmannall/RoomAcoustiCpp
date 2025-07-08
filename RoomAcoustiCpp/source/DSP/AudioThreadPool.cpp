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

namespace RAC
{
	namespace DSP
	{
        //////////////////// AudioThreadPool Class ////////////////////

        ////////////////////////////////////////

        AudioThreadPool::AudioThreadPool(size_t numThreads, int numFrames, int numLateReverbChannels)
            : stop(false), threadCount(numThreads), tasks(MAX_IMAGESOURCES + MAX_SOURCES)
        {
            threadOutputBuffers.resize(threadCount, Buffer(2 * numFrames));
            threadReverbBuffers.resize(threadCount, Matrix(numLateReverbChannels, numFrames));

            for (size_t i = 0; i < threadCount; ++i)
            {
                workers.emplace_back([this, i] {
#ifdef USE_UNITY_PROFILER
                    int id = RegisterAudioThread();
#endif
                    std::shared_ptr<AudioTaskBase> task;
                    while (!stop.load(std::memory_order_acquire))
                    {
                        while (tasks.try_dequeue(task))
                            task->Run(threadOutputBuffers[i], threadReverbBuffers[i]);
                        // Once the queue is empty, often a large wait until next used. _mm_pause() or SpinLock cause performance issues here causes 
                        std::this_thread::yield();
                    }
#ifdef USE_UNITY_PROFILER
                    UnregisterAudioThread(id);
#endif
                    });
            }
        }

        ////////////////////////////////////////

        void AudioThreadPool::ProcessAllSources(std::array<std::optional<Source>, MAX_SOURCES>& sources, ImageSourceManager& imageSources, Buffer<>& outputBuffer, Matrix& reverbInput, Real lerpFactor)
        {
            if (stop.load(std::memory_order_acquire))
                return;

            SpinLock tasksRemaining(MAX_SOURCES + MAX_IMAGESOURCES);

            for (size_t t = 0; t < threadCount; ++t)
            {
                threadOutputBuffers[t].Reset();
                threadReverbBuffers[t].Reset();
            }

            for (int i = 0; i < MAX_SOURCES; ++i)
            {
                if (sources[i]->CanEdit())
                {
                    tasksRemaining.Subtract();
                    continue;
                }
                Enqueue(&sources[i].value(), &tasksRemaining, lerpFactor);
            }

            for (int i = 0; i < MAX_IMAGESOURCES; ++i)
            {
                if (imageSources.at(i).CanEdit())
                {
                    tasksRemaining.Subtract();
                    continue;
                }
                Enqueue(&imageSources.at(i), &tasksRemaining, lerpFactor);
            }

            tasksRemaining.Lock();

            for (size_t t = 0; t < threadCount; ++t)
            {
                outputBuffer += threadOutputBuffers[t];
                reverbInput += threadReverbBuffers[t];
            }
        }

        ////////////////////////////////////////

        void AudioThreadPool::ProcessReverbSources(std::vector<std::unique_ptr<ReverbSource>>& reverbSources, Buffer<>& outputBuffer)
        {
            if (stop.load(std::memory_order_acquire))
                return;

            SpinLock tasksRemaining(reverbSources.size());

            for (size_t t = 0; t < threadCount; ++t)
                threadOutputBuffers[t].Reset();

            for (size_t i = 0; i < reverbSources.size(); ++i)
                Enqueue(reverbSources[i].get(), &tasksRemaining);

            tasksRemaining.Lock();

            for (size_t t = 0; t < threadCount; ++t)
                outputBuffer += threadOutputBuffers[t];
        }
	}
}