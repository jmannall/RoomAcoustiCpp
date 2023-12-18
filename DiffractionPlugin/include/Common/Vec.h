#pragma once

#include <iostream>
#include <math.h>
#include <cassert>
#include <random>
#include <chrono>
#include "matrix.h"


static std::default_random_engine generator (100);

class vec : public matrix
{
public:
	vec() : matrix(), length(1)	{ Init(); }
	vec(const int& len) : matrix(len, 1), length(len) { Init(); }
	vec(const float* vec, const int& len) : matrix(len, 1), length(len) { Init(vec); }
	vec(const matrix& mat);

	void Normalise();
	float CalculateNormal();
	void RandomNormalDistribution();
	void RandomUniformDistribution();
	void RandomUniformDistribution(float a, float b);
	float Mean() const;

	inline float& operator[] (const int& i) const { return e[i][0]; }

	inline vec operator=(const matrix& mat)
	{
		assert(mat.Cols() == 1);

		rows = mat.Rows();
		cols = mat.Cols();
		Init();
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				e[i][j] = mat.GetEntry(i, j);
			}
		}
		return *this;
	}

private:
	int length;
};

class rowvec : public matrix
{
public:
	rowvec() : matrix() { Init(); }
	rowvec(const int& c) : matrix(1, c) { Init(); }
	rowvec(const float* vec, const int& c) : matrix(1, c) { Init(vec); }
	rowvec(const matrix& mat);
	~rowvec() {}

	inline float& operator[] (const int& i) const { return e[0][i]; }

	inline rowvec operator=(const matrix& mat)
	{
		assert(mat.Rows() == 1);

		rows = mat.Rows();
		cols = mat.Cols();
		Init();
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				e[i][j] = mat.GetEntry(i, j);
			}
		}
		return *this;
	}
};