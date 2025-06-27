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
#include "DSP/Parameter.h"

namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{
		namespace Diffraction
		{
			class Model
			{
			public:
				Model() {};
				virtual ~Model() {};

				virtual void SetTargetParameters(const Path& path) = 0;
				virtual void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) = 0;

			protected:
				std::atomic<bool> isInitialised{ false };	// True if the model has been initialised, false otherwise
			};

			//////////////////// Attenuate class ////////////////////

			class Attenuate : public Model
			{
			public:
				Attenuate(const Path& path) : Model(), gain(CalculateGain(path))
				{
					isInitialised.store(true);
				};

				~Attenuate() {};

				inline void SetTargetParameters(const Path& path) override { gain.SetTarget(CalculateGain(path)); }

				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;
			private:

				inline Real CalculateGain(const Path& path) const { return (path.valid && path.inShadow) ? 1.0 : 0.0; }
			
				Parameter gain;
			};

			//////////////////// LPF class ////////////////////

			class LPF : public Model
			{
			public:
				LPF(const Path& path, const int fs) : Model(), gain(CalculateGain(path)), filter(1000.0, fs)
				{
					isInitialised.store(true);
				}

				~LPF() {};

				inline void SetTargetParameters(const Path& path) override { gain.SetTarget(CalculateGain(path)); }

				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;
			private:

				inline Real CalculateGain(const Path& path) const { return (path.valid && path.inShadow) ? 1.0 : 0.0; }

				LowPass1 filter;
				Parameter gain;
			};

			//////////////////// UDFA class ////////////////////

			enum UDFAModel
			{
				Pierce,
				SingleTerm
			};

			template <UDFAModel model>
			class UDFABase : public Model
			{
				static constexpr int numShelvingFilters{ 4 };
				static constexpr int numUDFAFilters = model == UDFAModel::SingleTerm ? 2 : 4;
				
				using ParametersI = Coefficients<std::array<Real, numShelvingFilters>>;
				using ParametersT = Coefficients<std::array<Real, numShelvingFilters + 1>>;

				struct FilterParameters
				{
					Coefficients<std::array<Real, numShelvingFilters>> fc;
					Coefficients<std::array<Real, numShelvingFilters>> g;
					Real gain;
					FilterParameters() : gain(0.0), fc({ 45.0, 350.0, 2800.0, 21700.0 }), g({ 1.0, 1.0, 1.0, 1.0 }) {}
				};

				struct Parameters
				{
					Real fc;
					Real gain;
					Real blend{ 1.44 };
					Real Q{ 0.2 };

					Parameters(Real f, Real g, Real tDiff) : fc(f), gain(g) {
						Real halfGain = CalculateHalfGain(fc, tDiff);
						Real halfGainSq = halfGain * halfGain;
						fc *= (1.0 / halfGainSq);
						gain *= halfGain;
						blend = 1.0 + (blend - 1.0) * halfGainSq;
						Q = 0.5 + (Q - 0.5) * halfGainSq;
					}

					inline Real CalculateHalfGain(Real fc, Real tDiff) const {
						return (2 / PI_1) * atan(PI_1 * sqrt(2.0 * fc * tDiff));
					}
				};

				struct Constants {
					Real tDiffBase{ 0.0 }, tDiffTop{ 0.0 };
					Real fc1{ 0.0 }, fc2{ 0.0 };
					Real gain1{ 0.0 }, gain2{ 0.0 };

					Constants(const Path& path)
					{
						Real v = PI_1 / path.wData.t;
						Real cosVpi = cos(v * PI_1);
						Real sinVpi = sin(v * PI_1);

						Real cosVt1 = cos(v * (path.rData.t - path.sData.t));
						Real cosVt2 = cos(v * (path.rData.t + path.sData.t));

						Real dStar = 2.0 * path.sData.r * path.rData.r / (path.sData.d + path.rData.d);
						Real front = 2.0 * SPEED_OF_SOUND / (PI_SQ * dStar);

						Real nV1 = CalcNv(cosVt1, v, cosVpi);
						Real nV2 = CalcNv(cosVt2, v, cosVpi);

						fc1 = front * nV1 * nV1;
						fc2 = front * nV2 * nV2;

						gain1 = CalcGv(cosVt1, cosVpi, sinVpi);
						gain2 = CalcGv(cosVt2, cosVpi, sinVpi);

						if constexpr (model == UDFAModel::SingleTerm)
						{
							Real fcTerm = gain1 * sqrt(fc1) + gain2 * sqrt(fc2);
							fc1 = fcTerm * fcTerm;
							gain1 = 1.0;
						}
						else
						{
							if (!path.inShadow)
								gain1 = -gain1;
							if (path.inRelfZone)
								gain2 = -gain2;
						}

						Real d = path.sData.d + path.rData.d;
						tDiffBase = (path.GetD(0.0) - d) / SPEED_OF_SOUND;
						tDiffTop = (path.GetD(path.wData.z) - d) / SPEED_OF_SOUND;
					}

					inline Real CalcNv(Real cosVt, Real v, Real cosVpi) const {
						return (v * sqrt(1 - cosVpi * cosVt)) / (cosVpi - cosVt);
					}

					inline Real CalcGv(Real cosVt, Real cosVpi, Real sinVpi) const {
						return 0.5 * sinVpi / sqrt(1 - cosVpi * cosVt);
					}
				};

			public:
				UDFABase(const Path& path, const int fs) : ft(CalcFT(fs)), fi(CalcFI())
				{
					if (!path.valid || (model == UDFAModel::SingleTerm && !path.inShadow))
					{
						for (int j= 0; j < numUDFAFilters; j++)
							PopulateFilters(FilterParameters(), j, fs);
					}
					else
					{
						std::array<Parameters, numUDFAFilters> parameters = CalculateUDFAParameters(path);
						for (int j = 0; j < numUDFAFilters; j++)
						{
							FilterParameters fp = CalculateParameters(parameters[j]);
							PopulateFilters(fp, j, fs);
						}
					}
					isInitialised.store(true);
				}

				~UDFABase() {}

				void SetTargetParameters(const Path& path) override
				{
					if (!path.valid || (model == UDFAModel::SingleTerm && !path.inShadow))
					{
						for (int i = 0; i < numUDFAFilters; i++)
							gain[i]->SetTarget(0.0);
						return;
					}

					std::array<Parameters, numUDFAFilters> parameters = CalculateUDFAParameters(path);

					for (int j = 0; j < numUDFAFilters; j++)
					{
						FilterParameters filterParameters = CalculateParameters(parameters[j]);
						gain[j]->SetTarget(filterParameters.gain);
						for (int i = 0; i < numShelvingFilters; i++)
							filters[j * numShelvingFilters + i]->SetTargetParameters(filterParameters.fc[i], filterParameters.g[i]);
					}
				}

				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override
				{
					if (!isInitialised.load())
						return;

					const int totalFilters = static_cast<int>(filters.size());

					FlushDenormals();
					for (int i = 0; i < inBuffer.Length(); i++)
					{
						Real outSample = 0.0;
						for (int j = 0; j < numUDFAFilters; j++)
						{
							Real out = filters[j * numShelvingFilters]->GetOutput(inBuffer[i], lerpFactor);
							for (int k = 1; k < numShelvingFilters; k++)
								out = filters[j * numShelvingFilters + k]->GetOutput(out, lerpFactor);
							outSample += out * gain[j]->Use(lerpFactor);
						}
						outBuffer[i] = outSample;
					}
					NoFlushDenormals();
				}

			private:

				inline void PopulateFilters(const FilterParameters& fp, const int j, const int fs)
				{
					gain[j].emplace(fp.gain);
					for (int i = 0; i < numShelvingFilters; i++)
						filters[j * numShelvingFilters + i].emplace(fp.fc[i], fp.g[i], fs);
				}

				std::array<Parameters, numUDFAFilters> CalculateUDFAParameters(const Path& path) const
				{
					Constants constants(path);
					if constexpr (model == UDFAModel::Pierce)
					{
						return std::array<Parameters, numUDFAFilters>({
							Parameters(constants.fc1, constants.gain1, constants.tDiffBase),
							Parameters(constants.fc1, constants.gain1, constants.tDiffTop),
							Parameters(constants.fc2, constants.gain2, constants.tDiffBase),
							Parameters(constants.fc2, constants.gain2, constants.tDiffTop)
							});
					}
					else if (model == UDFAModel::SingleTerm)
					{
						return std::array<Parameters, numUDFAFilters>({
							Parameters(constants.fc1, constants.gain1, constants.tDiffBase),
							Parameters(constants.fc1, constants.gain1, constants.tDiffTop)
							});
					}
					else
						assert(false && "Invalid UDFAModel specified.");
				}

				FilterParameters CalculateParameters(const Parameters& parameters) const
				{
					ParametersT gt = CalcGT(parameters);
					ParametersI gi = CalcGI(gt, parameters);

					FilterParameters filterParameters;
					for (int i = 0; i < numShelvingFilters; i++)
						filterParameters.g[i] = gt[i + 1] / gt[i];

					const Coefficients giSq = gi * gi;
					const Coefficients gSq = filterParameters.g * filterParameters.g;
					filterParameters.fc = fi * ((giSq - gSq) / (filterParameters.g * (1.0 - giSq))).Sqrt() * (1.0 + gSq / 12.0);
					filterParameters.gain = gt[0] * parameters.gain / 2;
					return filterParameters;
				}

				inline ParametersT CalcFT(int fs) const
				{
					ParametersT f(numShelvingFilters + 1);
					Real fMin = log10(10.0);
					Real fMax = log10(static_cast<Real>(fs));

					Real delta = (fMax - fMin) / numShelvingFilters;

					for (int i = 0; i <= numShelvingFilters; i++)
						f[i] = pow(10.0, fMin + delta * i);
					return f;
				}

				inline ParametersI CalcFI() const
				{
					ParametersI f(numShelvingFilters);
					for (int i = 0; i < numShelvingFilters; i++)
						f[i] = ft[i] * sqrt(ft[i + 1] / ft[i]);
					return f;
				}

				inline ParametersT CalcGT(const Parameters& parameters) const
				{
					ParametersT gt = ParametersT(numShelvingFilters + 1);
					for (int i = 0; i < numShelvingFilters + 1; i++)
						gt[i] = CalcG(ft[i], parameters);
					return gt;
				}

				inline ParametersI CalcGI(const ParametersT& gt, const Parameters& parameters) const
				{
					ParametersI gi = ParametersI(numShelvingFilters);
					for (int i = 0; i < numShelvingFilters; i++)
						gi[i] = CalcG(fi[i], parameters) / gt[i];
					return gi;
				}

				inline Real CalcG(Real f, const Parameters& parameters) const { return abs(CalcUDFA(f, parameters)); }

				inline Complex CalcUDFA(Real f, const Parameters& parameters) const
				{
					Real alpha = 0.5;
					Real r = 1.6;
					return pow(pow(imUnit * f / parameters.fc, 2.0 / parameters.blend) + pow(imUnit * f / (parameters.Q * parameters.fc), 1.0 / (pow(parameters.blend, r))) + Complex(1.0, 0.0), -alpha * parameters.blend / 2.0);
				}

			private:
				const ParametersT ft;
				const ParametersI fi;

				std::array<std::optional<Parameter>, numUDFAFilters> gain;
				std::array<std::optional<HighShelfMatched>, numShelvingFilters * numUDFAFilters> filters;
			};


			class UDFA : public UDFABase<UDFAModel::Pierce>
			{
			public:
				UDFA(const Path& path, int fs) : UDFABase(path, fs) {}
			};

			class UDFAI : public UDFABase<UDFAModel::SingleTerm>
			{
			public:
				UDFAI(const Path& path, int fs) : UDFABase(path, fs) {}
			};

			//////////////////// NN class ////////////////////

			class NN : public Model	// Only accurate at 48kHz
			{	
				static constexpr int numInputs = 8;	// Number of inputs to the neural network
				using Input = std::array<float, numInputs>;

				struct Parameters
				{
				public:
					Coefficients<std::array<Real, 5>> data{ 0.0 };

					Parameters(float z[2], float p[2], float k)
					{
						if (z[0] < z[1])
						{
							data[0] = static_cast<Real>(z[0]);
							data[1] = static_cast<Real>(z[1]);
						}
						else
						{
							data[0] = static_cast<Real>(z[1]);
							data[1] = static_cast<Real>(z[0]);
						}

						if (p[0] < p[1])
						{
							data[2] = static_cast<Real>(p[0]);
							data[3] = static_cast<Real>(p[1]);
						}
						else
						{
							data[2] = static_cast<Real>(p[1]);
							data[3] = static_cast<Real>(p[0]);
						}
						data[4] = k;
					}
				};

			public:
				NN(const Path& path, void(*nnFunction)(const float*, float*, float*, float*)) : nnFunction(nnFunction), Model(), filter(CalculateParameters(path).data, 48000)
				{
					if (!path.valid)
						filter.SetTargetGain(0.0);
					isInitialised.store(true);
				};

				virtual ~NN() {};

				void SetTargetParameters(const Path& path) override;
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;

			private:
				inline Parameters RunNN(const Input& input) const
				{
					float z[2], p[2], k;
					nnFunction(input.data(), z, p, &k);
					return Parameters(z, p, k);
				}
				Parameters CalculateParameters(const Path& path) const;
				Input CalcInput(const Path& path) const;
				void AssignInputRZ(const SRData& one, const SRData& two, Real zW, Input& input) const;

				std::function<void(const float*, float*, float*, float*)> nnFunction;
				ZPKFilter filter;
			};

			class NNBest : public NN
			{
			public:
				NNBest(const Path& path) : NN(path, &myBestNN) {};

				~NNBest() {};
			};

			class NNSmall : public NN
			{
			public:
				NNSmall(const Path& path) : NN(path, &mySmallNN) {};

				~NNSmall() {};
			};

			//////////////////// UTD class ////////////////////

			class UTD : public Model
			{
				typedef Coefficients<std::array<Real, 4>> Parameters;

				constexpr std::array<Complex, 4> InitE()
				{
					std::array<Complex, 4> E;
					for (int i = 0; i < 4; ++i)
						E[i] = std::exp(-imUnit * (PI_1 / 4.0)) / (2.0 * std::sqrt(PI_2 * k[i]));
					return E;
				}

			public:
				UTD(const Path& path, int fs);
				~UTD() {};

				void SetTargetParameters(const Path& path) override;
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;

			private:
				Parameters CalcUTD(const Path& path);
				inline Complex EqHalf(Real t, const int i, Real n, Real L) { return EqQuarter(t, true, i, n, L) + EqQuarter(t, false, i, n, L); }
				Complex EqQuarter(Real t, bool plus, const int i, Real n, Real L);
				Real PM(Real t, bool plus);
				Real CalcTArg(Real t, bool plus, Real n);
				Real Apm(Real t, bool plus, Real n);
				Complex FuncF(Real x);

				const Parameters k = LinkwitzRiley::DefaultFM() * PI_2 / SPEED_OF_SOUND;
				const std::array<Complex, 4> E = InitE();

				LinkwitzRiley lrFilter;
			};

			//////////////////// BTM class ////////////////////

			struct IntegralLimits
			{
				Real p, m;
				IntegralLimits() : p(0.0), m(0.0) {}
				IntegralLimits(Real _p, Real _m) : p(_p), m(_m) {}
			};

			class BTM : public Model
			{
				using Parameters = Coefficients<std::array<Real, 4>>;

				struct Constants
				{
					Real R0{ 0.0 }, R0Sq{ 0.0 };
					Real dSSq{ 0.0 }, dRSq{ 0.0 };
					Real rr{ 0.0 };
					Real zSRel{ 0.0 }, zRRel{ 0.0 };
					Real dz{ 0.0 }, dzSq{ 0.0 };
					Real v{ 0.0 }, vSq{ 0.0 };
					Real rSSq{ 0.0 }, rRSq{ 0.0 };
					Real edgeHi{ 0.0 }, edgeLo{ 0.0 };
					Parameters theta{ 0.0 }, thetaSq{ 0.0 }, vTheta{ 0.0 };
					Parameters sinTheta{ 0.0 }, cosTheta{ 0.0 }, absTheta{ 0.0 };
					Real zRange{ 0.0 }, zRangeApex{ 0.0 };
					bool splitIntegral{ true };
					Real rho{ 0.0 }, rhoSq{ 0.0 }, rhoOne{ 0.0 }, rhoOneSq{ 0.0 };
					Real R0rho{ 0.0 };
					Real sinPsi{ 0.0 }, cospsi{ 0.0 };
					Real factor{ 0.0 };


					Constants(const Path& path, const Real samplesPerMetre)
					{
						dSSq = path.sData.d * path.sData.d;
						dRSq = path.rData.d * path.rData.d;
						rSSq = path.sData.r * path.sData.r;
						rRSq = path.rData.r * path.rData.r;
						rr = path.sData.r * path.rData.r;

						zSRel = path.sData.z - path.zA;
						zRRel = path.rData.z - path.zA;
						dz = zSRel - zRRel;
						dzSq = dz * dz;
						v = PI_1 / path.wData.t;
						vSq = v * v;

						edgeHi = path.wData.z - path.zA;
						edgeLo = -path.zA;

						Real plus = path.sData.t + path.rData.t;
						Real minus = path.sData.t - path.rData.t;
						theta = Parameters({ PI_1 + plus, PI_1 + minus, PI_1 - minus, PI_1 - plus });
						vTheta = v * theta;
						thetaSq = theta * theta;
						absTheta = Abs(vTheta);
						sinTheta = Sin(vTheta);
						cosTheta = Cos(vTheta);

						R0 = path.sData.d + path.rData.d;
						R0Sq = R0 * R0;
						int n0 = (int)round(samplesPerMetre * R0);
						Real x = (n0 + 0.5) / samplesPerMetre;
						Real xSq = x * x;
						Real MSq = rSSq + zSRel * zSRel;
						Real LSq = rRSq + zRRel * zRRel;
						Real K = MSq - LSq - xSq;
						Real denom = dzSq - xSq;
						Real a = (2.0 * xSq * zRRel - K * dz) / denom;
						Real b = ((K * K / 4) - xSq * LSq) / denom;
						zRangeApex = -a / 2.0 + sqrt(a * a / 4.0 - b);
						zRange = 0.1 * std::min(path.sData.r, path.rData.r);

						if (zRange > zRangeApex)
						{
							zRange = abs(zRangeApex);
							splitIntegral = false;
						}

						rho = path.rData.r / path.sData.r;
						rhoSq = rho * rho;
						rhoOne = rho + 1.0;
						rhoOneSq = rhoOne * rhoOne;
						sinPsi = (path.sData.r + path.rData.r) / R0;
						cospsi = (path.rData.z - path.sData.z) / R0;
						factor = rhoOneSq * sinPsi * sinPsi - 2.0 * rho;

						R0rho = R0 * rho;
					}
				};

			public:
				BTM(const Path& path, int fs);
				~BTM() {};

				void SetTargetParameters(const Path& path) override;
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;

			private:
				Real SkewCase(const Path& path, const Constants& constants);

				Real NonSkewCase(const Path& path, const Constants& constants);

				inline Real CalculateGain(const Path& path) const
				{
					return path.valid ? 1.0 : 0.0;
				}
				Buffer CalcBTM(const Path& path);
				Real CalcSample(int n, const Constants& constants);
				IntegralLimits CalcLimits(Real delta, const Constants& constants);
				Real QuadStep(Real x1, Real x3, Real y1, Real y3, Real y5, const Constants& constants);
				Real CalcIntegral(Real zn1, Real zn2, const Constants& constants);
				Real CalcIntegrand(Real z, const Constants& constants);

				Real samplesPerMetre;
				static constexpr size_t maxIrLength = 2048;
				Path lastPath;

				FIRFilter firFilter;

			};
		}
	}
}

#endif