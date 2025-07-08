/*
* @class Source
*
* @brief Declaration of Source class
*
*/

#ifndef RoomAcoustiCpp_Source_h
#define RoomAcoustiCpp_Source_h

// C++ headers
#include <tuple>

// Common headers
#include "Common/Matrix.h"
#include "Common/Types.h" 
#include "Common/Vec3.h"
#include "Common/Vec4.h"
#include "Common/Access.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/AirAbsorption.h"
#include "Spatialiser/ImageSource.h"
#include "Spatialiser/ImageSourceManager.h"

// 3DTI headers
#include "BinauralSpatializer/Core.h"
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		//////////////////// Data structures ////////////////////

		/**
		* @brief Class that represents a sound source
		*/
		class Source : public Access
		{
		public:
			/**
			* @brief Struct that stores direct sound audio data
			*/
			struct AudioData
			{
				Absorption<> directivity;	// Frequency dependent directivity
				bool feedsFDN;				// True if direct sound feeds the late reverberation, false otherwise

				AudioData(int len, bool feedsFDN) : directivity(len), feedsFDN(feedsFDN) {}
			};

			/**
			* @brief Struct that stores source position, orientation and directivity
			*/
			struct Data
			{
				size_t id;							// Source ID
				Vec3 position;						// Source Position
				Vec4 orientation;					// Source orientation
				Vec3 forward;						// Source forward direction
				SourceDirectivity directivity;		// Source directivity pattern
				bool hasChanged;					// True if source data has changed since last update, false otherwise

				Data(size_t id, const Vec3& position, const Vec4& orientation, const SourceDirectivity& directivity, bool hasChanged)
					: id(id), position(position), forward(orientation.Forward()), orientation(orientation), directivity(directivity), hasChanged(hasChanged) {}
			};

			/**
			* @brief Default constructor
			* 
			* @param core The 3DTI processing core
			* @param imageSources Reference to the image source array
			* @params config The spatialiser configuration
			*/
			Source(Binaural::CCore* core, ImageSourceManager& imageSources, const std::shared_ptr<Config>& config) : Access(), mCore(core), imageSources(imageSources), inputBuffer(config->numFrames)
			{
				dataMutex = std::make_shared<std::mutex>();
			}

			/**
			* @brief Default deconstructor
			*/
			~Source()
			{
				Remove();
				if (mSource)
					RemoveSource();
				if (mReverbSendSource)
					RemoveReverbSendSource();
			}

			/**
			* @brief Intialises source and allows access
			* 
			* @param config The spatialiser configuration
			*/
			void Init(const std::shared_ptr<Config>& config);

			/**
			* @brief Removes access to the source and flags image sources and input buffer for clearing
			*/
			void Remove();

			/**
			* @brief Reset the input buffer to zeros
			*/
			inline void ResetInputBuffer()
			{
				if (clearInputBuffer.exchange(false, std::memory_order_acq_rel))
					inputBuffer.Reset();
			}

			/**
			* @brief Write audio to the input buffer
			* 
			* @param data The audio data to write to the input buffer
			*/
			inline void SetInputBuffer(const Buffer& data)
			{
				assert(data.Length() == inputBuffer.Length());
				std::transform(data.begin(), data.end(), inputBuffer.begin(), [](Real val) { return val; });
			}

			/**
			* @brief Updates the target spatialisation mode for the HRTF processing
			*
			* @params mode The new spatialisation mode
			*/
			inline void UpdateSpatialisationMode(const SpatialisationMode mode) { spatialisationMode.store(mode, std::memory_order_release); }

			/**
			* @brief Updates the target impulse response mode
			*
			* @params mode True if disable 3DTI Interpolation, false otherwise.
			*/
			inline void UpdateImpulseResponseMode(const bool mode) { impulseResponseMode.store(mode, std::memory_order_release); }

			/**
			* @brief Updates the source directivity
			* 
			* @params directivity The new source directivity
			*/
			void UpdateDirectivity(const SourceDirectivity directivity, const Coefficients<>& frequencyBands, const int numLateReverbChannels);

			/**
			* @brief Updates the source position and orientation
			* 
			* @params position The new source position
			* @params orientation The new source orientation
			* @params distance The distance of the source from the listener
			*/
			void Update(const Vec3& position, const Vec4& orientation, const Real distance);

			/**
			* @brief Updates the source audio DSP parameters and image sources
			* 
			* @params source The source audio DSP parameters
			* @params vSources The current image sources
			*/
			void UpdateData(const Source::AudioData source, const ImageSourceDataMap& imageSourceData, const std::shared_ptr<Config>& config);

			std::optional<Data> GetData();

			/**
			* @return The current source position
			*/
			inline Vec3 GetPosition() const { lock_guard<std::mutex>lock(*dataMutex); return currentPosition; };

			/**
			* @return The current source orientation
			*/
			inline Vec4 GetOrientation() const { lock_guard<std::mutex>lock(*dataMutex); return currentOrientation; };

			/**
			* @return The current source directivity
			*/
			inline SourceDirectivity GetDirectivity() const { return mDirectivity.load(std::memory_order_acquire); }

			/**
			* @return True if the source has changed since the last check
			*/
			inline bool HasChanged() { return hasChanged.exchange(false, std::memory_order_acq_rel); }
			
			/**
			* @brief Process a single audio frame
			* 
			* @param outputBuffer The output buffer to write to
			* @param reverbInput The reverb input buffer to write to
			* @param lerpFactor The lerp factor for interpolation
			*/
			void ProcessAudio(Buffer& outputBuffer, Matrix& reverbInput, const Real lerpFactor);

			/**
			* @brief Resets the source (if not in use) by clearing the buffers and removing the source from the 3DTI processing core
			*/
			void Reset();

		private:
			/**
			* @brief Flags all current image sources for removal
			*/
			inline void RemoveImageSources()
			{
				for (auto& [key, vSource] : currentImageSources)
					imageSources.at(vSource.first).Remove();
				currentImageSources.clear();
			}

			/**
			* @brief Update the spatialisation mode for the HRTF processing
			*
			* @params New spatialisation mode
			*/
			void SetSpatialisationMode(const SpatialisationMode mode);

			/**
			* @brief Updates the interpolation settings for recording impulse responses
			*
			* @params mode True if disable 3DTI Interpolation, false otherwise.
			*/
			void SetImpulseResponseMode(const bool mode);

			/**
			* @brief Initialises the source in the 3DTI core
			*/
			void InitSource();

			/**
			* @brief Removes the source from the 3DTI core
			*/
			void RemoveSource();

			/**
			* @brief Initialises the reverb send source in the 3DTI core
			*
			* @param impulseResponseMode True if disable 3DTI Interpolation, false otherwise.
			*/
			void InitReverbSendSource(const bool impulseResponseMode);

			/**
			* @brief Removes the reverb send source from the 3DTI core
			*/
			void RemoveReverbSendSource();

			/**
			* @brief Initialises the internal audio buffers
			* 
			* @param numFrames The number of frames per audio buffer
			*/
			void InitBuffers(int numFrames);

			/**
			* @brief Clears the internal audio buffers
			*/
			void ClearBuffers();

			/**
			* @brief Clears all shared and unique pointers
			*/
			void ClearPointers();

			/**
			* @brief Updates the target source transform
			*/
			void UpdateTransform(const Vec3& position, const Vec4& orientation);

			/**
			* @brief Updates the current image sources from the target image sources
			*/
			void UpdateImageSourceDataMap(const ImageSourceDataMap& imageSourceData);

			/**
			* @brief Updates the audio thread image sources from the current image sources
			*/
			void UpdateImageSources(const std::shared_ptr<Config>& config);

			/**
			* @brief Updates the audio thread data for a given image source
			* 
			* @params data The new image source data
			* @return True if the image was destroyed successfully, false otherwise
			*/
			bool UpdateImageSource(int& id, ImageSourceData& data, const std::shared_ptr<Config>& config);

			/**
			* @return The next free FDN channel
			*/
			int AssignFDNChannel(const int numLateReverbChannels);

			/**
			* @brief Resets the free FDN channel slots
			*/
			void ResetFDNSlots();

			std::unique_ptr<AirAbsorption> mAirAbsorption;		// Air absorption filter
			std::unique_ptr<GraphicEQ> directivityFilter;		// Directivity filter
			std::unique_ptr<GraphicEQ> reverbInputFilter;		// Reverb energy based on directivity

			std::atomic<SourceDirectivity> mDirectivity;	// Source directivity
			std::atomic<bool> feedsFDN{ false };			// True if the source feeds the FDN

			std::atomic<bool> clearInputBuffer{ false };	// True if the input buffer should be cleared, false otherwise
			Buffer inputBuffer;								// Input audio buffer for the source
			Buffer bStore;									// Internal scratch audio buffer
			Buffer bStoreReverb;							// Internal audio buffer reverb send

			Vec3 currentPosition;						// Current source position
			Vec4 currentOrientation;					// Current source orientation
			std::atomic<bool> hasChanged{ true };		// Flag to check if the source has changed
			
			ImageSourceDataMap currentImageSources;		// Current image sources
			std::vector<int> freeFDNChannels;			// Free FDN channels

			Binaural::CCore* mCore;										// 3DTI core
			shared_ptr<Binaural::CSingleSourceDSP> mSource;				// 3DTI source
			shared_ptr<Binaural::CSingleSourceDSP> mReverbSendSource;	// 3DTI reverb send source

			std::atomic<std::shared_ptr<CTransform>> transform;		// 3DTI source transform
			CMonoBuffer<float> bInput;								// 3DTI mono input buffer
			CEarPair<CMonoBuffer<float>> bOutput;					// 3DTI stereo output buffer
			CMonoBuffer<float> bMonoOutput;							// 3DTI mono output buffer
				
			shared_ptr<std::mutex> dataMutex;		// Protects currentPosition, currentOrientation

			std::atomic<bool> impulseResponseMode{ false };		// True if the image source should be in impulse response mode, false otherwise
			bool currentImpulseResponseMode{ false };			// True if the image source is in impulse response mode, false otherwise

			std::atomic<SpatialisationMode> spatialisationMode{ SpatialisationMode::none };		// Target spatialisation mode
			SpatialisationMode currentSpatialisationMode{ SpatialisationMode::none };			// Current spatialisation mode

			ImageSourceManager& imageSources;	// Image source manager for the audio thread

			static ReleasePool releasePool;		// Garbage collector for shared pointers after atomic replacement
		};
	}
}

#endif