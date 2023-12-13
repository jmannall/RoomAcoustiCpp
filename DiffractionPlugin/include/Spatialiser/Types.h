/*
*
*  \Spatialiser data structures
*
*/

#ifndef Spatialiser_Types_h
#define Spatialiser_Types_h

namespace Spatialiser
{
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
		bool direct, reflection, diffraction, reflectionDiffraction, specularDiffraction;

		ISMConfig() : order(0), direct(true), reflection(false), diffraction(false), reflectionDiffraction(false), specularDiffraction(false) {};
		ISMConfig(int _order, bool dir, bool ref, bool diff, bool refDif, bool spDiff) : order(_order), direct(dir), reflection(ref), diffraction(diff), reflectionDiffraction(refDif), specularDiffraction(spDiff) {};
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
#endif