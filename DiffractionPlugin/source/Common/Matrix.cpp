/*
*
*  \matrix class
*
*/

#include "Common/Matrix.h"
#include "Common/Vec.h"

namespace UIE
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
	}
}