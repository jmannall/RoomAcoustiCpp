/*
* @class VirtualSource, VirtualsourceData
*
* @brief Declaration of VirtualSource and VirtualsourceData classes
*
* @remarks Currently, it can only handle one diffracting edge per path/virtual source
*
*/

#ifndef RoomAcoustiCpp_VirtualSource_h
#define RoomAcoustiCpp_VirtualSource_h

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
#include "Spatialiser/Types.h"
#include "Diffraction/Path.h"
#include "Diffraction/Models.h"
#include "Spatialiser/AirAbsorption.h"

// 3DTI headers
#include "BinauralSpatializer/SingleSourceDSP.h"
#include "Common/Transform.h"

// Unity headers
#include "Unity/Debug.h"

// DSP headers
#include "DSP/GraphicEQ.h"

using namespace Common;
namespace RAC
{
	using namespace Common;
	using namespace Unity;
	namespace Spatialiser
	{

		struct Part
		{
			bool isReflection;
			size_t id;
			Part(const size_t _id, const bool _isReflection) : id(_id), isReflection(_isReflection) {};
		};

		//////////////////// VirtualSourceData class ////////////////////

		class VirtualSourceData
		{
		public:

			// Load and Destroy
			VirtualSourceData(size_t numBands) : valid(false), visible(false), feedsFDN(false), mFDNChannel(-1), order(0), reflection(false), diffraction(false), key(""), mAbsorption(numBands),
				distance(0.0), lastUpdatedCycle(false), idKey{'0'}, reflectionKey{'r'}, diffractionKey{'d'} {};
			~VirtualSourceData() { /*Clear();*/ };

			// Plane
			inline void AddPlaneID(const size_t id)
			{
				pathParts.emplace_back(id, true);
				order++;
				reflection = true;
				auto [ptr, ec] = std::to_chars(idKey.data(), idKey.data() + idKey.size(), id);
				if (ec == std::errc()) // Null-terminate manually if conversion is successful
					*ptr = '\0';
				else// Failed to convert id to char array
				{
					key += reflectionKey[0];
					return;
				}

				key += idKey.data();
				key += reflectionKey[0];
			}

			// Absorption
			inline void AddAbsorption(const Absorption& absorption) { mAbsorption *= absorption; }
			inline void ResetAbsorption() { mAbsorption = 1.0; }
			inline Absorption& GetAbsorptionRef() { return mAbsorption; }

			// Edge
			inline void AddEdgeID(const size_t id)
			{
				pathParts.emplace_back(id, false);
				order++;
				diffraction = true;
				auto [ptr, ec] = std::to_chars(idKey.data(), idKey.data() + idKey.size(), id);
				if (ec == std::errc()) // Null-terminate manually if conversion is successful
					*ptr = '\0';
				else// Failed to convert id to char array
				{
					key += diffractionKey[0];
					return;
				}
				key += idKey.data();
				key += diffractionKey[0];
			}

			// Getters
			size_t GetID() const { return pathParts.back().id; }
			size_t GetID(int i) const { return pathParts[i].id; }
			// std::vector<size_t> GetPlaneIDs() const;
			std::string GetKey() const { return key; }
			inline bool IsReflection(int i) const { return pathParts[i].isReflection; }
			inline Absorption GetAbsorption() const { return mAbsorption; }
			inline size_t GetOrder() const { return order; }

			// Transforms
			void SetTransform(const vec3& vSourcePosition);
			void SetTransform(const vec3& vSourcePosition, const vec3& vEdgeSourcePosition);

			inline vec3 GetPosition() const { return mPositions.back(); }
			vec3 GetPosition(int i) const;
			
			// Diffraction
			inline void UpdateDiffractionPath(const vec3& source, const vec3& receiver, const Edge& edge)
			{
				mDiffractionPath.UpdateParameters(source, receiver, edge);
				//vec3 position = mDiffractionPath.CalculateVirtualPostion();
				SetTransform(source, mDiffractionPath.CalculateVirtualPostion());
			}
			inline void UpdateDiffractionPath(const vec3& source, const vec3& receiver, const Plane& plane)
			{
				mDiffractionPath.ReflectEdgeInPlane(plane);
				mDiffractionPath.UpdateParameters(source, receiver);
				//vec3 position = mDiffractionPath.CalculateVirtualPostion();
				SetTransform(source, mDiffractionPath.CalculateVirtualPostion());
			}
			const inline Edge& GetEdge() const { return mDiffractionPath.GetEdge(); }
			inline vec3 GetApex() const { return mDiffractionPath.GetApex(); }

			// Visibility and Validity
			inline void Visible(const bool fdn) { visible = true; feedsFDN = fdn; }
			inline void Invisible() { visible = false; }
			inline void Valid() { valid = true; }
			inline void Invalid() { valid = false; }

			// Reset
			inline void Reset() { Invalid();  Invisible(); ResetAbsorption(); }
			inline void Clear()
			{ 
				Reset();
				pathParts.clear();
				mPositions.clear();
				reflection = false;
				diffraction = false;
				order = 0;
				key = "";
			}
			inline void Update(const VirtualSourceData& vS)
			{
				pathParts = vS.pathParts;
				mPositions = vS.mPositions;
				reflection = vS.reflection;
				diffraction = vS.diffraction;
				if (diffraction)
					mDiffractionPath = vS.mDiffractionPath;
				order = vS.order;
				key = vS.key;
			}

			void SetDistance(const vec3& listenerPosition);

			inline vec4 GetPreviousPlane() const { return previousPlane; }
			inline void SetPreviousPlane(const vec4& plane) { previousPlane = plane; }

			VirtualSourceData Trim(const int i);

			// Status data
			bool reflection;
			bool diffraction;
			bool valid;
			bool visible;
			bool feedsFDN;

			// DSP 
			int mFDNChannel;
			CTransform transform;
			Diffraction::Path mDiffractionPath;
			Real distance;

			bool lastUpdatedCycle;

		private:
			std::string key;
			std::array<char, 21> idKey;
			std::array<char, 1> reflectionKey;
			std::array<char, 1> diffractionKey;
			std::vector<Part> pathParts;
			std::vector<vec3> mPositions;
			size_t order;
			Absorption mAbsorption;
			vec4 previousPlane;
		};

		//////////////////// VirtualSource class ////////////////////

		class VirtualSource // Divide into sub classes types: reflection, diffraction, both
		{
		public:

			// Load and Destroy
			VirtualSource(Binaural::CCore* core, const Config& config, const VirtualSourceData& data, const int fdnChannel);
			~VirtualSource();

			inline bool IsInit() const { return isInitialised; }
			inline bool Exists() const
			{
				if (mCore == nullptr)
					return false;
				return true;
			}
			inline int GetFDNChannel() const { return mFDNChannel; }

			void UpdateSpatialisationMode(const SpatMode mode);
			void UpdateDiffractionModel(const DiffractionModel model);

			// Updates
			bool UpdateVirtualSource(const VirtualSourceData& data, int& fdnChannel);
			inline void UpdateTransform() { if (updateTransform) { mSource->SetSourceTransform(transform); } }

			// Audio
			void ProcessAudio(const Buffer& data, matrix& reverbInput, Buffer& outputBuffer);

			// Deactivate
			inline void Deactivate() { mSource = nullptr; }

		private:
			void Init(const VirtualSourceData& data);
			void Remove();
			void Update(const VirtualSourceData& data, int& fdnChannel);

			void InitAudioData();

			inline void UpdateDiffraction()
			{
				mDiffractionModel->UpdatePath(&mDiffractionPath);
				mDiffractionModel->UpdateParameters();
			}

			inline void ProcessDiffraction(const Buffer& inBuffer, Buffer& outBuffer)
			{
				if (isCrossFading)
				{
					assert(mOldDiffractionModel != nullptr);

					mOldDiffractionModel->ProcessAudio(inBuffer, bDiffStore, bDiffStore.Length(), mConfig.lerpFactor);
					mDiffractionModel->ProcessAudio(inBuffer, outBuffer, mConfig.numFrames, mConfig.lerpFactor);

					for (int i = 0; i < bDiffStore.Length(); ++i)
					{
						float crossfadeFactor = static_cast<float>(crossfadeCounter) / crossfadeLengthSamples;
						outBuffer[i] *= crossfadeFactor;
						outBuffer[i] += bDiffStore[i] * (1.0f - crossfadeFactor);
						++crossfadeCounter;

						if (crossfadeCounter >= crossfadeLengthSamples)
						{
							isCrossFading = false;
							mOldDiffractionModel = nullptr;
							return;
						}
					}
				}
				else
					mDiffractionModel->ProcessAudio(inBuffer, outBuffer, mConfig.numFrames, mConfig.lerpFactor);
			}

			/**
			* 3DTI components
			*/
			Binaural::CCore* mCore;
			shared_ptr<Binaural::CSingleSourceDSP> mSource;

			Config mConfig;
			bool feedsFDN;
			int mFDNChannel;
			int order;

			/**
			* Audio Buffers
			*/
			Buffer bStore;							// Internal working buffer
			Buffer bDiffStore;						// Internal diffraction crossfade buffer
			CMonoBuffer<float> bInput;				// 3DTI Input buffer
			CEarPair<CMonoBuffer<float>> bOutput;	// 3DTI Stero Output buffer
			CMonoBuffer<float> bMonoOutput;			// 3DTI Mono output buffer

			Real mCurrentGain;
			Real mTargetGain;
			GraphicEQ mFilter;
			Diffraction::Path mDiffractionPath;	// Diffraction path

			/**
			* Diffraction Models
			*/
			Diffraction::Attenuate attenuate;	// Diffraction model: Attenuate
			Diffraction::LPF lowPass;			// Diffraction model: LPF
			Diffraction::UDFA udfa;				// Diffraction model: UDFA
			Diffraction::UDFAI udfai;			// Diffraction model: UDFAI
			Diffraction::NNSmall nnSmall;		// Diffraction model: NNSmall
			Diffraction::NNBest nnBest;			// Diffraction model: NNBest
			Diffraction::UTD utd;				// Diffraction model: UTD
			Diffraction::BTM btm;				// Diffraction model: BTM

			/**
			* Diffraction Model Pointers
			*/
			Diffraction::Model* mDiffractionModel;		// Current diffraction model
			Diffraction::Model* mOldDiffractionModel;	// Previous diffraction model

			bool isCrossFading;
			int crossfadeLengthSamples;
			int crossfadeCounter;

			AirAbsorption mAirAbsorption;
			CTransform transform;

			bool isInitialised;
			bool reflection;
			bool diffraction;
			bool updateTransform;

			shared_ptr<std::mutex> audioMutex; // Prevents concurrent audio and update operations
		};
	}
}

#endif