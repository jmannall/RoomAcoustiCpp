/*
* @class ImageEdge
*
* @brief Declaration of ImageEdge class
*
* @remarks Image edge model based after The image edge model. Armin E, Jonas S, and Michael V. 2015
*
*/

#ifndef RoomAcoustiCpp_ImageEdge_h
#define RoomAcoustiCpp_ImageEdge_h

// C++ headers
#include <unordered_set>

// Common headers
#include "Common/Vec3.h" 

// Spatialiser headers
#include "Spatialiser/Types.h"
#include "Spatialiser/SourceManager.h"
#include "Spatialiser/Room.h"
#include "Spatialiser/Reverb.h"

namespace RAC
{
	namespace Spatialiser
	{
		/**
		* @brief Class that runs the image edge model
		*/
		class ImageEdge
		{
		public:
			/**
			* @brief Constructor that initialises the ImageEdge class
			* 
			* @param room Pointer to the room class
			* @param sourceManager Pointer to the source manager class
			* @param reverb Pointer to the late reverb class
			* @param frequencyBands Frequency bands for graphic equalisers
			*/
			ImageEdge(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, shared_ptr<Reverb> reverb, const Coefficients& numFrequencyBands);
			
			/**
			* @brief Default deconstructor
			*/
			~ImageEdge() {}

			/**
			* @brief Updates the image edge model configuration
			* 
			* @param config The new configuration
			*/
			inline void UpdateIEMConfig(const IEMConfig& config) { lock_guard<std::mutex> lock(dataStoreMutex); mIEMConfigStore = config; configChanged = true; }

			/**
			* @brief Updates the stored listener position
			*/
			inline void SetListenerPosition(const Vec3& position) { lock_guard<std::mutex> lock(dataStoreMutex); mListenerPositionStore = position; }

			/**
			* @brief Process the image edge model and update the target image source data
			*/
			void RunIEM();

		private:
			/**
			* @brief Update the receiver validity for planes and edges
			*/
			void UpdateRValid();

			/**
			* @brief Locates intersection points along an image source path
			*
			* @params imageSource The image source to find intersections for
			* @params intersections A vector to store the intersection points
			*
			* @return True if all intersections are valid, false otherwise
			*/
			bool FindIntersections(ImageSourceData& imageSource, std::vector<Vec3>& intersections) const;

			/**
			* @brief Check for a valid intersection between a line and a plane
			*
			* @param start Start point of the line
			* @param end End point of the line
			* @param plane The plane to check for intersection
			* @param absorption Image source absorption to write to
			* @param intersection A vec3 to store the intersection point
			*
			* @return True if a valid intersection is found, false otherwise
			*/
			bool LinePlaneIntersection(const Vec3& start, const Vec3& end, const Plane& plane, Absorption& absorption, Vec3& intersection) const;

			/**
			* @brief Locate intersection between a line and a collection of walls
			* 
			* @param start Start point of the line
			* @param end End point of the line
			* @param wallIDs The walls to check for intersection
			* @param absorption Image source absorption to write to
			* @param intersection A vec3 to store the intersection point
			* 
			* @return True if a valid intersection is found, false otherwise
			*/
			bool LineWallIntersection(const Vec3& start, const Vec3& end, const std::vector<size_t>& wallIDs, Absorption& absorption, Vec3& intersection) const;

			/**
			* @brief Check for obstructions along an image source path
			* 
			* @param source The source position
			* @param imageSource The image source path data
			* @param intersections The intersection points along the path
			*/
			bool CheckObstructions(const Vec3& source, const ImageSourceData& imageSource, const std::vector<Vec3>& intersections) const;

			/**
			* @brief Check for obstruction between a line and the room
			*
			* @param start Start point of the line
			* @param end End point of the line
			* @param excludedPlaneIds Planes at the start or end points to ignore
			*
			* @return True if an obstruction is found, false otherwise
			*/
			bool LineRoomObstruction(const Vec3& start, const Vec3& end, const std::unordered_set<size_t>& excludedPlaneIds = {}) const;

			/**
			* @brief Check for obstruction between a line and a plane
			* 
			* @param start Start point of the line
			* @param end End point of the line
			* @param plane The plane to check for obstruction
			* 
			* @return True if an obstruction is found, false otherwise
			*/
			bool LinePlaneObstruction(const Vec3& start, const Vec3 end, const Plane& plane) const;

			/**
			* @brief Check for obstruction between a line and a collection of walls
			*
			* @param start Start point of the line
			* @param end End point of the line
			* @param wallIDs The walls to check for obstruction
			*
			* @return True if an obstruction is found, false otherwise
			*/
			bool LineWallObstruction(const Vec3& start, const Vec3& end, const std::vector<size_t>& wallIDs) const;

			/**
			* @brief Calculate the directivity of a source
			*
			* @param source The current source data
			* @param point A point in the direction the source is emitting sound
			*
			* @return The directivity of the source
			* 
			* @remarks Directivities taken from: Eargle's the Microphone Book : From Mono to Stereo to Surround - a Guide to Microphone Design and Application
			*/
			Absorption CalculateDirectivity(const SourceData& source, const Vec3& point) const;

			/**
			* @brief Run the image edge model for the given source
			*
			* @param source The current source data to run the image edge model for
			* @param imageSources The image source data to write to
			* @param direct The direct sound audio data for the current source
			*/
			void ReflectPointInRoom(const SourceData& source, SourceAudioData& direct, ImageSourceDataMap& imageSources);

			/**
			* @brief Find all first order diffractions
			* 
			* @param source The current source data
			* @param imageSources The image source data to write to
			* 
			* @return The number of first order diffractions found
			*/
			size_t FirstOrderDiffraction(const SourceData& source, ImageSourceDataMap& imageSources);

			/**
			* @brief Find all first order reflections
			* 
			* @param source The current source data
			* @param imageSources The image source data to write to
			* @param counter The number of first order diffractions found so far
			* 
			* @return counter + The number of first order reflections found
			*/
			size_t FirstOrderReflections(const SourceData& source, ImageSourceDataMap& imageSources, size_t counter);

			/**
			* @brief Find all higher order reflection and diffraction paths
			* 
			* @params source The current source data
			* @params imageSources The image source data to write to
			*/
			void HigherOrderPaths(const SourceData& source, ImageSourceDataMap& imageSources);

			/**
			* @brief Initialise an image source
			* 
			* @param source The current source data
			* @param intersection The first intersection of the image source path
			* @param imageSource The image source data to save
			* @param imageSources The image source data to write to
			* @param feedsFDN True if the image source should feed the FDN, false otherwise
			*/
			void InitImageSource(const SourceData& source, const Vec3& intersection, ImageSourceData& imageSource, ImageSourceDataMap& imageSources, bool feedsFDN);

			/**
			* @brief Run simple ray tracing and update the late reverberation reflection filters
			*/
			void UpdateLateReverbFilters();

			/**
			* @brief Erase old entries from the image source data map
			* 
			* @param imageSources The image source data map to erase old entries from
			*/
			void EraseOldEntries(ImageSourceDataMap& imageSources);

			weak_ptr<Room> mRoom;							// Pointer to the room class
			weak_ptr<SourceManager> mSourceManager;			// Pointer to the source manager class
			weak_ptr<Reverb> mReverb;						// Pointer to the late reverb class

			PlaneMap mPlanes;									// Store planes
			WallMap mWalls;										// Store walls
			EdgeMap mEdges;										// Store edges
			std::vector<SourceData> mSources;					// Store sources
			std::vector<ImageSourceDataMap> imageSources;		// Store image sources
			std::vector<SourceAudioData> mSourceAudioDatas;		// Store source audio data
			ImageSourceDataStore sp;							// Store valid image sources while the image edge model is being run
			std::vector<bool> mCurrentCycles;					// Oscillates true and false each time the image edge model is run

			IEMConfig mIEMConfig;					// The image edge model configuration (can be accessed freely)
			IEMConfig mIEMConfigStore;				// Stores the image edge model configuration (Mutex must be locked to access)
			Vec3 mListenerPosition;					// The listener position (can be accessed freely)
			Vec3 mListenerPositionStore;			// Stores the listener position (Mutex must be locked to access)

			std::vector<Vec3> reverbDirections;				// The directions of the late reverb sources
			std::vector<Absorption> reverbAbsorptions;		// The absorption coefficients of the late reverb sources

			Coefficients frequencyBands;	// Frequency bands for graphic equalisers
			bool currentCycle;				// Stores the current cycle of the currently processed source
			bool configChanged;				// True if the image edge model configuration has changed since the last run

			std::mutex dataStoreMutex;				// Protects mListenerPositionStore, mIEMConfigStore
		};
	}
}

#endif