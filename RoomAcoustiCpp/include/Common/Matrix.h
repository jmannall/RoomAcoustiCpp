/*
* @class Matrix
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

namespace RAC
{
	namespace Common
	{
		/**
		* @brief Class that stores a matrix data structure
		*/
		class Matrix
		{
		public:

			/**
			* Default constructor that initialises an empty Matrix
			*/
			Matrix() : Matrix(0, 0) {};

			/**
			* Constructor that initialises a Matrix with zeros
			*
			* @param r The number of rows
			* @param c The number of columns
			*/
			Matrix(const int r, const int c) : rows(r), cols(c) { AllocateSpace(); };

			/**
			* Constructor that initialises a Matrix with data
			*
			* @param matrix The input data to initialise the matrix
			*/
			Matrix(const std::vector<std::vector<Real>>& matrix) { Init(matrix); }

			/**
			* @brief Default deconstructor
			*/
			~Matrix() {};

			/**
			* @brief Reset the matrix to zeros
			*/
			inline void Reset() { std::fill(data.begin(), data.end(), std::vector<Real>(cols, 0.0)); }

			/**
			* @brief Update data for a column of the matrix
			* 
			* @param column The input column to add
			* @param c The column to add the vector to
			*/
			inline void AddColumn(const std::vector<Real>& column, const int c)
			{
				assert(column.size() == rows);
				assert(c < cols);
				for (int i = 0; i < rows; i++)
					data[i][c] = column[i];
			}

			/**
			* @brief Update data for a row of the matrix
			*
			* @param row The input row to add
			* @param r The row to add the vector to
			*/
			inline void AddRow(const std::vector<Real>& row, const int r)
			{
				assert(row.size() == cols);
				assert(r < rows);
				data[r] = row;
			}

			/**
			* @brief Get a row of the matrix
			* 
			* @param r The row to get
			* @return Reference (const) to the row
			*/
			inline const std::vector<Real>& GetRow(int r) const { assert(r < rows); return data[r]; }

			/**
			* @brief Get a column of the matrix
			*
			* @param c The column to get
			* @return Reference (const) to the column
			*/
			inline const std::vector<Real>& GetColumn(int c)
			{
				assert(c < cols);
				for (int i = 0; i < rows; i++)
					column[i] = data[i][c];
				return column;
			}

			/**
			* @brief Get all data from the matrix
			*
			* @return Const reference to the matrix data
			*/
			inline const std::vector<std::vector<Real>>& Data() const { return data; }

			/**
			* @return The number of rows
			*/
			inline int Rows() const { return rows; }

			/**
			* @return The number of columns
			*/
			inline int Cols() const { return cols; }

			/**
			* @return The transpose of the matrix
			*/
			Matrix Transpose();

			/**
			* @brief Inverts the matrix
			*/
			void Inverse();

			/**
			* @brief Calculates the Log10 of the matrix (element-wise)
			*/
			void Log10();

			/**
			* @brief Calculates 10 raised to the power of each matrix entry (element-wise)
			*/
			void Pow10();

			/**
			* @brief Applies the max function to each element in the matrix
			*
			* @param min The minimum value
			*/
			void Max(const Real min);

			/**
			* @brief Applies the min function to each element in the matrix
			*
			* @param max The maximum value
			*/
			void Min(const Real max);

			/**
			* @brief Access the matrix row at the specified index
			* 
			* @param r The index of the row to return
			* @return A reference to the matrix row at the specified index
			*/
			inline std::vector<Real>& operator[](const int r) { return data[r]; }

			/**
			* @brief Access the matrix row at the specified index
			* 
			* @param r The index of the row to return
			* @return A const reference to the matrix row at the specified index
			*/
			inline const std::vector<Real>& operator[](const int r) const { return data[r]; }

			/**
			* @brief Adds a matrix to the current matrix
			*/
			inline Matrix operator+=(const Matrix& matrix)
			{
				assert(rows == matrix.Rows());
				assert(cols == matrix.Cols());

				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						data[i][j] += matrix[i][j];
				}
				return *this;
			}

			/**
			* @brief Subtracts a matrix from the current matrix
			*/
			inline Matrix operator-=(const Matrix& matrix)
			{
				assert(rows == matrix.Rows());
				assert(cols == matrix.Cols());

				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						data[i][j] -= matrix[i][j];
				}
				return *this;
			}

			/**
			* @brief Multiplies the matrix entries by a given value
			*/
			inline Matrix operator*=(const Real a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						data[i][j] *= a;
				}
				return *this;
			}

			/**
			* @brief Divides the matrix entries by a given value
			*/
			inline Matrix operator/=(const Real a) { return *this *= (1.0 / a); }

			/**
			* @brief Adds a single value to all matrix entries
			*/
			inline Matrix operator+=(const Real a)
			{
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
						data[i][j] += a;
				}
				return *this;
			}

			/**
			* @brief Subtracts a single value from all matrix entries
			*/
			inline Matrix operator-=(const Real a) { return *this += (-a); }

		protected:

			int rows, cols;							// Matrix dimensions
			std::vector<std::vector<Real>> data;	// Matrix data
			std::vector<Real> column;				// Stores a single column

		private:

			/**
			* @brief Initialise matrix data from std::vector structure
			*
			* @params matrix Data to inialise the matrix from
			*/
			void Init(const std::vector<std::vector<Real>>& matrix);

			/**
			* @brief Initialise matrix data with zeros
			*/
			void AllocateSpace();

			/**
			* @brief Clear matrix data
			*/
			inline void DeallocateSpace() { rows = 0; cols = 0; column.clear(); data.clear(); };
		};

		//////////////////// Operators ////////////////////

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs are equal, false otherwise
		*/
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

		/**
		* @return The sum of two matrices
		*/
		inline Matrix operator+(const Matrix& u, const Matrix& v)
		{
			assert(u.Rows() == v.Rows());
			assert(u.Cols() == v.Cols());

			Matrix out = Matrix(u.Rows(), u.Cols());
			for (int i = 0; i < u.Rows(); i++)
			{
				for (int j = 0; j < u.Cols(); j++)
				{
					Real entry = u[i][j] + v[i][j];
					out[i][j] = entry;
				}
			}
			return out;
		}

		/**
		* @brief Inverts the sign of all the matrix entries
		*/
		inline Matrix operator-(const Matrix& mat)
		{
			Matrix out = Matrix(mat.Rows(), mat.Cols());
			for (int i = 0; i < mat.Rows(); i++)
			{
				for (int j = 0; j < mat.Cols(); j++)
					out[i][j] = -mat[i][j];
			}
			return out;
		}

		/**
		* @return Subtracts a matrix from another
		*/
		inline Matrix operator-(const Matrix& u, const Matrix& v)
		{
			assert(u.Rows() == v.Rows());
			assert(u.Cols() == v.Cols());
			Matrix out = Matrix(u.Rows(), u.Cols());
			for (int i = 0; i < u.Rows(); i++)
			{
				for (int j = 0; j < u.Cols(); j++)
					out[i][j] = u[i][j] - v[i][j];
			}
			return out;
		}

		/**
		* @brief Performs a matrix multiplication
		*/
		inline void Mulitply(Matrix& out, const Matrix& u, const Matrix& v)
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
						sum += u[i][k] * v[k][j];
					out[i][j] = sum;
				}
			}
		}

		/**
		* @brief Performs a matrix multiplication
		*/
		inline Matrix operator*(const Matrix& u, const Matrix& v)
		{
			Matrix out = Matrix(u.Rows(), v.Cols());
			Mulitply(out, u, v);
			return out;
		}

		/**
		* @brief Multiplies all entries in a matrix by a given value
		*/
		inline Matrix operator*(const Real a, const Matrix& mat)
		{
			Matrix out = Matrix(mat.Rows(), mat.Cols());
			for (int i = 0; i < mat.Rows(); i++)
			{
				for (int j = 0; j < mat.Cols(); j++)
				{
					for (int k = 0; k < mat.Cols(); k++)
						out[i][j] = a * mat[i][j];
				}
			}
			return out;
		}

		/**
		* @brief Multiplies all entries in a matrix by a given value
		*/
		inline Matrix operator*(const Matrix& mat, const Real a) { return a * mat; }

		/**
		* @brief Divides all entries in a matrix by a given value
		*/
		inline Matrix operator/(const Matrix& mat, const Real& a) { return (1.0 / a) * mat; }
	}
}

#endif