/*
* @brief Declaration of Config struct classes used within RAC
*
*/

#ifndef RoomAcoustiCpp_Configs_h
#define RoomAcoustiCpp_Configs_h

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
		//////////////////// Struct Data Types ////////////////////

		/**
		* @brief Struct that stores DSP configuration data
		*
		* @details Used to store data that is used at initialisation and then remains constant
		*/
		struct DSPData
		{
		public:
			// TODO: Should fs be a float?
			int fs{ 48000 };							// Sample rate in Hz
			int numFrames{ 512 };						// Number of frames per audio callback
			int numReverbSources{ 12 };					// Number of output channels for late reverberation
			int fdnSize{ 12 };							// Size of the FDN (number of delay lines)
			Real Q{ REAL_CONST(0.98) };					// Q factor for the GraphicEQ
			Coefficients<> frequencyBands;				// Frequency band center frequencies
			int numFrequencyBands{ 0 };					// Number of frequency bands

			/**
			* @brief Default constructor for the DSPData struct
			*/
			DSPData() : frequencyBands(Coefficients<>(std::vector<Real>({ (Real)250.0, (Real)500.0, (Real)1000.0, (Real)2000.0 })))
			{
			}

			/**
			* @brief Constructor for the DSPData struct
			*
			* @param sampleRate The sample rate in Hz
			* @param numFrames The number of frames per audio callback
			* @param numReverbSources The maximum number of reverb sources
			* @param fdnSize The requested size of each FDN (number of delay lines)
			* @param lerpFactor The interpolation factor for DSP parameter interpolation
			* @param Q The Q factor for the GraphicEQ
			* @param frequencyBands The frequency band center frequencies
			*/
			DSPData(int sampleRate, int numFrames, int numReverbSources, int fdnSize, Real lerpFactor, Real Q, Coefficients<> frequencyBands) :
				fs(sampleRate), numFrames(numFrames), numReverbSources(CalculateNumReverbSources(numReverbSources)), fdnSize(CalculateFDNSize(fdnSize, this->numReverbSources)),
				lerpFactor(CalculateLerpFactor(lerpFactor)), Q(Q), frequencyBands(frequencyBands), numFrequencyBands(ToInt(frequencyBands.Length()))
			{
			}

			/**
			* @brief Returns the lerp factor for DSP parameter interpolation
			*/
			inline Real GetLerpFactor() const { return lerpFactor; }

			/**
			* @brief Updates the lerp factor for DSP parameter interpolation
			*
			* @param lerpFactor The new lerp factor
			*/
			inline void UpdateLerpFactor(Real lerpFactor) { this->lerpFactor = CalculateLerpFactor(lerpFactor); }

		private:
			/**
			* @brief Calculates the lerp factor for DSP parameter interpolation
			*
			* @param lerpFactor The new lerp factor
			* @return The calculated lerp factor constrained between 1/fs and 1.0
			*/
			constexpr Real CalculateLerpFactor(Real lerpFactor)
			{
				Real factor = (Real)96.0 * lerpFactor / fs;
				return std::max(std::min(factor, (Real)1.0), (Real)1.0 / fs);
			}

			/**
			* @brief Calculates the number of reverb sources based on the maximum allowed
			* @details Only certain discrete values are implemented for the number of reverb sources
			*/
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

			/**
			* @brief Calculates the size of the FDN based on the number of reverb sources and fdnSize requested
			* @details The FDN size must be at least MIN_FDNSIZE and at most MAX_FDNSIZE and at least equal to the number of reverb sources
			*
			* @param fdnSize The requested FDN size
			* @param numReverbSources The number of reverb sources
			* @return The calculated FDN size
			*/
			constexpr int CalculateFDNSize(int fdnSize, int numReverbSources)
			{
				fdnSize = std::min(std::max(fdnSize, MIN_FDNSIZE), MAX_FDNSIZE);
				return std::max(fdnSize, numReverbSources);
			}

			Real lerpFactor{ CalculateLerpFactor((Real)2.0) };	// Interpolation factor for DSP parameter interpolation
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
			*/
			DSPConfig(const DSPData& data) : data(data) {};

			/**
			* @return The current spatialisation mode
			*/
			inline SpatialisationMode GetSpatialisationMode() const { return spatialisationMode.load(std::memory_order_acquire); }

			/**
			* @return The current diffraction model
			*/
			inline DiffractionModel GetDiffractionModel() const { return diffractionModel.load(std::memory_order_acquire); }

			/**
			* @return The current late reverberation model
			*/
			inline LateReverbModel GetLateReverbModel() const { return lateReverbModel.load(std::memory_order_acquire); }

			/**
			* @return The current number of FDNs
			*/
			inline int GetNumFDNs() const { return numFDNs.load(std::memory_order_acquire); }

			/**
			* @return True if impulse response mode is enabled, false otherwise
			*/
			inline bool GetImpulseResponseMode() const { return impulseResponseMode.load(std::memory_order_acquire); }

			/**
			* @return True if internal buffers should be cleared this audio callback, false otherwise
			*/
			inline bool GetClearBuffers() { return clearBuffers.exchange(false, std::memory_order_acq_rel); }

			/**
			* @return True if early reverberation is enabled, false otherwise
			*/
			inline bool GetEarlyReverbEnabled() const { return earlyReverbEnabled.load(std::memory_order_acquire); }

			/**
			* @return True if late reverberation is enabled, false otherwise
			*/
			inline bool GetLateReverbEnabled() const { return lateReverbEnabled.load(std::memory_order_acquire); }

			/**
			* @brief Updates the spatialisation mode
			*/
			inline void UpdateSpatialisationMode(const SpatialisationMode mode) { spatialisationMode.store(mode, std::memory_order_release); }

			/**
			* @brief Updates the diffraction model
			*/
			inline void UpdateDiffractionModel(const DiffractionModel model) { diffractionModel.store(model, std::memory_order_release); }

			/**
			* @brief Updates the late reverberation model and number of FDNs
			*/
			inline void UpdateLateReverbModel(const LateReverbModel model, int numFDNs)
			{
				lateReverbModel.store(model, std::memory_order_release);
				this->numFDNs.store(numFDNs, std::memory_order_release);
			}

			/**
			* @brief Updates the impulse response mode
			* @details If true, DSP parameter interpolation is disabled
			*/
			inline void UpdateImpulseResponseMode(const bool mode) { impulseResponseMode.store(mode, std::memory_order_release); }

			/**
			* @brief Flags that internal buffers should be cleared next audio callback
			*/
			inline void FlagClearBuffers() { clearBuffers.store(true, std::memory_order_release); }

			/**
			* @brief Enables or disables early reverberation
			*/
			inline void EnableEarlyReverb(const bool enable) { earlyReverbEnabled.store(enable, std::memory_order_release); }

			/**
			* @brief Enables or disables late reverberation
			*/
			inline void EnableLateReverb(const bool enable) { lateReverbEnabled.store(enable, std::memory_order_release); }

			/**
			* @return The lerp factor for DSP parameter interpolation
			*/
			Real GetLerpFactor() const { return impulseResponseMode.load(std::memory_order_acquire) ? (Real)1.0 : data.GetLerpFactor(); }

			/**
			* @return The input dimensions for the late reverberation input matrix
			* @details The dimensions depend on the late reverberation model
			*/
			inline std::pair<int, int> GetReverbInputDimensions()
			{
				switch (GetLateReverbModel())
				{
				default:
				case LateReverbModel::fdn:
					return { data.fdnSize, data.numFrames };
				case LateReverbModel::raves:
					return { GetNumFDNs(), 2 * data.numFrames };
				}
			}

			/**
			* @return A constant reference to the DSP data
			*/
			inline const DSPData& GetData() const { return data; }

			const DSPData data;		// DSP data that remains constant once initialised

		private:
			std::atomic<SpatialisationMode> spatialisationMode{ SpatialisationMode::none };		// Spatialisation mode
			std::atomic<DiffractionModel> diffractionModel{ DiffractionModel::nnSmall };		// Diffraction model
			std::atomic<LateReverbModel> lateReverbModel{ LateReverbModel::none };				// Late reverberation model
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

			/**
			* @brief Constructor for the EarlyReverbData struct
			*
			* @param direct The direct sound visibility model
			* @param reflOrder The maximum number of reflections in reflection only paths
			* @param shadowDiffOrder The maximum number of reflections or diffractions in shadowed diffraction paths
			* @param specularDiffOrder The maximum number of reflections or diffractions in specular diffraction paths
			* @param minEdgeLength The minimum edge length for diffraction
			* @param maxPathLength The maximum path length for image sources
			*/
			EarlyReverbData(DirectSound direct, int reflOrder, int shadowDiffOrder, int specularDiffOrder, Real minEdgeLength, Real maxPathLength) :
				direct(direct), reflOrder(reflOrder), shadowDiffOrder(shadowDiffOrder), specularDiffOrder(specularDiffOrder),
				minEdgeLength(minEdgeLength), maxPathLength(maxPathLength) {
			}

		private:
			friend class ImageEdge;

			/**
			* @brief Copy constructor for the EarlyReverbData struct
			*
			* @param data The EarlyReverbData to copy
			* @param model The current diffraction model
			*/
			EarlyReverbData(const EarlyReverbData& data, DiffractionModel model) :
				direct(data.direct), reflOrder(data.reflOrder), shadowDiffOrder(data.shadowDiffOrder), specularDiffOrder(data.specularDiffOrder),
				minEdgeLength(data.minEdgeLength), maxPathLength(data.maxPathLength)
			{
				UpdateMaxOrder();
				UpdateSpecularOrder(model);
			}

			/**
			* @param brief Updates the EarlyReverbData with new configuration data
			*
			* @param data The new EarlyReverbData
			* @param model The current diffraction model
			*/
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

			/**
			* @brief Updates the maximum order of the image edge model
			*/
			inline void UpdateMaxOrder()
			{
				maxOrder = std::max(std::max(reflOrder, shadowDiffOrder), specularDiffOrder);
			}

			/**
			* @brief Updates the specular diffraction order based on the current diffraction model
			*
			* @param model The current diffraction model
			* @return True if the specular diffraction order has changed, false otherwise
			*/
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

			/**
			* @brief Checks if the current order is the maximum order and feeds the FDN
			*
			* @param order The order to check
			* @return True if urrent order is the maximum order and feeds the FDN, false otherwise
			*/
			inline bool FeedsFDN(int order) const { return maxOrder == order; }

			int maxOrder{ 0 };					// Maximum order of the image edge model
			int specularDiffOrderStore{ 0 };	// Store the specular diffraction order
		};

		/**
		* @brief Struct that stores the user late reverberation configuration data
		*/
		struct LateReverbData
		{
		public:
			bool enabled{ false };		// True if late reverberation is enabled, false otherwise
			int numRays{ 1000 };		// Number of rays to use for ray tracing

			FDNMatrix feedbackMatrix{ FDNMatrix::randomOrthogonal };	// Feedback matrix type for the FDN

			/**
			* @brief Constructor for the LateReverbData struct
			*
			* @param enabled Enable or disable late reverberation
			* @param numRays Number of rays to use for ray tracing
			*/
			LateReverbData(bool enabled, int numRays)
				: LateReverbData(enabled, numRays, FDNMatrix::householder) {
			}

			/**
			* @brief Constructor for the LateReverbData struct
			*
			* @param enabled Enable or disable late reverberation
			* @param numRays Number of rays to use for ray tracing
			* @param feedbackMatrix Feedback matrix type for the FDN
			*/
			LateReverbData(bool enabled, int numRays, FDNMatrix feedbackMatrix)
				: enabled(enabled), numRays(numRays), feedbackMatrix(feedbackMatrix) {
			}
		};

		/**
		* @brief Struct that stores the user room configuration data
		*/
		struct RoomData
		{
		public:
			/**
			* @brief Constructor for the RoomData struct with default T60 and dimensions
			*
			* @param numFrequencyBands The number of frequency bands
			*/
			RoomData(int numFrequencyBands) : RoomData((Real)34.0, Coefficients<>(numFrequencyBands), ReverbFormula::Sabine, Vec<>())
			{
				Validate(numFrequencyBands);
			}

			/**
			* @brief Constructor for the RoomData struct
			*
			* @param volume The room volume in cubic meters
			* @param t60 The late reverberation time in seconds for each frequency band
			* @param reverbFormula The reverberation formula to use
			* @param dimensions The primary room dimensions in meters
			*/
			RoomData(Real volume, const Coefficients<>& t60, ReverbFormula formula, const Vec<>& dimensions)
				: volume(volume), formula(formula), dimensions(dimensions), customT60(t60)
			{
				Validate(ToInt(t60.Length()));
			}

			ReverbFormula formula{ ReverbFormula::Sabine };	// Reverberation formula
			Real volume;									// Room volume
			Vec<> dimensions;								// Room dimensions
			Coefficients<> customT60;						// Custom T60

		private:
			friend class Room;

			/**
			* @brief Validates and corrects the room data
			* @details Dimensions are set to default if not provided, volume is clamped to a minimum value, and custom T60 must be positive and match the number of frequency bands
			*
			* @param numFrequencyBands The number of frequency bands
			*/
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

		/**
		* @brief Struct that stores the MoD-ART late reverberation configuration data
		*/
		struct MoDARTData : public LateReverbData
		{
		public:
			Matrix<int> indexing;					// Path indexing matrix
			Vec<int> frequencyIndexing;				// Frequency band indexing for each FDN
			Vec<> t60s;								// Late reverberation time for each FDN
			Vec<> energyDecay;						// Energy decay per sample for each FDN
			std::vector<Vec<>> leftEigenvectors;	// Left eigenvectors for each FDN
			std::vector<Vec<>> rightEigenvectors;	// Right eigenvectors for each FDN
			Real delay{ REAL_CONST(0.0) };			// Delay before late reverberation starts in seconds
			Real minimumT60{ REAL_CONST(0.1) };		// Minimum T60 to process in seconds

			/**
			* @brief Constructor for the MoDARTData struct
			*
			* @param enabled Enable or disable late reverberation
			* @param numRays Number of rays to use for ray tracing
			* @param feedbackMatrix Feedback matrix type for the FDN
			* @param delay Delay before late reverberation starts in seconds
			* @param minimumT60 Minimum T60 to process in seconds
			* @param indexing Path indexing matrix
			* @param frequencyIndexing Frequency band indexing for each FDN
			* @param t60s Late reverberation time for each FDN
			* @param leftEigenvectors Left eigenvectors for each FDN
			* @param rightEigenvectors Right eigenvectors for each FDN
			*/
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
					this->energyDecay(i) = Pow10(REAL_CONST(-6.0) / this->t60s(i));
			}
		};

		/**
		* @brief Struct that passes audio configuration data each audio callback
		*/
		struct AudioData
		{
			/**
			* @brief Default constructor for the AudioData struct
			*/
			AudioData() {}

			/**
			* @brief Constructor for the AudioData struct
			*
			* @param dspConfig The current DSP configuration
			*/
			AudioData(const std::shared_ptr<DSPConfig>& dspConfig)
				: lerpFactor(dspConfig->GetLerpFactor()), lateReverbModel(dspConfig->GetLateReverbModel()),
				spatialisationMode(dspConfig->GetSpatialisationMode()), impulseResponseMode(dspConfig->GetImpulseResponseMode()),
				clearBuffers(dspConfig->GetClearBuffers()), earlyReverbEnabled(dspConfig->GetEarlyReverbEnabled()), lateReverbEnabled(dspConfig->GetLateReverbEnabled())
			{
				RAC_DEBUG_ASSERT(0.0 < lerpFactor && lerpFactor <= 1.0, "Interpolation factor must be between 0 and 1: " + ToString(lerpFactor));
			}

			Real lerpFactor;						// Lerp factor for DSP parameter interpolation
			LateReverbModel lateReverbModel;		// Late reverberation model
			SpatialisationMode spatialisationMode;	// Spatialisation mode
			bool impulseResponseMode;				// True if impulse response mode is enabled, false otherwise
			bool clearBuffers;						// True if internal buffers should be cleared this audio callback, false otherwise

			bool earlyReverbEnabled;	// True if early reverberation is enabled, false otherwise
			bool lateReverbEnabled;		// True if late reverberation is enabled, false otherwise
		};
	}
}

#endif