/*
* @class Matrix
*
* @brief Declaration of matrix class
*
*/

// C++ headers
#include <random>

// Common headers
#include "Common/Matrix.h"
#include "Common/Definitions.h"

namespace RAC
{
	namespace Common
	{
		//////////////////// Matrix ////////////////////

		static std::default_random_engine generator(100); // Seed the generator

		////////////////////////////////////////

		void Matrix::Init(const std::vector<std::vector<Real>>& matrix)
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
			column = std::vector<Real>(rows);
		}

		////////////////////////////////////////

		void Matrix::AllocateSpace()
		{
			for (int i = 0; i < rows; i++)
				data.push_back(std::vector<Real>(cols, 0.0));
			column = std::vector<Real>(rows);
		}

		////////////////////////////////////////

		Matrix Matrix::Transpose()
		{
			assert(rows == data.size());
			Matrix matrix = Matrix(cols, rows);
			for (int i = 0; i < rows; i++)
			{
				assert(cols == data[i].size());
				for (int j = 0; j < cols; j++)
					matrix[j][i] = data[i][j];
			}
			return matrix;
		}

		////////////////////////////////////////

		void Matrix::Inverse()
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

		void Matrix::Log10()
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = RAC::Common::Log10(data[i][j]);
			}
		}

		////////////////////////////////////////

		void Matrix::Pow10()
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = RAC::Common::Pow10(data[i][j]);
			}
		}

		////////////////////////////////////////

		void Matrix::Max(const Real min)
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = std::max(min, data[i][j]);
			}
		}

		////////////////////////////////////////

		void Matrix::Min(const Real max)
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = std::min(max, data[i][j]);
			}
		}

		////////////////////////////////////////

		void Matrix::RandomUniformDistribution()
		{
			std::uniform_real_distribution<Real> distribution; // a 0, b 1
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
					data[i][j] = distribution(generator);
			}
		}

		////////////////////////////////////////

		void Matrix::RandomUniformDistribution(Real a, Real b)
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