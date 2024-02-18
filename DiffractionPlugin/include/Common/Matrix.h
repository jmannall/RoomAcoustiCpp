/*
*
*  \matrix class
*
*/

#ifndef Common_Matrix_h
#define Common_Matrix_h

#include <cassert>
#include <vector>

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
			matrix() : rows(0), cols(0) { AllocateSpace(); };
			matrix(const int r, const int c) : rows(r), cols(c) { AllocateSpace(); };
			matrix(const std::vector<std::vector<Real>>& mat);
			~matrix() { DeallocateSpace(); };

			// Init
			void Init(const std::vector<std::vector<Real>>& mat);
			inline void Reset()
			{
				for (int i = 0; i < rows; i++)
					std::fill(e[i].begin(), e[i].end(), 0.0);
			};

			// Adders
			virtual inline void AddEntry(const Real& in, const int& r, const int& c) { e[r][c] = in; }
			virtual inline void IncreaseEntry(const Real& in, const int& r, const int& c) { e[r][c] += in; }
			inline void AddColumn(const std::vector<Real>& v, const int& c)
			{
				for (int i = 0; i < rows; i++)
					e[i][c] = v[i];
			}
			inline void AddRow(const std::vector<Real>& v, const int& r) { e[r] = v; }

			// Getters
			virtual inline Real GetEntry(const int& r, const int& c) const { return e[r][c]; } // Add bounds checking
			inline std::vector<Real> GetRow(int idx) const { return e[idx];	}
			inline std::vector<Real> GetColumn(int idx) const
			{
				std::vector<Real> column(rows);
				for (int i = 0; i < rows; i++)
					column[i] = e[i][idx];
				return column;
			}

			inline int Rows() const { return rows; }
			inline int Cols() const { return cols; }
			inline std::vector<std::vector<Real>> Data() const { return e; }
			matrix Transpose();

			// Operators
			inline matrix operator=(const matrix& mat)
			{
				rows = mat.Rows();
				cols = mat.Cols();
				Init(mat.Data());
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

			// Memory allocation
			void AllocateSpace();
			void DeallocateSpace();

			// Member variables
			int rows, cols;
			std::vector<std::vector<Real>> e;
		};

		//////////////////// Operators ////////////////////

		inline bool operator==(const matrix& u, const matrix& v)
		{
			assert(u.Rows() == v.Rows());
			assert(u.Cols() == v.Cols());

			bool equal = true;
			int i = 0;
			while (equal && i < u.Rows())
			{
				equal = u.GetRow(i) == v.GetRow(i);
				i++;
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
			assert(u.Rows() == v.Rows());
			assert(u.Cols() == v.Cols());
			matrix out = matrix(u.Rows(), u.Cols());
			for (int i = 0; i < u.Rows(); i++)
			{
				for (int j = 0; j < u.Cols(); j++)
				{
					Real entry = u.GetEntry(i, j) - v.GetEntry(i, j);
					out.AddEntry(entry, i, j);
				}
			}
			return out;
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