/*
*
*  \Diffraction models
*
*/

// Spatialiser headers
#include "Spatialiser/Diffraction/Models.h"

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

			void Attenuate::ProcessAudio(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor)
			{
				if (!isInitialised.load(std::memory_order_acquire))
				{
					outBuffer.Reset();
					return;
				}

				for (int i = 0; i < inBuffer.Length(); i++)
					outBuffer[i] = gain.Use(lerpFactor) * inBuffer[i];
			}

			//////////////////// LPF class ////////////////////

			////////////////////////////////////////

			void LPF::ProcessAudio(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor)
			{
				if (!isInitialised.load(std::memory_order_acquire))
				{
					outBuffer.Reset();
					return;
				}

				for (int i = 0; i < inBuffer.Length(); i++)
					outBuffer[i] = gain.Use(lerpFactor) * filter.GetOutput(inBuffer[i], lerpFactor);
			}

			//////////////////// NN class ////////////////////

			////////////////////////////////////////

			void NN::SetTargetParameters(const Path& path)
			{
				if (!(path.valid && path.inShadowZone))
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
				Input input = CalculateInput(path);
				Parameters output = RunNN(input);
				if (!path.inShadowZone)
					output.data[4] = (Real)0.0;
				return output;
			}

			////////////////////////////////////////

			NN::Input NN::CalculateInput(const Path& path) const
			{
				Input input;
				input[0] = static_cast<float>(path.eData.t);
				if (path.inShadowZone)
				{
					input[1] = static_cast<float>(path.bA);
					input[2] = static_cast<float>(path.mA);
				}
				else
				{
					input[1] = static_cast<float>(PI_1);
					input[2] = static_cast<float>(std::min(path.mA, path.eData.t - PI_1));
				}
				input[3] = static_cast<float>(path.eData.z);
				if (path.sData.r < path.rData.r)
					AssignInputRZ(path.sData, path.rData, path.eData.z, input);
				else
					AssignInputRZ(path.rData, path.sData, path.eData.z, input);
				return input;
			}

			////////////////////////////////////////

			void NN::AssignInputRZ(const Path::SRData& one, const Path::SRData& two, Real zW, Input& input) const
			{
				Debug::Assert(one.r <= two.r, "Second NN input radius is less than the first");

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

			void NN::ProcessAudio(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor)
			{
				if (!isInitialised)
				{
					outBuffer.Reset();
					return;
				}

				for (int i = 0; i < inBuffer.Length(); i++)
					filter.GetOutput(inBuffer[i], outBuffer[i], lerpFactor);
			}

			//////////////////// UTD class ////////////////////

			////////////////////////////////////////

			UTD::Parameters UTD::CalculateUTD(const Path& path) const
			{
				if (!path.valid || !path.inShadowZone)
					return Parameters::Zero();

				Real n = path.eData.t / PI_1; // fig. 5b (KP)
				Real B0 = sin(path.phi); // fig. 5a (KP)
				Real dSR = path.sData.d + path.rData.d;
				Real A = sqrt(path.sData.d * path.rData.d * dSR) * n * B0; // eq. 23 and 25 (excluding E) (spherical wave) (KP)
				Real L = path.sData.d * path.rData.d * B0 * B0 / dSR; // eq. 32 (spherical wave) (KP)

				Parameters g = Parameters::Zero();
				Parameters gSB = Parameters::Zero();
				for (int i = 0; i < 4; i++)
				{
					Complex AD = -std::exp(-imUnit * k[i] * dSR) * E[i] / A;
					g[i] = std::abs(AD * (EqHalf(path.rData.t - path.sData.t, k[i], n, L) + EqHalf(path.rData.t + path.sData.t, k[i], n, L))); // eq. 25
					gSB[i] = std::abs(AD * (EqHalf(PI_EPS, k[i], n, L) + EqHalf(2 * path.sData.t + PI_EPS, k[i], n, L)));
				}
				Real idx = (path.bA - PI_1) / (path.eData.t - path.sData.t - PI_1);
				Coefficients<Real, 4> oldGains = (REAL_CONST(1.0) - idx) * g / gSB + idx * g * dSR;
				return (g / gSB).Pow(REAL_CONST(1.0) - idx) * (g * dSR).Pow(idx);
			}

			////////////////////////////////////////

			Complex UTD::EqQuarter(Real t, bool plus, Real k, Real n, Real L) const
			{
				Real cotArg = (PI_1 + PM(t, plus)) / (REAL_CONST(2.0) * n); // eq. 25 (KP)
				if (std::abs(cotArg) < REAL_CONST(0.001))
				{
					Real tArg = PM(-CalculateAlphaPMCosineInput(t, plus, n), plus);
					Real eps = PI_1 + tArg;
					if (eps == REAL_CONST(0.0))
						eps = REAL_CONST(0.001);
					Real kL2 = REAL_CONST(2.0) * k * L;
					return n * std::exp(imUnit * PI_1 / REAL_CONST(4.0)) * (sqrt(PI_1 * kL2) * Sign(eps) - kL2 * eps * std::exp(imUnit * PI_1 / REAL_CONST(4.0)));
				}
				return cot(cotArg) * FresnelIntegral(k * L * AlphaPM(t, plus, n));
			}

			////////////////////////////////////////

			Real UTD::AlphaPM(Real t, bool plus, Real n) const
			{
				Real arg = CalculateAlphaPMCosineInput(t, plus, n);
				Real cosValue = cos(arg / REAL_CONST(2.0));
				return REAL_CONST(2.0) * cosValue * cosValue; // eq. 27 (KP)
			}

			////////////////////////////////////////

			Real UTD::CalculateAlphaPMCosineInput(Real t, bool plus, Real n) const
			{
				Real N;
				Real PI_2n = PI_2 * n;
				if (plus)
					N = round((PI_1 + t) / PI_2n); // eq. 28a (KP)
				else
					N = round((-PI_1 + t) / PI_2n); // eq. 28b (KP)
				return PI_2n * N - t;
			}

			////////////////////////////////////////

			Complex UTD::FresnelIntegral(Real x) const
			{
				Real sqrtX = sqrt(x);
				Complex output = std::exp(imUnit * PI_1 / REAL_CONST(4.0) * (REAL_CONST(1.0) - sqrtX / (x + REAL_CONST(1.4)))); // eq. 22 (K)
				if (x < 0.8)
					output *= sqrt(PI_1 * x) * (REAL_CONST(1.0) - (sqrtX / (REAL_CONST(0.7) * sqrtX + REAL_CONST(1.2)))); // eq. 22 (K)
				else
				{
					Real store = x + REAL_CONST(1.25);
					output *= 1 - REAL_CONST(0.8) / (store * store); // eq. 22 (K)
				}
				return output;
			}

			////////////////////////////////////////

			void UTD::ProcessAudio(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor)
			{
				for (int i = 0; i < inBuffer.Length(); i++)
					outBuffer[i] = lrFilter.GetOutput(inBuffer[i], lerpFactor);
			}

			//////////////////// BTM class ////////////////////

			////////////////////////////////////////

			void BTM::SetTargetParameters(const Path& path)
			{
				if (lastPath == path)
					return;
				lastPath = path;

				if (!path.valid)
					return;

				Buffer<> ir = CalculateBTM(path);
				if (ir.Length() > maxIrLength)
					ir.Resize(maxIrLength);
				if (ir.Valid())
					firFilter.SetTargetIR(ir);
			}

			////////////////////////////////////////

			Real BTM::NonSkewCase(const Path& path, const Constants& constants)
			{
				Real sqrtB3 = SQRT_2 * constants.R0 * constants.rho / constants.rhoOne / sqrt(constants.factor);

				Real output = 0.0;
				for (int i = 0; i < constants.theta.Length(); i++)
				{
					bool singularterm = constants.absTheta[i] < 10.0 * std::numeric_limits<Real>::min() || abs(constants.absTheta[i] - PI_2) < 10.0 * std::numeric_limits<Real>::min();
					if (singularterm)
						continue;

					bool useserialexp1 = constants.absTheta[i] < 0.01;
					bool useserialexp2 = abs(constants.absTheta[i] - PI_2) < 0.01;
					bool useserialexp = useserialexp1 || useserialexp2;

					Real sqrtB1vec = 0.0;
					Real fifactvec = 0.0;
					if (!useserialexp)
					{
						sqrtB1vec += (sqrt(REAL_CONST(2.0) * (REAL_CONST(1.0) - constants.cosTheta[i])) * constants.R0rho / constants.rhoOneSq / constants.v);
						fifactvec += (REAL_CONST(1.0) - constants.cosTheta[i]) / constants.vSq;
					}

					if (useserialexp1)
					{
						sqrtB1vec += (constants.theta[i] * sqrt(REAL_CONST(1.0) - constants.thetaSq[i] / REAL_CONST(12.0)) * constants.R0rho / constants.rhoOneSq);
						fifactvec += constants.thetaSq[i] / REAL_CONST(2.0) * (REAL_CONST(1.0) - constants.thetaSq[i] / REAL_CONST(12.0));

					}

					if (useserialexp2)
					{
						Real fivecPI2ny = constants.theta[i] - PI_2 / constants.v;
						Real nyfivecPI2sq = (constants.vTheta[i] - PI_2) * (constants.vTheta[i] - PI_2);
						sqrtB1vec += fivecPI2ny * sqrt(REAL_CONST(1.0) - nyfivecPI2sq / REAL_CONST(12.0)) * constants.R0rho / constants.rhoOneSq;
						fifactvec += fivecPI2ny * fivecPI2ny / REAL_CONST(2.) * (1 - nyfivecPI2sq / REAL_CONST(12.0));
					}

					bool usespecialcase = abs(sqrtB3 - sqrtB1vec) < REAL_CONST(1e-14);

					Real temp1vec = constants.sinTheta[i] / (constants.rhoOneSq - constants.factor * fifactvec + std::numeric_limits<Real>::min() * REAL_CONST(10.0));


					Real temp1_2vec = (constants.sinTheta[i] + REAL_CONST(10.0) * std::numeric_limits<Real>::min()) / (constants.rhoOneSq - constants.factor * fifactvec) / (sqrtB1vec + REAL_CONST(10.0) * std::numeric_limits<Real>::min()) * atan(constants.zRange / (sqrtB1vec + std::numeric_limits<Real>::min()));

					Real temp3vec = REAL_CONST(-1.0) / sqrtB3 * atan(constants.zRange / sqrtB3);

					Real approxintvalvec = REAL_CONST(2.0) / constants.vSq * constants.rho * (temp1_2vec + temp1vec * temp3vec);

					if (constants.sinTheta[i] == 0.0 || usespecialcase)
						approxintvalvec = REAL_CONST(0.0);

					if (usespecialcase)
					{
						Real specialcasevalue = REAL_CONST(1.0) / (REAL_CONST(2.0) * sqrtB3 * sqrtB3) * (constants.zRange / (constants.zRange * constants.zRange + sqrtB3 * sqrtB3) + REAL_CONST(1.) / sqrtB3 * atan(constants.zRange / (sqrtB3 + std::numeric_limits<Real>::min())));
						approxintvalvec += REAL_CONST(4.0) * constants.R0Sq * constants.rhoSq * constants.rho * constants.sinTheta[i] / constants.vSq / pow(constants.rhoOne, REAL_CONST(4.0)) / constants.factor * specialcasevalue;
					}
					output += approxintvalvec;
				}
				return output;
			}

			////////////////////////////////////////

			Real BTM::SkewCase(const Path& path, const Constants& constants)
			{
				Real output = 0.0;
				Real cosPhi = (path.rData.z - path.sData.z) / constants.R0;
				for (int i = 0; i < constants.theta.Length(); i++)
				{
					bool singularterm = constants.absTheta[i] < REAL_CONST(10.0) * std::numeric_limits<Real>::min() || abs(constants.absTheta[i] - PI_2) < REAL_CONST(10.0) * std::numeric_limits<Real>::min();
					if (singularterm)
						continue;

					bool useserialexp1 = constants.absTheta[i] < REAL_CONST(0.01);
					bool useserialexp2 = abs(constants.absTheta[i] - PI_2) < REAL_CONST(0.01);
					bool useserialexp = useserialexp1 || useserialexp2;

					Real sqrtB1vec = 0.0;
					Real fifactvec = 0.0;
					if (!useserialexp)
					{
						sqrtB1vec += sqrt(REAL_CONST(2.0) * (REAL_CONST(1.0) - constants.cosTheta[i])) * constants.R0rho / constants.rhoOneSq / constants.v;
						fifactvec += (REAL_CONST(1.0) - constants.cosTheta[i]) / constants.vSq;
					}

					if (useserialexp1)
					{
						sqrtB1vec += (constants.theta[i] * sqrt(REAL_CONST(1.0) - constants.thetaSq[i] / REAL_CONST(12.0)) * constants.R0rho / constants.rhoOneSq);
						fifactvec += constants.thetaSq[i] / REAL_CONST(2.0) * (REAL_CONST(1.0) - constants.thetaSq[i] / REAL_CONST(12.0));
					}

					if (useserialexp2)
					{
						Real fivecPI2ny = constants.theta[i] - PI_2 / constants.v;
						Real nyfivecPI2sq = (constants.vTheta[i] - PI_2) * (constants.vTheta[i] - PI_2);
						sqrtB1vec += fivecPI2ny * sqrt(REAL_CONST(1.0) - nyfivecPI2sq / REAL_CONST(12.0)) * constants.R0rho / constants.rhoOneSq;
						fifactvec += fivecPI2ny * fivecPI2ny / REAL_CONST(2.) * (1 - nyfivecPI2sq / REAL_CONST(12.0));
					}

					Real B3 = REAL_CONST(2.0) * constants.R0Sq * constants.rhoSq / constants.rhoOne / constants.rhoOne / constants.factor;
					Real B1vec = sqrtB1vec * sqrtB1vec;
					Real B2 = -REAL_CONST(2.0) * constants.R0 * (1 - constants.rho) * constants.rho * cosPhi / constants.rhoOne / constants.factor;
					Real E1vec = REAL_CONST(4.0) * constants.R0Sq * constants.rhoSq * constants.rho * constants.sinTheta[i] / constants.vSq / pow(constants.rhoOne, REAL_CONST(4.0)) / constants.factor;

					Real multfact = -E1vec * B2 / (B1vec * B2 * B2 + (B1vec - B3) * (B1vec - B3));

					Real P1 = REAL_CONST(0.5) * log(abs(B3) * abs(constants.zRange * constants.zRange + B1vec) / abs(B1vec) / abs(constants.zRange * constants.zRange + B2 * constants.zRange + B3));
					Real P2 = (B1vec - B3) / (sqrtB1vec + REAL_CONST(10.0) * std::numeric_limits<Real>::min()) / B2 * atan(constants.zRange / (sqrtB1vec + REAL_CONST(10.0) * std::numeric_limits<Real>::min()));
					Real q = REAL_CONST(4.0) * B3 - B2 * B2;

					Real P3 = (REAL_CONST(2.0) * (B3 - B1vec) - B2 * B2) / REAL_CONST(2.0) / B2;
					if (q > 0)
					{
						Real sqrtq = sqrt(q);
						P3 *= 2 / sqrtq * (atan((REAL_CONST(2.0) * constants.zRange + B2) / sqrtq) - atan(B2 / sqrtq));
					}
					else if (q < 0)
					{
						Real sqrtminq = sqrt(-q);
						P3 *= REAL_CONST(1.0) / sqrtminq * log(abs(REAL_CONST(2.0) * constants.zRange + B2 - sqrtminq) * abs(B2 + sqrtminq) / abs(REAL_CONST(2.0) * constants.zRange + B2 + sqrtminq) / abs(B2 - sqrtminq));
					}
					else // q = 0
						P3 *= REAL_CONST(4.0) * constants.zRange / B2 / (REAL_CONST(2.0) * constants.zRange + B2);

					output += multfact * (P1 + P2 + P3);
				}
				return output;
			}

			////////////////////////////////////////

			Buffer<> BTM::CalculateBTM(const Path& path)
			{
				if (!path.valid)
					return Buffer<>();
				Constants constants(path, samplesPerMetre);

				int n0 = (int)round(samplesPerMetre * constants.R0);
				int nir = (int)round(samplesPerMetre * path.GetMaxD());
				int irLen = nir - n0 + 1;

				Buffer<> ir(irLen);

				if (path.sData.z == path.rData.z || path.sData.r == path.rData.r)
					ir[0] = NonSkewCase(path, constants);
				else
					ir[0] = SkewCase(path, constants);

				if (constants.splitIntegral)
					ir[0] += CalculateIntegral(constants.zRange, constants.zRangeApex, constants);


				Real d = path.sData.d + path.rData.d; // Remove 1 / r as applied by HRTF processing
				ir[0] *= -constants.v * d / PI_2; // Multiply by 2 for pos and neg wedge part - add check for edge hi and lo?

				for (int i = 1; i < irLen; i++)
					ir[i] = d * CalculateSample(n0 + i, constants);
				return ir;
			}

			////////////////////////////////////////

			Real BTM::CalculateSample(int n, const Constants& constants)
			{
				IntegralLimits zn1 = CalculateLimits((n - REAL_CONST(0.5)) / samplesPerMetre, constants);
				IntegralLimits zn2 = CalculateLimits((n + REAL_CONST(0.5)) / samplesPerMetre, constants);

				if (isnan(zn2.p))	// Both limits of integration are imaginary
					return 0.0;		// Entire sample has no existing edge

				if (isnan(zn1.p))	// Only lower limit of integration is imaginary
				{					// Instead start integrating at apex point
					zn1.p = 0.0;
					zn1.m = 0.0;
				}

				// Check ranges against edge boundries
				// The two ranges are [zn2.m, zn1.m] and [zn1.p, zn2.p] (neg to pos)

				if (zn2.m < constants.edgeLo)
					zn2.m = constants.edgeLo;

				if (zn1.m < constants.edgeLo)
					zn1.m = constants.edgeLo;

				if (zn1.p > constants.edgeHi)
					zn1.p = constants.edgeHi;

				if (zn2.p > constants.edgeHi)
					zn2.p = constants.edgeHi;

				Real output = CalculateIntegral(zn2.m, zn1.m, constants) * REAL_CONST(0.5);
				output += CalculateIntegral(zn1.p, zn2.p, constants) * REAL_CONST(0.5);
				return -constants.v / PI_2 * output;
			}

			////////////////////////////////////////

			BTM::IntegralLimits BTM::CalculateLimits(Real delta, const Constants& constants)
			{
				Real dSq = delta * delta;
				Real kq = constants.dSSq - constants.dRSq - dSq;
				Real aq = constants.dzSq - dSq;
				Real bq = REAL_CONST(2.0) * dSq * constants.zRRel - kq * constants.dz;
				Real cq = (kq * kq) / REAL_CONST(4.0) - dSq * constants.dRSq;

				bq /= aq;
				cq /= aq;
				Real sq = (bq * bq) - REAL_CONST(4.0) * cq;
				if (sq < 0.0)
					return IntegralLimits(NAN, NAN);
				sq = sqrt(sq);
				return IntegralLimits((-bq + sq) / REAL_CONST(2.0), (-bq - sq) / REAL_CONST(2.0));
			}

			////////////////////////////////////////

			Real BTM::QuadStep(Real x1, Real x3, Real y1, Real y2, Real y3, const Constants& constants)
			{
				Real x2 = (x1 + x3) / REAL_CONST(2.0);
				Real y12 = CalculateIntegrand((x1 + x2) / 2, constants);
				Real y23 = CalculateIntegrand((x3 + x2) / 2, constants);

				Real tempVec = (x3 - x1) / REAL_CONST(6.0);
				Real yTemp = y1 + REAL_CONST(2.0) * y2 + y3;
				Real output = tempVec * (yTemp + REAL_CONST(2.0) * y2);
				yTemp = tempVec / REAL_CONST(2.0) * (yTemp + 4 * y12 + 4 * y23);
				output = yTemp + (yTemp - output) / 15;

				if (std::abs(yTemp - output) > REAL_CONST(1e-11))
				{
					Real output1 = QuadStep(x1, x2, y1, y12, y2, constants);
					Real output2 = QuadStep(x2, x3, y2, y23, y3, constants);
					return output1 + output2;
				}
				return output;
			}

			////////////////////////////////////////

			Real BTM::CalculateIntegral(Real zn1, Real zn2, const Constants& constants)
			{
				if (zn1 == zn2)	// If the two limits are the same then return 0
					return 0.0;

				// Quadstep simpson's rule
				Real h = REAL_CONST(0.13579) * (zn2 - zn1);

				Real x1 = zn1;
				Real x2 = zn1 + h;
				Real x3 = zn1 + REAL_CONST(2.0) * h;
				Real x4 = (zn1 + zn2) * REAL_CONST(0.5);
				Real x5 = zn2 - REAL_CONST(2.0) * h;
				Real x6 = zn2 - h;
				Real x7 = zn2;

				Real y1 = CalculateIntegrand(x1, constants);
				Real y2 = CalculateIntegrand(x2, constants);
				Real y3 = CalculateIntegrand(x3, constants);
				Real y4 = CalculateIntegrand(x4, constants);
				Real y5 = CalculateIntegrand(x5, constants);
				Real y6 = CalculateIntegrand(x6, constants);
				Real y7 = CalculateIntegrand(x7, constants);

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

			////////////////////////////////////////

			Real BTM::CalculateIntegrand(Real z, const Constants& constants)
			{
				Real dzS = z - constants.zSRel;
				Real dzR = z - constants.zRRel;

				Real dzSSq = dzS * dzS;
				Real dzRSq = dzR * dzR;

				Real dS = std::sqrt(dzSSq + constants.rSSq);
				Real dR = std::sqrt(dzRSq + constants.rRSq);

				Real ml = dS * dR;
				Real y = std::max((Real)1.0, (ml + dzS * dzR) / constants.rr); // limit to 1 -> real(sqrt(y ^ 2 - 1)) returns 0 if y <= 1
				Real A = y + std::sqrt(y * y - (Real)1.0);
				Real Apow = std::pow(A, constants.v);
				Real coshvtheta = (Apow + ((Real)1.0 / Apow)) / (Real)2.0;

				Real Btotal = (constants.sinTheta / (coshvtheta - constants.cosTheta)).Sum();
				return Btotal / ml;
			}

			////////////////////////////////////////

			void BTM::ProcessAudio(const Buffer<>& inBuffer, Buffer<>& outBuffer, const Real lerpFactor)
			{
				if (!isInitialised.load(std::memory_order_acquire))
				{
					outBuffer.Reset();
					return;
				}

				for (int i = 0; i < inBuffer.Length(); i++)
					outBuffer[i] = firFilter.GetOutput(inBuffer[i], lerpFactor);
			}
		}
	}
}

#pragma endregion