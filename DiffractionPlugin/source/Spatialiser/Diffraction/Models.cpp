/*
*
*  \Diffraction models
*
*/

#include "Spatialiser/Diffraction/Models.h"

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{
		namespace Diffraction
		{

			//////////////////// Attenuate class ////////////////////

			void Attenuate::UpdateParameters()
			{
				std::lock_guard<std::mutex> lock(*m);
				if (mPath->valid && mPath->inShadow)
					targetGain = 1.0;
				else
					targetGain = 0.0;
			}

			void Attenuate::ProcessAudio(Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor)
			{
				// Apply gain
				for (int i = 0; i < numFrames; i++)
				{
					outBuffer[i] = inBuffer[i] * currentGain;
					std::lock_guard<std::mutex> lock(*m);
					if (currentGain != targetGain)
						currentGain = Lerp(currentGain, targetGain, lerpFactor);
				}
			}

			//////////////////// LPF class ////////////////////

			LPF::LPF(Path* path, int fs) : fc(1000.0), targetGain(0.0), currentGain(0.0), mPath(path)
			{
				m = new std::mutex();
				filter.SetT(fs);
				filter.UpdateParameters(fc);
				UpdateParameters();
			}

			void LPF::UpdateParameters()
			{
				std::lock_guard<std::mutex> lock(*m);
				if (mPath->valid && mPath->inShadow)
					targetGain = 1.0;
				else
					targetGain = 0.0;
			}

			void LPF::ProcessAudio(Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor)
			{
				// Apply filter
				for (int i = 0; i < numFrames; i++)
				{
					outBuffer[i] = filter.GetOutput(inBuffer[i]) * currentGain;
					std::lock_guard<std::mutex> lock(*m);
					if (currentGain != targetGain)
						currentGain = Lerp(currentGain, targetGain, lerpFactor);
				}
			}

			//////////////////// UDFA class ////////////////////

			UDFA::UDFA(Path* path, int fs) : numFilters(4), target(), current(), params(), mPath(path)
			{
				m = new std::mutex();
				for (int i = 0; i < numFilters; i++)
				{
					filters[i].SetT(fs);
				}

				CalcF(fs);
				UpdateParameters();
			};

			void UDFA::CalcF(int fs)
			{
				CalcFT(fs);
				CalcFI();
			}

			void UDFA::CalcFT(int fs)
			{
				Real fMin = log10(10.0);
				Real fMax = log10(static_cast<Real>(fs));

				Real delta = (fMax - fMin) / numFilters;
				for (int i = 0; i <= numFilters; i++)
				{
					ft[i] = pow(10.0, fMin + delta * i);
				}
			}

			void UDFA::CalcFI()
			{
				for (int i = 0; i < numFilters; i++)
				{
					fi[i] = ft[i] * sqrt(ft[i + 1] / ft[i]);
				}
			}

			void UDFA::UpdateParameters()
			{
				if (mPath->valid)
				{
					UpdateConstants();
					CalcGT();
					for (int i = 0; i < numFilters; i++)
					{
						params.g[i] = gt[i + 1] / gt[i];
						gi[i] = CalcG(fi[i]) / gt[i];
						Real giSq = pow(gi[i], 2.0);
						Real gSq = pow(params.g[i], 2.0);
						params.fc[i] = fi[i] * sqrt((giSq - gSq) / (params.g[i] * (1.0 - giSq))) * (1.0 + gSq / 12.0);
					}
					params.gain = gt[0];
					std::lock_guard<std::mutex> lock(*m);
					target = params;
				}
				else
				{
					std::lock_guard<std::mutex> lock(*m);
					for (int i = 0; i < numFilters; i++)
					{
						target.fc[i] = 1000.0;
						target.g[i] = 1.0;
					}
					target.gain = 0.0;
				}
			}

			void UDFA::UpdateConstants()
			{
				Real d = 2.0 * mPath->sData.d * mPath->rData.d / (mPath->sData.d + mPath->rData.d);
				v = PI_1 / mPath->wData.t;
				t0 = (mPath->sData.d + mPath->rData.d) / SPEED_OF_SOUND;
				front = 2.0 * SPEED_OF_SOUND / (PI_SQ * d * pow(sin(mPath->phi), 2.0));
			}

			void UDFA::CalcGT()
			{
				for (int i = 0; i <= numFilters; i++)
				{
					gt[i] = CalcG(ft[i]);
				}
			}

			Real UDFA::CalcG(Real f)
			{
				return abs((CalcHpm(0.0, f) + CalcHpm(mPath->wData.z, f)) / Complex(4.0, 0.0));
			}

			Complex UDFA::CalcHpm(Real z, Real f)
			{
				return CalcH(z, mPath->sData.t + mPath->rData.t, f) + CalcH(z, mPath->rData.t - mPath->sData.t, f);
			}

			Complex UDFA::CalcH(Real z, Real t, Real f)
			{
				Real fc = front * pow(CalcNv(t), 2);

				Real t1 = mPath->GetD(z) / SPEED_OF_SOUND;

				Real g = (2.0 / PI_1) * atan(PI_1 * sqrt(2.0 * fc * (t1 - t0)));
				fc *= (1 / pow(g, 2.0));
				return g * CalcUDFA(f, fc, g);
			}

			Complex UDFA::CalcUDFA(Real f, Real fc, Real g)
			{
				Real alpha = 0.5;
				Real b = 1.44;
				Real Q = 0.2;
				Real r = 1.6;

				Real gSq = pow(g, 2.0);

				b = 1 + (b - 1.0) * gSq;
				Q = 0.5 + (Q - 0.5) * gSq;

				return pow(pow(imUnit * f / fc, 2.0 / b) + pow(imUnit * f / (Q * fc), 1.0 / (pow(b, r))) + Complex(1.0, 0.0), -alpha * b / 2.0);
			}

			Real UDFA::CalcNv(Real t)
			{
				return (v * sqrt(1 - cos(v * PI_1) * cos(v * t))) / (cos(v * PI_1) - cos(v * t));
			}

			void UDFA::UpdateFilterParameters()
			{
				for (int i = 0; i < numFilters; i++)
				{
					filters[i].UpdateParameters(current.fc[i], current.g[i]);
				}
			}

			void UDFA::ProcessAudio(const Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor)
			{
				// Apply gain
				for (int i = 0; i < numFrames; i++)
				{
					outBuffer[i] = inBuffer[i];
					for (int j = 0; j < numFilters; j++)
						outBuffer[i] = filters[j].GetOutput(outBuffer[i]);
					outBuffer[i] *= current.gain;
					std::lock_guard<std::mutex> lock(*m);
					if (current.gain != target.gain || current.fc != target.fc || current.g != target.g)
					{
						current.gain = Lerp(current.gain, target.gain, lerpFactor);
						for (int j = 0; j < numFilters; j++)
						{
							current.fc[j] = Lerp(current.fc[j], target.fc[j], lerpFactor);
							current.g[j] = Lerp(current.g[j], target.g[j], lerpFactor);
						}
						UpdateFilterParameters();
					}
				}
			}

			//////////////////// UDFA-I class ////////////////////

			void UDFAI::UpdateParameters()
			{
				if (mPath->valid && mPath->inShadow)
				{
					UpdateConstants();
					CalcGT();
					for (int i = 0; i < numFilters; i++)
					{
						params.g[i] = gt[i + 1] / gt[i];
						gi[i] = CalcG(fi[i]) / gt[i];
						Real giSq = pow(gi[i], 2.0);
						Real gSq = pow(params.g[i], 2.0);
						params.fc[i] = fi[i] * sqrt((giSq - gSq) / (params.g[i] * (1.0 - giSq))) * (1.0 + gSq / 12.0);
					}
					params.gain = gt[0];
					std::lock_guard<std::mutex> lock(*m);
					target = params;
				}
				else
				{
					std::lock_guard<std::mutex> lock(*m);
					for (int i = 0; i < numFilters; i++)
					{
						target.fc[i] = 1000.0;
						target.g[i] = 1.0;
					}
					target.gain = 0.0;
				}
			}

			void UDFAI::UpdateConstants()
			{
				Real d = 2.0 * mPath->sData.d * mPath->rData.d / (mPath->sData.d + mPath->rData.d);
				v = PI_1 / mPath->wData.t;
				t0 = (mPath->sData.d + mPath->rData.d) / SPEED_OF_SOUND;
				front = SPEED_OF_SOUND / (PI_SQ * d * pow(sin(mPath->phi), 2.0));

				Real scale = 0;
				Real theta[2] = { mPath->sData.t + mPath->rData.t, mPath->rData.t - mPath->sData.t };
				for (int i = 0; i < 2; i++)
				{
					scale += Sign(theta[i] - PI_1) / fabs(cos(v * PI_1) - cos(v * theta[i]));
				}
				scale = pow(scale, 2.0);
				front = scale * front * pow(v * sin(v * PI_1), 2.0) / 2.0;
			}

			Complex UDFAI::CalcH(Real z, Real t, Real f)
			{
				Real fc = front;
				Real t1 = mPath->GetD(z) / SPEED_OF_SOUND;

				Real g = (2 / PI_1) * atan(PI_1 * sqrt(2 * fc * (t1 - t0)));
				fc *= (1.0 / pow(g, 2.0));
				return g * CalcUDFA(f, fc, g);
			}

#ifndef _ANDROID

			//////////////////// NN class ////////////////////

			NN::NN(Path* path) : mInput{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, target(), current(), filter(48000), mPath(path)
			{
				m = new std::mutex();
				UpdateParameters();
			};

			void NN::UpdateParameters()
			{
				if (mPath->valid && mPath->inShadow)
				{
					CalcInput();
					RunNN();
					OrderZP();
					std::lock_guard<std::mutex> lock(*m);
					target = params;
				}
				else
				{
					std::lock_guard<std::mutex> lock(*m);
					target.p[0] = 0.99;
					target.z[0] = 0.25;
					target.p[1] = -0.25;
					target.z[1] = -0.99;
					target.k = 0.0;
				}
			}

			void NN::OrderZP()
			{
				if (params.z[0] < params.z[1])
				{
					Real temp = params.z[1];
					params.z[1] = params.z[0];
					params.z[0] = temp;
				}
				if (params.p[0] < params.p[1])
				{
					Real temp = params.p[1];
					params.p[1] = params.p[0];
					params.p[0] = temp;
				}
			}

			void NN::CalcInput()
			{
				mInput[0] = static_cast<float>(mPath->wData.t);
				mInput[1] = static_cast<float>(mPath->bA);
				mInput[2] = static_cast<float>(mPath->mA);
				mInput[3] = static_cast<float>(mPath->wData.z);
				bool sourceIsOne = mPath->sData.r < mPath->rData.r;
				if (sourceIsOne)
					AssignInputRZ(&mPath->sData, &mPath->rData);
				else
					AssignInputRZ(&mPath->rData, &mPath->sData);
			}

			void NN::AssignInputRZ(SRData* one, SRData* two)
			{
				mInput[4] = static_cast<float>(one->r);
				mInput[5] = static_cast<float>(two->r);
				bool useZ0 = one->z < mPath->wData.z / 2;
				if (useZ0)
				{
					mInput[6] = static_cast<float>(one->z);
					mInput[7] = static_cast<float>(two->z);
				}
				else
				{
					mInput[6] = static_cast<float>(mPath->wData.z - one->z);
					mInput[7] = static_cast<float>(mPath->wData.z - two->z);
				}
			}

			void NN::ProcessAudio(Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor)
			{
				// Apply filter
				for (int i = 0; i < numFrames; i++)
				{
					outBuffer[i] = filter.GetOutput(inBuffer[i]);
					std::lock_guard<std::mutex> lock(*m);
					if (current.z != target.z || current.p != target.p || current.k != target.k)
					{
						for (int j = 0; j < 2; j++)
						{
							current.z[j] = Lerp(current.z[j], target.z[j], lerpFactor);
							current.p[j] = Lerp(current.p[j], target.p[j], lerpFactor);
						}
						current.k = Lerp(current.k, target.k, lerpFactor);
						filter.UpdateParameters(current);
					}
				}
			}

#endif

			//////////////////// UTD class ////////////////////

			UTD::UTD(Path* path, int fs) : lrFilter(fs), target(), current(), params(), mPath(path)
			{
				m = new std::mutex();
				for (int i = 0; i < 4; i++)
				{
					k[i] = PI_2 * lrFilter.fm[i] / SPEED_OF_SOUND;
					E[i] = exp(-1.0 * imUnit * PI_1 / 4.0) / (2.0 * sqrt(PI_2 * k[i]));
				}
				UpdateParameters();
			};

			void UTD::UpdateParameters()
			{
				if (mPath->valid && mPath->inShadow)
				{
					CalcUTD();
					std::lock_guard<std::mutex> lock(*m);
					target = params;
				}
				else
				{
					std::lock_guard<std::mutex> lock(*m);
					for (int i = 0; i < 4; i++)
						target.g[i] = 0.0;
				}
			}

			void UTD::CalcUTD()
			{
				n = mPath->wData.t / PI_1;
				Real B0 = sin(mPath->phi);
				Real dSR = mPath->sData.d + mPath->rData.d;
				Real temp = sqrt(mPath->sData.d * mPath->rData.d * dSR) * n * B0;
				L = mPath->sData.d * mPath->rData.d * pow(B0, 2.0) / (dSR);

				Real idx = (mPath->bA - PI_1) / ((Real)mPath->wData.t - (Real)mPath->sData.t - PI_1);
				for (int i = 0; i < 4; i++)
				{
					Complex A = -exp(-imUnit * k[i] * dSR) * E[i] / temp;
					g[i] = abs(A * (EqHalf(mPath->rData.t - mPath->sData.t, i) + EqHalf(mPath->rData.t + mPath->sData.t, i)));
					gSB[i] = abs(A * (EqHalf(PI_EPS, i) + EqHalf(2 * mPath->sData.t + PI_EPS, i)));
					params.g[i] = (1.0 - idx) * g[i] / gSB[i] + idx * g[i] * dSR;
				}
			}

			Complex UTD::EqHalf(Real t, const int i)
			{
				return EqQuarter(t, true, i) + EqQuarter(t, false, i);
			}

			Complex UTD::EqQuarter(Real t, bool plus, const int i)
			{
				Real cotArg = (PI_1 + PM(t, plus)) / (2.0 * n);
				if (fabs(cotArg) < 0.001f)
				{
					Real tArg = PM(-CalcTArg(t, plus), plus);
					Real eps = PI_1 + tArg;
					if (eps == 0.0)
						eps = 0.001f;
					Real kL2 = 2.0 * k[i] * L;
					return n * exp(imUnit * PI_1 / 4.0) * (sqrt(PI_1 * kL2) * Sign(eps) - kL2 * eps * exp(imUnit * PI_1 / 4.0));
				}
				return cot(cotArg) * FuncF(k[i] * L * Apm(t, plus));
			}

			Real UTD::PM(Real t, bool plus)
			{
				if (plus)
					return t;
				else
					return -t;
			}

			Real UTD::Apm(Real t, bool plus)
			{
				Real tArg = CalcTArg(t, plus);
				return 2.0 * pow(cos(tArg / 2.0), 2.0);
			}

			Real UTD::CalcTArg(Real t, bool plus)
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
				Real temp;
				Real sqrtX = sqrt(x);
				if (x < 0.8)
				{
					temp = sqrt(PI_1 * x) * (1.0 - (sqrtX / (0.7 * sqrtX + 1.2)));
				}
				else
					temp = 1 - 0.8 / pow(x + 1.25, 2.0);
				return temp * exp(imUnit * PI_1 / 4.0 * (1.0 - sqrtX / (x + 1.4)));
			}

			void UTD::ProcessAudio(Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor)
			{
				// Apply filter
				for (int i = 0; i < numFrames; i++)
				{
					outBuffer[i] = lrFilter.GetOutput(inBuffer[i]);
					std::lock_guard<std::mutex> lock(*m);
					if (current.g != target.g)
					{
						for (int j = 0; j < 4; j++)
							current.g[j] = Lerp(current.g[j], target.g[j], lerpFactor);
						lrFilter.UpdateParameters(current.g);
					}
				}
			}

			//////////////////// BTM class ////////////////////

			BTM::BTM(Path* path, int fs) : mPath(path), firFilter(currentIr)
			{
				m = new std::mutex();
				samplesPerMetre = fs / SPEED_OF_SOUND;
				UpdateParameters();
			};

			void BTM::UpdateParameters()
			{
				if (mPath->valid)
				{
					CalcBTM();
					std::lock_guard<std::mutex> lock(*m);
					if (ir.Valid())
						targetIr = ir;
				}
				// else do nothing
			}

			void BTM::InitParameters()
			{
				if (mPath->valid)
				{
					CalcBTM();
					std::lock_guard<std::mutex> lock(*m);
					if (ir.Valid())
						currentIr = ir;
				}
			}

			void BTM::CalcBTM()
			{
				int n0 = (int)floor(samplesPerMetre * (mPath->sData.d + mPath->rData.d));
				int nir = (int)ceil(samplesPerMetre * mPath->GetMaxD());
				int irLen = nir - n0;
				ir.ResizeBuffer((size_t)irLen);

				dSSq = pow(mPath->sData.d, 2.0);
				dRSq = pow(mPath->rData.d, 2.0);
				rSSq = pow(mPath->sData.r, 2.0);
				rRSq = pow(mPath->rData.r, 2.0);
				rr = mPath->sData.r * mPath->rData.r;

				zSRel = mPath->sData.z - mPath->zA;
				zRRel = mPath->rData.z - mPath->zA;
				dz = zSRel - zRRel;
				dzSq = pow(dz, 2);
				v = PI_1 / mPath->wData.t;

				edgeHi = mPath->wData.z - mPath->zA;
				edgeLo = -mPath->zA;

				Real plus = mPath->sData.t + mPath->rData.t;
				Real minus = mPath->rData.t - mPath->sData.t;
				vTheta[0] = v * (PI_1 + plus);
				vTheta[1] = v * (PI_1 + minus);
				vTheta[2] = v * (PI_1 - plus);
				vTheta[3] = v * (PI_1 - minus);

				for (int i = 0; i < 4; i++)
				{
					sinTheta[i] = sin(vTheta[i]);
					cosTheta[i] = cos(vTheta[i]);
				}

				Real d = mPath->sData.d + mPath->rData.d;
				for (int i = 0; i < irLen; i++)
				{
					int n = n0 + i;
					ir[i] = d * CalcSample(n);
				}
			}

			Real BTM::CalcSample(int n)
			{
				IntegralLimits zn1 = CalcLimits((n - 0.5) / samplesPerMetre);
				IntegralLimits zn2 = CalcLimits((n + 0.5) / samplesPerMetre);

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

				if (zn2.m < edgeLo) { zn2.m = edgeLo; }
				if (zn1.m > edgeHi) { zn1.m = edgeHi; }
				if (zn1.p < edgeLo) { zn1.p = edgeLo; }
				if (zn2.p > edgeHi) { zn2.p = edgeHi; }

				Real output = 0.0;
				if (zn2.m < zn1.m)
				{
					output += CalcIntegral(zn2.m, zn1.m);
				}
				if (zn1.p < zn2.p)
				{
					output += CalcIntegral(zn1.p, zn2.p);
				}
				output *= -v / PI_4;

				return output;
			}

			IntegralLimits BTM::CalcLimits(Real delta)
			{
				Real dSq = pow(delta, 2.0);
				Real kq = dSSq - dRSq - dSq;
				Real aq = dzSq - dSq;
				Real bq = 2.0 * dSq * zRRel - kq * dz;
				Real cq = pow(kq, 2.0) / 4.0 - dSq * dRSq;

				bq /= aq;
				cq /= aq;
				Real sq = pow(bq, 2.0) - 4.0 * cq;
				if (sq < 0.0)
				{
					return IntegralLimits(NAN, NAN);
				}
				sq = sqrt(sq);
				return IntegralLimits((-bq + sq) / 2.0, (-bq - sq) / 2.0);
			}

			Real BTM::CalcIntegral(Real zn1, Real zn2)
			{
				Real mid = (zn1 + zn2) / 2.0;
				return (zn2 - zn1) / 6.0 * (CalcIntegrand(zn1) + 4.0 * CalcIntegrand(mid) + CalcIntegrand(zn2));
			}

			Real BTM::CalcIntegrand(Real z)
			{
				Real dzS = z - zSRel;
				Real dzR = z - zRRel;

				Real dS = sqrt(pow(dzS, 2.0) + rSSq);
				Real dR = sqrt(pow(dzR, 2.0) + rRSq);

				Real ml = sqrt(pow(dzS, 2.0) + rSSq) * sqrt(pow(dzR, 2.0) + rRSq);
				Real y = std::max(1.0, (ml + dzS * dzR) / rr); // limit to 1 - real(sqrt(y ^ 2)) returns 0 if y <= 1
				Real A = y + sqrt(pow(y, 2.0) - 1.0);
				Real Apow = pow(A, v);
				Real coshvtheta = (Apow + (1.0 / Apow)) / 2.0;

				Real Btotal = 0;
				for (int i = 0; i < 4; i++)
				{
					Btotal += CalcB(i, coshvtheta);
				}
				return Btotal / ml;
			}

			Real BTM::CalcB(int i, Real coshvtheta)
			{
				return sinTheta[i] / (coshvtheta - cosTheta[i]);
			}

			void BTM::ProcessAudio(const Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor)
			{
				if (BuffersEqual(currentIr, targetIr))
				{
					for (int i = 0; i < numFrames; i++)
						outBuffer[i] = firFilter.GetOutput(inBuffer[i]);
				}
				else
				{
					std::lock_guard<std::mutex> lock(*m);
					if (currentIr.Length() != targetIr.Length())
						currentIr.ResizeBuffer(targetIr.Length());
					for (int i = 0; i < numFrames; i++)
					{
						outBuffer[i] = firFilter.GetOutput(inBuffer[i]);
						for (int j = 0; j < currentIr.Length(); j++)
							currentIr[j] = Lerp(currentIr[j], targetIr[j], lerpFactor);
						firFilter.SetImpulseResponse(currentIr);
					}
				}
			}
		}
	}
}

#pragma endregion