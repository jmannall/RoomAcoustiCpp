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
			* @param data The user defined IEM configuration data
			* @param frequencyBands Frequency bands for graphic equalisers
			*/
			ImageEdge(shared_ptr<Room> room, shared_ptr<SourceManager> sourceManager, const EarlyReverbData& data, const std::shared_ptr<DSPConfig>& dspConfig);
			
			/**
			* @brief Default deconstructor
			*/
			~ImageEdge() {}

			/**
			* @brief Updates the image edge model configuration
			* 
			* @param data The user defined IEM configuration data
			* @param dspConfig The current DSP configuration
			* 
			* @details We must pass DSPConfig here (instead of DiffractionModel) to ensure that
			* if the diffraction model is changed at the same time as the IEM config we can't end up
			* with a stale diffraction model.
			*/
			inline void UpdateIEMConfig(const EarlyReverbData& data, const std::shared_ptr<DSPConfig> dspConfig)
			{
				lock_guard<std::mutex> lock(dataStoreMutex);
				earlyReverbDataIncoming.Update(data, dspConfig->GetDiffractionModel());
				configChanged = true;
			}

			inline void UpdateDiffractionModel(const DiffractionModel model)
			{
				lock_guard<std::mutex> lock(dataStoreMutex);
				configChanged = configChanged || earlyReverbDataIncoming.UpdateSpecularOrder(model);
			}

			/**
			* @brief Updates the stored listener position
			*/
			inline void SetListenerPosition(const Vec3& position)
			{
				lock_guard<std::mutex> lock(dataStoreMutex);
				if ((position - mListenerPositionIncoming).Normal() < EPS_POSITION)
					return;
				mListenerPositionIncoming = position;
				listenerMoved = true;
			}

			/**
			* @brief Process the image edge model and update the target image source data
			*/
			void RunIEM();

			inline void ResetEndFlag()
			{
				while (!iemStartFlag.load(std::memory_order_acquire))
					std::this_thread::yield();
				iemEndFlag.store(false, std::memory_order_release);
			}

			inline bool HasCompleted() { return iemEndFlag.load(std::memory_order_acquire); }

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
			bool LinePlaneIntersection(const Vec3& start, const Vec3& end, const Plane& plane, Coefficients<>& absorption, Vec3& intersection) const;

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
			bool LineWallIntersection(const Vec3& start, const Vec3& end, const std::vector<size_t>& wallIDs, Coefficients<>& absorption, Vec3& intersection) const;

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
			Coefficients<> CalculateDirectivity(const Source::Data& source, const Vec3& point) const;

			/**
			* @brief Run the image edge model for the given source
			*
			* @param source The current source data to run the image edge model for
			* @param imageSources The image source data to write to
			* @param direct The direct sound audio data for the current source
			*/
			void ReflectPointInRoom(const Source::Data& source, Source::DSPParameters& direct, ImageSourceDataMap& imageSources);

			Coefficients<> Direct(const Source::Data& source, bool lineOfSight);

			/**
			* @brief Find all first order diffractions
			* 
			* @param source The current source data
			* @param imageSources The image source data to write to
			* 
			* @return The number of first order diffractions found
			*/
			size_t FirstOrderDiffraction(const Source::Data& source, ImageSourceDataMap& imageSources);

			/**
			* @brief Find all first order reflections
			* 
			* @param source The current source data
			* @param imageSources The image source data to write to
			* @param counter The number of first order diffractions found so far
			* 
			* @return counter + The number of first order reflections found
			*/
			size_t FirstOrderReflections(const Source::Data& source, ImageSourceDataMap& imageSources, size_t counter);

			/**
			* @brief Find all higher order reflection and diffraction paths
			* 
			* @params source The current source data
			* @params imageSources The image source data to write to
			*/
			void HigherOrderPaths(const Source::Data& source, ImageSourceDataMap& imageSources);

			/**
			* @brief Initialise an image source
			* 
			* @param source The current source data
			* @param intersection The first intersection of the image source path
			* @param imageSource The image source data to save
			* @param imageSources The image source data to write to
			* @param feedsFDN True if the image source should feed the FDN, false otherwise
			*/
			void InitImageSource(const Source::Data& source, const Vec3& intersection, ImageSourceData& imageSource, ImageSourceDataMap& imageSources, bool feedsFDN);

			/**
			* @brief Run simple ray tracing and update the late reverberation reflection filters
			*/
			bool UpdateLateReverbFilters(bool updateFilters);

			/**
			* @brief Erase old entries from the image source data map
			* 
			* @param imageSources The image source data map to erase old entries from
			*/
			void EraseOldEntries(ImageSourceDataMap& imageSources);

			weak_ptr<Room> mRoom;							// Pointer to the room class
			weak_ptr<SourceManager> mSourceManager;			// Pointer to the source manager class

			PlaneMap mPlanes;									// Store planes
			WallMap mWalls;										// Store walls
			MaterialMap mMaterials;								// Store materials
			EdgeMap mEdges;										// Store edges
			std::vector<Source::Data> mSources;					// Store sources
			std::vector<ImageSourceDataMap> imageSources;		// Store image sources
			std::vector<Source::DSPParameters> mSourceAudioDatas;		// Store source audio data
			ImageSourceDataStore sp;							// Store valid image sources while the image edge model is being run
			std::vector<bool> mCurrentCycles;					// Oscillates true and false each time the image edge model is run

			EarlyReverbData earlyReverbData;					// The user defined IEM configuration data (can be accessed freely)
			EarlyReverbData earlyReverbDataIncoming;			// The user defined IEM configuration data (Mutex must be locked to access)

			int specularDiffractionStore;			// Stores the specular diffraction order (Mutex must be locked to access)
			bool doSpecularDiffraction;
			Vec3 mListenerPosition;					// The listener position (can be accessed freely)
			Vec3 mListenerPositionIncoming;			// The listener position (Mutex must be locked to access)

			std::vector<Vec3> reverbDirections;				// The directions of the late reverb sources
			std::vector<Coefficients<>> reverbAbsorptions;		// The absorption Coefficients<> of the late reverb sources

			Coefficients<> frequencyBands;			// Frequency bands for graphic equalisers
			bool currentCycle{ false };				// Stores the current cycle of the currently processed source
			bool configChanged{ true };				// True if the image edge model configuration has changed since the last run
			bool listenerMoved{ true };				// True if the listener has moved since the last run
			bool reverbRunning{ false };				// True if the late reverb is running, false otherwise

			std::mutex dataStoreMutex;					// Protects mListenerPositionStore, mIEMConfigStore
			std::atomic<bool> iemStartFlag{ false };	// True if the image edge model is running, false otherwise
			std::atomic<bool> iemEndFlag{ false };		// True if the image edge model has finished running, false otherwise
		};
	}
}

#endif