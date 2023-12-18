
#include "vec.h"

vec::vec(const matrix& mat)
{
	assert(mat.Cols() == 1);
	length = mat.Rows();
	float* v = mat.GetColumn(0);
	Init(v);
}

void vec::Normalise()
{
	float norm = CalculateNormal();
	for (int i = 0; i < length; i++)
	{
		e[i][0] = e[i][0] / norm;
	}
}

float vec::CalculateNormal()
{
	float mag = 0;
	for (int i = 0; i < length; i++)
	{
		mag += powf(e[i][0], 2);
	}
	return sqrtf(mag);
}

void vec::RandomNormalDistribution()
{
	std::normal_distribution<float> distribution; // mean 0, standard deviation 1
	for (int i = 0; i < length; i++)
	{
		e[i][0] = distribution(generator);
	}
}

void vec::RandomUniformDistribution()
{
	std::uniform_real_distribution<float> distribution; // a 0, b 1
	for (int i = 0; i < length; i++)
	{
		e[i][0] = distribution(generator);
	}
}

void vec::RandomUniformDistribution(float a, float b)
{
	std::uniform_real_distribution<float> distribution(a, b);
	for (int i = 0; i < length; i++)
	{
		e[i][0] = distribution(generator);
	}
}

float vec::Mean() const
{
	float out = 0;
	for (int i = 0; i < length; i++)
	{
		out += e[i][0];
	}
	return out / length;
}

rowvec::rowvec(const matrix& mat)
{
	assert(mat.Rows() == 1);
	float* v = mat.GetRow(0);
	Init(v);
}