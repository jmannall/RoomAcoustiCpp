/*
* @class Source
*
* @brief Declaration of Source class
*
*/

#ifndef RoomAcoustiCpp_Source_h
#define RoomAcoustiCpp_Source_h

#include <latch>
#include "Common/ThreadPool.h"
#include <tuple>

// Common headers
#include "Common/Matrix.h"
#include "Common/Types.h" 
#include "Common/Vec3.h"
#include "Common/Vec4.h"

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/AirAbsorption.h"
#include "Spatialiser/ImageSource.h"
#include "Spatialiser/Globals.h"

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

		// typedef std::pair<Absorption, bool> SourceAudioData;

		struct SourceAudioData
		{
			Absorption directivity;
			bool feedsFDN;

			SourceAudioData(int len, bool feedsFDN) : directivity(len), feedsFDN(feedsFDN) {}
		};

		/**
		* @brief Describes source position, orientation and directivity
		*/
		struct SourceData
		{
			size_t id;
			Vec3 position;
			Vec3 forward;
			Vec4 orientation;
			SourceDirectivity directivity;
			bool hasChanged;
		};

		/**
		* @brief Class that represents a sound source
		*/
		class Source
		{
		public:

			/**
			* @brief Constructor that initialises the source with the given configuration.
			* 
			* @param core The 3DTI processing core
			* @param config The spatialiser configuration
			*/
			Source(Binaural::CCore* core, const Config& config);

			/**
			* @brief Default deconstructor
			*/
			~Source();

			void InitReverbSendSource();

			void RemoveReverbSendSource();

			/**
			* @brief Update the spatialisation mode for the HRTF processing
			* 
			* @param mode The new spatialisation mode
			*/
			void UpdateSpatialisationMode(const SpatialisationMode mode);

			/**
			* @brief Updates the interpolation settings for recording impulse responses
			*
			* @params lerpFactor New interpolation factor
			* @params mode True if disable 3DTI Interpolation, false otherwise.
			*/
			void UpdateImpulseResponseMode(const Real lerpFactor, const bool mode);

			/**
			* @brief Updates the diffraction model
			* 
			* @params model The new diffraction model
			*/
			void UpdateDiffractionModel(const DiffractionModel model);

			/**
			* @brief Updates the source directivity
			* 
			* @params directivity The new source directivity
			*/
			void UpdateDirectivity(const SourceDirectivity directivity);

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
			inline void UpdateData(const SourceAudioData source, const ImageSourceDataMap& vSources)
			{ 
				{
					lock_guard<mutex> lock(*dataMutex); 
					directivityFilter.SetTargetGains(source.directivity);
					feedsFDN = source.feedsFDN;

					if (feedsFDN && !mReverbSendSource)
						InitReverbSendSource();
					else if (!feedsFDN && mReverbSendSource)
						RemoveReverbSendSource();
				}
				targetImageSources = vSources;
				UpdateImageSourceDataMap();
				UpdateImageSources();
			}

			/**
			* @return The current source position
			*/
			inline Vec3 GetPosition() const { lock_guard<std::mutex>lock(*currentDataMutex); return currentPosition; };

			/**
			* @return The current source orientation
			*/
			inline Vec4 GetOrientation() const { lock_guard<std::mutex>lock(*currentDataMutex); return currentOrientation; };

			/**
			* @return The current source directivity
			*/
			inline SourceDirectivity GetDirectivity() const { lock_guard<mutex> lock(*dataMutex); return mDirectivity; }

			/**
			* @return True if the source has changed since the last check
			*/
			inline bool HasChanged() { lock_guard<mutex> lock(*currentDataMutex); if (hasChanged) { hasChanged = false; return true; } return false; }
			
			/**
			* @brief Process a single audio frame
			* 
			* @params data The input audio buffer
			* @params reverbInput The reverb input buffer to write to
			* @params outputBuffer The output buffer to write to
			*/
			void ProcessAudio(const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer);

			/**
			* @brief Destroys all image sources
			*/
			inline void Reset()
			{ 
				{ lock_guard<std::mutex> lock(*imageSourcesMutex); mImageSources.clear(); threadResults.clear(); }
				ResetFDNSlots();
			}

		private:
			/**
			* @brief Updates the current image sources from the target image sources
			*/
			void UpdateImageSourceDataMap();

			/**
			* @brief Updates the audio thread image sources from the current image sources
			*/
			void UpdateImageSources();

			/**
			* @brief Updates the audio thread data for a given image source
			* 
			* @params data The new image source data
			* @return True if the image was destroyed successfully, false otherwise
			*/
			bool UpdateImageSource(const ImageSourceData& data);

			/**
			* @return The next free FDN channel
			*/
			int AssignFDNChannel();

			/**
			* @brief Resets the free FDN channel slots
			*/
			void ResetFDNSlots();

			void ProcessDirect(const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer);

			Config mConfig;							// The spatialiser configuration
			AirAbsorption mAirAbsorption;			// Air absorption filter
			SourceDirectivity mDirectivity;			// Source directivity
			Buffer bStore;							// Internal audio buffer
			Buffer bStoreReverb;					// Internal audio buffer reverb send

			Real targetGain;				// Target source gain
			Real currentGain;				// Current source gain
			GraphicEQ directivityFilter;	// Directivity filter
			GraphicEQ reverbInputFilter;	// Reverb energy based on directivity
			bool feedsFDN;					// True if the source feeds the FDN

			Vec3 currentPosition;			// Current source position
			Vec4 currentOrientation;		// Current source orientation
			bool hasChanged;				// Flag to check if the source has changed
			
			ImageSourceMap mImageSources;				// Audio thread image sources
			ImageSourceDataMap currentImageSources;		// Current image sources
			ImageSourceDataMap targetImageSources;		// Target image sources
			std::vector<int> freeFDNChannels;			// Free FDN channels

			Binaural::CCore* mCore;								// 3DTI core
			shared_ptr<Binaural::CSingleSourceDSP> mSource;		// 3DTI source
			shared_ptr<Binaural::CSingleSourceDSP> mReverbSendSource;	// 3DTI reverb send source

			CTransform transform;								// 3DTI source transform
			CMonoBuffer<float> bInput;							// 3DTI mono input buffer
			CEarPair<CMonoBuffer<float>> bOutput;				// 3DTI stereo output buffer
			CMonoBuffer<float> bMonoOutput;						// 3DTI mono output buffer

			shared_ptr<std::mutex> dataMutex;					// Protects isVisible, targetGain, currentGain, mDirectivity
			shared_ptr<std::mutex> imageSourceDataMutex;		// Protects currentImageSources, targetImageSources
			shared_ptr<std::mutex> imageSourcesMutex;			// Protects mImageSources
			shared_ptr<std::mutex> currentDataMutex;			// Protects currentPosition, currentOrientation, hasChanged

			std::vector<std::tuple<Buffer, Buffer, int>> threadResults;
		};
	}
}

#endif