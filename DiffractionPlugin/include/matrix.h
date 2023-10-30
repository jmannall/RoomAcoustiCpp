#pragma once

#include <iostream>
#include <math.h>
#include <cassert>

class vec;

class matrix
{
public:
	matrix();
	matrix(const int r, const int c);
	matrix(const float** mat, const int r, const int c);
	matrix(const float* in, const int r, const int c);

	void Init(const float* in);
	void Init(const float** mat);
	float* GetColumn(int idx) const;
	float* GetRow(int idx) const;

	int Rows() const { return rows; }
	int Cols() const { return cols; }

	virtual inline void AddEntry(const float& in, const int& r, const int& c) { e[r][c] = in; }
	virtual inline void IncreaseEntry(const float& in, const int& r, const int& c) { e[r][c] += in; }
	void AddColumn(const vec& v, const int& c);
	void AddRow(const vec& v, const int& r);
	inline float GetEntry(const int& r, const int& c) const { return e[r][c]; } // Add bounds checking

	matrix Transpose();

	inline matrix operator=(const matrix& mat)
	{
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

	inline matrix operator+=(const matrix& mat)
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				this->e[i][j] += mat.GetEntry(i, j);
			}
		}
		return *this;
	}

	inline matrix operator-=(const matrix& mat)
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				this->e[i][j] -= mat.GetEntry(i, j);
			}
		}
		return *this;
	}

	inline matrix operator*=(const float& a)
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				this->e[i][j] *= a;
			}
		}
		return *this;
	}

	inline matrix operator/=(const float& a)
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				this->e[i][j] /= a;
			}
		}
		return *this;
	}

protected:
	void AllocateSpace();
	void Init();

	int rows, cols;
	float** e;
};

inline bool operator==(const matrix& u, const matrix& v)
{
	assert(u.Rows() == v.Rows());
	assert(u.Cols() == v.Cols());
	matrix out = matrix(u.Rows(), u.Cols());
	bool equal = true;
	int i = 0;
	int j = 0;
	while (equal && j < u.Cols())
	{
		equal = u.GetEntry(i, j) == v.GetEntry(i, j);
		i++;
		if (i == u.Rows())
		{
			i = 0;
			j++;
		}
	}
	return equal;
}

inline matrix operator+(const matrix& u, const matrix& v)
{
	assert(u.Rows() == v.Rows());
	assert(u.Cols() == v.Cols());
	matrix out = matrix(u.Rows(), u.Cols());
	for (int i = 0; i < u.Rows(); i++)
	{
		for (int j = 0; j < u.Cols(); j++)
		{
			float entry = u.GetEntry(i, j) + v.GetEntry(i, j);
			out.AddEntry(entry, i, j);
		}
	}
	return out;
}

inline matrix operator-(const matrix& u)
{
	matrix out = matrix(u.Rows(), u.Cols());
	for (int i = 0; i < u.Rows(); i++)
	{
		for (int j = 0; j < u.Cols(); j++)
		{
			out.AddEntry(-u.GetEntry(i, j), i, j);
		}
	}
	return out;
}

inline matrix operator-(const matrix& u, const matrix& v)
{
	return -v + u;
}

inline matrix operator*(const matrix& u, const matrix& v)
{
	assert(u.Cols() == v.Rows());
	matrix out = matrix(u.Rows(), v.Cols());
	for (int i = 0; i < u.Rows(); i++)
	{
		for (int j = 0; j < v.Cols(); j++)
		{
			float entry = 0.0f;
			for (int k = 0; k < u.Cols(); k++)
			{
				entry += u.GetEntry(i, k) * v.GetEntry(k, j);
			}
			out.AddEntry(entry, i, j);
		}
	}
	return out;
}

inline matrix operator*(const float& a, const matrix& u)
{
	matrix out = matrix(u.Rows(), u.Cols());
	for (int i = 0; i < u.Rows(); i++)
	{
		for (int j = 0; j < u.Cols(); j++)
		{
			for (int k = 0; k < u.Cols(); k++)
			{
				out.AddEntry(a * u.GetEntry(i, j), i, j);
			}
		}
	}
	return out;
}

inline matrix operator*(const matrix& u, const float& a)
{
	return a * u;
}

inline matrix operator/(const matrix& u, const float& a)
{
	return (1 / a) * u;
}