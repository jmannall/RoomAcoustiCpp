
#include "DiffractionModel.h"

void Attenuate::UpdateParameters()
{
	std::lock_guard<std::mutex> lock(*m);
	if (mPath->valid && mPath->inShadow)
		targetGain = 1.0f;
	else
		targetGain = 0.0f;
}

void Attenuate::ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor)
{
	// Apply gain
	for (int i = 0; i < numFrames; i++)
	{
		outBuffer[i] = inBuffer[i] * currentGain;
		std::lock_guard<std::mutex> lock(*m);
		if (currentGain != targetGain)
			currentGain = LERP_FLOAT(currentGain, targetGain, lerpFactor);
	}
}

LPF::LPF(DiffractionPath* path, int fs) : fc(1000.0f), targetGain(0.0f), currentGain(0.0f), mPath(path)
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
		targetGain = 1.0f;
	else
		targetGain = 0.0f;
}

void LPF::ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor)
{
	// Apply filter
	for (int i = 0; i < numFrames; i++)
	{
		outBuffer[i] = filter.GetOutput(inBuffer[i]) * currentGain;
		std::lock_guard<std::mutex> lock(*m);
		if (currentGain != targetGain)
			currentGain = LERP_FLOAT(currentGain, targetGain, lerpFactor);
	}
}

UDFA::UDFA(DiffractionPath* path, int fs) : numFilters(4), target(), current(), params(), mPath(path)
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
	float fMin = log10f(10.0f);
	float fMax = log10f((float)fs);

	float delta = (fMax - fMin) / numFilters;
	for (int i = 0; i <= numFilters; i++)
	{
		ft[i] = powf(10, fMin + delta * i);
	}
}

void UDFA::CalcFI()
{
	for (int i = 0; i < numFilters; i++)
	{
		fi[i] = ft[i] * sqrtf(ft[i + 1] / ft[i]);
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
			float giSq = powf(gi[i], 2);
			float gSq = powf(params.g[i], 2);
			params.fc[i] = fi[i] * sqrtf((giSq - gSq) / (params.g[i] * (1 - giSq))) * (1 + gSq / 12);
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
			target.fc[i] = 1000.0f;
			target.g[i] = 1.0f;
		}
		target.gain = 0.0f;
	}
}

void UDFA::UpdateConstants()
{
	float d = 2 * mPath->sData.d * mPath->rData.d / (mPath->sData.d + mPath->rData.d);
	v = PI / mPath->wData.t;
	t0 = (mPath->sData.d + mPath->rData.d) / SPEED_OF_SOUND;
	front = 2 * SPEED_OF_SOUND / (PI_SQ * d * powf(sinf(mPath->phi), 2));
}

void UDFA::CalcGT()
{
	for (int i = 0; i <= numFilters; i++)
	{
		gt[i] = CalcG(ft[i]);
	}
}

float UDFA::CalcG(float f)
{
	return abs((CalcHpm(0.0f, f) + CalcHpm(mPath->wData.z, f)) / complex(4.0f, 0.0f));
}

complex UDFA::CalcHpm(float z, float f)
{
	return CalcH(z, mPath->sData.t + mPath->rData.t, f) + CalcH(z, mPath->rData.t - mPath->sData.t, f);
}

complex UDFA::CalcH(float z, float t, float f)
{
	float fc = front * powf(CalcNv(t), 2);

	float t1 = mPath->GetD(z) / SPEED_OF_SOUND;

	float g = (2 / PI) * atanf(PI * sqrtf(2 * fc * (t1 - t0)));
	fc *= (1 / powf(g, 2));
	return g * CalcUDFA(f, fc, g);
}

complex UDFA::CalcUDFA(float f, float fc, float g)
{
	float alpha = 0.5f;
	float b = 1.44f;
	float Q = 0.2f;
	float r = 1.6f;

	float gSq = powf(g, 2);

	b = 1 + (b - 1) * gSq;
	Q = 0.5f + (Q - 0.5f) * gSq;

	return pow(pow(imUnit * f / fc, 2 / b) + pow(imUnit * f / (Q * fc), 1 / (powf(b, r))) + complex(1.0f, 0.0f), -alpha * b / 2);
}

float UDFA::CalcNv(float t)
{
	return (v * sqrtf(1 - cosf(v * PI) * cosf(v * t))) / (cosf(v * PI) - cosf(v * t));
}

void UDFA::UpdateFilterParameters()
{
	for (int i = 0; i < numFilters; i++)
	{
		filters[i].UpdateParameters(current.fc[i], current.g[i]);
	}
}

void UDFA::ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor)
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
			current.gain = LERP_FLOAT(current.gain, target.gain, lerpFactor);
			for (int j = 0; j < numFilters; j++)
			{
				current.fc[j] = LERP_FLOAT(current.fc[j], target.fc[j], lerpFactor);
				current.g[j] = LERP_FLOAT(current.g[j], target.g[j], lerpFactor);
			}
			UpdateFilterParameters();
		}
	}
}

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
			float giSq = powf(gi[i], 2);
			float gSq = powf(params.g[i], 2);
			params.fc[i] = fi[i] * sqrtf((giSq - gSq) / (params.g[i] * (1 - giSq))) * (1 + gSq / 12);
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
			target.fc[i] = 1000.0f;
			target.g[i] = 1.0f;
		}
		target.gain = 0.0f;
	}
}

void UDFAI::UpdateConstants()
{
	float d = 2 * mPath->sData.d * mPath->rData.d / (mPath->sData.d + mPath->rData.d);
	v = PI / mPath->wData.t;
	t0 = (mPath->sData.d + mPath->rData.d) / SPEED_OF_SOUND;
	front = SPEED_OF_SOUND / (PI_SQ * d * powf(sinf(mPath->phi), 2));

	float scale = 0;
	float theta[2] = { mPath->sData.t + mPath->rData.t, mPath->rData.t - mPath->sData.t };
	for (int i = 0; i < 2; i++)
	{
		scale += Sign(theta[i] - PI) / fabs(cosf(v * PI) - cosf(v * theta[i]));
	}
	scale = powf(scale, 2);
	front = scale * front * powf(v * sinf(v * PI), 2) / 2;
}

complex UDFAI::CalcH(float z, float t, float f)
{
	float fc = front;
	float t1 = mPath->GetD(z) / SPEED_OF_SOUND;

	float g = (2 / PI) * atanf(PI * sqrtf(2 * fc * (t1 - t0)));
	fc *= (1 / powf(g, 2));
	return g * CalcUDFA(f, fc, g);
}

NN::NN(DiffractionPath* path) : mInput{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, target(), current(), filter(48000), mPath(path)
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
		target.p[0] = 0.99f;
		target.z[0] = 0.25f;
		target.p[1] = -0.25f;
		target.z[1] = -0.99f;
		target.k = 0.0f;
	}
}

void NN::OrderZP()
{
	if (params.z[0] < params.z[1])
	{
		float temp = params.z[1];
		params.z[1] = params.z[0];
		params.z[0] = temp;
	}
	if (params.p[0] < params.p[1])
	{
		float temp = params.p[1];
		params.p[1] = params.p[0];
		params.p[0] = temp;
	}
}

void NN::CalcInput()
{
	mInput[0] = mPath->wData.t;
	mInput[1] = mPath->bA;
	mInput[2] = mPath->mA;
	mInput[3] = mPath->wData.z;
	bool sourceIsOne = mPath->sData.r < mPath->rData.r;
	if (sourceIsOne)
		AssignInputRZ(&mPath->sData, &mPath->rData);
	else
		AssignInputRZ(&mPath->rData, &mPath->sData);
}

void NN::AssignInputRZ(SRData* one, SRData* two)
{
	mInput[4] = one->r;
	mInput[5] = two->r;
	bool useZ0 = one->z < mPath->wData.z / 2;
	if (useZ0)
	{
		mInput[6] = one->z;
		mInput[7] = two->z;
	}
	else
	{
		mInput[6] = mPath->wData.z - one->z;
		mInput[7] = mPath->wData.z - two->z;
	}
}

void NN::ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor)
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
				current.z[j] = LERP_FLOAT(current.z[j], target.z[j], lerpFactor);
				current.p[j] = LERP_FLOAT(current.p[j], target.p[j], lerpFactor);
			}
			current.k = LERP_FLOAT(current.k, target.k, lerpFactor);
			filter.UpdateParameters(current);
		}
	}
}

UTD::UTD(DiffractionPath* path, int fs) : lrFilter(fs), target(), current(), params(), mPath(path)
{
	m = new std::mutex();
	for (int i = 0; i < 4; i++)
	{
		k[i] = PI_2 * lrFilter.fm[i] / SPEED_OF_SOUND;
		E[i] = exp(-1.0f * imUnit * PI / 4.0f) / (2.0f * sqrtf(PI_2 * k[i]));
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
			target.g[i] = 0.0f;
	}
}

void UTD::CalcUTD()
{
	n = mPath->wData.t / PI;
	float B0 = sinf(mPath->phi);
	float dSR = mPath->sData.d + mPath->rData.d;
	float temp = sqrtf(mPath->sData.d * mPath->rData.d * dSR) * n * B0;
	L = mPath->sData.d * mPath->rData.d * powf(B0, 2) / (dSR);

	float idx = (mPath->bA - PI) / ((float)mPath->wData.t - (float)mPath->sData.t - PI);
	for (int i = 0; i < 4; i++)
	{
		complex A = -exp(-imUnit * k[i] * dSR) * E[i] / temp;
		g[i] = abs(A * (EqHalf(mPath->rData.t - mPath->sData.t, i) + EqHalf(mPath->rData.t + mPath->sData.t, i)));
		gSB[i] = abs(A * (EqHalf(PI_EPS, i) + EqHalf(2 * mPath->sData.t + PI_EPS, i)));
		params.g[i] = (1 - idx) * g[i] / gSB[i] + idx * g[i] * dSR;
	}
}

complex UTD::EqHalf(float t, const int i)
{
	return EqQuarter(t, true, i) + EqQuarter(t, false, i);
}

complex UTD::EqQuarter(float t, bool plus, const int i)
{
	float cotArg = (PI + PM(t, plus)) / (2 * n);
if (fabs(cotArg) < 0.001f)
{
	float tArg = PM(-CalcTArg(t, plus), plus);
	float eps = PI + tArg;
	if (eps == 0)
		eps = 0.001f;
	float kL2 = 2 * k[i] * L;
	return n * exp(imUnit * PI / 4.0f) * (sqrtf(PI * kL2) * Sign(eps) - kL2 * eps * exp(imUnit * PI / 4.0f));
}
return cot(cotArg) * FuncF(k[i] * L * Apm(t, plus));
}

float UTD::PM(float t, bool plus)
{
	if (plus)
		return t;
	else
		return -t;
}

float UTD::Apm(float t, bool plus)
{
	float tArg = CalcTArg(t, plus);
	return 2.0f * powf(cosf(tArg / 2.0f), 2);
}

float UTD::CalcTArg(float t, bool plus)
{
	float N;
	float PI_2n = PI_2 * n;
	if (plus)
		N = roundf((PI + t) / PI_2n);
	else
		N = roundf((-PI + t) / PI_2n);
	return PI_2n * N - t;
}

complex UTD::FuncF(float x)
{
	float temp;
	float sqrtX = sqrtf(x);
	if (x < 0.8)
	{
		temp = sqrtf(PI * x) * (1.0f - (sqrtX / (0.7f * sqrtX + 1.2f)));
	}
	else
		temp = 1 - 0.8f / powf(x + 1.25f, 2);
	return temp * exp(imUnit * PI / 4.0f * (1.0f - sqrtX / (x + 1.4f)));
}

void UTD::ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor)
{
	// Apply filter
	for (int i = 0; i < numFrames; i++)
	{
		outBuffer[i] = lrFilter.GetOutput(inBuffer[i]);
		std::lock_guard<std::mutex> lock(*m);
		if (current.g != target.g)
		{
			for (int j = 0; j < 4; j++)
				current.g[j] = LERP_FLOAT(current.g[j], target.g[j], lerpFactor);
			lrFilter.UpdateParameters(current.g);
		}
	}
}

BTM::BTM(DiffractionPath* path, int fs) : mPath(path), firFilter(currentIr)
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
	else
	{
		std::lock_guard<std::mutex> lock(*m);
		targetIr.ResetBuffer();
	}
}

void BTM::CalcBTM()
{
	int n0 = (int)floor(samplesPerMetre * (mPath->sData.d + mPath->rData.d));
	int nir = (int)ceil(samplesPerMetre * mPath->GetMaxD());
	int irLen = nir - n0;
	ir.ResizeBuffer((size_t)irLen);

	dSSq = powf(mPath->sData.d, 2);
	dRSq = powf(mPath->rData.d, 2);
	rSSq = powf(mPath->sData.r, 2);
	rRSq = powf(mPath->rData.r, 2);
	rr = mPath->sData.r * mPath->rData.r;

	zSRel = mPath->sData.z - mPath->zA;
	zRRel = mPath->rData.z - mPath->zA;
	dz = zSRel - zRRel;
	dzSq = powf(dz, 2);
	v = PI / mPath->wData.t;

	edgeHi = mPath->wData.z - mPath->zA;
	edgeLo = -mPath->zA;

	float plus = mPath->sData.t + mPath->rData.t;
	float minus = mPath->rData.t - mPath->sData.t;
	vTheta[0] = v * (PI + plus);
	vTheta[1] = v * (PI + minus);
	vTheta[2] = v * (PI - plus);
	vTheta[3] = v * (PI - minus);

	for (int i = 0; i < 4; i++)
	{
		sinTheta[i] = sinf(vTheta[i]);
		cosTheta[i] = cosf(vTheta[i]);
	}

	float d = mPath->sData.d + mPath->rData.d;
	for (int i = 0; i < irLen; i++)
	{
		int n = n0 + i;
		ir[i] = d * CalcSample(n);
	}
}

float BTM::CalcSample(int n)
{
	IntegralLimits zn1 = CalcLimits((n - 0.5f) / samplesPerMetre);
	IntegralLimits zn2 = CalcLimits((n + 0.5f) / samplesPerMetre);

	if (isnan(zn2.p))
	{
		// Both limits of integration are imaginary
		// Entire sample has no existing edge
		return 0.0f;
	}
	if (isnan(zn1.p))
	{
		// Only lower limit of integration is imaginary
		// Instead start integrating at apex point
		zn1.p = 0.0f;
		zn1.m = 0.0f;
	}

	// Check ranges against edge boundries
	// The two ranges are [zn2.m, zn1.m] and [zn1.p, zn2.p] (neg to pos)

	if (zn2.m < edgeLo) { zn2.m = edgeLo; }
	if (zn1.m > edgeHi) { zn1.m = edgeHi; }
	if (zn1.p < edgeLo) { zn1.p = edgeLo; }
	if (zn2.p > edgeHi) { zn2.p = edgeHi; }

	float output = 0.0f;
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

IntegralLimits BTM::CalcLimits(float delta)
{
	float dSq = powf(delta, 2);
	float kq = dSSq - dRSq - dSq;
	float aq = dzSq - dSq;
	float bq = 2.0f * dSq * zRRel - kq * dz;
	float cq = powf(kq, 2) / 4.0f - dSq * dRSq;

	bq /= aq;
	cq /= aq;
	float sq = powf(bq, 2) - 4.0f * cq;
	if (sq < 0.0f)
	{
		return IntegralLimits(NAN, NAN);
	}
	sq = sqrt(sq);
	return IntegralLimits((-bq + sq) / 2.0f, (-bq - sq) / 2.0f);
}

float BTM::CalcIntegral(float zn1, float zn2)
{
	float mid = (zn1 + zn2) / 2.0f;
	return (zn2 - zn1) / 6.0f * (CalcIntegrand(zn1) + 4.0f * CalcIntegrand(mid) + CalcIntegrand(zn2));
}

float BTM::CalcIntegrand(float z)
{
	float dzS = z - zSRel;
	float dzR = z - zRRel;

	float dS = sqrt(powf(dzS, 2) + rSSq);
	float dR = sqrt(powf(dzR, 2) + rRSq);

	float dSdR = dS * dR;
	float y = (dSdR + dzS * dzR) / rr;
	float A = y + sqrt(powf(y, 2) - 1.0f);
	float Apow = powf(A, v);
	float coshvtheta = (Apow + (1.0f / Apow)) / 2.0f;

	float Btotal = 0;
	for (int i = 0; i < 4; i++)
	{
		Btotal += CalcB(i, coshvtheta);
	}
	return Btotal / dSdR;
}

float BTM::CalcB(int i, float coshvtheta)
{
	return sinTheta[i] / (coshvtheta - cosTheta[i]);
}

//void BTM::ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor)
//{
//	// Apply filter
//	for (int i = 0; i < numFrames; i++)
//	{
//		outBuffer[i] = firFilter.Filter(inBuffer[i]);
//		firFilter.SetImpulseResponse(ir.GetBuffer(), lerpFactor);
//	}
//}

void BTM::ProcessAudio(float* inBuffer, float* outBuffer, int numFrames, float lerpFactor)
{
	// Apply filter
	for (int i = 0; i < numFrames; i++)
	{
		outBuffer[i] = firFilter.GetOutput(inBuffer[i]);
		std::lock_guard<std::mutex> lock(*m);
		if (!BuffersEqual(currentIr, targetIr))
		{
			for (int j = 0; j < currentIr.Length(); j++)
				if (!std::isnan(targetIr[j]))
					currentIr[j] = LERP_FLOAT(currentIr[j], targetIr[j], lerpFactor);
			firFilter.SetImpulseResponse(currentIr);
		}
	}
}