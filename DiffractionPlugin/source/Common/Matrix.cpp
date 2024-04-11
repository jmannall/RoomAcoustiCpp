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
		matrix::matrix(const std::vector<std::vector<Real>>& mat) : rows(mat.size())
		{
			if (rows > 0)
			{
				cols = mat[0].size();
				e = mat;
			}
			else
				cols = 0;
			column = std::vector<Real>(rows);
		}

		void matrix::AllocateSpace()
		{
			int r = rows;
			for (int i = 0; i < r; i++)
				e.push_back(std::vector<Real>(cols, 0.0));
			column = std::vector<Real>(rows);
		}

		void matrix::DeallocateSpace()
		{
			e.clear();
			/*for (int i = 0; i < rows; i++)
				e[i].clear();
			e.clear();*/
		}

		void matrix::Init(const const std::vector<std::vector<Real>>& mat)
		{
			AllocateSpace();
			e = mat;
		}

		matrix matrix::Transpose()
		{
			matrix mat = matrix(cols, rows);
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					mat.AddEntry(e[i][j], j, i);
				}
			}
			return mat;
		}

		void matrix::Inverse()
		{
			if (rows != cols) // Matrix must be square
				return;

			// Create the augmented matrix [A|I]
			for (int i = 0; i < rows; i++)
			{
				e[i].resize(2 * cols, 0.0);
				e[i][cols + i] = 1.0;
			}

			// Perform row operations
			for (int i = 0; i < rows; i++)
			{
				// Divide row by pivot
				Real pivot = e[i][i];
				for (int j = 0; j < 2 * cols; j++)
					e[i][j] /= pivot;

				// Eliminate other rows
				for (int j = 0; j < rows; j++)
				{
					if (j != i)
					{
						Real factor = e[j][i];
						for (int k = 0; k < 2 * cols; k++)
							e[j][k] -= factor * e[i][k];
					}
				}
			}

			// Remove the identity matrix
			for (int i = 0; i < rows; i++)
			{
				e[i].erase(e[i].begin(), e[i].begin() + cols);
				e[i].shrink_to_fit();
			}
		}
	}
}