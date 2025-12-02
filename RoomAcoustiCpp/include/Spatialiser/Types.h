/*
* @brief Declaration of Typedefs and Enums used within RAC
*
*/

#ifndef RoomAcoustiCpp_Types_h
#define RoomAcoustiCpp_Types_h

// C++ headers
#include <unordered_map>
#include <array>
#include <variant>

// Common headers
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

		typedef std::unordered_map<size_t, Plane> PlaneMap;																	// Store planes
		typedef std::unordered_map<size_t, Wall> WallMap;																	// Store walls
		typedef std::unordered_map<size_t, Coefficients<>> MaterialMap;														// Store materials
		typedef std::unordered_map<size_t, Edge> EdgeMap;																	// Store edges
		typedef std::unordered_map<size_t, Source> SourceMap;																// Store sources
		typedef std::unordered_map<std::string, ImageSource> ImageSourceMap;												// Store image sources
		typedef std::unordered_map<std::string, std::shared_ptr<ImageSourceData>> ImageSourceDataMap;						// Store image source data
		typedef std::unordered_map<std::string, std::pair<int, std::shared_ptr<ImageSourceData>>> ImageSourceDataAudioMap;	// Store image source data

		typedef std::vector<std::vector<std::shared_ptr<ImageSourceData>>> ImageSourceDataStore;						// Store image source data

		typedef std::array<Vec3, 3> Vertices;		// Store vertices

		//////////////////// Enum Data Types ////////////////////

		/**
		* @param imageEdge Thread ID processing the image edge model
		* @param rayTracing Thread ID processing the ray tracing model
		*/
		enum class ThreadID
		{
			imageEdge,
			rayTracing
		};

		/**
		* @param Sabine Use Sabine's formula to estimate reverb time
		* @param Eyring Use Eyring's formula to estimate reverb time
		* @param Custom Use custom reverb time provided by user
		*/
		enum class ReverbFormula
		{
			Sabine,
			Eyring,
			Custom
		};

		/**
		* @param householder Efficient to compute, but can produce more colouration
		* @param randomOrthogonal More computationally expensive, but less colouration
		*/
		enum class FDNMatrix
		{
			householder,
			randomOrthogonal
		};

		/**
		* @param attenuate (shadow zone only) Apply distance attenuation only
		* @param lowPass (shadow zone only) Apply a 1kHz low pass filter
		* @param udfa Based after A Universal Filter Approximation of Edge Diffraction for Geometrical Acoustics. Kirsch C and Ewert S 2023
		* @param udfai (shadow zone only) Based after A Universal Filter Approximation of Edge Diffraction for Geometrical Acoustics. Kirsch C and Ewert S 2023
		* @param nnBest (shadow zone only) Based after Efficient diffraction modeling using neural networks and infinite impulse response filters. Mannall J et al. 2023
		* @param nnSmall (shadow zone only) Based after Efficient diffraction modeling using neural networks and infinite impulse response filters. Mannall J et al. 2023
		* @param utd (shadow zone only) Based after Fast Diffraction Pathfinding for Dynamic Sound Propagation. Schissler et al. 2021
		* @param btm Based after An analytic secondary source model of edge diffraction impulse responses. Svensson U et al. 1999
		*/
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

		/**
		* @param omni Omnidirectional directivity
		* @param subcardioid Subcardioid directivity
		* @param cardioid Cardioid directivity
		* @param supercardioid Supercardioid directivity
		* @param hypercardioid Hypercardioid directivity
		* @param bidirectional Figure of 8 directivity
		* @param genelec8020c Genelec 8020c speaker directivity from the BRAS database
		* @param genelec8020cDTF Same as genelec8020c but with the common transfer function (CTF) removed
		* @param qscK8 QSC K8 speaker directivity from the BRAS database
		*/
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

		/**
		* @param quality High quality spatialisation mode using HRTFs
		* @param performance Performance spatialisation mode using ITDs and ILDs
		* @param none No spatialisation (mono)
		*/
		enum class SpatialisationMode { quality, performance, none };

		/**
		* @param none No late reverberation
		* @param fdn Late reverberation using a feedback delay network (FDN)
		* @param raves Late reverberation using the MoD-ART algorithm
		*/
		enum class LateReverbModel { none, fdn, raves };

		/**
		* @param none No direct sound
		* @param doCheck Perform visibility check for direct sound
		* @param alwaysTrue Always consider direct sound present, ignoring visibility
		*/
		enum class DirectSound { none, doCheck, alwaysTrue };

		/**
		* @param none No diffraction sound
		* @param shadowZone Only render diffraction in the shadow zone (more efficient - recommended)
		* @param allZones Render diffraction in all zones (specular, direct and shadow)
		*/
		enum class DiffractionSound { none, shadowZone, allZones };
	}
}
#endif