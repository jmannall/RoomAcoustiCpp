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
		typedef std::unordered_map<std::string, ImageSourceData> ImageSourceDataMap;		// Store image source datas

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
			genelec8020cDTF
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

		/**
		* @brief Configuration struct for RAC
		*/
		struct Config
		{
			int fs;							// Sample rate
			int numFrames;					// Number of frames per audio callback
			int numLateReverbChannels;		// Number of channels for late reverbration

			/**
			* @details 1 means DSP parameters are lerped over only 1 audio callback,
			* 5 means lerped over 5 separate audio callbacks. Must be greater than 0
			*/
			Real lerpFactor;		// Linear interpolation factor for audio processing

			Real Q;								// Q factor for the GraphicEQ
			Coefficients frequencyBands;		// Frequency bands for the GraphicEQ

			DiffractionModel diffractionModel;			// Diffraction model
			SpatialisationMode spatialisationMode;		// Spatialisation mode

			/**
			* @brief Default constructor for the Config class
			*/
			Config() : Config(48000, 512, 12, 2.0, 0.98, Coefficients({ 250.0, 500.0, 1000.0, 2000.0 }), DiffractionModel::btm, SpatialisationMode::none) {}

			/**
			* @brief Constructor for the Config class
			* 
			* @param sampleRate The sample rate
			* @param numFrames The number of frames per audio callback
			* @param numLateReverbChannels The number of FDN channels for late reverberation
			* @param lerpFactor The linear interpolation factor for audio processing
			* @param Q The Q factor for the GraphicEQ
			* @param fBands The frequency bands for the GraphicEQ
			*/
			Config(int sampleRate, int numFrames, int numLateReverbChannels, Real lerpFactor, Real Q, Coefficients fBands) : Config(sampleRate, numFrames, numLateReverbChannels, lerpFactor, Q, fBands, DiffractionModel::btm, SpatialisationMode::none) {}

			/**
			* @brief Constructor for the Config class
			* 
			* @param sampleRate The sample rate
			* @param numFrames The number of frames per audio callback
			* @param numLateReverbChannels The number of FDN channels for late reverberation
			* @param lerpFactor The linear interpolation factor for audio processing
			* @param Q The Q factor for the GraphicEQ
			* @param fBands The frequency bands for the GraphicEQ
			* @param model The diffraction model
			* @param mode The spatialisation mode
			*/
			Config(int sampleRate, int numFrames, int numLateReverbChannels, Real lerpFactor, Real Q, Coefficients fBands, DiffractionModel model, SpatialisationMode mode) : 
				fs(sampleRate), numFrames(numFrames), numLateReverbChannels(numLateReverbChannels), lerpFactor(96.0 * lerpFactor / static_cast<Real>(sampleRate)),
				Q(Q), frequencyBands(fBands), diffractionModel(model), spatialisationMode(mode)
			{
				this->lerpFactor = std::max(std::min(this->lerpFactor, 1.0), 1.0 / static_cast<Real>(sampleRate));
			};

			Real UpdateLerpFactor(const Real lerpFactor)
			{
				this->lerpFactor = 96.0 * lerpFactor / fs;
				this->lerpFactor = std::max(std::min(this->lerpFactor, 1.0), 1.0 / fs);
				return this->lerpFactor;
			}
		};
	}
}

#endif