/*
* @class AudioThreadPool
*
* @brief Declaration of AudioThreadPool class
*
*/

#ifndef RoomAcoustiCpp_AudioThreadPool_h
#define RoomAcoustiCpp_AudioThreadPool_h

// C++ headers
#include <thread>

// DSP headers
#include "DSP/Buffer.h"

// Spatialiser headers
#include "Spatialiser/Source.h"
#include "Spatialiser/ImageSourceManager.h"
#include "Spatialiser/Reverb.h"

// Common headers
#include "Common/Definitions.h"
#include "Common/Matrix.h"
#include "Common/SpinLock.h"

// moodycamel headers
#include "moodycamel/concurrentqueue.h"

namespace RAC
{
    namespace DSP
    {
        /**
		* @brief Class that implements a lock free thread pool for processing audio tasks
        */
        class AudioThreadPool
        {
            /**
			* @brief Base struct for audio tasks
            */
            struct AudioTaskBase
            {
                /**
				* @brief Pure virtual function to run the audio task
                */
                virtual void Run(Buffer& out, Matrix& reverb) = 0;

                /**
				* @brief Default virtual destructor
                */
                virtual ~AudioTaskBase() = default;
            };

            /**
			* @brief Template struct for audio tasks that can process different types of sources
            */
            template<typename T>
            struct AudioTask : public AudioTaskBase
            {
				T* source;      // Pointer to the audio source (Source, ImageSource or ReverbSource)
				Real lerpFactor;    // Interpolation factor for audio processing
                SpinLock* tasksRemaining;     // Pointer to the spin lock for tracking remaining tasks

                AudioTask(T* source = nullptr, SpinLock* tasksRemaining = nullptr, Real lerpFactor = 0.0f)
                    : source(source), lerpFactor(lerpFactor), tasksRemaining(tasksRemaining) {
                }

                void Run(Buffer& out, Matrix& reverb) override
                {
                    ProcessAudio(out, reverb, std::is_same<T, ReverbSource>{});
                    tasksRemaining->Subtract();
                }

            private:
                // General case
                void ProcessAudio(Buffer& out, Matrix& reverb, std::false_type) { source->ProcessAudio(out, reverb, lerpFactor); }

                // Specialized case for ReverbSource
                void ProcessAudio(Buffer& out, Matrix& reverb, std::true_type) { source->ProcessAudio(out); }
            };

        public:
            /**
			* @brief Constructor that initialises the audio thread pool with a given number of threads
            * 
			* @param numThreads The number of threads to create in the pool
			* @param numFrames The number of frames per audio buffer
			* @param numLateReverbChannels The number of channels for late reverb processing
            */
            AudioThreadPool(size_t numThreads, int numFrames, int numLateReverbChannels);

            /**
			* @brief Default destructor that stops all threads
            */
            ~AudioThreadPool() { Stop(); }

            /**
			* @brief Adds an audio task to the queue
            * 
			* @param source Pointer to the audio source object (Source, ImageSource)
			* @param lerpFactor Interpolation factor for audio processing
			* @param tasksRemaining Pointer to the spin lock for tracking remaining tasks
            */
            template <typename T>
            void Enqueue(T* source, SpinLock* tasksRemaining, Real lerpFactor) {
                static_assert(std::is_member_function_pointer_v<decltype(&T::ProcessAudio)>, "T must have a ProcessAudio member function");
                static_assert(std::is_same_v<decltype(&T::ProcessAudio), void (T::*)(Buffer&, Matrix&, Real)>, "T::ProcessAudio must be of type void (T::*)(Buffer&, Matrix&, Real)");
                std::shared_ptr<AudioTaskBase> task = std::make_shared<AudioTask<T>>(source, tasksRemaining, lerpFactor);
                tasks.try_enqueue(std::move(task));
            }

            /**
			* @brief Adds an audio task to the queue (specialized for ReverbSource)
            *
            * @param source Pointer to the ReverbSource object
            * @param tasksRemaining Pointer to the spin lock for tracking remaining tasks
            */
            void Enqueue(ReverbSource* source, SpinLock* tasksRemaining) {
                static_assert(std::is_member_function_pointer_v<decltype(&ReverbSource::ProcessAudio)>, "T must have a ProcessAudio member function");
                static_assert(std::is_same_v<decltype(&ReverbSource::ProcessAudio), void (ReverbSource::*)(Buffer&)>, "T::ProcessAudio must be of type void (T::*)(Buffer&)");
                std::shared_ptr<AudioTaskBase> task = std::make_shared<AudioTask<ReverbSource>>(source, tasksRemaining);
                tasks.try_enqueue(std::move(task));
            }

            /**
			* @brief Stops all threads in the audio thread pool
            */
            inline void Stop()
            {
				if (stop.exchange(true, std::memory_order_acq_rel))
					return;
                for (auto& worker : workers)
                {
                    if (worker.joinable())
                        worker.join();
                }
            }

            /**
			* @brief Processes sources and image sources
            * 
			* @param sources Sources to process
			* @param imageSources Image sources to process
			* @param outputBuffer Output buffer to write to
			* @param reverbInput Reverb input matrix to write to
			* @param lerpFactor Interpolation factor for audio processing
            */
            void ProcessAllSources(std::array<std::optional<Source>, MAX_SOURCES>& sources, ImageSourceManager& imageSources, Buffer& outputBuffer, Matrix& reverbInput, Real lerpFactor);

            /**
			* @brief Processes reverb sources
            * 
			* @param reverbSources Reverb sources to process
			* @param outputBuffer Output buffer to write to
            */
            void ProcessReverbSources(std::vector<std::unique_ptr<ReverbSource>>& reverbSources, Buffer& outputBuffer);

        private:
            moodycamel::ConcurrentQueue<std::shared_ptr<AudioTaskBase>> tasks;  // Lock-free queue

            std::vector<std::thread> workers;   // Worker threads
            std::atomic<bool> stop;             // Flag to stop the thread pool
			size_t threadCount;                 // Number of threads in the pool

			std::vector<Buffer> threadOutputBuffers;    // Output buffers for each thread
			std::vector<Matrix> threadReverbBuffers;    // Reverb input matrices for each thread
        };
    }
}

#endif