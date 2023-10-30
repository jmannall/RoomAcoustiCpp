
#include "Spatialiser/FDN.h"

using namespace Spatialiser;

Channel::Channel(int fs) : mT(0.0f), sampleRate(fs), mAbsorptionFilter(4, fs), idx(0)
{
	SetDelay();
	SetAbsorption({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
}

Channel::Channel(float t, const FrequencyDependence& T60, int fs) : mT(t), sampleRate(fs), mAbsorptionFilter(4, fs), idx(0)
{
	SetDelay();
	SetAbsorption(T60);
}

void Channel::SetParameters(const FrequencyDependence& T60, const float& t)
{
	SetDelay(t);
	SetAbsorption(T60);
}

void Channel::SetAbsorption(const FrequencyDependence& T60)
{
	float g[5];
	float t[5];

	T60.GetValues(&t[0]);
	for (int i = 0; i < 5; i++)
	{
		g[i] = powf(10, -3.0f * mT / t[i]); // 20 * log10(H(f)) = -60 * t / t60(f);
	}
	SetAbsorption(g);
}

void Channel::SetAbsorption(float g[])
{
	float fc[] = { 250, 500, 1000, 2000, 4000 };
	mAbsorptionFilter.UpdateParameters(fc, g);
}

void Channel::SetDelay()
{
	mDelay = std::max(round(mT * sampleRate), 1.0f);
	Debug::Log("FDN Delay: " + IntToStr(mDelay), Color::White);
	mBuffer.ResizeBuffer(mDelay);
}

float Channel::GetOutput(const float input)
{
	float out = mAbsorptionFilter.GetOutput(mBuffer[idx]);
	mBuffer[idx] = input;
	idx++;
	idx = idx % mDelay;
	return out;
}

FDN::FDN(size_t numChannels, int fs) : mNumChannels(numChannels), x(mNumChannels), y(mNumChannels), mat(mNumChannels, mNumChannels)
{
	mChannels.reserve(mNumChannels);
	std::fill_n(std::back_inserter(mChannels), mNumChannels, Channel(fs));
	InitMatrix();
}

FDN::FDN(const FrequencyDependence& T60, const vec& dimensions, size_t numChannels, int fs) : mNumChannels(numChannels), x(mNumChannels), y(mNumChannels), mat(mNumChannels, mNumChannels)
{
	vec t = vec(mNumChannels);
	CalculateTimeDelay(dimensions, t);
	mChannels.reserve(mNumChannels);
	for (int i = 0; i < mNumChannels; i++)
	{
		mChannels.push_back(Channel(t[i], T60, fs));
	}
	InitMatrix();
}

void FDN::SetParameters(const FrequencyDependence& T60, const vec& dimensions)
{
	vec t = vec(mNumChannels);
	CalculateTimeDelay(dimensions, t);

	float temp[5];
	T60.GetValues(&temp[0]);
	for (int i = 0; i < 5; i++)
	{
		Debug::Log("FDN T60: " + FloatToStr(temp[i]) + " I: " + IntToStr(i), Color::White);
	}
	for (int i = 0; i < mNumChannels; i++)
	{
		mChannels[i].SetParameters(T60, t[i]);
	}
}

void FDN::CalculateTimeDelay(const vec& dimensions, vec& t)
{
	float idx = mNumChannels / dimensions.Rows();
	assert(t.Rows() == mNumChannels);
	assert(dimensions.Rows() <= mNumChannels);
	assert(idx == floor(idx)); // length of dimensions must be a multiple of mNumChannels

	t.RandomUniformDistribution(-0.1f, 0.1f);
	t *= dimensions.Mean();

	int k = 0;
	for (int j = 0; j < mNumChannels / idx; j++)
	{
		for (int i = 0; i < idx; i++)
		{
			t[k++] += dimensions[j];
		}
	}
	t *= INV_SPEED_OF_SOUND;
}

void FDN::InitMatrix()
{
	vec vector = vec(mNumChannels);
	vector.RandomNormalDistribution();
	vector.Normalise();
	mat.AddColumn(vector, 0);

	float tol = 0.000001;
	for (int j = 1; j < mNumChannels; j++)
	{
		float norm = 0;
		while (norm < tol)
		{
			vector.RandomNormalDistribution();

			matrix section = matrix(mNumChannels, j);

			for (int i = 0; i < mNumChannels; i++)
			{
				for (int k = 0; k < j; k++)
				{
					section.AddEntry(mat.GetEntry(i, k), i, k);
				}
			}

			vector -= section * (section.Transpose() * vector);
			norm = vector.CalculateNormal();
		}
		vector /= norm;
		mat.AddColumn(vector, j);
	}
}

rowvec FDN::GetOutput(const float* data)
{
	float output = 0;

	for (int i = 0; i < mNumChannels; i++)
	{
		y[i] = mChannels[i].GetOutput(x[i] + data[i]);
	}
	ProcessMatrix();
	return y;
}

