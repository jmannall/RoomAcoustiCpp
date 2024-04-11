/*
*
*  \Diffraction models
*
*/

#ifndef RoomAcoustiCpp_Diffraction_Models_h
#define RoomAcoustiCpp_Diffraction_Models_h

// C++ headers
#include <mutex>

// NN headers
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>
#include "myBestNN.h"
#include "myNN_initialize.h"
#include "myNN_terminate.h"
#include "mySmallNN.h"

// Common headers
#include "Common/Types.h"
#include "Common/Complex.h"

// Spatialiser headers
#include "Spatialiser/Diffraction/Path.h"

// DSP headers
#include "DSP/FIRFilter.h"
#include "DSP/IIRFilter.h"
#include "DSP/LinkwitzRileyFilter.h"

namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{
		namespace Diffraction
		{

			//////////////////// Attenuate class ////////////////////

			class Attenuate
			{
			public:
				Attenuate(Path* path) : targetGain(0.0), currentGain(0.0), mPath(path) { m = new std::mutex(); UpdateParameters(); };
				~Attenuate() {};

				void UpdateParameters();
				void ProcessAudio(Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor);
			private:
				Real targetGain;
				Real currentGain;

				Path* mPath;
				std::mutex* m;
			};

			//////////////////// LPF class ////////////////////

			class LPF
			{
			public:
				LPF(Path* path, int fs);
				~LPF() {};

				void UpdateParameters();
				void ProcessAudio(Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor);
			private:
				Real fc;
				Real targetGain;
				Real currentGain;
				LowPass filter;

				Path* mPath;
				std::mutex* m;
			};

			//////////////////// UDFA class ////////////////////

			struct UDFAParameters
			{
				Real gain;
				Real fc[4];
				Real g[4];
				UDFAParameters() : gain(0.0), fc{ 1000.0, 1000.0, 1000.0, 1000.0 }, g{ 1.0, 1.0, 1.0, 1.0 } {};
				UDFAParameters(Real _fc, Real _g) : gain(0.0), fc{ _fc, _fc, _fc, _fc }, g{ _g, _g, _g, _g } {};
			};

			class UDFA
			{
			public:
				UDFA(Path* path, int fs);
				~UDFA() {};

				virtual void UpdateParameters();
				void ProcessAudio(const Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor);
				void UpdatePath(Path* path) { mPath = path; }
			protected:
				void CalcF(int fs);
				void CalcFT(int fs);
				void CalcFI();
				void CalcGT();
				Real CalcG(Real f);
				Complex CalcHpm(Real z, Real f);
				virtual Complex CalcH(Real z, Real t, Real f);
				Real CalcNv(Real t);
				Complex CalcUDFA(Real f, Real fc, Real g);
				virtual void UpdateConstants();
				void UpdateFilterParameters();

				int numFilters;
				std::vector<HighShelf> filters;
				Real ft[5];
				Real gt[5];
				Real fi[4];
				Real gi[4];
				Real t0;
				Real front;
				Real v;

				UDFAParameters params;
				UDFAParameters target;
				UDFAParameters current;

				Path* mPath;
				std::mutex* m;
			};

			//////////////////// UDFA-I class ////////////////////

			class UDFAI : public UDFA
			{
			public:
				UDFAI(Path* path, int fs) : UDFA(path, fs) { UpdateParameters(); };

				void UpdateParameters() override;
			private:
				void UpdateConstants() override;
				Complex CalcH(Real z, Real t, Real f) override;
			};

			//////////////////// NN class ////////////////////

			class NN	// Only accurate at 48kHz
			{
				using NNParameters = ZPKParameters;
			public:
				NN(Path* path);
				~NN() {};

				void UpdateParameters();
				void ProcessAudio(Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor);

			protected:
				float mInput[8];
				NNParameters params;
				NNParameters target;

			private:
				virtual inline void RunNN() {};
				void OrderZP();
				void CalcInput();
				void AssignInputRZ(SRData* one, SRData* two);

				NNParameters current;
				ZPKFilter filter;

				Path* mPath;
				std::mutex* m;
			};

			class NNBest : public NN
			{
			public:
				NNBest(Path* path) : NN(path) {};
				~NNBest() {};

			private:
				inline void RunNN() override
				{
					float z[2], p[2], k;
					myBestNN(mInput, z, p, &k);
					params.z[0] = static_cast<Real>(z[0]);
					params.z[1] = static_cast<Real>(z[1]);
					params.p[0] = static_cast<Real>(z[0]);
					params.p[1] = static_cast<Real>(z[1]);
					params.k = static_cast<Real>(k);
				}
			};

			class NNSmall : public NN
			{
			public:
				NNSmall(Path* path) : NN(path) {};
				~NNSmall() {};

			private:
				inline void RunNN() override
				{
					float z[2], p[2], k;
					mySmallNN(mInput, z, p, &k);
					params.z[0] = static_cast<Real>(z[0]);
					params.z[1] = static_cast<Real>(z[1]);
					params.p[0] = static_cast<Real>(z[0]);
					params.p[1] = static_cast<Real>(z[1]);
					params.k = static_cast<Real>(k);
				}
			};

			//////////////////// UTD class ////////////////////

			class UTD
			{
			public:
				UTD(Path* path, int fs);
				~UTD() {};

				void UpdateParameters();
				void ProcessAudio(Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor);

			private:
				void CalcUTD();
				Complex EqHalf(Real t, const int i);
				Complex EqQuarter(Real t, bool plus, const int i);
				Real PM(Real t, bool plus);
				Real CalcTArg(Real t, bool plus);
				Real Apm(Real t, bool plus);
				Complex FuncF(Real x);

				Real k[4];
				Complex E[4];
				Real n;
				Real L;
				LinkwitzRiley lrFilter;

				Real g[4];
				Real gSB[4];
				Coefficients params;
				Coefficients target;
				Coefficients current;

				Path* mPath;
				std::mutex* m;
			};

			//////////////////// BTM class ////////////////////

			struct IntegralLimits
			{
				Real p, m;
				IntegralLimits() : p(0.0), m(0.0) {}
				IntegralLimits(Real _p, Real _m) : p(_p), m(_m) {}
			};

			class BTM
			{
			public:
				BTM(Path* path, int fs);
				~BTM() {};

				void UpdateParameters();
				void InitParameters();
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor);
				void UpdatePath(Path* path) { mPath = path; }

#ifdef _TEST
#pragma optimize("", off)
				void AddIr(Buffer& buffer)
				{ 
					int irLen = ir.Length();
					int bufferLen = buffer.Length();

					buffer.ResizeBuffer(irLen + bufferLen);
					Real d = mPath->sData.d + mPath->rData.d;
					int j = 0;
					for (int i = bufferLen; i < irLen + bufferLen; i++, j++)
						buffer[i] = ir[j] / d;
				}
#pragma optimize("", on)
#endif

			private:
				void CalcBTM();
				Real CalcSample(int n);
				IntegralLimits CalcLimits(Real delta);
				Real CalcIntegral(Real zn1, Real zn2);
				Real CalcIntegrand(Real z);
				Real CalcB(int i, Real coshvtheta);

				Real samplesPerMetre;
				Buffer ir;
				Buffer targetIr;
				Buffer currentIr;
				FIRFilter firFilter;
				Real dSSq;
				Real dRSq;
				Real rr;
				Real zSRel;
				Real zRRel;
				Real dz;
				Real dzSq;
				Real v;
				Real rSSq;
				Real rRSq;

				Real edgeHi;
				Real edgeLo;

				Real absvTheta;
				Real absvThetaPi;
				Real theta[4];
				Real vTheta;
				Real sinTheta[4];
				Real cosTheta[4];
				bool singular;
				Real temp1_2vec;
				Real temp1vec;
				Real sqrtB1vec;
				Real fifactvec;
				Real sampleOneVec[4];

				Path* mPath;
				Path lastPath;
				std::mutex* m; // Protects currentIr and targetIr
			};
		}
	}
}

#endif