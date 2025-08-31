/*
* @class Matrix
*
* @brief Declaration of matrix class
*
*/

#ifndef Common_Matrix_private_h
#define Common_Matrix_private_h

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
		template<typename T = Real>
		class Matrix
		{
			/**
			* @brief Struct that stores matrix data in a contiguous vector
			*/
			struct ContiguousData
			{
				/**
				* @brief Access an element in the matrix
				* 
				* @param r The index of the row
				* @param c The index of the column
				* @return A reference to the matrix element at the specified index
				*/
				T& operator()(const int r, const int c) { return matrix[r * cols + c]; }

				/**
				* @brief Access an element in the matrix
				*
				* @param r The index of the row
				* @param c The index of the column
				* @return A const reference to the matrix element at the specified index
				*/
				const T& operator()(const int r, const int c) const { return matrix[r * cols + c]; }

				int rows, cols;				// Matrix dimensions
				std::vector<T> matrix;		// Contiguous data storage

				ContiguousData(int r, int c) : rows(r), cols(c), matrix(r * c, 0.0) {}
			};

		public:

			/**
			* Default constructor that initialises an empty Matrix
			*/
			Matrix() : Matrix(0, 0) {};

			/**
			* Constructor that initialises a Matrix with zeros
			*
			* @param r The number of data.rows
			* @param c The number of columns
			*/
			Matrix(const int r, const int c) : data(r, c), row(c), column(r) {};

			/**
			* Constructor that initialises a Matrix with data
			*
			* @param matrix The input data to initialise the matrix
			*/
			// Matrix(const std::vector<std::vector<T>>& matrix) : data(matrix.size(), 0) { Init(matrix); }

			/**
			* @brief Default deconstructor
			*/
			~Matrix() {};

			/**
			* @brief Reset the matrix to zeros
			*/
			inline void Reset()
			{
				std::fill(data.matrix.begin(), data.matrix.end(), 0.0);
			}

			/**
			* @brief Update data for a column of the matrix
			*
			* @param column The input column to add
			* @param c The column to add the vector to
			*/
			inline void AddColumn(const std::vector<T>& column, const int c)
			{
				assert(column.size() == data.rows);
				assert(c < data.cols);
				for (int i = 0; i < data.rows; i++)
					data(i, c) = column[i];
			}

			/**
			* @brief Update data for a row of the matrix
			*
			* @param row The input row to add
			* @param r The row to add the vector to
			*/
			inline void AddRow(const std::vector<T>& row, const int r)
			{
				assert(row.size() == data.cols);
				assert(r < data.rows);
				for (int i = 0; i < data.cols; i++)
					data(r, i) = row[i];
			}

			/**
			* @brief Get a row of the matrix
			*
			* @param r The row to get
			* @return Reference (const) to the row
			*/
			inline const std::vector<T>& GetRow(int r)
			{
				assert(r < data.rows);
				for (int i = 0; i < data.cols; i++)
					row[i] = data(r, i);
				return row;
			}

			/**
			* @brief Get a column of the matrix
			*
			* @param c The column to get
			* @return Reference (const) to the column
			*/
			inline const std::vector<T>& GetColumn(int c)
			{
				assert(c < data.cols);
				for (int i = 0; i < data.rows; i++)
					column[i] = data(i, c);
				return column;
			}

			const T* GetRowStartPtr(const int r) const { return &data(r, 0); }

			/**
			* @return The number of data.rows
			*/
			inline int Rows() const { return data.rows; }

			/**
			* @return The number of columns
			*/
			inline int Cols() const { return data.cols; }

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
			* @brief Fills the matrix with random values from a uniform distribution between 0 and 1
			*/
			void RandomUniformDistribution();

			/**
			* @brief Fills the matrix with random values from a uniform distribution
			* 
			* @param a The lower bound of the distribution
			* @param b The upper bound of the distribution
			*/
			void RandomUniformDistribution(Real a, Real b);

			/**
			* @brief Access an element in the matrix
			*
			* @param r The index of the row
			* @param c The index of the column
			* @return A reference to the matrix element at the specified index
			*/
			inline T& operator()(const int r, const int c) { return data(r, c); }

			/**
			* @brief Access an element in the matrix
			*
			* @param r The index of the row
			* @param c The index of the column
			* @return A const reference to the matrix element at the specified index
			*/
			inline const T& operator()(const int r, const int c) const { return data(r, c); }

			/**
			* @brief Adds a matrix to the current matrix
			*/
			inline Matrix operator+=(const Matrix& matrix)
			{
				assert(data.rows == matrix.Rows());
				assert(data.cols == matrix.Cols());

				for (int i = 0; i < data.rows; i++)
				{
					for (int j = 0; j < data.cols; j++)
						data(i, j) += matrix(i, j);
				}
				return *this;
			}

			/**
			* @brief Subtracts a matrix from the current matrix
			*/
			inline Matrix operator-=(const Matrix& matrix)
			{
				assert(data.rows == matrix.Rows());
				assert(data.cols == matrix.Cols());

				for (int i = 0; i < data.rows; i++)
				{
					for (int j = 0; j < data.cols; j++)
						data(i, j) -= matrix(i, j);
				}
				return *this;
			}

			/**
			* @brief Multiplies the matrix entries by a given value
			*/
			inline Matrix operator*=(const Real a)
			{
				for (int i = 0; i < data.rows; i++)
				{
					for (int j = 0; j < data.cols; j++)
						data(i, j) *= a;
				}
				return *this;
			}

			/**
			* @brief Divides the matrix entries by a given value
			*/
			inline Matrix operator/=(const Real a) { return *this *= ((Real)1.0 / a); }

			/**
			* @brief Adds a single value to all matrix entries
			*/
			inline Matrix operator+=(const T a)
			{
				for (int i = 0; i < data.rows; i++)
				{
					for (int j = 0; j < data.cols; j++)
						data(i, j) += a;
				}
				return *this;
			}

			/**
			* @brief Subtracts a single value from all matrix entries
			*/
			inline Matrix operator-=(const T a) { return *this += (-a); }

		protected:

			ContiguousData data;		// Store matrix data in a contiguous vector
			std::vector<T> row;			// Stores a single row
			std::vector<T> column;		// Stores a single column

		private:

			/**
			* @brief Initialise matrix data from std::vector structure
			*
			* @params matrix Data to inialise the matrix from
			*/
			void Init(const std::vector<std::vector<T>>& matrix);
		};

		//////////////////// Operators ////////////////////

		/**
		* @brief Performs an element-wise comparison
		* @return True if all element pairs are equal, false otherwise
		*/
		template<typename T>
		inline bool operator==(const Matrix<T>& u, const Matrix<T>& v)
		{
			assert(u.Rows() == v.Rows());
			assert(u.Cols() == v.Cols());

			if (u.Rows() != v.Rows())
				return false;

			if (u.Cols() != v.Cols())
				return false;

			bool equal = true;
			int i = 0;
			while (equal && i < u.Rows())
			{
				int j = 0;
				while (equal && j < u.Cols())
				{
					equal = u(i, j) == v(i, j);
					j++;
				}
				i++;
			}
			return equal;
		}

		/**
		* @return The sum of two matrices
		*/
		template<typename T>
		inline Matrix<T> operator+(const Matrix<T>& u, const Matrix<T>& v)
		{
			assert(u.Rows() == v.Rows());
			assert(u.Cols() == v.Cols());

			Matrix out = Matrix(u.Rows(), u.Cols());
			for (int i = 0; i < u.Rows(); i++)
			{
				for (int j = 0; j < u.Cols(); j++)
				{
					Real entry = u(i, j) + v(i, j);
					out(i, j) = entry;
				}
			}
			return out;
		}

		/**
		* @brief Inverts the sign of all the matrix entries
		*/
		template<typename T>
		inline Matrix<T> operator-(const Matrix<T>& mat)
		{
			Matrix out = Matrix(mat.Rows(), mat.Cols());
			for (int i = 0; i < mat.Rows(); i++)
			{
				for (int j = 0; j < mat.Cols(); j++)
					out(i, j) = -mat(i, j);
			}
			return out;
		}

		/**
		* @return Subtracts a matrix from another
		*/
		template<typename T>
		inline Matrix<T> operator-(const Matrix<T>& u, const Matrix<T>& v)
		{
			assert(u.Rows() == v.Rows());
			assert(u.Cols() == v.Cols());
			Matrix out = Matrix(u.Rows(), u.Cols());
			for (int i = 0; i < u.Rows(); i++)
			{
				for (int j = 0; j < u.Cols(); j++)
					out(i, j) = u(i, j) - v(i, j);
			}
			return out;
		}

		/**
		* @brief Performs a matrix multiplication
		*/
		template<typename T>
		inline void Multiply(Matrix<T>& out, const Matrix<T>& u, const Matrix<T>& v)
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
						sum += u(i, k) * v(k, j);
					out(i, j) = sum;
				}
			}
		}

		/**
		* @brief Performs a matrix multiplication
		*/
		template<typename T>
		inline Matrix<T> operator*(const Matrix<T>& u, const Matrix<T>& v)
		{
			Matrix out = Matrix(u.Rows(), v.Cols());
			Multiply(out, u, v);
			return out;
		}

		/**
		* @brief Multiplies all entries in a matrix by a given value
		*/
		template<typename T>
		inline Matrix<T> operator*(const Real a, const Matrix<T>& mat)
		{
			Matrix out = Matrix(mat.Rows(), mat.Cols());
			for (int i = 0; i < mat.Rows(); i++)
			{
				for (int j = 0; j < mat.Cols(); j++)
				{
					for (int k = 0; k < mat.Cols(); k++)
						out(i, j) = a * mat(i, j);
				}
			}
			return out;
		}

		/**
		* @brief Multiplies all entries in a matrix by a given value
		*/
		template<typename T>
		inline Matrix<T> operator*(const Matrix<T>& mat, const Real a) { return a * mat; }

		/**
		* @brief Divides all entries in a matrix by a given value
		*/
		template<typename T>
		inline Matrix<T> operator/(const Matrix<T>& mat, const Real& a) { return (1.0 / a) * mat; }
	}
}

#endif // Common_Matrix_private_h