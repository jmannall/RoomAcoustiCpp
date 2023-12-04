
#include "AudioManager.h"

#pragma region Buffer
Buffer::Buffer()
{
	InitialiseBuffer(512);
}

Buffer::Buffer(int n)
{
	InitialiseBuffer(n);
}

void Buffer::ResizeBuffer(size_t numSamples)
{
	size_t capacity = mBuffer.capacity();
	if (capacity < numSamples)
	{
		mBuffer.reserve(numSamples);
		std::fill_n(std::back_inserter(mBuffer), numSamples - capacity, 0.0f);
	}
	else
	{
		mBuffer.resize(numSamples);
	}
}

void Buffer::ResetBuffer()
{
	std::fill(mBuffer.begin(), mBuffer.end(), 0.0f);
};

void Buffer::InitialiseBuffer(int n)
{
	mBuffer.reserve(n);
	std::fill_n(std::back_inserter(mBuffer), n, 0.0f);
}

bool Buffer::Valid()
{
	for (int i = 0; i < mBuffer.size(); i++)
	{
		if(std::isnan(mBuffer[i]))
			return false;
	}
	return true;
}

bool BuffersEqual(Buffer x, Buffer y)
{
	if (x.Length() != y.Length())
	{
		return false;
	}
	for (int i = 0; i < x.Length(); i++)
	{
		if (x[i] != y[i])
		{
			return false;
		}
	}
	return true;
}
#pragma endregion

#pragma region FIRFilter
float FIRFilter::GetOutput(float input)
{
	x[count] = input;
	float output = 0.0f;
	size_t xLen = x.Length();
	if (irLen > xLen)
	{
		x.ResizeBuffer(irLen);
	}
	else if(irLen < xLen)
	{
		Buffer store = x;
		for (int i = 0; i < xLen; i++)
		{
			x[i] = store[(count + i) % xLen];
		}
		x.ResizeBuffer(irLen);
		count = 0;
	}
	for (int i = 0; i <= count; i++)
	{
		output += x[count - i] * mIr[i];
	}
	int n = irLen + count;
	for (int i = count + 1; i < irLen; i++)
	{
		output += x[n - i] * mIr[i];
	}

	//for (int i = 0; i < count; i++)
	//{
	//	output += x[i] * mIr[count - i];
	//}
	//for (int i = 0; i < irLen - count; i++)
	//{
	//	output += x[count + i] * mIr[count + i];
	//}
	count = (count + 1) % (int)irLen;
	return output;
}
#pragma endregion

#pragma region IIRFilter
float IIRFilter::GetOutput(float input)
{
	//std::cout << "b: " << b[0] << ", " << b[1] << "\n";
	//std::cout << "a: " << a[0] << ", " << a[1] << "\n";
	x[0] = input;
	y[0] = 0.0f;
	for (int i = 0; i < order; i++)
	{
		y[0] += b[i] * x[i] - a[i + 1] * y[i + 1];
		//y[0] += b[i] * x[i] + a[i + 1] * y[i + 1];
	}
	y[0] += b[order] * x[order];

	//std::cout << "x: " << x[0] << ", " << x[1] << "\n";
	//std::cout << "y: " << y[0] << ", " << y[1] << "\n";
	for (int i = order; i > 0; i--)
	{
		x[i] = x[i - 1];
		y[i] = y[i - 1];
	}
	return y[0];
}

void IIRFilter::SetT(int fs)
{
	T = 1 / (float)fs;
}
#pragma endregion

#pragma region ParametricEQ
ParametricEQ::ParametricEQ(size_t order) : mOrder(order), numFilters(4), mGain(0.0f)
{
	InitBands(48000);
}

ParametricEQ::ParametricEQ(size_t order, int fs) : mOrder(order), numFilters(4), mGain(0.0f)
{
	InitBands(fs);
}

ParametricEQ::ParametricEQ(size_t order, float fc[], float gain[], int fs) : mOrder(order), numFilters(4)
{
	InitBands(fs);
	UpdateParameters(fc, gain);
}

void ParametricEQ::UpdateParameters(const float fc[], float gain[])
{
	mGain = gain[numFilters];
	for (int i = 0; i < numFilters + 1; i++)
	{
		gain[i] = std::max(EPS, gain[i]); // Prevent division by zero
	}
	for (int i = 0; i < numFilters; i++)
	{
		fb[i] = fc[i] * sqrtf(fc[i + 1] / fc[i]);
		g[i] = gain[i] / gain[i + 1];
		// g[i] = gain[i + 1] / gain[i];
		bands[i].UpdateParameters(fb[i], g[i], FilterShape(FilterShape::lbf));
	}
}

float ParametricEQ::GetOutput(const float input)
{
	float out = input;
	for (int i = 0; i < numFilters; i++)
	{
		out = bands[i].GetOutput(out);
	}
	return mGain * out;
}

void ParametricEQ::InitBands(int fs)
{
	for (int i = 0; i < numFilters; i++)
	{
		bands[i].InitFilters(mOrder, fs);
	}
}
#pragma endregion

#pragma region BandPass
BandPass::BandPass() : numFilters(0), M(0) {};

BandPass::BandPass(size_t order)
{
	InitFilters(order, 48000);
};

BandPass::BandPass(size_t order, int fs)
{
	InitFilters(order, fs);
};

BandPass::BandPass(size_t order, FilterShape shape, float fb, float g, int fs)
{
	InitFilters(order, fs);
	UpdateParameters(fb, g, shape);
};

void BandPass::InitFilters(int order, int fs)
{
	M = order;
	numFilters = order / 2.0f;
	filters.reserve(numFilters);
	std::fill_n(std::back_inserter(filters), numFilters, TransDF2(fs));
}

void BandPass::UpdateParameters(float fb, float g, FilterShape shape)
{
	for (int i = 0; i < numFilters; i++)
	{
		filters[i].UpdateParameters(fb, g, i + 1, M, shape);
	}
}

float BandPass::GetOutput(const float input)
{
	float out = input;
	for (int i = 0; i < numFilters; i++)
	{
		out = filters[i].GetOutput(out);
	}
	return out;
}
#pragma endregion

#pragma region HighShelf
void HighShelf::UpdateParameters(float fc, float g)
{
	float omegaC = cot(PI_2 * fc * T / 2);
	float sqrtG = sqrtf(g);

	float temp = omegaC / sqrtG;
	a[0] = 1 + temp;
	a[1] = (1 - temp) / a[0];

	temp = omegaC * sqrtG;
	b[0] = (1 + temp) / a[0];
	b[1] = (1 - temp) / a[0];

	/*float K = PI_2 * fc * T;
	float g_2 = 2 * g;
	 
	a[0] = K + 2;
	a[1] = (K - 2) / a[0];
	b[0] = (K + g_2) / a[0];
	b[1] = K - g_2 / a[0];*/
}
#pragma endregion

#pragma region LowPass
void LowPass::UpdateParameters(float fc)
{
	float K = PI_2 * fc * T;

	a[0] = K + 2;
	a[1] = (K - 2) / a[0];

	b[0] = K / a[0];
	b[1] = K / a[0];

	/*std::cout << "Initialise filter parameters" << "\n";
	std::cout << "PI_2: " << PI_2 << "\n";
	std::cout << "fc: " << fc << "\n";
	std::cout << "T: " << T << "\n";
	std::cout << "K: " << K << "\n";
	std::cout << "b: " << b[0] << ", " << b[1] << "\n";
	std::cout << "a: " << a[0] << ", " << a[1] << "\n";*/
}
#pragma endregion

#pragma region TransDF2
void TransDF2::UpdateParameters(TransDF2Parameters zpk)
{
	b[0] = zpk.k;
	b[1] = -zpk.k * (zpk.z[0] + zpk.z[1]);
	b[2] = zpk.k * zpk.z[0] * zpk.z[1];

	a[1] = -(zpk.p[0] + zpk.p[1]);
	a[2] = zpk.p[0] * zpk.p[1];
}

void TransDF2::UpdateParameters(float fc, FilterShape shape)
{
	switch (shape)
	{
	case FilterShape::lpf:
		UpdateLPF(fc);
		break;
	case FilterShape::hpf:
		UpdateHPF(fc);
		break;
	}
}
void TransDF2::UpdateParameters(float fb, float g, int m, int M, FilterShape shape)
{
	switch (shape)
	{
	case FilterShape::lbf:
		UpdateLBF(fb, g, m, M);
		break;
	case FilterShape::hbf:
		UpdateHBF(fb, g, m, M);
		break;
	}
}

void TransDF2::UpdateLPF(float fc)
{
	//float omega = cot(PI_2 * fc * T / 2.0f);
	float omega = cot(PI_1 * fc * T);
	float omega_sq = powf(omega, 2);

	float a0 = 1.0f / (1.0f + SQRT_2 * omega + omega_sq);
	b[0] = a0;
	b[1] = 2.0f * a0;
	b[2] = a0;

	a[1] = (2.0f - 2.0f * omega_sq) * a0;
	a[2] = (1.0f - SQRT_2 * omega + omega_sq) * a0;
}

void TransDF2::UpdateHPF(float fc)
{
	//float omega = cot(PI_2 * fc * T / 2.0f);
	float omega = cot(PI_1 * fc * T);
	float omega_sq = powf(omega, 2);

	float a0 = 1.0f / (1.0f + SQRT_2 * omega + omega_sq);
	b[0] = omega_sq * a0;
	b[1] = -2.0f * omega_sq * a0;
	b[2] = omega_sq * a0;

	a[1] = (2.0f - 2.0f * omega_sq) * a0;
	a[2] = (1.0f - SQRT_2 * omega + omega_sq) * a0;
}

void TransDF2::UpdateLBF(float fb, float g, int m, int M)
{
	float K = tanf(PI_1 * fb * T);
	float K_2 = 2 * K;
	float K_sq = powf(K, 2);
	float K_sq_2 = 2 * K_sq;
	float M_2 = 2 * M;
	float V = powf(g, 1.0f / M) - 1;
	float VK = V * K;
	float VK_2 = 2 * VK;
	float VK_sq = powf(VK, 2);

	float alpha = (0.5f - (2.0f * m - 1.0f) / (M_2)) * PI_1;
	float cm = cosf(alpha);
	float K2cm = K_2 * cm;
	float a0 = 1.0f / (1.0f + K2cm + K_sq);
	a[1] = (K_sq_2 - 2.0f) * a0;
	a[2] = (1.0f - K2cm + K_sq) * a0;

	b[0] = a[0] + (VK_2 * (K + cm) + VK_sq) * a0;
	b[1] = a[1] + (VK_2 * K_2 + VK_sq * 2.0f) * a0;
	b[2] = a[2] + (VK_2 * (K - cm) + VK_sq) * a0;
}

void TransDF2::UpdateHBF(float fb, float g, int m, int M)
{
	float K = tanf(PI_1 * fb * T);
	float K_2 = 2 * K;
	float K_sq = powf(K, 2);
	float K_sq_2 = 2 * K_sq;
	float M_2 = 2 * M;
	float V = powf(g, 1.0f / M) - 1;
	float VK = V * K;
	float VK_2 = 2 * VK;
	float VK_sq = powf(VK, 2);

	float alpha = (0.5f - (2.0f * m - 1.0f) / (M_2)) * PI_1;
	float cm = cosf(alpha);
	float K2cm = K_2 * cm;
	float a0 = 1.0f / (1.0f + K2cm + K_sq);
	a[1] = (2.0f - K_sq_2) * a0;
	a[2] = (1.0f - K2cm + K_sq) * a0;

	b[0] = a[0] + (VK_2 * (K + cm) + VK_sq) * a0;
	b[1] = a[1] - (VK_2 * K_2 + VK_sq * 2.0f) * a0;
	b[2] = a[2] + (VK_2 * (K - cm) + VK_sq) * a0;
}
#pragma endregion

#pragma region LinkwitzRiley
LinkwitzRiley::LinkwitzRiley(int fs) : fc{ 176.0f, 775.0f, 3408.0f }, g{ 0.0f, 0.0f, 0.0f, 0.0f }, filters()
{
	InitFilters(fs);
	CalcFM();
};

LinkwitzRiley::LinkwitzRiley(float fc0, float fc1, float fc2, int fs) : fc{ fc0, fc1, fc2 }, g{ 1.0f, 1.0f, 1.0f, 1.0f }, filters()
{
	InitFilters(fs);
	CalcFM();
};

void LinkwitzRiley::InitFilters(int fs)
{
	TransDF2 lpFilter[3] = { TransDF2(fc[0], FilterShape(FilterShape::lpf), fs), TransDF2(fc[1], FilterShape(FilterShape::lpf), fs), TransDF2(fc[2], FilterShape(FilterShape::lpf), fs) };
	TransDF2 hpFilter[3] = { TransDF2(fc[0], FilterShape(FilterShape::hpf), fs), TransDF2(fc[1], FilterShape(FilterShape::hpf), fs), TransDF2(fc[2], FilterShape(FilterShape::hpf), fs) };

	filters[0] = lpFilter[1];
	filters[1] = lpFilter[1];
	filters[2] = lpFilter[2];
	filters[3] = lpFilter[2];
	filters[4] = hpFilter[2];
	filters[5] = hpFilter[2];
	filters[6] = lpFilter[0];
	filters[7] = lpFilter[0];
	filters[8] = hpFilter[0];
	filters[9] = hpFilter[0];
	filters[10] = hpFilter[1];
	filters[11] = hpFilter[1];
	filters[12] = lpFilter[0];
	filters[13] = lpFilter[0];
	filters[14] = hpFilter[0];
	filters[15] = hpFilter[0];
	filters[16] = lpFilter[2];
	filters[17] = lpFilter[2];
	filters[18] = hpFilter[2];
	filters[19] = hpFilter[2];
}

void LinkwitzRiley::CalcFM()
{
	float fMin;
	float fMax;
	for (int i = 0; i < 4; i++)
	{
		if (i == 0)
			fMin = 20;
		else
			fMin = fc[i - 1];
		if (i == 3)
			fMax = 20000;
		else
			fMax = fc[i];

		fm[i] = sqrtf(fMin * fMax);
	}
}

void LinkwitzRiley::UpdateParameters(float gain[])
{
	for (int i = 0; i < 4; i++)
		g[i] = *gain++;
}

float LinkwitzRiley::GetOutput(const float input)
{
	float mid[2];
	mid[0] = filters[1].GetOutput(filters[0].GetOutput(input));
	mid[1] = filters[11].GetOutput(filters[10].GetOutput(input));

	mid[0] = filters[3].GetOutput(filters[2].GetOutput(mid[0])) + filters[5].GetOutput(filters[4].GetOutput(mid[0]));
	mid[1] = filters[13].GetOutput(filters[12].GetOutput(mid[1])) + filters[15].GetOutput(filters[14].GetOutput(mid[1]));

	float out[4];
	out[0] = g[0] * filters[7].GetOutput(filters[6].GetOutput(mid[0]));
	out[1] = g[1] * filters[9].GetOutput(filters[8].GetOutput(mid[0]));
	out[2] = g[2] * filters[17].GetOutput(filters[16].GetOutput(mid[1]));
	out[3] = g[3] * filters[19].GetOutput(filters[18].GetOutput(mid[1]));

	return out[0] + out[1] + out[2] + out[3];
}
#pragma endregion
