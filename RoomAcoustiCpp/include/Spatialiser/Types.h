/*
* @brief Declaration of Typedefs, Enums and Struct classes used within RAC
*
*/

#ifndef RoomAcoustiCpp_Types_h
#define RoomAcoustiCpp_Types_h

// C++ headers
#include <unordered_map>
#include <array>
#include <variant>

// Common headers
#include "Common/Types.h"
#include "Common/Complex.h"
#include "Common/Coefficients.h"
#include "Common/Vec3.h"
#include "Common/Matrix.h"
#include "Common/Vec.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// Typedefs ////////////////////

		/**
		* Class predeclarations
		*/
		class Plane;
		class Wall;
		class Edge;
		class Source;
		class ImageSource;
		class ImageSourceData;

		typedef std::unordered_map<size_t, Plane> PlaneMap;									// Store planes
		typedef std::unordered_map<size_t, Wall> WallMap;									// Store walls
		typedef std::unordered_map<size_t, Coefficients<>> MaterialMap;						// Store materials
		typedef std::unordered_map<size_t, Edge> EdgeMap;									// Store edges
		typedef std::unordered_map<size_t, Source> SourceMap;								// Store sources
		typedef std::unordered_map<std::string, ImageSource> ImageSourceMap;				// Store image sources
		typedef std::unordered_map<std::string, std::pair<int, ImageSourceData>> ImageSourceDataMap;		// Store image source datas

		typedef std::vector<std::vector<ImageSourceData>> ImageSourceDataStore;				// Store image source data

		typedef std::array<Vec3, 3> Vertices;		// Store vertices

		//////////////////// Enum Data Types ////////////////////

		enum class ThreadID
		{
			imageEdge,
			rayTracing
		};

		enum class ReverbFormula
		{
			Sabine,
			Eyring,
			Custom
		};

		enum class FDNMatrix
		{
			householder,
			randomOrthogonal			
		};

		enum class DiffractionModel
		{
			attenuate,
			lowPass,
			udfa,
			udfai,
			nnBest,
			nnSmall,
			utd,
			btm
		};

		enum class SourceDirectivity
		{
			omni,
			subcardioid,
			cardioid,
			supercardioid,
			hypercardioid,
			bidirectional,
			genelec8020c,
			genelec8020cDTF,
			qscK8
		};

		enum class SpatialisationMode { quality, performance, none };

		enum class LateReverbModel { none, fdn, raves };

		enum class DirectSound { none, doCheck, alwaysTrue };

		enum class DiffractionSound { none, shadowZone, allZones };

		//////////////////// Struct Data Types ////////////////////

		/**
		* @brief Struct that stores DSP configuration data
		* 
		* @details Used to store data that is used at initialisation and then remains constant
		*/
		struct DSPData
		{
		public:
			int fs{ 48000 };							// Sample rate
			int numFrames{ 512 };						// Number of frames per audio callback
			int numReverbSources{ 12 };					// Number of output channels for late reverberation
			int fdnSize{ 12 };							// Size of the FDN (number of delay lines)
			Real Q{ 0.98 };								// Q factor for the GraphicEQ
			Coefficients<> frequencyBands;				// Frequency band center frequencies
			int numFrequencyBands{ 0 };					// Number of frequency bands

			DSPData() : frequencyBands(Coefficients<>(std::vector<Real>({ (Real)250.0, (Real)500.0, (Real)1000.0, (Real)2000.0 })))
			{}

			DSPData(int sampleRate, int numFrames, int numReverbSources, int fdnSize, Real lerpFactor, Real Q, Coefficients<> frequencyBands) :
				fs(sampleRate), numFrames(numFrames), numReverbSources(CalculateNumReverbSources(numReverbSources)), fdnSize(CalculateFDNSize(fdnSize, this->numReverbSources)),
				lerpFactor(CalculateLerpFactor(lerpFactor)), Q(Q), frequencyBands(frequencyBands), numFrequencyBands(ToInt(frequencyBands.Length()))
			{}

			inline Real GetLerpFactor() const { return lerpFactor; }

			inline void UpdateLerpFactor(Real lerpFactor) { this->lerpFactor = CalculateLerpFactor(lerpFactor); }

		private:
			constexpr Real CalculateLerpFactor(Real lerpFactor)
			{
				Real factor = (Real)96.0 * lerpFactor / fs;
				return std::max(std::min(factor, (Real)1.0), (Real)1.0 / fs);
			}

			constexpr int CalculateNumReverbSources(int maxReverbSources)
			{
				if (maxReverbSources < 2)
					return 1;
				else if (maxReverbSources < 4)
					return 2;
				else if (maxReverbSources < 6)
					return 4;
				else if (maxReverbSources < 8)
					return 6;
				else if (maxReverbSources < 12)
					return 8;
				else if (maxReverbSources < 16)
					return 12;
				else if (maxReverbSources < 20)
					return 16;
				else if (maxReverbSources < 24)
					return 20;
				else if (maxReverbSources < 32)
					return 24;
				else
					return 32;
			}

			constexpr int CalculateFDNSize(int fdnSize, int numReverbSources)
			{
				fdnSize = std::min(std::max(fdnSize, MIN_FDNSIZE), MAX_FDNSIZE);
				return std::max(fdnSize, numReverbSources);
			}

			/**
			* @details 1 means DSP parameters are lerped over only 1 audio callback,
			* 5 means lerped over 5 separate audio callbacks. Must be greater than 0
			*/
			Real lerpFactor{ CalculateLerpFactor((Real)2.0) };
		};

		/**
		* @brief Configuration struct for RAC
		*/
		class DSPConfig
		{
		public:

			/**
			* @brief Default constructor for the DSPConfig class
			*/
			DSPConfig() : data() {};

			/**
			* @brief Constructor for the DSPConfig class
			*
			* @param data DSP data that remains constant once initialised
			* @param models The diffraction and late reverberation models
			*/
			DSPConfig(const DSPData& data) : data(data) {};

			inline SpatialisationMode GetSpatialisationMode() const { return spatialisationMode.load(std::memory_order_acquire); }
			
			inline DiffractionModel GetDiffractionModel() const { return diffractionModel.load(std::memory_order_acquire); }

			inline LateReverbModel GetLateReverbModel() const { return lateReverbModel.load(std::memory_order_acquire); }

			inline int GetNumFDNs() const { return numFDNs.load(std::memory_order_acquire); }

			inline bool GetImpulseResponseMode() const { return impulseResponseMode.load(std::memory_order_acquire); }

			inline bool GetClearBuffers() { return clearBuffers.exchange(false, std::memory_order_acq_rel); }

			inline bool GetEarlyReverbEnabled() const { return earlyReverbEnabled.load(std::memory_order_acquire); }

			inline bool GetLateReverbEnabled() const { return lateReverbEnabled.load(std::memory_order_acquire); }

			inline void UpdateSpatialisationMode(const SpatialisationMode mode) { spatialisationMode.store(mode, std::memory_order_release); }

			inline void UpdateDiffractionModel(const DiffractionModel model) { diffractionModel.store(model, std::memory_order_release); }

			inline void UpdateLateReverbModel(const LateReverbModel model, int numFDNs)
			{
				lateReverbModel.store(model, std::memory_order_release);
				this->numFDNs.store(numFDNs, std::memory_order_release);
			}

			inline void UpdateImpulseResponseMode(const bool mode) { impulseResponseMode.store(mode, std::memory_order_release); }

			inline void FlagClearBuffers() { clearBuffers.store(true, std::memory_order_release); }

			inline void EnableEarlyReverb(const bool enable) { earlyReverbEnabled.store(enable, std::memory_order_release); }

			inline void EnableLateReverb(const bool enable) { lateReverbEnabled.store(enable, std::memory_order_release); }

			inline bool CompareSpatialisationMode(const SpatialisationMode mode) const { return spatialisationMode.load(std::memory_order_acquire) != mode; }

			Real GetLerpFactor() const { return impulseResponseMode.load(std::memory_order_acquire) ? (Real)1.0 : data.GetLerpFactor(); }

			inline std::pair<int, int> GetReverbInputDimensions()
			{
				switch (GetLateReverbModel())
				{
				default:
				case LateReverbModel::fdn:
					return { data.fdnSize, data.numFrames };
				case LateReverbModel::raves:
					return { GetNumFDNs(), 2 * data.numFrames};
				}
			}

			inline const DSPData& GetData() const { return data; }

			const DSPData data;

		private:
			std::atomic<SpatialisationMode> spatialisationMode{ SpatialisationMode::none };		// Spatialisation mode
			std::atomic<DiffractionModel> diffractionModel{ DiffractionModel::nnSmall };										// Diffraction model
			std::atomic<LateReverbModel> lateReverbModel{ LateReverbModel::none };										// Late reverberation model
			std::atomic<bool> impulseResponseMode{ false };										// True if dsp interpolation is disabled, false otherwise
			std::atomic<int> numFDNs{ 1 };														// Number of FDNs

			std::atomic<bool> clearBuffers{ false };			// True if internal buffers should be cleared next audio frame
			std::atomic<bool> earlyReverbEnabled{ false };		// True if early reverberation is enabled, false otherwise
			std::atomic<bool> lateReverbEnabled{ false };		// True if late reverberation is enabled, false otherwise
		};

		/**
		* @brief Struct that stores the user early reverberation configuration data
		*/
		struct EarlyReverbData
		{
		public:
			DirectSound direct{ DirectSound::none };		// Direct sound visibiilty model
			int reflOrder{ 0 };								// Maximum number of reflections in reflection only paths
			int shadowDiffOrder{ 0 };						// Maximum number of reflections or diffractions in shadowed diffraction paths
			int specularDiffOrder{ 0 };						// Maximum number of reflections or diffractions in specular diffraction paths
			Real minEdgeLength{ 0.0 };						// Minimum edge length for diffraction
			Real maxPathLength{ 1e10 };						// Maximum path length for imageSources

			EarlyReverbData(DirectSound direct, int reflOrder, int shadowDiffOrder, int specularDiffOrder, Real minEdgeLength, Real maxPathLength) :
				direct(direct), reflOrder(reflOrder), shadowDiffOrder(shadowDiffOrder), specularDiffOrder(specularDiffOrder),
				minEdgeLength(minEdgeLength), maxPathLength(maxPathLength) {
			}

		private:
			friend class ImageEdge;

			EarlyReverbData(const EarlyReverbData& data, DiffractionModel model) :
				direct(data.direct), reflOrder(data.reflOrder), shadowDiffOrder(data.shadowDiffOrder), specularDiffOrder(data.specularDiffOrder),
				minEdgeLength(data.minEdgeLength), maxPathLength(data.maxPathLength)
			{
				UpdateMaxOrder();
				UpdateSpecularOrder(model);
			}

			inline void Update(const EarlyReverbData& data, DiffractionModel model)
			{
				this->direct = data.direct;
				this->reflOrder = data.reflOrder;
				this->shadowDiffOrder = data.shadowDiffOrder;
				this->specularDiffOrder = data.specularDiffOrder;
				this->minEdgeLength = data.minEdgeLength;
				this->maxPathLength = data.maxPathLength;
				UpdateMaxOrder();
				UpdateSpecularOrder(model);
			}

			inline void UpdateMaxOrder()
			{
				maxOrder = std::max(std::max(reflOrder, shadowDiffOrder), specularDiffOrder);
			}

			inline bool UpdateSpecularOrder(DiffractionModel model)
			{
				specularDiffOrderStore = specularDiffOrder; // Store the specular diffraction order
				if (model == DiffractionModel::btm || model == DiffractionModel::udfa)
				{
					bool hasChanged = specularDiffOrder == 0 && specularDiffOrderStore > 0;
					specularDiffOrder = specularDiffOrderStore;
					return hasChanged;
				}
				else
				{
					bool hasChanged = specularDiffOrder != 0;
					specularDiffOrder = 0;	// Only BTM and UDFA support specular diffraction
					return hasChanged;
				}
			}

			inline bool FeedsFDN(int order) const { return maxOrder == order; }

			int maxOrder{ 0 };					// Maximum order of the image edge model
			int specularDiffOrderStore{ 0 };	// Store the specular diffraction order
		};

		struct LateReverbData
		{
		public:
			bool enabled{ false };
			int numRays{ 1000 };
			FDNMatrix feedbackMatrix{ FDNMatrix::randomOrthogonal };

			LateReverbData(bool enabled, int numRays)
				: LateReverbData(enabled, numRays, FDNMatrix::householder) {}

			LateReverbData(bool enabled, int numRays, FDNMatrix feedbackMatrix)
				: enabled(enabled), numRays(numRays), feedbackMatrix(feedbackMatrix) {}
		};

		struct RoomData
		{
		public:

			RoomData(int numFrequencyBands) : RoomData((Real)34.0, Coefficients<>(numFrequencyBands), ReverbFormula::Sabine, Vec<>())
			{
				Validate(numFrequencyBands);
			}

			RoomData(Real volume, const Coefficients<>& t60, ReverbFormula formula, const Vec<>& dimensions)
				: volume(volume), formula(formula), dimensions(dimensions), customT60(t60)
			{}

			ReverbFormula formula{ ReverbFormula::Sabine };	// Reverberation formula
			Real volume;									// Room volume
			Vec<> dimensions;								// Room dimensions
			Coefficients<> customT60;						// Custom T60

		private:
			friend class Room;

			inline void Validate(int numFrequencyBands)
			{
				if (dimensions.Length() < 1) // No dimensions provided
					dimensions = Vec<>(std::vector<Real>({ (Real)2.5, (Real)4.0, (Real)3.4 })); // Default dimensions
				volume = std::max(volume, (Real)0.001);

				if (customT60.Length() != numFrequencyBands) // Invalid number of frequency bands
					customT60 = Coefficients<>::Constant(numFrequencyBands, (Real)1.0); // Default T60
				for (int i = 0; i < customT60.Length(); i++) // Ensure T60 is positive
					customT60[i] = std::max(customT60[i], (Real)0.0);
			}
		};

		struct MoDARTData : public LateReverbData
		{
		public:
			Matrix<int> indexing;
			Vec<int> frequencyIndexing;
			Vec<> t60s;
			Vec<> energyDecay;
			std::vector<Vec<>> leftEigenvectors;
			std::vector<Vec<>> rightEigenvectors;
			Real delay{ 0.0 };
			Real minimumT60{ 0.1 };

			MoDARTData(bool enabled, int numRays, FDNMatrix feedbackMatrix, Real delay, Real minimumT60, Matrix<int> indexing, Vec<int> frequencyIndexing, Vec<> t60s, std::vector<Vec<>> leftEigenvectors, std::vector<Vec<>> rightEigenvectors)
				: LateReverbData(enabled, numRays, feedbackMatrix), indexing(indexing), delay(delay), minimumT60(minimumT60)
			{
				std::vector<Real> t60sData;
				std::vector<int> frequencyIndexingData;
				for (int i = 0; i < t60s.Length(); i++)
				{
					if (t60s(i) > EPS)
					{
						t60sData.push_back(t60s(i));
						frequencyIndexingData.push_back(frequencyIndexing(i));
						this->leftEigenvectors.push_back(leftEigenvectors[i]);
						this->rightEigenvectors.push_back(rightEigenvectors[i]);
					}
				}
				this->t60s = Vec<>(t60sData);
				this->frequencyIndexing = Vec<int>(frequencyIndexingData);

				this->energyDecay = Vec<>(t60s.Length());
				for (int i = 0; i < this->t60s.Length(); i++)
					this->energyDecay(i) = Pow10(- 6.0 / this->t60s(i));
			}
		};

		struct AudioData
		{
			AudioData(const std::shared_ptr<DSPConfig>& dspConfig)
				: lerpFactor(dspConfig->GetLerpFactor()), lateReverbModel(dspConfig->GetLateReverbModel()),
				spatialisationMode(dspConfig->GetSpatialisationMode()), impulseResponseMode(dspConfig->GetImpulseResponseMode()),
				clearBuffers(dspConfig->GetClearBuffers()), earlyReverbEnabled(dspConfig->GetEarlyReverbEnabled()), lateReverbEnabled(dspConfig->GetLateReverbEnabled())
			{}

			Real lerpFactor;
			LateReverbModel lateReverbModel;
			SpatialisationMode spatialisationMode;
			bool impulseResponseMode;
			bool clearBuffers;

			bool earlyReverbEnabled;
			bool lateReverbEnabled;
		};
	}
}

#endif