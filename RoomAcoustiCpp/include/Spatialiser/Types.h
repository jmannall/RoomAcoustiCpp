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
#include "Common/Complex.h"
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

		enum class LateReverbModel { none, fdn, raves };

		enum class DirectSound { none, doCheck, alwaysTrue };

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
			Real maxPathLength{ 1e10 };						// Maximum path length for imageSources

			IEMData() {}

			IEMData(DirectSound direct, int reflOrder, int shadowDiffOrder, int specularDiffOrder, bool lateReverb) :
				direct(direct), reflOrder(reflOrder), shadowDiffOrder(shadowDiffOrder), specularDiffOrder(specularDiffOrder), lateReverb(lateReverb) {}

			IEMData(DirectSound direct, int reflOrder, int shadowDiffOrder, int specularDiffOrder, bool lateReverb, Real minEdgeLength, Real maxPathLength) :
				direct(direct), reflOrder(reflOrder), shadowDiffOrder(shadowDiffOrder), specularDiffOrder(specularDiffOrder), lateReverb(lateReverb),
				minEdgeLength(minEdgeLength), maxPathLength(maxPathLength) {}
		};

		/**
		* @brief Configuration struct for the image edge model
		*/
		class IEMConfig
		{
		public:

			IEMConfig(DiffractionModel diffractionModel, LateReverbModel lateReverb) :
				data(), lateReverbModel(lateReverb)
			{
				UpdateMaxOrder();
				UpdateDiffractionModel(diffractionModel);
			};

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

			inline bool UpdateLateReverbModel(LateReverbModel model)
			{
				if (lateReverbModel == model)
					return false;
				lateReverbModel = model;
				return data.lateReverb; // If no late reverb, no need to update the model
			}

			inline void UpdateMaxOrder()
			{
				maxOrder = std::max(std::max(data.reflOrder, data.shadowDiffOrder), data.specularDiffOrder);
			}

			inline void Update(IEMData data, DiffractionModel diffractionModel, LateReverbModel lateReverb)
			{
				this->data = data;
				UpdateMaxOrder();
				UpdateDiffractionModel(diffractionModel);
				UpdateLateReverbModel(lateReverb);
			}

			inline int MaxOrder() const { return maxOrder; }

			inline LateReverbModel GetLateReverbModel(bool checkData = true) const
			{
				if (checkData)
					return data.lateReverb ? lateReverbModel : LateReverbModel::none;
				return lateReverbModel;
			}

			inline bool FeedsFDN(int order) const
			{
				return data.lateReverb && lateReverbModel == LateReverbModel::fdn && maxOrder == order;
			}

			IEMData data;		// IEM configuration data

		private:
			int maxOrder;						// Maximum order of the IEM
			int specularDiffOrderStore;			// Store the specular diffraction order

			LateReverbModel lateReverbModel;		// Late reverb model
		};

		// class Context;

		/**
		* @brief Configuration struct for RAC
		*/
		class Config
		{
		public:
			/**
			* @brief Default constructor for the Config class
			*/
			Config() : Config(48000, 512, 12, (Real)2.0, (Real)0.98, Coefficients({ (Real)250.0, (Real)500.0, (Real)1000.0, (Real)2000.0 }), SpatialisationMode::none) {}

			/**
			* @brief Constructor for the Config class
			*
			* @param sampleRate The sample rate
			* @param numFrames The number of frames per audio callback
			* @param numReverbSources The number of output channels for late reverberation
			* @param lerpFactor The linear interpolation factor for audio processing
			* @param Q The Q factor for the GraphicEQ
			* @param fBands The frequency bands for the GraphicEQ
			*/
			Config(int sampleRate, int numFrames, int numReverbSources, Real lerpFactor, Real Q, Coefficients<> fBands) : Config(sampleRate, numFrames, numReverbSources, lerpFactor, Q, fBands, SpatialisationMode::none) {}

			/**
			* @brief Constructor for the Config class
			*
			* @param sampleRate The sample rate
			* @param numFrames The number of frames per audio callback
			* @param numReverbSources The number of output channels for late reverberation
			* @param lerpFactor The linear interpolation factor for audio processing
			* @param Q The Q factor for the GraphicEQ
			* @param fBands The frequency bands for the GraphicEQ
			* @param mode The spatialisation mode
			*/
			Config(int sampleRate, int numFrames, int numReverbSources, Real lerpFactor, Real Q, Coefficients<> fBands, SpatialisationMode mode) :
				fs(sampleRate), numFrames(numFrames), numReverbSources(numReverbSources), Q(Q), frequencyBands(fBands),
				spatialisationMode(mode)
			{
				UpdateLerpFactor(lerpFactor);
			};

			bool GetImpulseResponseMode() const { return impulseResponseMode.load(std::memory_order_acquire); }

			bool CompareImpulseResponseMode(const bool mode) const { return impulseResponseMode.load(std::memory_order_acquire) != mode; }

			SpatialisationMode GetSpatialisationMode() const { return spatialisationMode.load(std::memory_order_acquire); }
			
			LateReverbModel GetLateReverbModel() const { return lateReverbModel.load(std::memory_order_acquire); }

			bool CompareSpatialisationMode(const SpatialisationMode mode) const { return spatialisationMode.load(std::memory_order_acquire) != mode; }

			DiffractionModel GetDiffractionModel() const { return diffractionModel.load(std::memory_order_acquire); }

			Real GetLerpFactor() const { return lerpFactor.load(std::memory_order_acquire); }

			const int fs;							// Sample rate
			const int numFrames;					// Number of frames per audio callback
			const int numReverbSources;						// Number of channels for late reverbration
			const int numRavesFDNs{ 8 };			// Number of RAVES FDNs to use for late reverberation

			const Real Q;								// Q factor for the GraphicEQ
			const Coefficients<> frequencyBands;		// Frequency bands for the GraphicEQ

		private:

			friend class Context;		// Allow Context to access private members	

			/**
			* @details 1 means DSP parameters are lerped over only 1 audio callback,
			* 5 means lerped over 5 separate audio callbacks. Must be greater than 0
			*/
			std::atomic<Real> lerpFactor;		// Linear interpolation factor for audio processing

			std::atomic<DiffractionModel> diffractionModel{ DiffractionModel::btm };	// Diffraction model
			std::atomic<SpatialisationMode> spatialisationMode;		// Spatialisation mode
			std::atomic<LateReverbModel> lateReverbModel{ LateReverbModel::none };	// Late reverberation mode

			std::atomic<bool> impulseResponseMode;
			
			Real UpdateLerpFactor(const Real lerpFactor)
			{
				Real factor = (Real)96.0 * lerpFactor / fs;
				factor = std::max(std::min(factor, (Real)1.0), (Real)1.0 / fs);
				this->lerpFactor.store(factor, std::memory_order_release);
				return factor;
			}
		};
	}
}

#endif