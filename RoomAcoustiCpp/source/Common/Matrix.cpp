/*
*
*  \matrix class
*
*/

#include "Common/Matrix.h"
#include "Common/Vec.h"

namespace RAC
{
	namespace Common
	{
		Matrix::Matrix(const std::vector<std::vector<Real>>& matrix) : rows(static_cast<int>(matrix.size()))
		{
			Init(matrix);
		}

		void Matrix::Init(const std::vector<std::vector<Real>>& matrix)
		{
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

		void Matrix::AllocateSpace()
		{
			for (int i = 0; i < rows; i++)
				data.push_back(std::vector<Real>(cols, 0.0));
			column = std::vector<Real>(rows);
		}

		Matrix Matrix::Transpose()
		{
			assert(rows == data.size());
			Matrix mat = Matrix(cols, rows);
			for (int i = 0; i < rows; i++)
			{
				assert(cols == data[i].size());
				for (int j = 0; j < cols; j++)
					mat[j][i] = data[i][j];
			}
			return mat;
		}

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
	}
}