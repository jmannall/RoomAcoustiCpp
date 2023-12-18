#pragma once

#include "complex.h"
#include <mutex>
#include "AudioManager.h"
#include "Spatialiser/Diffraction/Path.h"

#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>
#include "myBestNN.h"
#include "myNN_initialize.h"
#include "myNN_terminate.h"
#include "mySmallNN.h"

namespace Spatialiser
{
	namespace Diffraction
	{
		class Attenuate
		{
		public:
			Attenuate(Path* path) : targetGain(0.0f), currentGain(0.0f), mPath(path) { m = new std::mutex(); UpdateParameters(); };
			~Attenuate() {};

			void UpdateParameters();
			void ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor);
		private:
			float targetGain;
			float currentGain;

			Path* mPath;
			std::mutex* m;
		};

		class LPF
		{
		public:
			LPF(Path* path, int fs);
			~LPF() {};

			void UpdateParameters();
			void ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor);
		private:
			float fc;
			float targetGain;
			float currentGain;
			LowPass filter;

			Path* mPath;
			std::mutex* m;
		};

		struct UDFAParameters
		{
			float gain;
			float fc[4];
			float g[4];
			UDFAParameters() : gain(0.0f), fc{ 1000.0f, 1000.0f, 1000.0f, 1000.0f }, g{ 1.0f, 1.0f, 1.0f, 1.0f } {};
			UDFAParameters(float _fc, float _g) : gain(0.0f), fc{ _fc, _fc, _fc, _fc }, g{ _g, _g, _g, _g } {};
		};

		class UDFA
		{
		public:
			UDFA(Path* path, int fs);
			~UDFA() {};

			virtual void UpdateParameters();
			void ProcessAudio(const float* inBuffer, float* outBuffer, int numFrames, float lerpFactor);
			void UpdatePath(Path* path) { mPath = path; }
		protected:
			void CalcF(int fs);
			void CalcFT(int fs);
			void CalcFI();
			void CalcGT();
			float CalcG(float f);
			complexF CalcHpm(float z, float f);
			virtual complexF CalcH(float z, float t, float f);
			float CalcNv(float t);
			complexF CalcUDFA(float f, float fc, float g);
			virtual void UpdateConstants();
			void UpdateFilterParameters();

			int numFilters;
			HighShelf filters[4];
			float ft[5];
			float gt[5];
			float fi[4];
			float gi[4];
			float t0;
			float front;
			float v;

			UDFAParameters params;
			UDFAParameters target;
			UDFAParameters current;

			Path* mPath;
			std::mutex* m;
		};

		class UDFAI : public UDFA
		{
		public:
			UDFAI(Path* path, int fs) : UDFA(path, fs) { UpdateParameters(); };

			void UpdateParameters() override;
		private:
			void UpdateConstants() override;
			complexF CalcH(float z, float t, float f) override;
		};


		class NN	// Only accurate at 48kHz
		{
			using NNParameters = TransDF2Parameters;
		public:
			NN(Path* path);
			~NN() {};

			void UpdateParameters();
			void ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor);

		protected:
			float mInput[8];
			NNParameters params;
			NNParameters target;

		private:
			virtual void RunNN() {};
			void OrderZP();
			void CalcInput();
			void AssignInputRZ(SRData* one, SRData* two);

			NNParameters current;
			TransDF2 filter;

			Path* mPath;
			std::mutex* m;
		};

		class NNBest : public NN
		{
		public:
			NNBest(Path* path) : NN(path) {};
			~NNBest() {};

		private:
			void RunNN() override { myBestNN(mInput, params.z, params.p, &params.k); }
		};

		class NNSmall : public NN
		{
		public:
			NNSmall(Path* path) : NN(path) {};
			~NNSmall() {};

		private:
			void RunNN() override { mySmallNN(mInput, params.z, params.p, &params.k); }
		};

		struct UTDParameters
		{
			float g[4];
			UTDParameters() : g{ 0.0f, 0.0f, 0.0f, 0.0f } {}
		};

		class UTD
		{
		public:
			UTD(Path* path, int fs);
			~UTD() {};

			void UpdateParameters();
			void ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor);

		private:
			void CalcUTD();
			complexF EqHalf(float t, const int i);
			complexF EqQuarter(float t, bool plus, const int i);
			float PM(float t, bool plus);
			float CalcTArg(float t, bool plus);
			float Apm(float t, bool plus);
			complexF FuncF(float x);

			float k[4];
			complexF E[4];
			float n;
			float L;
			LinkwitzRiley lrFilter;

			float g[4];
			float gSB[4];
			UTDParameters params;
			UTDParameters target;
			UTDParameters current;

			Path* mPath;
			std::mutex* m;
		};

		struct IntegralLimits
		{
			float p, m;
			IntegralLimits() : p(0.0f), m(0.0f) {}
			IntegralLimits(float _p, float _m) : p(_p), m(_m) {}
		};

		class BTM
		{
		public:
			BTM(Path* path, int fs);
			~BTM() {};

			void UpdateParameters();
			void ProcessAudio(const float* inBuffer, float* outBuffer, int numFrames, float lerpFactor);
			void UpdatePath(Path* path) { mPath = path; }
		private:
			void CalcBTM();
			float CalcSample(int n);
			IntegralLimits CalcLimits(float delta);
			float CalcIntegral(float zn1, float zn2);
			float CalcIntegrand(float z);
			float CalcB(int i, float coshvtheta);

			float samplesPerMetre;
			Buffer ir;
			Buffer targetIr;
			Buffer currentIr;
			FIRFilter firFilter;
			float dSSq;
			float dRSq;
			float rr;
			float zSRel;
			float zRRel;
			float dz;
			float dzSq;
			float v;
			float rSSq;
			float rRSq;

			float edgeHi;
			float edgeLo;

			float vTheta[4];
			float sinTheta[4];
			float cosTheta[4];

			Path* mPath;
			std::mutex* m;
		};
	}
}