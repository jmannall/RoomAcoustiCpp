/*
*
*  \Spatialiser type definitions
*
*/

#ifndef RoomAcoustiCpp_Types_h
#define RoomAcoustiCpp_Types_h

// C++ headers
#include <unordered_map>

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"

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
// #define DEBUG_ISM_THREAD
// #define DEBUG_HRTF
// #define DEBUG_VIRTUAL_SOURCE
#define DEBUG_ISM

		//////////////////// Data Types ////////////////////

		// Class predeclarations
		class Plane;
		class Wall;
		class Edge;
		class Source;
		class SourceData;
		class VirtualSource;
		class VirtualSourceData;

		typedef std::unordered_map<size_t, Plane> PlaneMap;
		typedef std::unordered_map<size_t, Wall> WallMap;
		typedef std::unordered_map<size_t, Edge> EdgeMap;
		typedef std::unordered_map<size_t, Source> SourceMap;
		typedef std::unordered_map<size_t, SourceData> SourceDataMap;
		typedef std::unordered_map<size_t, VirtualSource> VirtualSourceMap;
		typedef std::unordered_map<std::string, VirtualSourceData> VirtualSourceDataMap;
		typedef std::vector<std::vector<VirtualSourceData>> VirtualSourceDataStore;

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

		enum class Model
		{
			attenuate,
			off,
			lowPass,
			udfa,
			udfai,
			nnBest,
			nnSmall,
			utd,
			btm
		};

		static const int NUM_ABSORPTION_FREQ = 5;
		static const Real ABSORPTION_FREQ[] = { 250.0, 500.0, 1000.0, 2000.0, 4000.0 };

		enum class HRTFMode { quality, performance, none };

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

		struct ISMConfig
		{
			int order;
			DirectSound direct;
			DiffractionSound diffraction, reflectionDiffraction;
			bool reflection, lateReverb;

			ISMConfig() : order(0), direct(DirectSound::doCheck), reflection(false), diffraction(DiffractionSound::none), reflectionDiffraction(DiffractionSound::none), lateReverb(false) {};
			ISMConfig(int _order, DirectSound dir, bool ref, DiffractionSound diff, DiffractionSound refDif, bool rev) : order(_order), direct(dir), reflection(ref), diffraction(diff), reflectionDiffraction(refDif), lateReverb(rev) {};
		};

		class SPATConfig
		{
		public:
			SPATConfig() : quality(-2), performance(-2) {};
			SPATConfig(int qualityOrder, int performanceOrder) : quality(qualityOrder), performance(performanceOrder) {};
		
			inline HRTFMode GetMode(const int& order) const
			{
				if (quality == -1 || order <= quality)
					return HRTFMode::quality;
				if (performance == -1 || order <= performance)
					return HRTFMode::performance;
				return HRTFMode::none;
			}
		private:
			int quality, performance;
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
			SPATConfig spatConfig;

			Config() : fs(44100), numFrames(512), numFDNChannels(12), lerpFactor(1.0 / ((Real)numFrames * 2.0)), Q(0.77), frequencyBands({250.0, 500.0, 1000.0, 20000.0}), spatConfig() {};
			Config(int _fs, int _numFrames, int _numFDNChannels, Real _lerpFactor, Real q, Coefficients fBands) : fs(_fs), numFrames(_numFrames), numFDNChannels(_numFDNChannels), lerpFactor(1.0 / ((Real)numFrames * _lerpFactor)), Q(q), frequencyBands(fBands), spatConfig() {};
			Config(int _fs, int _numFrames, int _numFDNChannels, Real _lerpFactor, Real q, Coefficients fBands, int quality, int performance) : fs(_fs), numFrames(_numFrames), numFDNChannels(_numFDNChannels), lerpFactor(1.0 / ((Real)numFrames * _lerpFactor)), Q(q), frequencyBands(fBands), spatConfig(quality, performance) {};
		};
	}
}

#endif