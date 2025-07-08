/*
* @class ImageSource, ImageSourceData
*
* @brief Declaration of ImageSource and ImageSourceData classes
*
* @remarks Currently, it can only handle one diffracting edge per path/image source
*
*/

#ifndef RoomAcoustiCpp_ImageSource_h
#define RoomAcoustiCpp_ImageSource_h

// C++ headers
#include <vector>
#include <unordered_map>
#include <charconv>
#include <array>

// Common headers
#include "Common/Vec3.h"
#include "Common/Vec4.h"
#include "Common/Matrix.h"
#include "Common/Access.h"

// Spatialiser headers
#include "Spatialiser/Wall.h"
#include "Spatialiser/Edge.h"
#include "Spatialiser/Types.h"
#include "Diffraction/Path.h"
#include "Diffraction/Models.h"
#include "Spatialiser/AirAbsorption.h"

// DSP headers
#include "DSP/GraphicEQ.h"
#include "DSP/Parameter.h"

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"

// Unity headers
#include "Unity/Debug.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		/**
		* @brief Stores data used to create an image source
		*/
		class ImageSourceData
		{
			/**
			* @brief Records a reflection or diffraction in the path of an image source
			*/
			struct Part
			{
				bool isReflection;		// True if the part is a reflection, false if it is a diffraction
				size_t id;				// ID of the reflecting plane or diffracting edge

				/**
				* @brief Constructor that initialises the part
				*
				* @param id The ID of the reflecting plane or diffracting edge
				* @param isReflection True if the part is a reflection, false if it is a diffraction
				*/
				Part(const size_t id, const bool isReflection) : id(id), isReflection(isReflection) {};
			};

			/**
			* @brief Stores the base and edge vector of an image edge
			*/
			struct ImageEdgeData
			{
				Vec3 base;				// Base coordinate of the image edge
				Vec3 edgeVector;		// Vector from the base to the top of the image edge

				/**
				* @brief Constructor that initialises the ImageEdgeData
				*
				* @param base The base coordinate of the image edge
				* @param edgeVector The vector from the base to the top of the image edge
				*/
				ImageEdgeData(const Vec3& base, const Vec3& edgeVector) : base(base), edgeVector(edgeVector) {};

				/**
				* @brief Returns the coordinate of the edge at the given z coordinate
				*
				* @param z The z coordinate of the edge
				*
				* @return The coordinate of the edge at the given z coordinate
				*/
				Vec3 GetEdgeCoordinate(const Real z) const { return base + z * edgeVector; }
			};

		public:

			/**
			* @brief Constructor that initialises the image source data
			* 
			* @param numFrequencyBands The number of frequency bands for applying wall absorption
			*/
			ImageSourceData(int numFrequencyBands, int sourceID) : valid(false), visible(false), feedsFDN(false), reflection(false), diffraction(false),
				mAbsorption(numFrequencyBands), distance(0.0), lastUpdatedCycle(false), idKey{ '0' }, sourceKey{ 's' }, reflectionKey { 'r' }, diffractionKey{ 'd' }
			{
				AddSourceID(sourceID);
			};

			/**
			* @brief Default deconstructor
			*/
			~ImageSourceData() {};

			/**
			* @brief Adds absorption to the image source
			* 
			* @param absorption The absorption to add
			*/
			inline void AddAbsorption(const Absorption<>& absorption) { mAbsorption *= absorption; }

			/**
			* @brief Resets the absorption of the image source to 1
			*/
			inline void ResetAbsorption() { mAbsorption = 1.0; }

			/**
			* @return A reference to the absorption of the image source
			*/
			inline Absorption<>& GetAbsorption() { return mAbsorption; }

			/**
			* @return The absorption of the image source
			*/
			inline const Absorption<>& GetAbsorption() const { return mAbsorption; }

			/**
			* @brief Adds a reflection to the image source path
			* 
			* @param id The ID of the reflecting plane
			*/
			void AddPlaneID(const size_t id);

			/**
			* @brief Adds a diffraction to the image source path
			* 
			* @param id The ID of the diffracting edge
			*/
			void AddEdgeID(const size_t id);

			/**
			* @return The previous plane or edge ID
			*/
			inline size_t GetID() const { assert(pathParts.size() > 0); return pathParts.back().id; }

			/**
			* @brief Returns the ID of the plane or edge at the given index within the image source path
			* 
			* @param i The index of the reflection or diffraction within the image source path
			* @return The ID of the plane or edge at the given index
			*/
			inline size_t GetID(int i) const { assert(i < pathParts.size()); return pathParts[i].id; }

			/**
			* @brief Returns whether given index within the image source path is a reflection or diffraction
			*
			* @param i The index of the reflection or diffraction within the image source path
			* @return True if the part is a reflection, false if it is a diffraction
			*/
			inline bool IsReflection(int i) const { assert(i < pathParts.size()); return pathParts[i].isReflection; }

			/**
			* @return The string key representing the image source path
			*/
			inline std::string GetKey() const { return key; }

			/**
			* @return The index of the corresponding image source
			*/
			inline int GetArrayID() const { return arrayID; }

			/**
			* @brief Update the index of the corresponding image source
			* 
			* @param id The index of the corresponding image source
			*/
			inline void UpdateArrayID(const int id) { arrayID = id; }

			/**
			* @brief Sets the 3DTI transform and stores the source position
			*
			* @param position The position of the image source
			*/
			void SetTransform(const Vec3& position);

			/**
			* @brief Sets the 3DTI transform and stores the source position
			*
			* @param position The position of the image source
			* @param rotatedEdgePosition The image source position rotated to provide direction of arrival via the apex point
			*/
			void SetTransform(const Vec3& position, const Vec3& rotatedEdgePosition);

			/**
			* @brief The previous position of the image source
			*/
			inline Vec3 GetPosition() const { return mPositions.back(); }

			/**
			* @brief Returns the position of the image source or image edge apex point at the given index
			* 
			* @param i The index of the position to return
			* @return The position of the image source or image edge apex point at the given index
			*/
			Vec3 GetPosition(int i) const;

			/**
			* @brief Updates the diffraction path of the image source
			* 
			* @param source The position of the image source
			* @param receiver The position of the listener
			* @param edge The edge to diffract around
			*/
			inline void UpdateDiffractionPath(const Vec3& source, const Vec3& receiver, const Edge& edge)
			{
				mEdges.emplace_back(edge.GetBase(), edge.GetEdgeVector());
				mDiffractionPath.UpdateParameters(source, receiver, edge);
				SetTransform(source, mDiffractionPath.CalculateVirtualPostion());
			}

			/**
			* @brief Updates the existing diffraction path following a reflection
			* 
			* @param source The position of the image source
			* @param receiver The position of the listener
			* @param plane The plane to reflect the edge in
			*/
			inline void UpdateDiffractionPath(const Vec3& source, const Vec3& receiver, const Plane& plane)
			{
				mDiffractionPath.ReflectEdgeInPlane(plane);
				mEdges.emplace_back(mDiffractionPath.GetEdge().GetBase(), mDiffractionPath.GetEdge().GetEdgeVector());
				mDiffractionPath.UpdateParameters(source, receiver);
				SetTransform(source, mDiffractionPath.CalculateVirtualPostion());
			}

			/**
			* @return The edge in the diffraction path
			*/
			const inline Edge& GetEdge() const { return mDiffractionPath.GetEdge(); }

			/**
			* @return The apex of the diffraction path
			*/
			inline Vec3 GetApex() const { assert(mEdges.size() > 0); return mEdges[0].GetEdgeCoordinate(mDiffractionPath.GetApexZ()); }

			/**
			* @brief Sets the image source as visible and whether it feeds the FDN
			* 
			* @param fdn True if the image source feeds the FDN, false otherwise
			*/
			inline void Visible(const bool fdn) { visible = true; feedsFDN = fdn; }

			/**
			* @brief Sets the image source as invisible
			*/
			inline void Invisible() { visible = false; }

			/**
			* @brief Sets the image source as valid
			*/
			inline void Valid() { valid = true; }

			/**
			* @brief Sets the image source as invalid
			*/
			inline void Invalid() { valid = false; }

			/**
			* Resets the image source as invalid, invisible, and with no absorption
			*/
			inline void Reset() { Invalid();  Invisible(); ResetAbsorption(); }

			/**
			* @brief Clears the image source data
			*/
			void Clear(int sourceID);

			/**
			* @brief Updates the image source data using another image source
			* 
			* @param imageSource The image source to update from
			*/
			void Update(const ImageSourceData& imageSource);

			/**
			* @brief Sets the distance of the image source from the listener
			* 
			* @param listenerPosition The position of the listener
			*/
			void SetDistance(const Vec3& listenerPosition);

			/**
			* @return Previous plane information where: w -> D, x, y, z -> Normal
			*/
			inline Vec4 GetPreviousPlane() const { return previousPlane; }

			/**
			* @brief Set plane information where: w -> D, x, y, z -> Normal
			* 
			* @param plane The new plane information
			*/
			inline void SetPreviousPlane(const Vec4& plane) { previousPlane = plane; }

			/**
			* @return True if the image source is valid, false otherwise
			*/
			inline bool IsValid() const { return valid; }

			/**
			* @return True if the image source is visible, false otherwise
			*/
			inline bool IsVisible() const { return visible; }

			/**
			* @return True if the image source should feed the FDN, false otherwise
			*/
			inline bool IsFeedingFDN() const { return feedsFDN; }

			/**
			* @return True if the image source path includes any reflections, false otherwise
			*/
			inline bool IsReflection() const { return reflection; }

			/**
			* @return True if the image source path includes any diffractions, false otherwise
			*/
			inline bool IsDiffraction() const { return diffraction; }

			/**
			* @return The distance of the image source from the listener
			*/
			inline Real GetDistance() const { return distance; }

			/**
			* @return The 3DTI transform of the image source
			*/
			inline CTransform GetTransform() const { return transform; }

			/**
			* @return The diffraction path of the image source
			*/
			inline Diffraction::Path GetDiffractionPath() const { assert(diffraction); return mDiffractionPath; }

			/**
			* @brief Updates the cycle the image source was last updated in
			*/
			inline void UpdateCycle(const bool thisCycle) { lastUpdatedCycle = thisCycle; }

			/**
			* @return True if the image source was updated in the current cycle, false otherwise
			*/
			inline bool UpdatedThisCycle(const bool thisCycle) const { return lastUpdatedCycle == thisCycle; }

		private:

			/**
			* @brief Adds the sourceID to the key
			*
			* @param id The ID of the source
			*/
			void AddSourceID(const size_t id);

			int arrayID{ -1 };

			std::string key;							// String key that defines the image source path
			std::array<char, 21> idKey;					// Char that stores the ID of a plane, edge or source
			std::array<char, 1> sourceKey;				// Char that stores the source key
			std::array<char, 1> reflectionKey;			// Char that stores the reflection key
			std::array<char, 1> diffractionKey;			// Char that stores the diffraction key

			std::vector<Part> pathParts;			// Reflection and diffraction parts of the image source path
			std::vector<Vec3> mPositions;			// Positions of the image source along the path
			std::vector<ImageEdgeData> mEdges;		// Image edges along the image source path
			int diffractionIndex;					// Index of the first diffraction in the image source path
			Vec4 previousPlane;						// Previous reflected plane information where: w -> D, x, y, z -> Normal

			Diffraction::Path mDiffractionPath;			// Diffraction path of the image source
			Absorption<> mAbsorption;					// Wall absorption of the image source
			Real distance;								// Distance of the image source from the listener
			CTransform transform;						// 3DTI transform of the image source

			bool valid;						// True if the image source is valid, false otherwise
			bool visible;					// True if the image source is visible, false otherwise
			bool feedsFDN;					// True if the image source should feed the FDN, false otherwise
			bool reflection;				// True if the image source path includes any reflections, false otherwise
			bool diffraction;				// True if the image source path includes any diffractions, false otherwise
			bool lastUpdatedCycle;			// True if the image source was updated in the current cycle, false otherwise
		};

		/**
		* @brief Represents an image source and processes its audio
		*/
		class ImageSource : public Access // Divide into sub classes types: reflection, diffraction, both?
		{
		public:

			/**
			* @brief Default constructor
			* 
			* @param core The 3DTI processing core
			*/
			ImageSource(Binaural::CCore* core) : Access(), mCore(core) {}

			/**
			* @brief Default deconstructor. Removes the image source from the 3DTI processing core
			*/
			~ImageSource()
			{
				if (mSource)
					RemoveSource();
			}

			/**
			* @brief Reset and initialise the image source with the given configuration and data
			* 
			* @param inputBuffer Pointer to the source input buffer
			* @param config The current RAC configuration
			* @param data The image source data to initialise with
			* @param fdnChannel The FDN channel to feed, -1 if the image source does not feed the FDN
			*/
			void Init(const Buffer<>* inputBuffer, const std::shared_ptr<Config>& config, const ImageSourceData& data, int fdnChannel);

			/**
			* @brief Update the image source and remove if no longer visible
			*
			* @params data The new image source data
			* @params fdnChannel The FDN channel to feed, gets updated to the previous FDN channel if feedsFDN has changed
			*
			* @return True if the image source should be removed
			*/
			bool Update(const ImageSourceData& data, int& fdnChannel);

			/**
			* @brief Remove the image source from any audio processing
			*/
			inline void Remove() { PreventAccess(); }

			/**
			* @return The FDN channel the image source feeds, -1 if the image source does not feed the FDN
			*/
			inline int GetFDNChannel() const { return mFDNChannel.load(std::memory_order_acquire); }

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
			* @brief Update the diffraction model
			* 
			* @params model The new diffraction model
			* @params fs The sample rate used to initialise the diffraction model
			*/
			void UpdateDiffractionModel(const DiffractionModel model, const int fs);

			/**
			* @brief Process a single audio frame
			* 
			* @param outputBuffer The output buffer to write to
			* @param reverbInput The reverb input buffer to write to
			* @param lerpFactor The lerp factor for interpolation
			*/
			void ProcessAudio(Buffer<>& outputBuffer, Matrix& reverbInput, const Real lerpFactor);

			/**
			* @brief Resets the image source by clearing the buffers and removing the source from the 3DTI processing core
			*/
			void Reset();

		private:
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
			* @brief Check that configuration parameters have not changed since initialisation and AllowAccess
			*
			* @param config The current RAC configuration
			*/
			void LateInit(const std::shared_ptr<Config>& config);

			/**
			* @brief Reset diffraction models and initialise the diffraction model with the given path
			*
			* @params model The diffraction model to initialise
			* @params path The diffraction path to initialise the model with
			* @params fs The sample rate used to initialise the diffraction model
			*/
			void InitDiffractionModel(const DiffractionModel model, const Diffraction::Path& path, const int fs);

			/**
			* @brief Initialises the image source in the 3DTI processing core
			*/
			void InitSource();

			/**
			* @brief Romeves the image source from the 3DTI processing core
			*/
			void RemoveSource();
			
			/**
			* @brief Updates the image source with the given data
			* 
			* @param data The image source data
			* @param fdnChannel The FDN channel to feed, gets updated to the previous channel if feedsFDN has changed
			*/
			void UpdateParameters(const ImageSourceData& data, int& fdnChannel);

			/**
			* @brief Stores the position and rotation source
			* 
			* @params newTransform The transform to update the image source with
			*/
			void UpdateTransform(const CTransform& newTransform);

			/**
			* @brief Process a single audio frame using the current diffraction model
			* 
			* @details Applies a cross fase if the diffraction model has been changed
			* 
			* @params inBuffer The input audio buffer
			* @params outBuffer The output audio buffer to write to
			* @params lerpFactor The lerp factor for interpolation
			*/
			void ProcessDiffraction(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor);

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

			std::atomic<bool> feedsFDN{ false };	// True if the image source feeds the FDN, false otherwise
			std::atomic<int> mFDNChannel{ -1 };		// The FDN channel the image source feeds, -1 if the image source does not feed the FDN

			const Buffer<>* inputBuffer{ nullptr };		// Pointer to the source input buffer
			Buffer<> bStore;								// Internal working buffer
			Buffer<> bDiffStore;							// Internal diffraction crossfade buffer
			CMonoBuffer<float> bInput;					// 3DTI Input buffer
			CEarPair<CMonoBuffer<float>> bOutput;		// 3DTI Stero Output buffer
			CMonoBuffer<float> bMonoOutput;				// 3DTI Mono output buffer for reverb send

			Parameter gain{ 0.0 };								// 1.0 if the source is visible, 0.0 otherwise
			std::unique_ptr<GraphicEQ> mFilter;					// Frequency dependent reflection and directivity filter
			std::unique_ptr<AirAbsorption> mAirAbsorption;		// Air absorption filter

			Parameter diffractionGain{ 1.0 };											// Gain for crossfading diffracton models
			Diffraction::Path mDiffractionPath;											// Diffraction path
			std::atomic<DiffractionModel> currentDiffractionModel;						// Current diffraction model
			std::shared_ptr<Diffraction::Model> activeModel;							// Active diffraction model for processing audio
			std::shared_ptr<Diffraction::Model> fadeModel{ nullptr };					// Next active diffraction model 
			std::atomic<std::shared_ptr<Diffraction::Model>> incomingModel{ nullptr };	// Incoming diffraction model after ongoing crossfade
			std::atomic<std::shared_ptr<Diffraction::Model>> nextModel{ nullptr };		// Queued diffraction model
			std::atomic<bool> isCrossFading{ false };									// True if currently crossfading between diffraction models, false otherwise

			bool reflection{ false };					// True if the image source path includes any reflections, false otherwise
			bool diffraction{ false };					// True if the image source path includes any diffractions, false otherwise

			std::atomic<bool> impulseResponseMode{ false };		// True if the image source should be in impulse response mode, false otherwise
			bool currentImpulseResponseMode{ false };			// True if the image source is in impulse response mode, false otherwise

			std::atomic<SpatialisationMode> spatialisationMode{ SpatialisationMode::none };		// Target spatialisation mode
			SpatialisationMode currentSpatialisationMode{ SpatialisationMode::none };			// Current spatialisation mode

			Binaural::CCore* mCore;										// 3DTI processing core
			shared_ptr<Binaural::CSingleSourceDSP> mSource{ nullptr };	// 3DTI source
			std::atomic<std::shared_ptr<CTransform>> transform;			// 3DTI source transform

			static ReleasePool releasePool;		// Garbage collector for shared pointers after atomic replacement
		};
	}
}

#endif