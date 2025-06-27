/*
*
*  \Diffraction models
*
*/

// Common headers

// Spatialiser headers
#include "Spatialiser/Diffraction/Models.h"
#include "Spatialiser/Types.h"

// Unity headers
#include "Unity/UnityInterface.h"

// DSP headers
#include "DSP/Interpolate.h"

namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{
		namespace Diffraction
		{

			//////////////////// Attenuate class ////////////////////

			////////////////////////////////////////

			void Attenuate::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor)
			{
				if (!isInitialised.load())
				{
					outBuffer.Reset();
					return;
				}

				FlushDenormals();
				for (int i = 0; i < inBuffer.Length(); i++)
					outBuffer[i] = gain.Use(lerpFactor) * inBuffer[i];
				NoFlushDenormals();
			}

			//////////////////// LPF class ////////////////////

			////////////////////////////////////////

			void LPF::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor)
			{
				if (!isInitialised.load())
				{
					outBuffer.Reset();
					return;
				}

				FlushDenormals();
				for (int i = 0; i < inBuffer.Length(); i++)
					outBuffer[i] = gain.Use(lerpFactor) * filter.GetOutput(inBuffer[i], lerpFactor);
				NoFlushDenormals();
			}

			//////////////////// UDFA class ////////////////////

			////////////////////////////////////////

			//UDFA::UDFA(const Path& path, const int fs) : Model(), ft(CalcFT(fs)), fi(CalcFI())
			//{
			//	if (!path.valid)
			//		return;

			//	Constants constants(path, false); // Infinite edge parameters. Pierce two term solutions

			//	std::array<Parameters, numUDFAFilters> parameters = {	// Adjust for finite edge
			//		Parameters(constants.fc1, constants.gain1, constants.tDiffBase),
			//		Parameters(constants.fc1, constants.gain1, constants.tDiffTop),
			//		Parameters(constants.fc2, constants.gain2, constants.tDiffBase),
			//		Parameters(constants.fc2, constants.gain2, constants.tDiffTop) };

			//	for (int j = 0; j < numUDFAFilters; j++)
			//	{
			//		FilterParameters filterParameters = CalculateParameters(parameters[j]);
			//		gain[j].emplace(filterParameters.gain);
			//		for (int i = 0; i < numShelvingFilters; i++)
			//			filters[j * numShelvingFilters + i].emplace(filterParameters.fc[i], filterParameters.g[i], fs);
			//	}
			//	isInitialised.store(true);
			//};

			//////////////////////////////////////////

			//void UDFA::SetTargetParameters(const Path& path)
			//{
			//	if (!path.valid)
			//	{
			//		for (int i = 0; i < numUDFAFilters; i++)
			//			gain[i]->SetTarget(0.0);
			//		return;
			//	}

			//	Constants constants(path, isUDFAI); // Infinite edge parameters.

			//	std::array<Parameters, 4> parameters = {	// Adjust for finite edge
			//		Parameters(constants.fc1, constants.gain1, constants.tDiffBase),
			//		Parameters(constants.fc1, constants.gain1, constants.tDiffTop),
			//		Parameters(constants.fc2, constants.gain2, constants.tDiffBase),
			//		Parameters(constants.fc2, constants.gain2, constants.tDiffTop) };

			//	for (int j = 0; j < numUDFAFilters; j++)
			//	{
			//		FilterParameters filterParameters = CalculateParameters(parameters[j]);
			//		gain[j]->SetTarget(filterParameters.gain);
			//		for (int i = 0; i < numShelvingFilters; i++)
			//			filters[j * numShelvingFilters + i]->SetTargetParameters(filterParameters.fc[i], filterParameters.g[i]);
			//	}
			//};

			//////////////////////////////////////////

			//UDFA::ParametersT UDFA::CalcFT(int fs) const
			//{
			//	ParametersT f(numShelvingFilters + 1);
			//	Real fMin = log10(10.0);
			//	Real fMax = log10(static_cast<Real>(fs));

			//	Real delta = (fMax - fMin) / numShelvingFilters;

			//	for (int i = 0; i <= numShelvingFilters; i++)
			//		f[i] = pow(10.0, fMin + delta * i);
			//	return f;
			//}

			//////////////////////////////////////////

			//UDFA::ParametersI UDFA::CalcFI() const
			//{
			//	ParametersI f(numShelvingFilters);
			//	for (int i = 0; i < numShelvingFilters; i++)
			//		f[i] = ft[i] * sqrt(ft[i + 1] / ft[i]);
			//	return f;
			//}

			//////////////////////////////////////////

			//UDFA::FilterParameters UDFA::CalculateParameters(const Parameters& parameters) const
			//{
			//	ParametersT gt = CalcGT(parameters);
			//	ParametersI gi = CalcGI(gt, parameters);

			//	FilterParameters filterParameters;
			//	for (int i = 0; i < numShelvingFilters; i++)
			//		filterParameters.g[i] = gt[i + 1] / gt[i];

			//	const Coefficients giSq = gi * gi;
			//	const Coefficients gSq = filterParameters.g * filterParameters.g;
			//	filterParameters.fc = fi * ((giSq - gSq) / (filterParameters.g * (1.0 - giSq))).Sqrt() * (1.0 + gSq / 12.0);
			//	filterParameters.gain = gt[0] * parameters.gain / 2;
			//	return filterParameters;
			//}

			//////////////////////////////////////////

			//UDFA::ParametersT UDFA::CalcGT(const Parameters& parameters) const
			//{
			//	ParametersT gt = ParametersT(numShelvingFilters + 1);
			//	for (int i = 0; i < numShelvingFilters + 1; i++)
			//		gt[i] = CalcG(ft[i], parameters);
			//	return gt;
			//}

			//////////////////////////////////////////

			//UDFA::ParametersI UDFA::CalcGI(const ParametersT& gt, const Parameters& parameters) const
			//{
			//	ParametersI gi = ParametersI(numShelvingFilters);
			//	for (int i = 0; i < numShelvingFilters; i++)
			//		gi[i] = CalcG(fi[i], parameters) / gt[i];
			//	return gi;
			//}

			//////////////////////////////////////////

			//Complex UDFA::CalcUDFA(Real f, const Parameters& parameters) const
			//{
			//	Real alpha = 0.5;
			//	Real r = 1.6;
			//	return pow(pow(imUnit * f / parameters.fc, 2.0 / parameters.blend) + pow(imUnit * f / (parameters.Q * parameters.fc), 1.0 / (pow(parameters.blend, r))) + Complex(1.0, 0.0), -alpha * parameters.blend / 2.0);
			//}

			//////////////////////////////////////////

			//void UDFA::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor)
			//{
			//	outBuffer.Reset();
			//	if (!isInitialised.load())
			//		return;

			//	const int totalFilters = static_cast<int>(filters.size());

			//	FlushDenormals();
			//	for (int i = 0; i < inBuffer.Length(); i++)
			//	{
			//		for (int j = 0; j < numUDFAFilters; j++)
			//		{
			//			Real out = filters[j * numShelvingFilters]->GetOutput(inBuffer[i], lerpFactor);
			//			for (int k = 1; k < numShelvingFilters; k++)
			//				out = filters[j * numShelvingFilters + k]->GetOutput(out, lerpFactor);
			//			outBuffer[i] += out * gain[j]->Use(lerpFactor);
			//		}
			//	}
			//	NoFlushDenormals();
			//}

			//////////////////// UDFA-I class ////////////////////

			////////////////////////////////////////


			////////////////////////////////////////

			//Complex UDFAI::CalcH(Real z, Real t, Real f, Real tDiff, const Constants& constants) const
			//{
			//	// Real fc = constants.front;
			//	Real fc = 100.0;
			//	Real g = (2 / PI_1) * atan(PI_1 * sqrt(2 * fc * tDiff));
			//	Real gSq = g * g;
			//	fc *= (1.0 / gSq);
			//	return g * CalcUDFA(f, fc, gSq);
			//}

			//////////////////// NN class ////////////////////

			void NN::SetTargetParameters(const Path& path)
			{
				if (!(path.valid && path.inShadow))
				{
					filter.SetTargetGain(0.0);
					return;
				}

				Parameters zpk = CalculateParameters(path);
				filter.SetTargetParameters(zpk.data);
			}

			////////////////////////////////////////

			NN::Parameters NN::CalculateParameters(const Path& path) const
			{
				Input input = CalcInput(path);
				Parameters output = RunNN(input);
				if (!path.inShadow)
					output.data[4] = 0.0;
				return output;
			}

			////////////////////////////////////////

			NN::Input NN::CalcInput(const Path& path) const
			{
				Input input;
				input[0] = static_cast<float>(path.wData.t);
				if (path.inShadow)
				{
					input[1] = static_cast<float>(path.bA);
					input[2] = static_cast<float>(path.mA);
				}
				else
				{
					input[1] = static_cast<float>(PI_1);
					input[2] = static_cast<float>(std::min(path.mA, path.wData.t - PI_1));
				}
				input[3] = static_cast<float>(path.wData.z);
				if (path.sData.r < path.rData.r)
					AssignInputRZ(path.sData, path.rData, path.wData.z, input);
				else
					AssignInputRZ(path.rData, path.sData, path.wData.z, input);
				return input;
			}

			////////////////////////////////////////

			void NN::AssignInputRZ(const SRData& one, const SRData& two, Real zW, Input& input) const
			{
				input[4] = static_cast<float>(one.r);
				input[5] = static_cast<float>(two.r);
				if (one.z < zW / 2)
				{
					input[6] = static_cast<float>(one.z);
					input[7] = static_cast<float>(two.z);
				}
				else
				{
					input[6] = static_cast<float>(zW - one.z);
					input[7] = static_cast<float>(zW - two.z);
				}
			}

			////////////////////////////////////////

			void NN::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor)
			{
				if (!isInitialised)
				{
					outBuffer.Reset();
					return;
				}

				FlushDenormals();
				for (int i = 0; i < inBuffer.Length(); i++)
					outBuffer[i] = filter.GetOutput(inBuffer[i], lerpFactor);
				NoFlushDenormals();
			}

			//////////////////// UTD class ////////////////////

			UTD::UTD(const Path& path, int fs) : Model(), lrFilter(CalcUTD(path), fs)
			{
				isInitialised.store(true);
			}

			void UTD::SetTargetParameters(const Path& path)
			{
				Parameters gains = CalcUTD(path);
				lrFilter.SetTargetGains(gains);
			}

			UTD::Parameters UTD::CalcUTD(const Path& path)
			{
				if (!path.valid || !path.inShadow)
					return 0.0;

				Real n = path.wData.t / PI_1;
				Real B0 = sin(path.phi);
				Real dSR = path.sData.d + path.rData.d;
				Real temp = sqrt(path.sData.d * path.rData.d * dSR) * n * B0;
				Real L = path.sData.d * path.rData.d * B0 * B0 / (dSR);

				Real idx = (path.bA - PI_1) / ((Real)path.wData.t - (Real)path.sData.t - PI_1);

				Parameters g{ 0.0 }, gSB{ 0.0 };
				for (int i = 0; i < 4; i++)
				{
					Complex A = -exp(-imUnit * k[i] * dSR) * E[i] / temp;
					g[i] = abs(A * (EqHalf(path.rData.t - path.sData.t, i, n, L) + EqHalf(path.rData.t + path.sData.t, i, n, L)));
					gSB[i] = abs(A * (EqHalf(PI_EPS, i, n, L) + EqHalf(2 * path.sData.t + PI_EPS, i, n, L)));
				}
				Coefficients<std::array<Real, 4>> oldGains = (1.0 - idx) * g / gSB + idx * g * dSR;
				return Pow(g / gSB, 1.0 - idx) * Pow(g * dSR, idx);
			}

			Complex UTD::EqQuarter(Real t, bool plus, const int i, Real n, Real L)
			{
				Real cotArg = (PI_1 + PM(t, plus)) / (2.0 * n);
				if (fabs(cotArg) < 0.001f)
				{
					Real tArg = PM(-CalcTArg(t, plus, n), plus);
					Real eps = PI_1 + tArg;
					if (eps == 0.0)
						eps = 0.001f;
					Real kL2 = 2.0 * k[i] * L;
					return n * exp(imUnit * PI_1 / 4.0) * (sqrt(PI_1 * kL2) * Sign(eps) - kL2 * eps * exp(imUnit * PI_1 / 4.0));
				}
				return cot(cotArg) * FuncF(k[i] * L * Apm(t, plus, n));
			}

			Real UTD::PM(Real t, bool plus)
			{
				if (plus)
					return t;
				else
					return -t;
			}

			Real UTD::Apm(Real t, bool plus, Real n)
			{
				Real tArg = CalcTArg(t, plus, n);
				Real cosArg = cos(tArg / 2.0);
				return 2.0 * cosArg * cosArg;
			}

			Real UTD::CalcTArg(Real t, bool plus, Real n)
			{
				Real N;
				Real PI_2n = PI_2 * n;
				if (plus)
					N = round((PI_1 + t) / PI_2n);
				else
					N = round((-PI_1 + t) / PI_2n);
				return PI_2n * N - t;
			}

			Complex UTD::FuncF(Real x)
			{
				Real temp = 0.0;
				Real sqrtX = sqrt(x);
				if (x < 0.8)
					temp = sqrt(PI_1 * x) * (1.0 - (sqrtX / (0.7 * sqrtX + 1.2)));
				else
				{
					Real store = x + 1.25;
					temp = 1 - 0.8 / (store * store);
				}
				return temp * exp(imUnit * PI_1 / 4.0 * (1.0 - sqrtX / (x + 1.4)));
			}

			void UTD::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor)
			{
				FlushDenormals();
				for (int i = 0; i < inBuffer.Length(); i++)
					outBuffer[i] = lrFilter.GetOutput(inBuffer[i], lerpFactor);
				NoFlushDenormals();
			}

			//////////////////// BTM class ////////////////////

			BTM::BTM(const Path& path, int fs) : Model(), samplesPerMetre(fs* INV_SPEED_OF_SOUND), firFilter(CalcBTM(path), maxIrLength)
			{
				if (!path.valid)
					return;
				isInitialised.store(true);
			};

			void BTM::SetTargetParameters(const Path& path)
			{
				if (lastPath == path)
					return;
				lastPath = path;

				if (!path.valid)
					return;

				Buffer ir = CalcBTM(path);
				if (ir.Length() > maxIrLength)
					ir.ResizeBuffer(maxIrLength);
				if (ir.Valid())
					firFilter.SetTargetIR(ir);
			}

			Real BTM::NonSkewCase(const Path& path, const Constants& constants)
			{
				Real sqrtB3 = SQRT_2 * constants.R0 * constants.rho / constants.rhoOne / sqrt(constants.factor);

				Real output = 0.0;
				for (int i = 0; i < constants.theta.Length(); i++)
				{
					bool singularterm = constants.absTheta[i] < 10.0 * DBL_MIN || abs(constants.absTheta[i] - PI_2) < 10.0 * DBL_MIN;
					if (singularterm)
						continue;

					bool useserialexp1 = constants.absTheta[i] < 0.01;
					bool useserialexp2 = abs(constants.absTheta[i] - PI_2) < 0.01;
					bool useserialexp = useserialexp1 || useserialexp2;

					Real sqrtB1vec = 0.0;
					Real fifactvec = 0.0;
					if (!useserialexp)
					{
						sqrtB1vec += (sqrt(2.0 * (1.0 - constants.cosTheta[i])) * constants.R0rho / constants.rhoOneSq / constants.v);
						fifactvec += (1.0 - constants.cosTheta[i]) / constants.vSq;
					}

					if (useserialexp1)
					{
						sqrtB1vec += (constants.theta[i] * sqrt(1.0 - constants.thetaSq[i] / 12.0) * constants.R0rho / constants.rhoOneSq);
						fifactvec += constants.thetaSq[i] / 2.0 * (1.0 - constants.thetaSq[i] / 12.0);

					}

					if (useserialexp2)
					{
						Real fivecPI2ny = constants.theta[i] - PI_2 / constants.v;
						Real nyfivecPI2sq = (constants.vTheta[i] - PI_2) * (constants.vTheta[i] - PI_2);
						sqrtB1vec += fivecPI2ny * sqrt(1.0 - nyfivecPI2sq / 12.0) * constants.R0rho / constants.rhoOneSq;
						fifactvec += fivecPI2ny * fivecPI2ny / 2. * (1 - nyfivecPI2sq / 12.0);
					}

					bool usespecialcase = abs(sqrtB3 - sqrtB1vec) < 1e-14;

					Real temp1vec = constants.sinTheta[i] / (constants.rhoOneSq - constants.factor * fifactvec + DBL_MIN * 10.0);


					Real temp1_2vec = (constants.sinTheta[i] + 10.0 * DBL_MIN) / (constants.rhoOneSq - constants.factor * fifactvec) / (sqrtB1vec + 10.0 * DBL_MIN) * atan(constants.zRange / (sqrtB1vec + DBL_MIN));

					Real temp3vec = -1.0 / sqrtB3 * atan(constants.zRange / sqrtB3);

					Real approxintvalvec = 2.0 / constants.vSq * constants.rho * (temp1_2vec + temp1vec * temp3vec);

					if (constants.sinTheta[i] == 0.0 || usespecialcase)
						approxintvalvec = 0.0;

					if (usespecialcase)
					{
						Real specialcasevalue = 1.0 / (2.0 * sqrtB3 * sqrtB3) * (constants.zRange / (constants.zRange * constants.zRange + sqrtB3 * sqrtB3) + 1. / sqrtB3 * atan(constants.zRange / (sqrtB3 + DBL_MIN)));
						approxintvalvec += 4.0 * constants.R0Sq * constants.rhoSq * constants.rho * constants.sinTheta[i] / constants.vSq / pow(constants.rhoOne, 4.0) / constants.factor * specialcasevalue;
					}
					output += approxintvalvec;
				}
				return output;
			}

			Real BTM::SkewCase(const Path& path, const Constants& constants)
			{
				Real output = 0.0;
				for (int i = 0; i < constants.theta.Length(); i++)
				{
					bool singularterm = constants.absTheta[i] < 10.0 * DBL_MIN || abs(constants.absTheta[i] - PI_2) < 10.0 * DBL_MIN;
					if (singularterm)
						continue;

					bool useserialexp1 = constants.absTheta[i] < 0.01;
					bool useserialexp2 = abs(constants.absTheta[i] - PI_2) < 0.01;
					bool useserialexp = useserialexp1 || useserialexp2;

					Real sqrtB1vec = 0.0;
					Real fifactvec = 0.0;
					if (!useserialexp)
					{
						sqrtB1vec += sqrt(2.0 * (1.0 - constants.cosTheta[i])) * constants.R0rho / constants.rhoOneSq / constants.v;
						fifactvec += (1.0 - constants.cosTheta[i]) / constants.vSq;
					}

					if (useserialexp1)
					{
						sqrtB1vec += (constants.theta[i] * sqrt(1.0 - constants.thetaSq[i] / 12.0) * constants.R0rho / constants.rhoOneSq);
						fifactvec += constants.thetaSq[i] / 2.0 * (1.0 - constants.thetaSq[i] / 12.0);
					}

					if (useserialexp2)
					{
						Real fivecPI2ny = constants.theta[i] - PI_2 / constants.v;
						Real nyfivecPI2sq = (constants.vTheta[i] - PI_2) * (constants.vTheta[i] - PI_2);
						sqrtB1vec += fivecPI2ny * sqrt(1.0 - nyfivecPI2sq / 12.0) * constants.R0rho / constants.rhoOneSq;
						fifactvec += fivecPI2ny * fivecPI2ny / 2. * (1 - nyfivecPI2sq / 12.0);
					}

					Real B3 = 2.0 * constants.R0Sq * constants.rhoSq / constants.rhoOne / constants.rhoOne / constants.factor;
					Real B1vec = sqrtB1vec * sqrtB1vec;
					Real B2 = -2.0 * constants.R0 * (1 - constants.rho) * constants.rho * constants.cospsi / constants.rhoOne / constants.factor;
					Real E1vec = 4.0 * constants.R0Sq * constants.rhoSq * constants.rho * constants.sinTheta[i] / constants.vSq / pow(constants.rhoOne, 4.0) / constants.factor;
					
					Real multfact = -E1vec * B2 / (B1vec * B2 * B2 + (B1vec - B3) * (B1vec - B3));

					Real P1 = 0.5 * log(abs(B3) * abs(constants.zRange * constants.zRange + B1vec) / abs(B1vec) / abs(constants.zRange * constants.zRange + B2 * constants.zRange + B3));
					Real P2 = (B1vec - B3) / (sqrtB1vec + 10.0 * DBL_MIN) / B2 * atan(constants.zRange / (sqrtB1vec + 10.0 * DBL_MIN));
					Real q = 4.0 * B3 - B2 * B2;

					Real P3 = (2.0 * (B3 - B1vec) - B2 * B2) / 2.0 / B2;
					if (q > 0)
					{
						Real sqrtq = sqrt(q);
						P3 *= 2 / sqrtq * (atan((2.0 * constants.zRange + B2) / sqrtq) - atan(B2 / sqrtq));
					}
					else if (q < 0)
					{
						Real sqrtminq = sqrt(-q);
						P3 *= 1.0 / sqrtminq * log(abs(2.0 * constants.zRange + B2 - sqrtminq) * abs(B2 + sqrtminq) / abs(2.0 * constants.zRange + B2 + sqrtminq) / abs(B2 - sqrtminq));
					}
					else // q = 0
						P3 *= 4.0 * constants.zRange / B2 / (2.0 * constants.zRange + B2);
					
					output += multfact * (P1 + P2 + P3);
				}
				return output;
			}

			Buffer BTM::CalcBTM(const Path& path)
			{
				if (!path.valid)
					return Buffer();
				Constants constants(path, samplesPerMetre);

				int n0 = (int)round(samplesPerMetre * constants.R0);
				int nir = (int)round(samplesPerMetre * path.GetMaxD());
				int irLen = nir - n0;

				Buffer ir(irLen);

				if (path.sData.z == path.rData.z || path.sData.r == path.rData.r)
					ir[0] = NonSkewCase(path, constants);
				else
					ir[0] = SkewCase(path, constants);

				if (constants.splitIntegral)
					ir[0] += CalcIntegral(constants.zRange, constants.zRangeApex, constants);


				Real d = path.sData.d + path.rData.d; // Remove 1 / r as applied by HRTF processing
				ir[0] *= -constants.v * d / PI_2; // Multiply by 2 for pos and neg wedge part - add check for edge hi and lo?

				for (int i = 1; i < irLen; i++)
				{
					int n = n0 + i;
					ir[i] = d * CalcSample(n, constants);
				}
				return ir;
			}

			Real BTM::CalcSample(int n, const Constants& constants)
			{
				IntegralLimits zn1 = CalcLimits((n - 0.5) / samplesPerMetre, constants);
				IntegralLimits zn2 = CalcLimits((n + 0.5) / samplesPerMetre, constants);

				if (isnan(zn2.p))
				{
					// Both limits of integration are imaginary
					// Entire sample has no existing edge
					return 0.0;
				}
				if (isnan(zn1.p))
				{
					// Only lower limit of integration is imaginary
					// Instead start integrating at apex point
					zn1.p = 0.0;
					zn1.m = 0.0;
				}

				// Check ranges against edge boundries
				// The two ranges are [zn2.m, zn1.m] and [zn1.p, zn2.p] (neg to pos)

				Real output = 0.0;
				if (zn2.m >= constants.edgeLo)
				{
					if (zn2.p <= constants.edgeHi)
					{
						output = CalcIntegral(zn1.p, zn2.p, constants);
						output *= -constants.v / PI_2;
					}

					else
					{
						output += CalcIntegral(zn2.m, zn1.m, constants);
						if (zn1.p < constants.edgeHi)
							output += CalcIntegral(zn1.p, constants.edgeHi, constants);
						output *= -constants.v / PI_4;
					}
				}
				else
				{
					if (zn1.m > constants.edgeLo)
						output += CalcIntegral(constants.edgeLo, zn1.m, constants);
					if (zn2.p <= constants.edgeHi)
						output += CalcIntegral(zn1.p, zn2.p, constants);
					else if (zn1.p < constants.edgeHi)
						output += CalcIntegral(zn1.p, constants.edgeHi, constants);
					output *= -constants.v / PI_4;
				}

				return output;
			}

			IntegralLimits BTM::CalcLimits(Real delta, const Constants& constants)
			{
				Real dSq = delta * delta;
				Real kq = constants.dSSq - constants.dRSq - dSq;
				Real aq = constants.dzSq - dSq;
				Real bq = 2.0 * dSq * constants.zRRel - kq * constants.dz;
				Real cq = (kq * kq) / 4.0 - dSq * constants.dRSq;

				bq /= aq;
				cq /= aq;
				Real sq = (bq * bq) - 4.0 * cq;
				if (sq < 0.0)
					return IntegralLimits(NAN, NAN);
				sq = sqrt(sq);
				return IntegralLimits((-bq + sq) / 2.0, (-bq - sq) / 2.0);
			}

			Real BTM::QuadStep(Real x1, Real x3, Real y1, Real y3, Real y5, const Constants& constants)
			{
				Real x2 = (x1 + x3) / 2.0;
				Real y2 = CalcIntegrand((x1 + x2) / 2, constants);
				Real y4 = CalcIntegrand((x3 + x2) / 2, constants);

				Real tempVec = (x3 - x1) / 6.0;
				Real yTemp = y1 + 2.0 * y3 + y5;
				Real output = tempVec * (yTemp + 2.0 * y3);
				yTemp = tempVec / 2.0 * (yTemp + 4 * y2 + 4 * y4);
				output = yTemp + (yTemp - output) / 15;

				if (std::abs(yTemp - output) > 1e-11)
				{
					Real output1 = QuadStep(x1, x2, y1, y2, y3, constants);
					Real output2 = QuadStep(x2, x3, y3, y4, y5, constants);
					return output1 + output2;
				}
				return output;
			}

			Real BTM::CalcIntegral(Real zn1, Real zn2, const Constants& constants)
			{
				// Quadstep simpson's rule
				Real h = 0.13579 * (zn2 - zn1);

				Real x1 = zn1;
				Real x2 = zn1 + h;
				Real x3 = zn1 + 2.0 * h;
				Real x4 = (zn1 + zn2) * 0.5;
				Real x5 = zn2 - 2.0 * h;
				Real x6 = zn2 - h;
				Real x7 = zn2;

				Real y1 = CalcIntegrand(x1, constants);
				Real y2 = CalcIntegrand(x2, constants);
				Real y3 = CalcIntegrand(x3, constants);
				Real y4 = CalcIntegrand(x4, constants);
				Real y5 = CalcIntegrand(x5, constants);
				Real y6 = CalcIntegrand(x6, constants);
				Real y7 = CalcIntegrand(x7, constants);

				Real output1 = QuadStep(x1, x3, y1, y2, y3, constants);
				Real output2 = QuadStep(x3, x5, y3, y4, y5, constants);
				Real output3 = QuadStep(x5, x7, y5, y6, y7, constants);

				return output1 + output2 + output3;

				// Simpson's rule
				/*Real fa = CalcIntegrand(zn1, constants);
				Real fc = CalcIntegrand(mid, constants);
				Real fb = CalcIntegrand(zn2, constants);
				return (zn2 - zn1) / 6.0 * (CalcIntegrand(zn1, constants) + 4.0 * CalcIntegrand(mid, constants) + CalcIntegrand(zn2, constants));*/

				// Quadrature rule
				/*Real n = 6.0;
				Real output = (CalcIntegrand(zn1) + CalcIntegrand(zn2)) / 2.0;
				for (int i = 1; i < n; i++)
					output += CalcIntegrand(zn1 + i * (zn2 - zn1) / n);
				return (zn2 - zn1) / n * output;*/
			}

			Real BTM::CalcIntegrand(Real z, const Constants& constants)
			{
				Real dzS = z - constants.zSRel;
				Real dzR = z - constants.zRRel;

				Real dzSSq = dzS * dzS;
				Real dzRSq = dzR * dzR;

				Real dS = sqrt(dzSSq + constants.rSSq);
				Real dR = sqrt(dzRSq + constants.rRSq);

				Real ml = dS * dR;
				Real y = std::max(1.0, (ml + dzS * dzR) / constants.rr); // limit to 1 -> real(sqrt(y ^ 2 - 1)) returns 0 if y <= 1
				Real A = y + sqrt(y * y - 1.0);
				Real Apow = pow(A, constants.v);
				Real coshvtheta = (Apow + (1.0 / Apow)) / 2.0;

				Real Btotal = Sum(constants.sinTheta / (coshvtheta - constants.cosTheta));
				return Btotal / ml;
			}

			void BTM::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor)
			{
				if (!isInitialised.load())
				{
					outBuffer.Reset();
					return;
				}

				FlushDenormals();	// Only needed if interpolating but expensive to call every sample
				for (int i = 0; i < inBuffer.Length(); i++)
					outBuffer[i] = firFilter.GetOutput(inBuffer[i], lerpFactor);
				NoFlushDenormals();
			}
		}
	}
}

#pragma endregion