/*
*
*  \vec class
*
*/

#include <cmath>

#include "Common/Vec.h"

namespace UIE
{
	namespace Common
	{
		//////////////////// vec class ////////////////////

		vec::vec(matrix& mat)
		{
			assert(mat.Cols() == 1);
			Init(mat.GetColumn(0));
		}

		void vec::Init(const std::vector<Real>& vec)
		{
			assert(rows == vec.size());
			for (int i = 0; i < rows; i++)
				e[i][0] = vec[i];
		}

		// Distributions
		void vec::RandomNormalDistribution()
		{
			std::normal_distribution<Real> distribution; // mean 0, standard deviation 1
			for (int i = 0; i < rows; i++)
			{
				e[i][0] = distribution(generator);
			}
		}

		void vec::RandomUniformDistribution()
		{
			std::uniform_real_distribution<Real> distribution; // a 0, b 1
			for (int i = 0; i < rows; i++)
			{
				e[i][0] = distribution(generator);
			}
		}

		void vec::RandomUniformDistribution(Real a, Real b)
		{
			std::uniform_real_distribution<Real> distribution(a, b);
			for (int i = 0; i < rows; i++)
			{
				e[i][0] = distribution(generator);
			}
		}

		void vec::Normalise()
		{
			Real norm = CalculateNormal();
			for (int i = 0; i < rows; i++)
			{
				e[i][0] = e[i][0] / norm;
			}
		}

		// Getters
		Real vec::CalculateNormal() const
		{
			Real mag = 0.0;
			for (int i = 0; i < rows; i++)
			{
				mag += e[i][0] * e[i][0];
			}
			return sqrt(mag);
		}

		Real vec::Mean() const
		{
			Real out = 0;
			for (int i = 0; i < rows; i++)
			{
				out += e[i][0];
			}
			return out / rows;
		}

		//////////////////// rowvec class ////////////////////

		rowvec::rowvec(const matrix& mat)
		{
			assert(mat.Rows() == 1);
			Init(mat.GetRow(0));
		}

		void rowvec::Init(const std::vector<Real>& vec)
		{
			assert(cols == vec.size());
			e[0] = vec;
		}
	}
}