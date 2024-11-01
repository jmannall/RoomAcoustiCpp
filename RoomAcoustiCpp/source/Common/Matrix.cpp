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
		Matrix::Matrix(const std::vector<std::vector<Real>>& mat) : rows(mat.size())
		{
			if (rows > 0)
			{
				cols = mat[0].size();
				data = mat;
			}
			else
				cols = 0;
			column = std::vector<Real>(rows);
		}

		void Matrix::AllocateSpace()
		{
			int r = rows;
			for (int i = 0; i < r; i++)
				data.push_back(std::vector<Real>(cols, 0.0));
			column = std::vector<Real>(rows);
		}

		void Matrix::DeallocateSpace()
		{
			data.clear();
			/*for (int i = 0; i < rows; i++)
				e[i].clear();
			e.clear();*/
		}

		void Matrix::Init(const const std::vector<std::vector<Real>>& mat)
		{
			AllocateSpace();
			data = mat;
		}

		Matrix Matrix::Transpose()
		{
			Matrix mat = Matrix(cols, rows);
			for (int i = 0; i < rows; i++)
			{
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