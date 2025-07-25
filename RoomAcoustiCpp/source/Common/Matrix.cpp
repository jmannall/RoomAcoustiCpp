/*
* @class Matrix
*
* @brief Declaration of matrix class
*
*/

// C++ headers
#include <random>

// Common headers
#include "Common/Matrix_private.h"
#include "Common/Definitions.h"
#include "Common/Complex.h"

namespace RAC
{
	namespace Common
	{
		//////////////////// Matrix ////////////////////

		template class Matrix<Real>;
		template class Matrix<ComplexPair>;
		template class Matrix<Complex>;

		static std::default_random_engine generator(100); // Seed the generator

		////////////////////////////////////////

		template<typename T>
		void Matrix<T>::Init(const std::vector<std::vector<T>>& matrix)
		{
			rows = static_cast<int>(matrix.size());
			for (int i = 1; i < rows; i++)
				assert(matrix[i].size() == matrix[0].size());

			if (rows > 0)
			{
				cols = static_cast<int>(matrix[0].size());
				data = matrix;
			}
			else
				cols = 0;
			column = std::vector<T>(rows);
		}

		////////////////////////////////////////

		template<typename T>
		void Matrix<T>::AllocateSpace()
		{
			for (int i = 0; i < rows; i++)
				data.push_back(std::vector<T>(cols, 0.0));
			column = std::vector<T>(rows);
		}

		////////////////////////////////////////

		template<typename T>
		Matrix<T> Matrix<T>::Transpose()
		{
			assert(rows == data.size());
			Matrix<T> matrix = Matrix<T>(cols, rows);
			for (int i = 0; i < rows; i++)
			{
				assert(cols == data[i].size());
				for (int j = 0; j < cols; j++)
					matrix[j][i] = data[i][j];
			}
			return matrix;
		}

		////////////////////////////////////////

		void Matrix<Real>::Inverse()
		{
			assert(rows == cols); // Matrix must be square

			// Create the augmented matrix [A|I]
			for (int i = 0; i < rows; i++)
			{
				data[i].resize(2 * cols, 0.0);
				data[i][cols + i] = 1.0;
			}

			// Perform row operations
			for (int i = 0; i < rows; i++)
			{
				// Find the row with the largest absolute value in column i
				int maxRow = i;
				Real maxVal = std::abs(data[i][i]);
				for (int r = i + 1; r < rows; r++)
				{
					if (std::abs(data[r][i]) > maxVal)
					{
						maxVal = std::abs(data[r][i]);
						maxRow = r;
					}
				}

				// If pivot is too small, matrix is singular
				if (std::abs(data[maxRow][i]) < 1e-12)
					continue; // Matrix is singular, cannot invert

				// Swap rows if needed
				if (maxRow != i)
					std::swap(data[i], data[maxRow]);

				// Divide row by pivot
				Real pivot = data[i][i];
				for (int j = 0; j < 2 * cols; j++)
					data[i][j] /= pivot;

				// Eliminate other rows
				for (int j = 0; j < rows; j++)
				{
					if (j != i)
					{
						Real factor = data[j][i];
						for (int k = 0; k < 2 * cols; k++)
							data[j][k] -= factor * data[i][k];
					}
				}
			}

			// Remove the identity matrix
			for (int i = 0; i < rows; i++)
			{
				data[i].erase(data[i].begin(), data[i].begin() + cols);
				data[i].shrink_to_fit();
			}
		}

		////////////////////////////////////////

		void Matrix<Real>::Log10()
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = RAC::Common::Log10(data[i][j]);
			}
		}

		////////////////////////////////////////

		void Matrix<Real>::Pow10()
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = RAC::Common::Pow10(data[i][j]);
			}
		}

		////////////////////////////////////////

		void Matrix<Real>::Max(const Real min)
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = std::max(min, data[i][j]);
			}
		}

		////////////////////////////////////////

		void Matrix<Real>::Min(const Real max)
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = std::min(max, data[i][j]);
			}
		}

		////////////////////////////////////////

		void Matrix<Real>::RandomUniformDistribution()
		{
			std::uniform_real_distribution<Real> distribution; // a 0, b 1
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = distribution(generator);
			}
		}

		////////////////////////////////////////

		void Matrix<Real>::RandomUniformDistribution(Real a, Real b)
		{
			std::uniform_real_distribution<Real> distribution(a, b);
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = distribution(generator);
			}
		}
	}
}