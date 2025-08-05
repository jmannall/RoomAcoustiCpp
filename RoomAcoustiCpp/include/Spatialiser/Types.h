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

		enum class DirectSound { none, doCheck, alwaysTrue };

		enum class DiffractionSound { none, shadowZone, allZones };

		//////////////////// Struct Data Types ////////////////////

		/**
		* @brief Configuration struct for the image edge model
		*/
		class IEMConfig
		{
		public:

			/**
			* @brief Default constructor for the IEMConfig class
			*/
			IEMConfig() : IEMConfig(DirectSound::doCheck, 0, 0, 0, false, 0.0) {};

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
			IEMConfig(DirectSound direct, int reflOrder, int shadowDiffOrder, int specularDiffOrder, bool lateReverb, Real minEdgeLength) : 
				direct(direct), reflOrder(reflOrder), shadowDiffOrder(shadowDiffOrder), specularDiffOrder(specularDiffOrder),
				lateReverb(lateReverb), minEdgeLength(minEdgeLength)
			{
				maxOrder = std::max(std::max(reflOrder, shadowDiffOrder), specularDiffOrder);
			};

			int MaxOrder() { return maxOrder; }

			DirectSound direct;								// Direct sound visibiilty model
			int reflOrder;									// Maximum number of reflections in reflection only paths
			int shadowDiffOrder;							// Maximum number of reflections or diffractions in shadowed diffraction paths
			int specularDiffOrder;							// Maximum number of reflections or diffractions in specular diffraction paths
			bool lateReverb;								// Late reverberation flag
			Real minEdgeLength;								// Minimum edge length for diffraction

		private:
			int maxOrder;									// Maximum order of the IEM
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
			Config() : Config(48000, 512, 12, 2.0, 0.98, Coefficients<>({ 250.0, 500.0, 1000.0, 2000.0 }), DiffractionModel::btm, SpatialisationMode::none) {}

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
			Config(int sampleRate, int numFrames, size_t numReverbSources, Real lerpFactor, Real Q, Coefficients<> frequencyBands) : Config(sampleRate, numFrames, numReverbSources, lerpFactor, Q, frequencyBands, DiffractionModel::btm, SpatialisationMode::none) {}

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
			Config(int sampleRate, int numFrames, size_t numReverbSources, Real lerpFactor, Real Q, Coefficients<> frequencyBands, DiffractionModel model, SpatialisationMode mode) :
				fs(sampleRate), numFrames(numFrames), numReverbSources(CalculateNumReverbSources(numReverbSources)), lerpFactor(CalculateLerpFactor(lerpFactor)),
				Q(Q), frequencyBands(frequencyBands), diffractionModel(model), spatialisationMode(mode)
			{
				CalculateLerpFactor(lerpFactor);
			};

			bool GetImpulseResponseMode() const { return impulseResponseMode.load(std::memory_order_acquire); }

			SpatialisationMode GetSpatialisationMode() const { return spatialisationMode.load(std::memory_order_acquire); }
			
			bool CompareSpatialisationMode(const SpatialisationMode mode) const { return spatialisationMode.load(std::memory_order_acquire) != mode; }

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

			std::atomic<DiffractionModel> diffractionModel;			// Diffraction model
			std::atomic<SpatialisationMode> spatialisationMode;		// Spatialisation mode

			std::atomic<bool> impulseResponseMode;
		};
	}
}

#endif