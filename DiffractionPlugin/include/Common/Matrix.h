/*
*
*  \matrix class
*
*/

#ifndef Common_Matrix_h
#define Common_Matrix_h

#include <cassert>

#include "Common/Types.h"

namespace UIE
{
	namespace Common
	{
		class vec;

		//////////////////// matrix class ////////////////////

		class matrix
		{
		public:

			// Load and Destroy
			matrix();
			matrix(const int r, const int c);
			matrix(const Real** mat, const int r, const int c);
			matrix(const Real* in, const int r, const int c);
			~matrix() {};

			// Init
			void Init(const Real* in);
			void Init(const Real** mat);
			void Reset();

			// Adders
			virtual inline void AddEntry(const Real& in, const int& r, const int& c) { e[r][c] = in; }
			virtual inline void IncreaseEntry(const Real& in, const int& r, const int& c) { e[r][c] += in; }
			void AddColumn(const vec& v, const int& c);
			void AddRow(const vec& v, const int& r);

			// Getters
			inline Real GetEntry(const int& r, const int& c) const { return e[r][c]; } // Add bounds checking
			Real* GetColumn(int idx) const;
			Real* GetRow(int idx) const;
			int Rows() const { return rows; }
			int Cols() const { return cols; }

			matrix Transpose();

			// Operators
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

			inline matrix operator*=(const Real& a)
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

			inline matrix operator/=(const Real& a)
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

			// Init
			void AllocateSpace();
			void Init();

			// Member variables
			int rows, cols;
			Real** e;
		};

		//////////////////// Operators ////////////////////

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
					Real entry = u.GetEntry(i, j) + v.GetEntry(i, j);
					out.AddEntry(entry, i, j);
				}
			}
			return out;
		}

		inline matrix operator-(const matrix& mat)
		{
			matrix out = matrix(mat.Rows(), mat.Cols());
			for (int i = 0; i < mat.Rows(); i++)
			{
				for (int j = 0; j < mat.Cols(); j++)
				{
					out.AddEntry(-mat.GetEntry(i, j), i, j);
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

			for (int i = 0; i < u.Rows(); ++i)
			{
				for (int j = 0; j < v.Cols(); ++j)
				{
					for (int k = 0; k < u.Cols(); ++k)
					{
						out.IncreaseEntry(u.GetEntry(i, k) * v.GetEntry(k, j), i, j);		
					}
				}
			}
			return out;
		}

		inline matrix operator*(const Real& a, const matrix& mat)
		{
			matrix out = matrix(mat.Rows(), mat.Cols());
			for (int i = 0; i < mat.Rows(); i++)
			{
				for (int j = 0; j < mat.Cols(); j++)
				{
					for (int k = 0; k < mat.Cols(); k++)
					{
						out.AddEntry(a * mat.GetEntry(i, j), i, j);
					}
				}
			}
			return out;
		}

		inline matrix operator*(const matrix& mat, const Real& a)
		{
			return a * mat;
		}

		inline matrix operator/(const matrix& mat, const Real& a)
		{
			return (1.0 / a) * mat;
		}
	}
}

#endif