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

		//////////////////// #defines ////////////////////

		/**
		* @brief Debugging flags for printing debug messages
		*/
#define DEBUG_INIT
// #define DEBUG_UPDATE
// #define DEBUG_REMOVE
// #define DEBUG_AUDIO_THREAD
// #define DEBUG_IEM_THREAD
// #define DEBUG_HRTF
// #define DEBUG_VIRTUAL_SOURCE

		//////////////////// Typedefs ////////////////////

		/**
		* Class predeclarations
		*/
		class Plane;
		class Wall;
		class Edge;
		class Source;
		class VirtualSource;
		class VirtualSourceData;

		typedef std::unordered_map<size_t, Plane> PlaneMap;										// Store planes
		typedef std::unordered_map<size_t, Wall> WallMap;										// Store walls
		typedef std::unordered_map<size_t, Edge> EdgeMap;										// Store edges
		typedef std::unordered_map<size_t, Source> SourceMap;									// Store sources
		typedef std::unordered_map<std::string, VirtualSource> VirtualSourceMap;				// Store virtual sources
		typedef std::unordered_map<std::string, VirtualSourceData> VirtualSourceDataMap;		// Store virtual source datas

		typedef std::vector<std::vector<VirtualSourceData>> VirtualSourceDataStore;				// Store virtual source data

		typedef std::array<Vec3, 3> Vertices;		// Store vertices

		//////////////////// Enum Data Types ////////////////////

		enum class ReverbFormula
		{
			Sabine,
			Eyring
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
			cardioid
		};

		enum class SpatialisationMode { quality, performance, none };

		enum class DirectSound { none, doCheck, alwaysTrue };

		enum class DiffractionSound { none, shadowZone, allZones };

		//////////////////// Struct Data Types ////////////////////

		/**
		* @brief Configuration struct for the image edge model
		*/
		struct IEMConfig
		{
			int order;										// Maximum reflection/diffraction order of the IEM
			DirectSound direct;								// Direct sound visibiilty model
			DiffractionSound diffraction;					// Diffraction sound validity model
			DiffractionSound diffractedReflections;			// Diffraction sound validity model for reflections
			bool reflections;								// Reflection flag
			bool lateReverb;								// Late reverberation flag
			Real minEdgeLength;								// Minimum edge length for diffraction

			/**
			* @brief Default constructor for the IEMConfig class
			*/
			IEMConfig() : IEMConfig(0, DirectSound::doCheck, false, DiffractionSound::none, DiffractionSound::none, false, 0.0) {};

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
			IEMConfig(int order, DirectSound direct, bool reflections, DiffractionSound diffraction, DiffractionSound diffractedReflections, bool lateReverb, Real minEdgeLength) : 
				order(order), direct(direct), reflections(reflections), diffraction(diffraction), diffractedReflections(diffractedReflections),
				lateReverb(lateReverb), minEdgeLength(minEdgeLength) {};
		};

		/**
		* @brief Configuration struct for RAC
		*/
		struct Config
		{
			int fs;					// Sample rate
			int numFrames;			// Number of frames per audio callback
			int numFDNChannels;		// Number of FDN channels for late reverbration

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
			Config() : Config(44100, 512, 12, 2.0, 0.98, Coefficients({ 250.0, 500.0, 1000.0, 20000.0 }), DiffractionModel::btm, SpatialisationMode::none) {}

			/**
			* @brief Constructor for the Config class
			* 
			* @param sampleRate The sample rate
			* @param numFrames The number of frames per audio callback
			* @param numFDNChannels The number of FDN channels for late reverberation
			* @param lerpFactor The linear interpolation factor for audio processing
			* @param Q The Q factor for the GraphicEQ
			* @param fBands The frequency bands for the GraphicEQ
			*/
			Config(int sampleRate, int numFrames, int numFDNChannels, Real lerpFactor, Real Q, Coefficients fBands) : Config(sampleRate, numFrames, numFDNChannels, lerpFactor, Q, fBands, DiffractionModel::btm, SpatialisationMode::none) {}

			/**
			* @brief Constructor for the Config class
			* 
			* @param sampleRate The sample rate
			* @param numFrames The number of frames per audio callback
			* @param numFDNChannels The number of FDN channels for late reverberation
			* @param lerpFactor The linear interpolation factor for audio processing
			* @param Q The Q factor for the GraphicEQ
			* @param fBands The frequency bands for the GraphicEQ
			* @param model The diffraction model
			* @param mode The spatialisation mode
			*/
			Config(int sampleRate, int numFrames, int numFDNChannels, Real lerpFactor, Real Q, Coefficients fBands, DiffractionModel model, SpatialisationMode mode) : 
				fs(sampleRate), numFrames(numFrames), numFDNChannels(numFDNChannels), lerpFactor(1.0 / (static_cast<Real>(numFrames) * std::max(lerpFactor, 1.0 / static_cast<Real>(numFrames)))),
				Q(Q), frequencyBands(fBands), diffractionModel(model), spatialisationMode(mode) {};
		};
	}
}

#endif