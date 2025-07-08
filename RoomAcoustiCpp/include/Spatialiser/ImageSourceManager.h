/*
* @class ImageSourceManager
*
* @brief Declaration of ImageSourceManager class
*
*/

#ifndef RoomAcoustiCpp_ImageSourceManager_h
#define RoomAcoustiCpp_ImageSourceManager_h

// C++ headers
#include <array>

// Spatialiser headers
#include "Spatialiser/ImageSource.h"

// 3DTI headers
#include "BinauralSpatializer/Core.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		/**
		* @brief Class that manages a fixed number of image sources
		*/
		class ImageSourceManager
		{

		public:
			/**
			* @brief Constructor that initialises the image sources
			*/
			ImageSourceManager(Binaural::CCore* core)
			{
				for (auto& imageSource : mImageSources)
					imageSource.emplace(core);
			}

			/**
			* @brief Default destructor
			*/
			~ImageSourceManager() {}

			/**
			* @brief Process audio for all image sources
			* 
			* @param outputBuffer The output audio buffer to write to
			* @param reverbInput The reverb input matrix to write to
			* @param lerpFactor The lerp factor for interpolation
			*/
			inline void ProcessAudio(Buffer<>& outputBuffer, Matrix& reverbInput, const Real lerpFactor)
			{
				for (auto& imageSource : mImageSources)
					imageSource->ProcessAudio(outputBuffer, reverbInput, lerpFactor);
			}

			/**
			* @brief Update the spatialisation mode for all image sources
			* 
			* @param mode The new spatialisation mode
			*/
			inline void UpdateSpatialisationMode(SpatialisationMode mode)
			{
				for (auto& imageSource : mImageSources)
					imageSource->UpdateSpatialisationMode(mode);
			}

			/**
			* @brief Update the impulse response mode for all image sources
			*
			* @param mode True if disable AttuenationSmoothing, false otherwise
			*/
			inline void UpdateImpulseResponseMode(bool mode)
			{
				for (auto& imageSource : mImageSources)
					imageSource->UpdateImpulseResponseMode(mode);
			}

			/**
			* @brief Update the diffraction model for all image sources
			* 
			* @param model The new diffraction model
			* @param fs The sample rate used to initialise the diffraction model
			*/
			inline void UpdateDiffractionModel(DiffractionModel model, int fs)
			{
				for (auto& imageSource : mImageSources)
					imageSource->UpdateDiffractionModel(model, fs);
			}

			/**
			* @return The next free image source ID
			*/
			inline int NextID() const
			{
				int nextID = 0;
				for (const auto& imageSource : mImageSources)
				{
					if (imageSource.has_value() && imageSource->CanEdit())
						return nextID;
					nextID++;
				}
				return -1;
			}

			/**
			* @brief Reset any unused image sources
			*/
			inline void Reset()
			{
				for (auto& imageSource : mImageSources)
					imageSource->Reset();
			}

			/**
			* @brief Access a specific image source by index
			* 
			* @param i The index of the image source to access
			* @return A const reference to the image source at the given index
			*/
			inline const ImageSource& at(const size_t i) const { return mImageSources[i].value(); }

			/**
			* @brief Access a specific image source by index
			*
			* @param i The index of the image source to access
			* @return A reference to the image source at the given index
			*/
			inline ImageSource& at(const size_t i) { return mImageSources[i].value(); }

		private:
			std::array<std::optional<ImageSource>, MAX_IMAGESOURCES> mImageSources;		// Image sources for the audio thread
		};
	}
}

#endif
