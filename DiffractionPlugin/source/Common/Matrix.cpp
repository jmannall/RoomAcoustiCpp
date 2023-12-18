
#include "matrix.h"
#include "vec.h"

matrix::matrix() : rows(1), cols(1)
{
	Init();
}

matrix::matrix(const int r, const int c) : rows(r), cols(c)
{
	Init();
}

matrix::matrix(const float** mat, const int r, const int c) : rows(r), cols(c)
{
	Init(mat);
}

matrix::matrix(const float* in, const int r, const int c) : rows(r), cols(c)
{
	Init(in);
}

void matrix::AllocateSpace()
{
	e = new float*[rows];
	for (int i = 0; i < rows; i++)
	{
		e[i] = new float[cols];
	}
}

void matrix::Reset()
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			e[i][j] = 0.0f;
		}
	}
}

void matrix::Init()
{
	AllocateSpace();
	Reset();
}

void matrix::Init(const float* in)
{
	AllocateSpace();
	int idx = 0;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			e[i][j] = in[idx];
			idx++;
		}
	}
}

void matrix::Init(const float** mat)
{
	AllocateSpace();
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			e[i][j] = mat[i][j];
		}
	}
}

void matrix::AddColumn(const vec& v, const int& c)
{
	for (int i = 0; i < rows; i++)
	{
		e[i][c] = v[i];
	}
}

void matrix::AddRow(const vec& v, const int& r)
{
	for (int i = 0; i < cols; i++)
	{
		e[r][i] = v[i];
	}
}

float* matrix::GetColumn(int idx) const
{
	float* column = new float[rows];
	for (int i = 0; i < rows; i++)
	{
		column[i] = e[i][idx];
	}
	return column;
}

float* matrix::GetRow(int idx) const
{
	float* column = e[idx];
	return column;
}

matrix matrix::Transpose()
{
	matrix mat = matrix(cols, rows);
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			mat.AddEntry(e[i][j], j, i);
		}
	}
	return mat;
}