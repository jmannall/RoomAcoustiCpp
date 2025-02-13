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

// Spatialiser headers
#include "Spatialiser/Wall.h"
#include "Spatialiser/Edge.h"
#include "Spatialiser/Types.h"
#include "Diffraction/Path.h"
#include "Diffraction/Models.h"
#include "Spatialiser/AirAbsorption.h"

// DSP headers
#include "DSP/GraphicEQ.h"

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	namespace Spatialiser
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

		/**
		* @brief Stores data used to create an image source
		*/
		class ImageSourceData
		{
		public:

			/**
			* @brief Constructor that initialises the image source data
			* 
			* @param numFrequencyBands The number of frequency bands for applying wall absorption
			*/
			ImageSourceData(int numFrequencyBands, int sourceID) : valid(false), visible(false), feedsFDN(false), reflection(false), diffraction(false),
				mAbsorption(numFrequencyBands), mDirectivity(1.0), distance(0.0), lastUpdatedCycle(false), idKey{ '0' }, sourceKey{ 's' }, reflectionKey { 'r' }, diffractionKey{ 'd' }
			{
				AddSourceID(sourceID);
			};

			/**
			* @brief Default deconstructor
			*/
			~ImageSourceData() {};

			/**
			* @brief Sets the directivity of the image source
			* 
			* @param directivity The directivity to add
			*/
			inline void SetDirectivity(const Real directivity) { mDirectivity = directivity; }

			/**
			* @brief Adds absorption to the image source
			* 
			* @param absorption The absorption to add
			*/
			inline void AddAbsorption(const Absorption& absorption) { mAbsorption *= absorption; }

			/**
			* @brief Resets the absorption of the image source to 1
			*/
			inline void ResetAbsorption() { mAbsorption = 1.0; }

			/**
			* @return A reference to the absorption of the image source
			*/
			inline Absorption& GetAbsorption() { return mAbsorption; }

			/**
			* @return The absorption of the image source
			*/
			inline const Absorption& GetAbsorption() const { return mAbsorption; }

			/**
			* @return The directivity of the image source
			*/
			inline Real GetDirectivity() const { return mDirectivity; }

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
			* @parmas i The index of the reflection or diffraction within the image source path
			* 
			* @return The ID of the plane or edge at the given index
			*/
			inline size_t GetID(int i) const { assert(i < pathParts.size()); return pathParts[i].id; }

			/**
			* @brief Returns whether given index within the image source path is a reflection or diffraction
			*
			* @parmas i The index of the reflection or diffraction within the image source path
			*
			* @return True if the part is a reflection, false if it is a diffraction
			*/
			inline bool IsReflection(int i) const { assert(i < pathParts.size()); return pathParts[i].isReflection; }

			/**
			* @return The string key representing the image source path
			*/
			inline std::string GetKey() const { return key; }

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
			* 
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
			Absorption mAbsorption;						// Wall absorption of the image source
			Real mDirectivity;							// Directivity of the image source
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
		class ImageSource // Divide into sub classes types: reflection, diffraction, both?
		{
		public:

			/**
			* @brief Constructor that initialises the virtual source
			* 
			* @param core The 3DTI processing core
			* @param config The spatialiser configuration
			* @param data The image source data
			* @param fdnChannel The FDN channel to feed, -1 if the image source does not feed the FDN
			*/
			ImageSource(Binaural::CCore* core, const Config& config, const ImageSourceData& data, const int fdnChannel);

			/**
			* @brief Default deconstructor
			*/
			~ImageSource();

			/**
			* @return The FDN channel the image source feeds, -1 if the image source does not feed the FDN
			*/
			inline int GetFDNChannel() const { return mFDNChannel; }

			/**
			* @brief Update the spatialisation mode for the HRTF processing
			*
			* @params New spatialisation mode
			*/
			void UpdateSpatialisationMode(const SpatialisationMode mode);

			/**
			* @brief Update the diffraction model
			* 
			* @params model The new diffraction model
			*/
			void UpdateDiffractionModel(const DiffractionModel model);

			/**
			* @brief Update the image source
			* 
			* @params data The new image source data
			* @params fdnChannel The FDN channel to feed, gets updated to the previous channel if feedsFDN has changed
			* 
			* @return True if the image source should be removed
			*/
			bool Update(const ImageSourceData& data, int& fdnChannel);

			/**
			* @brief Process a single audio frame
			* 
			* @param data The audio data to process
			* @param reverbInput The reverb input buffer to write to
			* @param outputBuffer The output buffer to write to
			*/
			void ProcessAudio(const Buffer& data, Matrix& reverbInput, Buffer& outputBuffer);

			/**
			* @brief Prevents the 3DTI source from being destroyed when destructor called.
			*/
			inline void Deactivate() { mSource = nullptr; }

		private:
			/**
			* @brief Initialises the image source with the given data
			* 
			* @param data The image source data
			*/
			void Init(const ImageSourceData& data);

			/**
			* @brief Updates the image source with the given data
			* 
			* @param data The image source data
			* @param fdnChannel The FDN channel to feed, gets updated to the previous channel if feedsFDN has changed
			*/
			void UpdateParameters(const ImageSourceData& data, int& fdnChannel);

			/**
			* @brief Romeves the image source from the 3DTI processing core
			*/
			void Remove();

			/**
			* @brief Updated the diffraction path and target parameters of the current diffraction model
			*/
			inline void UpdateDiffraction()
			{
				mDiffractionModel->UpdatePath(&mDiffractionPath);
				mDiffractionModel->UpdateParameters();
			}

			/**
			* @brief Process a single audio frame using the current diffraction model
			* 
			* @details Applies a cross fase if the diffraction model has been changed
			* 
			* @params inBuffer The input audio buffer
			* @params outBuffer The output audio buffer to write to
			*/
			void ProcessDiffraction(const Buffer& inBuffer, Buffer& outBuffer);

			Config mConfig;			// Spatialiser configuration
			bool feedsFDN;			// True if the image source feeds the FDN, false otherwise
			int mFDNChannel;		// The FDN channel the image source feeds, -1 if the image source does not feed the FDN

			Buffer bStore;								// Internal working buffer
			Buffer bDiffStore;							// Internal diffraction crossfade buffer
			CMonoBuffer<float> bInput;					// 3DTI Input buffer
			CEarPair<CMonoBuffer<float>> bOutput;		// 3DTI Stero Output buffer
			CMonoBuffer<float> bMonoOutput;				// 3DTI Mono output buffer

			Real mCurrentGain;					// Current gain of the image source
			Real mTargetGain;					// Target gain of the image source
			GraphicEQ mFilter;					// Frequency dependent reflection filter
			AirAbsorption mAirAbsorption;		// Air absorption filter

			Diffraction::Path mDiffractionPath;				// Diffraction path
			Diffraction::Attenuate attenuate;				// Diffraction model: Attenuate
			Diffraction::LPF lowPass;						// Diffraction model: LPF
			Diffraction::UDFA udfa;							// Diffraction model: UDFA
			Diffraction::UDFAI udfai;						// Diffraction model: UDFAI
			Diffraction::NNSmall nnSmall;					// Diffraction model: NNSmall
			Diffraction::NNBest nnBest;						// Diffraction model: NNBest
			Diffraction::UTD utd;							// Diffraction model: UTD
			Diffraction::BTM btm;							// Diffraction model: BTM
			Diffraction::Model* mDiffractionModel;			// Current diffraction model
			Diffraction::Model* mOldDiffractionModel;		// Previous diffraction model

			bool isCrossFading;				// True if currently crossfading between diffraction models, false otherwise		
			int crossfadeLengthSamples;		// Length of diffraction model crossfase in samples
			int crossfadeCounter;			// Counter for the diffraction model crossfase


			bool isInitialised;				// True if the image source has been initialised, false otherwise
			bool reflection;				// True if the image source path includes any reflections, false otherwise
			bool diffraction;				// True if the image source path includes any diffractions, false otherwise

			Binaural::CCore* mCore;								// 3DTI processing core
			shared_ptr<Binaural::CSingleSourceDSP> mSource;		// 3DTI source
			CTransform transform;								// 3DTI source transform

			shared_ptr<std::mutex> audioMutex;		// Prevents concurrent audio and update operations
		};
	}
}

#endif