/*
*
*  \Spatialiser type definitions
*
*/

#ifndef RoomAcoustiCpp_Types_h
#define RoomAcoustiCpp_Types_h

// C++ headers
#include <unordered_map>
#include<array>

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"
#include "Common/Vec3.h"
#include "Common/Vec4.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// #defines ////////////////////

#define DEBUG_INIT
// #define DEBUG_UPDATE
// #define DEBUG_REMOVE
// #define DEBUG_AUDIO_THREAD
// #define DEBUG_IEM_THREAD
// #define DEBUG_HRTF
// #define DEBUG_VIRTUAL_SOURCE
#define DEBUG_IEM

		//////////////////// Data Types ////////////////////

		// Class predeclarations
		class Plane;
		class Wall;
		class Edge;
		class Source;
		class VirtualSource;
		class VirtualSourceData;

		typedef std::unordered_map<size_t, Plane> PlaneMap;
		typedef std::unordered_map<size_t, Wall> WallMap;
		typedef std::unordered_map<size_t, Edge> EdgeMap;
		typedef std::unordered_map<size_t, Source> SourceMap;
		typedef std::unordered_map<std::string, VirtualSource> VirtualSourceMap;
		typedef std::unordered_map<std::string, VirtualSourceData> VirtualSourceDataMap;
		typedef std::vector<std::vector<VirtualSourceData>> VirtualSourceDataStore;

		typedef std::array<Vec3, 3> Vertices;

		typedef std::unordered_map<size_t, std::vector<size_t>> EdgeIDMap;

		enum class ReverbTime
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
			cardioid,
			speaker
		};

		enum class ReturnState
		{
			failed,
			updated,
			remove
		};

		static const int NUM_ABSORPTION_FREQ = 5;
		static const Real ABSORPTION_FREQ[] = { 250.0, 500.0, 1000.0, 2000.0, 4000.0 };

		enum class SpatMode { quality, performance, none };

		enum class ReverbWall
		{
			posZ, negZ,
			posX, negX,
			posY, negY,
			none
		};

		enum class DirectSound
		{
			none, doCheck, alwaysTrue
		};

		enum class DiffractionSound
		{
			none, shadowZone, allZones
		};

		struct IEMConfig
		{
			int order;
			DirectSound direct;
			DiffractionSound diffraction, reflectionDiffraction;
			bool reflection, lateReverb;
			Real edgeLength;

			IEMConfig() : order(0), direct(DirectSound::doCheck), reflection(false), diffraction(DiffractionSound::none), reflectionDiffraction(DiffractionSound::none), lateReverb(false), edgeLength(0.0) {};
			IEMConfig(int _order, DirectSound dir, bool ref, DiffractionSound diff, DiffractionSound refDif, bool rev, Real edgeLen) : order(_order), direct(dir), reflection(ref), diffraction(diff), reflectionDiffraction(refDif), lateReverb(rev), edgeLength(edgeLen) {};
		};

		struct Config
		{
			// DSP parameters
			int fs, numFrames, numFDNChannels;
			// 1 means DSP parameters are lerped over only 1 audio callback
			// 5 means lerped over 5 separate audio callbacks
			// must be greater than 0
			Real lerpFactor, Q;
			Coefficients frequencyBands;
			DiffractionModel diffractionModel;
			SpatMode spatMode;

			Config() : 
				fs(44100), numFrames(512), numFDNChannels(12), lerpFactor(1.0 / ((Real)numFrames * 2.0)), Q(0.98), 
				frequencyBands({250.0, 500.0, 1000.0, 20000.0}), diffractionModel(DiffractionModel::btm), spatMode(SpatMode::none) {};

			Config(int _fs, int _numFrames, int _numFDNChannels, Real _lerpFactor, Real q, Coefficients fBands) : 
				fs(_fs), numFrames(_numFrames), numFDNChannels(_numFDNChannels), lerpFactor(1.0 / ((Real)numFrames * std::max(_lerpFactor, 1.0 / (Real)numFrames))),
				Q(q), frequencyBands(fBands), diffractionModel(DiffractionModel::btm), spatMode(SpatMode::none) {};

			Config(int _fs, int _numFrames, int _numFDNChannels, Real _lerpFactor, Real q, Coefficients fBands, DiffractionModel model, SpatMode mode) : 
				fs(_fs), numFrames(_numFrames), numFDNChannels(_numFDNChannels), lerpFactor(1.0 / ((Real)numFrames * std::max(_lerpFactor, 1.0 / (Real)numFrames))), Q(q),
				frequencyBands(fBands), diffractionModel(model), spatMode(mode) {};
		};
	}
}

#endif