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
		template class Matrix<Complex>;

		static std::default_random_engine generator(100); // Seed the generator

		////////////////////////////////////////

		template<typename T>
		void Matrix<T>::Init(const std::vector<std::vector<T>>& matrix)
		{
			data.rows = static_cast<int>(matrix.size());
			for (int i = 1; i < data.rows; i++)
				assert(matrix[i].size() == matrix[0].size());

			if (data.rows > 0)
			{
				data.cols = static_cast<int>(matrix[0].size());
				for (int i = 0; i < data.rows; i++)
				{
					for (int j = 0; j < data.cols; j++)
						data(i, j) = matrix[i][j];
				}
			}
			else
				data.cols = 0;
			column = std::vector<T>(data.rows);
			row = std::vector<T>(data.cols);
		}

		////////////////////////////////////////

		template<typename T>
		Matrix<T> Matrix<T>::Transpose()
		{
			assert(data.rows * data.cols == data.matrix.size());
			Matrix<T> matrix = Matrix<T>(data.cols, data.rows);
			for (int i = 0; i < data.rows; i++)
			{
				for (int j = 0; j < data.cols; j++)
					matrix(j, i) = data(i, j);
			}
			return matrix;
		}

		////////////////////////////////////////

		void Matrix<Real>::Inverse()
		{
			// TODO: Add unit tests for this function
			assert(data.rows == data.cols); // Matrix must be square

			// Create the augmented matrix [A|I]
			data.matrix.resize(2 * data.cols * data.rows, 0.0);
			for (int i = 0; i < data.rows; i++)
				data(data.rows + i, i) = 1.0;

			// Perform row operations
			for (int i = 0; i < data.rows; i++)
			{
				// Find the row with the largest absolute value in column i
				int maxRow = i;
				Real maxVal = std::abs(data(i, i));
				for (int r = i + 1; r < data.rows; r++)
				{
					if (std::abs(data(r, i)) > maxVal)
					{
						maxVal = std::abs(data(r, i));
						maxRow = r;
					}
				}

				// If pivot is too small, matrix is singular
				if (std::abs(data(maxRow, i)) < 1e-12)
					continue; // Matrix is singular, cannot invert

				// Swap data.rows if needed
				if (maxRow != i)
				{
					std::swap_ranges(
						data.matrix.begin() + i * data.cols,
						data.matrix.begin() + (i + 1) * data.cols,
						data.matrix.begin() + maxRow * data.cols
					);

					std::swap_ranges(
						data.matrix.begin() + (i + data.rows) * data.cols,
						data.matrix.begin() + (i + data.rows + 1) * data.cols,
						data.matrix.begin() + (maxRow + data.rows) * data.cols
					);
				}
				// std::swap(data[i], data[maxRow]);

				// Divide row by pivot
				Real pivot = data(i, i);
				for (int j = 0; j < data.cols; j++)
				{
					data(i, j) /= pivot;
					data(i + data.rows, j) /= pivot;
				}

				// Eliminate other data.rows
				for (int j = 0; j < data.rows; j++)
				{
					if (j != i)
					{
						Real factor = data(j, i);
						for (int k = 0; k < data.cols; k++)
						{
							data(j, k) -= factor * data(i, k);
							data(j + data.rows, k) -= factor * data(i + data.rows, k);
						}
					}
				}
			}

			// Remove the identity matrix
			data.matrix.erase(data.matrix.begin(), data.matrix.begin() + data.rows * data.cols);
			data.matrix.shrink_to_fit();
		}

		////////////////////////////////////////

		void Matrix<Real>::Log10()
		{
			for (auto& value : data.matrix)
				value = RAC::Common::Log10(value);
		}

		////////////////////////////////////////

		void Matrix<Real>::Pow10()
		{
			for (auto& value : data.matrix)
				value = RAC::Common::Pow10(value);
		}

		////////////////////////////////////////

		void Matrix<Real>::Max(const Real min)
		{
			for (auto& value : data.matrix)
				value = std::max(min, value);
		}

		////////////////////////////////////////

		void Matrix<Real>::Min(const Real max)
		{
			for (auto& value : data.matrix)
				value = std::min(max, value);
		}

		////////////////////////////////////////

		void Matrix<Real>::RandomUniformDistribution()
		{
			std::uniform_real_distribution<Real> distribution; // a 0, b 1
			for (auto& value : data.matrix)
				value = distribution(generator);
		}

		////////////////////////////////////////

		void Matrix<Real>::RandomUniformDistribution(Real a, Real b)
		{
			std::uniform_real_distribution<Real> distribution(a, b);
			for (auto& value : data.matrix)
				value = distribution(generator);
		}
	}
}