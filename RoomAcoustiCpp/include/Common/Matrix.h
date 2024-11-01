/*
* @class matrix
*
* @brief Declaration of matrix class
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

namespace RAC
{
	namespace Common
	{

		//////////////////// matrix class ////////////////////

		class Matrix
		{
		public:

			// Load and Destroy
			Matrix() : rows(0), cols(0) { AllocateSpace(); };
			Matrix(const int r, const int c) : rows(r), cols(c) { AllocateSpace(); };
			Matrix(const std::vector<std::vector<Real>>& mat);
			~Matrix() { /*DeallocateSpace(); */ };

			// Init
			void Init(const std::vector<std::vector<Real>>& mat);
			inline void Reset()
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						data[i][j] = 0.0;
				}
			}

			// Adders
			inline void AddColumn(const std::vector<Real>& v, const int c)
			{
				for (int i = 0; i < rows; i++)
					data[i][c] = v[i];
			}
			inline void AddRow(const std::vector<Real>& v, const int r) { data[r] = v; }

			// Getters
			inline Real GetEntry(const int r, const int c) const { return data[r][c]; }
			inline const std::vector<Real>& GetRow(int r) const { return data[r];	}
			inline const std::vector<Real>& GetColumn(int c)
			{
				for (int i = 0; i < rows; i++)
					column[i] = data[i][c];
				return column;
			}

			inline int Rows() const { return rows; }
			inline int Cols() const { return cols; }
			inline const std::vector<std::vector<Real>>& Data() const { return data; }
			Matrix Transpose();

			void Inverse();

			inline void Log10()
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->data[i][j] = RAC::Common::Log10(data[i][j]);
				}
			}

			inline void Pow10()
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->data[i][j] = RAC::Common::Pow10(data[i][j]);
				}
			}

			// Operators
			inline std::vector<Real>& operator[](const int r) { return data[r]; }

			inline Matrix operator=(const Matrix& mat)
			{
				rows = mat.Rows();
				cols = mat.Cols();
				Init(mat.Data());
				return *this;
			}

			inline Matrix operator+=(const Matrix& mat)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->data[i][j] += mat.GetEntry(i, j);
				}
				return *this;
			}

			inline Matrix operator-=(const Matrix& mat)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->data[i][j] -= mat.GetEntry(i, j);
				}
				return *this;
			}

			inline Matrix operator*=(const Real a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->data[i][j] *= a;
				}
				return *this;
			}

			inline Matrix operator/=(const Real a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->data[i][j] /= a;
				}
				return *this;
			}

			inline Matrix operator+=(const Real a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->data[i][j] += a;
				}
				return *this;
			}

			inline Matrix operator-=(const Real a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						this->data[i][j] -= a;
				}
				return *this;
			}

		protected:

			// Memory allocation
			void AllocateSpace();
			void DeallocateSpace();

			// Member variables
			int rows, cols;
			std::vector<std::vector<Real>> data;
			std::vector<Real> column;
		};

		//////////////////// Operators ////////////////////

		inline bool operator==(const Matrix& u, const Matrix& v)
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

		inline Matrix operator+(const Matrix& u, const Matrix& v)
		{
			assert(u.Rows() == v.Rows());
			assert(u.Cols() == v.Cols());

			Matrix out = Matrix(u.Rows(), u.Cols());
			for (int i = 0; i < u.Rows(); i++)
			{
				for (int j = 0; j < u.Cols(); j++)
				{
					Real entry = u.GetEntry(i, j) + v.GetEntry(i, j);
					out[i][j] = entry;
				}
			}
			return out;
		}

		inline Matrix operator-(const Matrix& mat)
		{
			Matrix out = Matrix(mat.Rows(), mat.Cols());
			for (int i = 0; i < mat.Rows(); i++)
			{
				for (int j = 0; j < mat.Cols(); j++)
					out[i][j] = -mat.GetEntry(i, j);
			}
			return out;
		}

		inline Matrix operator-(const Matrix& u, const Matrix& v)
		{
			assert(u.Rows() == v.Rows());
			assert(u.Cols() == v.Cols());
			Matrix out = Matrix(u.Rows(), u.Cols());
			for (int i = 0; i < u.Rows(); i++)
			{
				for (int j = 0; j < u.Cols(); j++)
					out[i][j] = u.GetEntry(i, j) - v.GetEntry(i, j);
			}
			return out;
		}

		inline Matrix Mulitply(Matrix& out, const Matrix& u, const Matrix& v)
		{
			assert(u.Cols() == v.Rows());
			assert(out.Rows() == u.Rows());
			assert(out.Cols() == v.Cols());

			Real sum = 0.0;
			for (int i = 0; i < u.Rows(); ++i)
			{
				for (int j = 0; j < v.Cols(); ++j)
				{
					sum = 0.0;
					for (int k = 0; k < u.Cols(); ++k)
						sum += u.GetEntry(i, k) * v.GetEntry(k, j);
					out[i][j] = sum;
				}
			}
			return out;
		}

		inline Matrix operator*(const Matrix& u, const Matrix& v)
		{
			Matrix out = Matrix(u.Rows(), v.Cols());
			return Mulitply(out, u, v);
		}

		inline Matrix operator*(const Real a, const Matrix& mat)
		{
			Matrix out = Matrix(mat.Rows(), mat.Cols());
			for (int i = 0; i < mat.Rows(); i++)
			{
				for (int j = 0; j < mat.Cols(); j++)
				{
					for (int k = 0; k < mat.Cols(); k++)
						out[i][j] = a * mat.GetEntry(i, j);
				}
			}
			return out;
		}

		inline Matrix operator*(const Matrix& mat, const Real a)
		{
			return a * mat;
		}

		inline Matrix operator/(const Matrix& mat, const Real& a)
		{
			return (1.0 / a) * mat;
		}
	}
}

#endif