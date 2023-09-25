
#include "AudioManager.h"

#pragma region Buffer
Buffer::Buffer()
{
	InitialiseBuffer(DEFAULT_BUFFER_SIZE);
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
	else if (capacity > DEFAULT_BUFFER_SIZE)
	{
		if (capacity > numSamples)
		{
			mBuffer.shrink_to_fit();
		}
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
			x[i] = store[count + i % xLen];
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
	switch (shape) {
	case FilterShape::lpf:
		UpdateLPF(fc);
		break;
	case FilterShape::hpf:
		UpdateHPF(fc);
		break;
	}
}

void TransDF2::UpdateLPF(float fc)
{
	float omega = cot(PI_2 * fc * T / 2);
	float omega_sq = powf(omega, 2);

	float a0 = 1.0f / (1 + SQRT_2 * omega + omega_sq);
	b[0] = a0;
	b[1] = 2.0f * a0;
	b[2] = a0;

	a[1] = (2 - 2 * omega_sq) * a0;
	a[2] = (1 - SQRT_2 * omega + omega_sq) * a0;
}

void TransDF2::UpdateHPF(float fc)
{
	float omega = cot(PI_2 * fc * T / 2);
	float omega_sq = powf(omega, 2);

	float a0 = 1.0f / (1 + SQRT_2 * omega + omega_sq);
	b[0] = omega_sq * a0;
	b[1] = -2.0f * omega_sq * a0;
	b[2] = omega_sq * a0;

	a[1] = (2 - 2 * omega_sq) * a0;
	a[2] = (1 - SQRT_2 * omega + omega_sq) * a0;
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

float LinkwitzRiley::GetOutput(float input)
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
