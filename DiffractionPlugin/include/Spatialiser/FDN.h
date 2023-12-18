#pragma once

#include <vector>
#include "Spatialiser/Types.h"
#include "Spatialiser/Wall.h"
#include "AudioManager.h"
#include "matrix.h"
#include "vec.h"
#include "Debug.h"

namespace Spatialiser
{
	class Channel
	{
	public:
		Channel(int fs);
		Channel(float t, const FrequencyDependence& T60, int fs);
		~Channel() {};

		void SetParameters(const FrequencyDependence& T60, const float& t);
		void SetAbsorption(const FrequencyDependence& T60);
		void SetDelay(const float& t) { mT = t; SetDelay(); }
		float GetOutput(const float input);

	private:
		void SetAbsorption(float g[]);
		void SetDelay();

		float mT;
		int sampleRate;
		size_t mDelay;
		Buffer mBuffer;
		ParametricEQ mAbsorptionFilter;
		LowPass mAirAbsorption;

		int idx;
	};

	class FDN
	{
	public:
		FDN(size_t numChannels, int fs);
		FDN(const FrequencyDependence& T60, const vec& dimensions, size_t numChannels, int fs);
		~FDN() {}

		void SetParameters(const FrequencyDependence& T60, const vec& dimensions);
		rowvec GetOutput(const float* data, bool valid);

	private:
		void CalculateTimeDelay(const vec& dimensions, vec& t);
		void InitMatrix();
		void ProcessMatrix() { x = y * mat; }

		size_t mNumChannels;
		std::vector<Channel> mChannels;
		rowvec x;
		rowvec y;
		matrix mat;
	};

}