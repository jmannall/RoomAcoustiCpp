/*
*
*  \matrix class
*
*/

#ifndef Common_Matrix_h
#define Common_Matrix_h

// C++ headers
#include <cassert>
#include <vector>

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"

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
			~matrix() { /*DeallocateSpace(); */ };

			// Init
			void Init(const std::vector<std::vector<Real>>& mat);
			inline void Reset()
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						e[i][j] = 0.0;
				}
			}

			// Adders
			virtual inline void AddEntry(const Real& in, const int& r, const int& c) {  /*CheckRows(r); CheckCols(c);*/ e[r][c] = in; }
			virtual inline void IncreaseEntry(const Real& in, const int& r, const int& c) { /*CheckRows(r); CheckCols(c);*/ e[r][c] += in; }
			inline void AddColumn(const std::vector<Real>& v, const int& c)
			{
				//CheckCols(c);
				for (int i = 0; i < rows; i++)
					e[i][c] = v[i];
			}
			inline void AddRow(const std::vector<Real>& v, const int& r) { CheckRows(r); e[r] = v; }

			inline void CheckRows(const int& r) const { if (r >= rows) throw "Row index out of bounds"; }
			inline void CheckCols(const int& c) const { if (c >= cols) throw "Column index out of bounds"; }

			// Getters
			virtual inline Real GetEntry(const int& r, const int& c) const { /*CheckRows(r); CheckCols(c);*/ return e[r][c]; }
			inline const std::vector<Real>& GetRow(int r) const { /*CheckRows(r);*/ return e[r];	}
			inline const std::vector<Real>& GetColumn(int c)
			{
				//CheckCols(c);
				for (int i = 0; i < rows; i++)
					column[i] = e[i][c];
				return column;
			}

			inline int Rows() const { return rows; }
			inline int Cols() const { return cols; }
			inline const std::vector<std::vector<Real>>& Data() const { return e; }
			matrix Transpose();

			void Inverse();

			inline void Log10()
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->e[i][j] = UIE::Common::Log10(e[i][j]);
				}
			}

			inline void Pow10()
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->e[i][j] = UIE::Common::Pow10(e[i][j]);
				}
			}

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
						this->e[i][j] += mat.GetEntry(i, j);
				}
				return *this;
			}

			inline matrix operator-=(const matrix& mat)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->e[i][j] -= mat.GetEntry(i, j);
				}
				return *this;
			}

			inline matrix operator*=(const Real& a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->e[i][j] *= a;
				}
				return *this;
			}

			inline matrix operator/=(const Real& a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->e[i][j] /= a;
				}
				return *this;
			}

			inline matrix operator+=(const Real& a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->e[i][j] += a;
				}
				return *this;
			}

			inline matrix operator-=(const Real& a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->e[i][j] -= a;
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
			std::vector<Real> column;
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

		inline matrix Mulitply(matrix& out, const matrix& u, const matrix& v)
		{
			assert(u.Cols() == v.Rows());
			assert(out.Rows() == u.Rows());
			assert(out.Cols() == v.Cols());

			out.Reset();
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

		inline matrix operator*(const matrix& u, const matrix& v)
		{
			matrix out = matrix(u.Rows(), v.Cols());
			return Mulitply(out, u, v);
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