/*
*
*  \Diffraction models
*
*/

// Common headers
#include "Common/AudioManager.h"

// Spatialiser headers
#include "Spatialiser/Diffraction/Models.h"
#include "Spatialiser/Types.h"

// Unity headers
#include "Unity/UnityInterface.h"

// DSP headers
#include "DSP/Interpolate.h"

namespace UIE
{
	using namespace Common;
	using namespace DSP;
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
						Lerp(currentGain, targetGain, lerpFactor);
				}
			}

			//////////////////// LPF class ////////////////////

			LPF::LPF(Path* path, int fs) : fc(1000.0), targetGain(0.0), currentGain(0.0), mPath(path), filter(fc, fs)
			{
				m = new std::mutex();
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
						Lerp(currentGain, targetGain, lerpFactor);
				}
			}

			//////////////////// UDFA class ////////////////////

			UDFA::UDFA(Path* path, int fs) : numFilters(4), target(), current(), params(), mPath(path)
			{
				m = new std::mutex();

				filters = std::vector<HighShelf>(numFilters, HighShelf(fs));
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
						Real giSq = gi[i] * gi[i];
						Real gSq = params.g[i] * params.g[i];
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
				Real sinPhi = sin(mPath->phi);
				front = 2.0 * SPEED_OF_SOUND / (PI_SQ * d * sinPhi * sinPhi);
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
				Real nV = CalcNv(t);
				Real fc = front * nV * nV;

				Real t1 = mPath->GetD(z) / SPEED_OF_SOUND;

				Real g = (2.0 / PI_1) * atan(PI_1 * sqrt(2.0 * fc * (t1 - t0)));
				fc *= (1 / (g * g));
				return g * CalcUDFA(f, fc, g);
			}

			Complex UDFA::CalcUDFA(Real f, Real fc, Real g)
			{
				Real alpha = 0.5;
				Real b = 1.44;
				Real Q = 0.2;
				Real r = 1.6;

				Real gSq = g * g;

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
						Lerp(current.gain, target.gain, lerpFactor);
						for (int j = 0; j < numFilters; j++)
						{
							Lerp(current.fc[j], target.fc[j], lerpFactor);
							Lerp(current.g[j], target.g[j], lerpFactor);
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
						Real giSq = gi[i] * gi[i];
						Real gSq = params.g[i] * params.g[i];
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
				Real sinPhi = sin(mPath->phi);
				front = SPEED_OF_SOUND / (PI_SQ * d * sinPhi * sinPhi);

				Real scale = 0;
				Real theta[2] = { mPath->sData.t + mPath->rData.t, mPath->rData.t - mPath->sData.t };
				for (int i = 0; i < 2; i++)
				{
					scale += Sign(theta[i] - PI_1) / fabs(cos(v * PI_1) - cos(v * theta[i]));
				}
				scale *= scale;
				Real vSin = v * sin(v * PI_1);
				front = scale * front * vSin * vSin / 2.0;
			}

			Complex UDFAI::CalcH(Real z, Real t, Real f)
			{
				Real fc = front;
				Real t1 = mPath->GetD(z) / SPEED_OF_SOUND;

				Real g = (2 / PI_1) * atan(PI_1 * sqrt(2 * fc * (t1 - t0)));
				fc *= (1.0 / (g * g));
				return g * CalcUDFA(f, fc, g);
			}

// #ifndef _ANDROID

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
							Lerp(current.z[j], target.z[j], lerpFactor);
							Lerp(current.p[j], target.p[j], lerpFactor);
						}
						Lerp(current.k, target.k, lerpFactor);
						filter.UpdateParameters(current);
					}
				}
			}

// #endif

			//////////////////// UTD class ////////////////////

			UTD::UTD(Path* path, int fs) : lrFilter(fs), target(4, 0.0), current(4, 0.0), params(4, 0.0), mPath(path)
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
						target[i] = 0.0;
				}
			}

			void UTD::CalcUTD()
			{
				n = mPath->wData.t / PI_1;
				Real B0 = sin(mPath->phi);
				Real dSR = mPath->sData.d + mPath->rData.d;
				Real temp = sqrt(mPath->sData.d * mPath->rData.d * dSR) * n * B0;
				L = mPath->sData.d * mPath->rData.d * B0 * B0 / (dSR);

				Real idx = (mPath->bA - PI_1) / ((Real)mPath->wData.t - (Real)mPath->sData.t - PI_1);
				for (int i = 0; i < 4; i++)
				{
					Complex A = -exp(-imUnit * k[i] * dSR) * E[i] / temp;
					g[i] = abs(A * (EqHalf(mPath->rData.t - mPath->sData.t, i) + EqHalf(mPath->rData.t + mPath->sData.t, i)));
					gSB[i] = abs(A * (EqHalf(PI_EPS, i) + EqHalf(2 * mPath->sData.t + PI_EPS, i)));
					params[i] = (1.0 - idx) * g[i] / gSB[i] + idx * g[i] * dSR;
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
				Real cosArg = cos(tArg / 2.0);
				return 2.0 * cosArg * cosArg;
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
				{
					Real store = x + 1.25;
					temp = 1 - 0.8 / (store * store);
				}
				return temp * exp(imUnit * PI_1 / 4.0 * (1.0 - sqrtX / (x + 1.4)));
			}

			void UTD::ProcessAudio(Real* inBuffer, Real* outBuffer, int numFrames, Real lerpFactor)
			{
				// Apply filter
				for (int i = 0; i < numFrames; i++)
				{
					outBuffer[i] = lrFilter.GetOutput(inBuffer[i]);
					std::lock_guard<std::mutex> lock(*m);
					if (current != target)
					{
						for (int j = 0; j < 4; j++)
							Lerp(current[j], target[j], lerpFactor);
						lrFilter.UpdateParameters(current);
					}
				}
			}

			//////////////////// BTM class ////////////////////

			BTM::BTM(Path* path, int fs) : mPath(path), firFilter(currentIr)
			{
				m = new std::mutex();
				samplesPerMetre = fs * INV_SPEED_OF_SOUND;
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
					size_t currentLen = currentIr.Length();
					size_t targetLen = targetIr.Length();

					if (currentLen % 8 != 0)
					{
						currentLen += 8 - currentLen % 8;
						currentIr.ResizeBuffer(currentLen);
					}
					if (targetLen % 8 != 0)
					{
						targetLen += 8 - targetLen % 8;
						targetIr.ResizeBuffer(targetLen);
					}

					if (currentLen > targetLen)
					{
						Real total = 0.0;
						int i = targetLen - 1;
						while (i < currentLen)
						{
							total += currentIr[i++];
							total += currentIr[i++];
							total += currentIr[i++];
							total += currentIr[i++];
							total += currentIr[i++];
							total += currentIr[i++];
							total += currentIr[i++];
							total += currentIr[i++];
							if (total <= MIN_VALUE)
							{
								i -= 8;
								break;
							}
							total = 0.0;
						}
						currentIr.ResizeBuffer(i);
						targetIr.ResizeBuffer(i);	
					}
					else if (currentLen < targetLen)
						currentIr.ResizeBuffer(targetLen);

					firFilter.Resize(currentIr.Length());
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
					{
						currentIr = ir;
						firFilter.SetImpulseResponse(currentIr);
					}
				}
			}

			void BTM::CalcBTM()
			{
				Real R0 = mPath->sData.d + mPath->rData.d;

				int n0 = (int)round(samplesPerMetre * R0);
				int nir = (int)round(samplesPerMetre * mPath->GetMaxD());
				int irLen = nir - n0;
				ir.ResizeBuffer((size_t)irLen);

				dSSq = mPath->sData.d * mPath->sData.d;
				dRSq = mPath->rData.d * mPath->rData.d;
				rSSq = mPath->sData.r * mPath->sData.r;
				rRSq = mPath->rData.r * mPath->rData.r;
				rr = mPath->sData.r * mPath->rData.r;

				zSRel = mPath->sData.z - mPath->zA;
				zRRel = mPath->rData.z - mPath->zA;
				dz = zSRel - zRRel;
				dzSq = dz * dz;
				v = PI_1 / mPath->wData.t;
				Real vSq = v * v;

				edgeHi = mPath->wData.z - mPath->zA;
				edgeLo = -mPath->zA;

				Real plus = mPath->sData.t + mPath->rData.t;
				Real minus = mPath->sData.t - mPath->rData.t;

				theta[0] = PI_1 + plus;
				theta[1] = PI_1 + minus;
				theta[2] = PI_1 - minus;
				theta[3] = PI_1 - plus;

				Real x = (n0 + 0.5) / samplesPerMetre;
				Real xSq = x * x;
				Real MSq = rSSq + zSRel * zSRel;
				Real LSq = rRSq + zRRel * zRRel;
				Real K = MSq - LSq - xSq;
				Real denom = dzSq - xSq;
				Real a = (2.0 * xSq * zRRel - K * dz) / denom;
				Real b = ((K * K / 4) - xSq * LSq) / denom;
				Real zRangeApex = -a / 2.0 + sqrt(a * a / 4.0 - b);
				Real zRange = 0.1 * std::min(mPath->sData.r, mPath->rData.r);

				bool splitIntegral = true;
				if (zRange > zRangeApex)
				{
					zRange = abs(zRangeApex);
					splitIntegral = false;
				}

				Real rho = mPath->rData.r / mPath->sData.r;
				Real rhoOne = rho + 1.0;
				Real rhoOneSq = rhoOne * rhoOne;
				Real sinPsi = (mPath->sData.r + mPath->rData.r) / R0;
				Real tempFact = rhoOneSq * sinPsi * sinPsi - 2.0 * rho;
				Real sqrtB3 = SQRT_2 * R0 * rho / rhoOne / sqrt(tempFact);
				Real temp3vec = -1.0 / sqrtB3 * atan(zRange / sqrtB3);
				ir[0] = 0.0;

				for (int i = 0; i < 4; i++)
				{
					vTheta = v * theta[i];
					absvTheta = abs(vTheta);
					absvThetaPi = abs(vTheta - PI_2);
					sinTheta[i] = sin(vTheta);
					cosTheta[i] = cos(vTheta);
					singular = absvTheta < MIN_VALUE || absvThetaPi < MIN_VALUE;

					if (absvTheta < 0.01)
					{
						Real store = 1.0 - absvTheta * absvTheta / 12;
						sqrtB1vec = theta[i] * sqrt(store) * R0 * rho / rhoOneSq;
						fifactvec = (theta[i] * theta[i]) / 2.0 * store;
					}
					else if (absvThetaPi < 0.01)
					{
						Real store1 = theta[i] - PI_2 / v;
						Real store2 = 1.0 - absvThetaPi * absvThetaPi / 12;
						sqrtB1vec = store1 * sqrt(store2) * R0 * rho / rhoOneSq;
						fifactvec = store1 * store1 / 2.0 * (store2);
					}
					else
					{
						Real store = 1 - cosTheta[i];
						sqrtB1vec = sqrt(2 * store) * R0 * rho / rhoOneSq / v;
						fifactvec = store / vSq;
					}

					temp1vec = sinTheta[i] / (rhoOneSq - tempFact * fifactvec + MIN_VALUE);
					temp1_2vec = (sinTheta[i] + MIN_VALUE) / (rhoOneSq - tempFact * fifactvec) / (sqrtB1vec + MIN_VALUE) * atan(zRange / (sqrtB1vec + DBL_MIN));
					sampleOneVec[i] = 2.0 / vSq * rho * (temp1_2vec + temp1vec * temp3vec);
					// Check for special case - see EDwedge1st_ir.m
					if (!singular)
						ir[0] += sampleOneVec[i];
				}


				if (splitIntegral)
				{
					ir[0] += CalcIntegral(zRange, zRangeApex);
				}
				Real d = mPath->sData.d + mPath->rData.d; // Remove 1 / r as applied by HRTF processing
				ir[0] *= -v * d / PI_2; // Multiply by 2 for pos and neg wedge part - add check for edge hi and lo?

				for (int i = 1; i < irLen; i++)
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

				Real output = 0.0;
				if (zn2.m >= edgeLo)
				{
					if (zn2.p <= edgeHi)
					{
						output = CalcIntegral(zn1.p, zn2.p);
						output *= -v / PI_2;
					}

					else
					{
						output += CalcIntegral(zn2.m, zn1.m);
						if (zn1.p < edgeHi)
							output += CalcIntegral(zn1.p, edgeHi);
						output *= -v / PI_4;
					}
				}
				else
				{
					if (zn1.m > edgeLo)
						output += CalcIntegral(edgeLo, zn1.m);
					if (zn2.p <= edgeHi)
						output += CalcIntegral(zn1.p, zn2.p);
					else if (zn1.p < edgeHi)
						output += CalcIntegral(zn1.p, edgeHi);
					output *= -v / PI_4;
				}

				//if (zn2.m < edgeLo) { zn2.m = edgeLo; }
				//if (zn1.m > edgeHi) { zn1.m = edgeHi; }
				//if (zn1.p < edgeLo) { zn1.p = edgeLo; }
				//if (zn2.p > edgeHi) { zn2.p = edgeHi; }

				//if (zn2.m < zn1.m)
				//{
				//	output += CalcIntegral(zn2.m, zn1.m);
				//}
				//if (zn1.p < zn2.p)
				//{
				//	output += CalcIntegral(zn1.p, zn2.p);
				//}
				//output *= -v / PI_4;

				return output;
			}

			IntegralLimits BTM::CalcLimits(Real delta)
			{
				Real dSq = delta * delta;
				Real kq = dSSq - dRSq - dSq;
				Real aq = dzSq - dSq;
				Real bq = 2.0 * dSq * zRRel - kq * dz;
				Real cq = (kq * kq) / 4.0 - dSq * dRSq;

				bq /= aq;
				cq /= aq;
				Real sq = (bq * bq) - 4.0 * cq;
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

				// Quadrature rule
				//Real n = 6.0;
				//Real output = (CalcIntegrand(zn1) + CalcIntegrand(zn2)) / 2.0;
				//for (int i = 1; i < n; i++)
				//{
				//	output += CalcIntegrand(zn1 + i * (zn2 - zn1) / n);
				//}
				//return (zn2 - zn1) / n * output;

				// Simpson's rule
				return (zn2 - zn1) / 6.0 * (CalcIntegrand(zn1) + 4.0 * CalcIntegrand(mid) + CalcIntegrand(zn2));
			}

			Real BTM::CalcIntegrand(Real z)
			{
				Real dzS = z - zSRel;
				Real dzR = z - zRRel;

				Real dzSSq = dzS * dzS;
				Real dzRSq = dzR * dzR;

				Real dS = sqrt(dzSSq + rSSq);
				Real dR = sqrt(dzRSq + rRSq);

				Real ml = dS * dR;
				Real y = std::max(1.0, (ml + dzS * dzR) / rr); // limit to 1 -> real(sqrt(y ^ 2 - 1)) returns 0 if y <= 1
				Real A = y + sqrt(y * y - 1.0);
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

			void BTM::ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor)
			{
				if (currentIr == targetIr)
				{
#ifdef PROFILE_AUDIO_THREAD
					BeginFIR();
#endif
					for (int i = 0; i < numFrames; i++)
					{
						outBuffer[i] = firFilter.GetOutput(inBuffer[i]);
					}
#ifdef PROFILE_AUDIO_THREAD
					EndFIR();
#endif

					/*BeginFIR();
					for (int i = 0; i < numFrames; i++)
					{
						outBuffer[i] = mclFirFilter.Filter(inBuffer[i]);
					}
					EndFIR();*/
				}
				else
				{
					std::lock_guard<std::mutex> lock(*m);

					if (currentIr.Length() != targetIr.Length())
						currentIr.ResizeBuffer(targetIr.Length());
#ifdef PROFILE_AUDIO_THREAD
					BeginLerp();
#endif
					for (int i = 0; i < numFrames; i++)
					{
						outBuffer[i] = firFilter.GetOutput(inBuffer[i]);
						Lerp(currentIr, targetIr, lerpFactor);
						firFilter.SetImpulseResponse(currentIr);
					}
#ifdef PROFILE_AUDIO_THREAD
					EndLerp();
#endif
					/*BeginLerp();
					for (int i = 0; i < numFrames; i++)
					{
						outBuffer[i] = mclFirFilter.Filter(inBuffer[i]);
						Lerp(currentIr, targetIr, lerpFactor);
						mclFirFilter.set_impulse_response(currentIr.GetBuffer());
					}
					EndLerp();*/
				}
			}
		}
	}
}

#pragma endregion