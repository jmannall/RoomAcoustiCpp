/*
* @brief Declaration of Typedefs, Enums and Struct classes used within RAC
*
*/

#ifndef RoomAcoustiCpp_Types_h
#define RoomAcoustiCpp_Types_h

// C++ headers
#include <unordered_map>
#include <array>

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"
#include "Common/Vec3.h"

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
		typedef std::unordered_map<size_t, Edge> EdgeMap;									// Store edges
		typedef std::unordered_map<size_t, Source> SourceMap;								// Store sources
		typedef std::unordered_map<std::string, ImageSource> ImageSourceMap;				// Store image sources
		typedef std::unordered_map<std::string, std::pair<int, ImageSourceData>> ImageSourceDataMap;		// Store image source datas

		typedef std::vector<std::vector<ImageSourceData>> ImageSourceDataStore;				// Store image source data

		typedef std::array<Vec3, 3> Vertices;		// Store vertices

		//////////////////// Enum Data Types ////////////////////

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

		enum class LateReverbModel { none, fdn };

		enum class DirectSound { none, check, ignoreCheck };

		enum class DiffractionSound { none, shadowZone, allZones };

		//////////////////// Struct Data Types ////////////////////

		struct IEMData
		{
		public:
			DirectSound direct{ DirectSound::none };		// Direct sound visibiilty model
			int reflOrder{ 0 };								// Maximum number of reflections in reflection only paths
			int shadowDiffOrder{ 0 };						// Maximum number of reflections or diffractions in shadowed diffraction paths
			int specularDiffOrder{ 0 };						// Maximum number of reflections or diffractions in specular diffraction paths
			bool lateReverb{ false };						// True when late reverb enabled, flase otherwise
			Real minEdgeLength{ 0.0 };						// Minimum edge length for diffraction

			IEMData() {}

			IEMData(DirectSound direct, int reflOrder, int shadowDiffOrder, int specularDiffOrder, bool lateReverb) :
				direct(direct), reflOrder(reflOrder), shadowDiffOrder(shadowDiffOrder), specularDiffOrder(specularDiffOrder), lateReverb(lateReverb) {
			}

			IEMData(DirectSound direct, int reflOrder, int shadowDiffOrder, int specularDiffOrder, bool lateReverb, Real minEdgeLength) :
				direct(direct), reflOrder(reflOrder), shadowDiffOrder(shadowDiffOrder), specularDiffOrder(specularDiffOrder), lateReverb(lateReverb),
				minEdgeLength(minEdgeLength) {
			}
		};

		/**
		* @brief Configuration struct for the image edge model
		*/
		class IEMConfig
		{
		public:
			/**
			* @brief Constructor for the IEMConfig class
			* 
			* @param diffractionModel The diffraction model to use
			* @param lateReverb The late reverberation model to use
			*/
			IEMConfig(DiffractionModel diffractionModel, LateReverbModel lateReverb) : IEMConfig(IEMData(), diffractionModel, lateReverb) {};

			/**
			* @brief Constructor for the IEMConfig class
			*
			* @param order The maximum reflection/diffraction order of the IEM
			* @param direct The direct sound visibility model
			* @param reflections The reflection flag
			* @param diffraction The diffraction sound validity model
			* @param diffractedReflections The diffraction sound validity model for reflections
			* @param lateReverb The late reverberation flag
			* @param minEdgeLength The minimum edge length for diffraction
			*/
			IEMConfig(IEMData data, DiffractionModel diffractionModel, LateReverbModel lateReverb) :
				data(data), lateReverbModel(lateReverb)
			{
				UpdateMaxOrder();
				UpdateDiffractionModel(diffractionModel);
			};

			/**
			* @brief Update the current diffraction model and check for specular diffraction support
			* 
			* @param model The new diffraction model
			* @return True if the support for specular diffraction has changed, false otherwise
			*/
			inline bool UpdateDiffractionModel(DiffractionModel model)
			{
				if (model == DiffractionModel::btm || model == DiffractionModel::udfa)
				{
					bool hasChanged = data.specularDiffOrder == 0 && specularDiffOrderStore > 0;
					data.specularDiffOrder = specularDiffOrderStore;
					return hasChanged;
				}
				else
				{
					bool hasChanged = data.specularDiffOrder != 0;
					data.specularDiffOrder = 0;	// Only BTM and UDFA support specular diffraction
					return hasChanged;
				}
			}

			/**
			* @brief Update the current late reverberation model
			* 
			* @param model The new late reverberation model
			* @return True if the late reverb model has changed and late reverberation is currently enabled, false otherwise
			*/
			inline bool UpdateLateReverbModel(LateReverbModel model)
			{
				if (lateReverbModel == model)
					return false;
				lateReverbModel = model;
				return data.lateReverb; // If no late reverb, no need to update the model
			}

			/**
			* @brief Calculate the maximum path order
			*/
			inline void UpdateMaxOrder()
			{
				maxOrder = std::max(std::max(data.reflOrder, data.shadowDiffOrder), data.specularDiffOrder);
			}

			/**
			* @brief Update the IEM configuration data
			* 
			* @param data The path configuration data
			* @param diffractionModel The diffraction model to use
			* @param lateReverb The late reverberation model to use
			*/
			inline void Update(IEMData data, DiffractionModel diffractionModel, LateReverbModel lateReverb)
			{
				this->data = data;
				UpdateMaxOrder();
				UpdateDiffractionModel(diffractionModel);
				UpdateLateReverbModel(lateReverb);
			}

			/**
			* @return The maximum order of the IE model
			*/
			inline int MaxOrder() const { return maxOrder; }

			/**
			* @brief Get the current late reverberation model
			* 
			* @param checkData If true, checks if late reverb is enabled in the data and returns LateReverbModel::none if not enabled
			* @return The current late reverberation model
			*/
			inline LateReverbModel GetLateReverbModel(bool checkData = true) const
			{
				if (checkData)
					return data.lateReverb ? lateReverbModel : LateReverbModel::none;
				return lateReverbModel;
			}

			/**
			* @brief Check if the current path order feeds the FDN
			* 
			* @param order The current path order to check
			* @return True if the current path order feeds the FDN late reverberation model, false otherwise
			*/
			inline bool FeedsFDN(int order) const
			{
				return data.lateReverb && lateReverbModel == LateReverbModel::fdn && maxOrder == order;
			}

			IEMData data;		// IE model path configuration data

		private:
			int maxOrder;						// Maximum order of the IEM
			int specularDiffOrderStore;			// Store the specular diffraction order

			LateReverbModel lateReverbModel;		// Late reverb model
		};

		/**
		* @brief Configuration struct for RAC
		*/
		class Config
		{
		public:
			/**
			* @brief Default constructor for the Config class
			*/
			Config() : Config(48000, 512, 12, 2.0, 0.98, Coefficients<>({ 250.0, 500.0, 1000.0, 2000.0, 4000.0 }), SpatialisationMode::none) {}

			/**
			* @brief Constructor for the Config class
			*
			* @param sampleRate The sample rate
			* @param numFrames The number of frames per audio callback
			* @param numReverbSources The number of reverb sources for late reverberation spatialisation
			* @param lerpFactor The linear interpolation factor for audio processing
			* @param Q The Q factor for the GraphicEQ
			* @param frequencyBands The frequency bands for frequency dependent filtering
			*/
			Config(int sampleRate, int numFrames, size_t numReverbSources, Real lerpFactor, Real Q, Coefficients<> frequencyBands) : Config(sampleRate, numFrames, numReverbSources, lerpFactor, Q, frequencyBands, SpatialisationMode::none) {}

			/**
			* @brief Constructor for the Config class
			*
			* @param sampleRate The sample rate
			* @param numFrames The number of frames per audio callback
			* @param numReverbSources The number of reverb sources for late reverberation spatialisation
			* @param lerpFactor The linear interpolation factor for audio processing
			* @param Q The Q factor for the GraphicEQ
			* @param frequencyBands The frequency bands for frequency dependent filtering
			* @param model The diffraction model
			* @param mode The spatialisation mode
			*/
			Config(int sampleRate, int numFrames, size_t numReverbSources, Real lerpFactor, Real Q, Coefficients<> frequencyBands, SpatialisationMode mode) :
				fs(sampleRate), numFrames(numFrames), numReverbSources(CalculateNumReverbSources(numReverbSources)), lerpFactor(CalculateLerpFactor(lerpFactor)),
				Q(Q), frequencyBands(frequencyBands), spatialisationMode(mode) {};

			/**
			* @return True if the impulse response mode is enabled (interpolation disabled), false otherwise
			*/
			bool GetImpulseResponseMode() const { return impulseResponseMode.load(std::memory_order_acquire); }

			/**
			* @return The current spatialisation mode (quality, performance, none)
			*/
			SpatialisationMode GetSpatialisationMode() const { return spatialisationMode.load(std::memory_order_acquire); }
			
			/**
			* @return The current late reverberation model (fdn, none)
			*/
			LateReverbModel GetLateReverbModel() const { return lateReverbModel.load(std::memory_order_acquire); }

			/**
			* @return True if the current spatialisation mode is different from the given mode, false otherwise
			*/
			bool CompareSpatialisationMode(const SpatialisationMode mode) const { return spatialisationMode.load(std::memory_order_acquire) != mode; }

			/**
			* @return The current diffraction model (attenuate, lowPass, utd, udfa, udfai, nnBest, nnSmall, btm)
			*/
			DiffractionModel GetDiffractionModel() const { return diffractionModel.load(std::memory_order_acquire); }

			/**
			* @return lerpFactor if the impulse response mode is disabled, otherwise returns 1
			*/
			Real GetLerpFactor() const { return impulseResponseMode.load(std::memory_order_acquire) ? 1.0 : lerpFactor; }

			const int fs;						// Sample rate
			const int numFrames;				// Number of frames per audio callback
			const size_t numReverbSources;		// Number of reverb sources for late reverberation spatialisation

			const Real Q;								// Q factor for the GraphicEQ
			const Coefficients<> frequencyBands;		// Frequency bands for frequency dependent filtering

		private:
			friend class Context;		// Allow Context to access private members	

			/**
			* @return A supported number of reverb sources based on the maximum number of reverb sources requested
			*/
			static constexpr size_t CalculateNumReverbSources(size_t maxNumReverbSources)
			{
				if (maxNumReverbSources < 1)
					return 0;
				else if (maxNumReverbSources < 2)
					return 1;
				else if (maxNumReverbSources < 4)
					return 2;
				else if (maxNumReverbSources < 6)
					return 4;
				else if (maxNumReverbSources < 8)
					return 6;
				else if (maxNumReverbSources < 12)
					return 8;
				else if (maxNumReverbSources < 16)
					return 12;
				else if (maxNumReverbSources < 20)
					return 16;
				else if (maxNumReverbSources < 24)
					return 20;
				else if (maxNumReverbSources < 32)
					return 24;
				else
					return 32;
			}

			/**
			* @brief Calculates the linear interpolation factor for audio processing
			* 
			* @param factor the input linear interpolation factor
			*/
			constexpr Real CalculateLerpFactor(Real factor)
			{
				factor *= (Real)96.0 / static_cast<Real>(fs);
				return std::max(std::min(factor, (Real)1.0), (Real)1.0 / static_cast<Real>(fs));
			}

			/**
			* @details 1 means DSP parameters are lerped over only 1 audio callback,
			* 5 means lerped over 5 separate audio callbacks. Must be greater than 0
			*/
			const Real lerpFactor;		// Linear interpolation factor for audio processing

			std::atomic<DiffractionModel> diffractionModel{ DiffractionModel::btm };	// Diffraction model
			std::atomic<SpatialisationMode> spatialisationMode;							// Spatialisation mode
			std::atomic<LateReverbModel> lateReverbModel{ LateReverbModel::fdn };		// Late reverberation mode

			std::atomic<bool> impulseResponseMode;		// True if impulse response mode is enabled (disables interpolation), false otherwise
		};
	}
}

#endif