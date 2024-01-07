/*
*
*  \Spatialiser type definitions
*
*/

#include <unordered_map>

#ifndef Spatialiser_Types_h
#define Spatialiser_Types_h

namespace UIE
{
	namespace Spatialiser
	{
		// Class predeclarations
		class Wall;
		class Edge;
		class Source;
		class SourceData;
		class VirtualSource;
		class VirtualSourceData;

		//////////////////// Data Types ////////////////////

		typedef std::unordered_map<size_t, Wall> WallMap;
		typedef std::unordered_map<size_t, Edge> EdgeMap;
		typedef std::unordered_map<size_t, Source> SourceMap;
		typedef std::unordered_map<size_t, SourceData> SourceDataMap;
		typedef std::unordered_map<size_t, VirtualSource> VirtualSourceMap;
		typedef std::unordered_map<std::string, VirtualSourceData> VirtualSourceDataMap;
		typedef std::unordered_multimap<size_t, VirtualSourceData> VirtualSourceDataStore;

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
		static const float ABSORPTION_FREQ[] = { 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f };

		enum class HRTFMode { quality, performance, none };

		enum class ReverbWall
		{
			posZ, negZ,
			posX, negX,
			posY, negY,
			none
		};

		struct ISMConfig
		{
			int order;
			bool direct, reflection, diffraction, reflectionDiffraction, lateReverb, specularDiffraction;

			ISMConfig() : order(0), direct(true), reflection(false), diffraction(false), reflectionDiffraction(false), lateReverb(false), specularDiffraction(false) {};
			ISMConfig(int _order, bool dir, bool ref, bool diff, bool refDif, bool rev, bool spDiff) : order(_order), direct(dir), reflection(ref), diffraction(diff), reflectionDiffraction(refDif), lateReverb(rev), specularDiffraction(spDiff) {};
		};


		struct Config
		{
			// DSP parameters
			int fs, numFrames, numChannels, numFDNChannels, hrtfResamplingStep;
			HRTFMode hrtfMode;

			// 1 means DSP parameters are lerped over only 1 audio callback
			// 5 means lerped over 5 separate audio callbacks
			// must be greater than 0
			float lerpFactor;

			// Binaural resource file paths
			//string resourcePath = "D:\\Joshua Mannall\\GitHub\\3dti_AudioToolkit\\resources";
			//string hrtfPath = "\\HRTF\\3DTI\\3DTI_HRTF_IRC1008_128s_48000Hz.3dti-hrtf";
			//string ildPath = "\\ILD\\NearFieldCompensation_ILD_48000.3dti-ild";

			Config() : fs(44100), numFrames(512), numChannels(2), numFDNChannels(12), lerpFactor(1.0f / ((float)numFrames * 2.0f)), hrtfResamplingStep(0), hrtfMode(HRTFMode::performance) {};
			Config(int _fs, int _numFrames, int _numChannels, int _numFDNChannels, float _lerpFactor, int hrtfStep, HRTFMode mode) : fs(_fs), numFrames(_numFrames), numChannels(_numChannels), numFDNChannels(_numFDNChannels), lerpFactor(1.0f / ((float)numFrames * _lerpFactor)), hrtfResamplingStep(hrtfStep), hrtfMode(mode) {};
		};
	}
}

#endif