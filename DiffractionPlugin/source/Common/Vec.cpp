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

		vec::vec(const matrix& mat)
		{
			assert(mat.Cols() == 1);
			length = mat.Rows();
			Real* v = mat.GetColumn(0);
			Init(v);
		}

		// Distributions
		void vec::RandomNormalDistribution()
		{
			std::normal_distribution<Real> distribution; // mean 0, standard deviation 1
			for (int i = 0; i < length; i++)
			{
				e[i][0] = distribution(generator);
			}
		}

		void vec::RandomUniformDistribution()
		{
			std::uniform_real_distribution<Real> distribution; // a 0, b 1
			for (int i = 0; i < length; i++)
			{
				e[i][0] = distribution(generator);
			}
		}

		void vec::RandomUniformDistribution(Real a, Real b)
		{
			std::uniform_real_distribution<Real> distribution(a, b);
			for (int i = 0; i < length; i++)
			{
				e[i][0] = distribution(generator);
			}
		}

		void vec::Normalise()
		{
			Real norm = CalculateNormal();
			for (int i = 0; i < length; i++)
			{
				e[i][0] = e[i][0] / norm;
			}
		}

		// Getters
		Real vec::CalculateNormal() const
		{
			Real mag = 0.0;
			for (int i = 0; i < length; i++)
			{
				mag += pow(e[i][0], 2.0);
			}
			return sqrt(mag);
		}

		Real vec::Mean() const
		{
			Real out = 0;
			for (int i = 0; i < length; i++)
			{
				out += e[i][0];
			}
			return out / length;
		}

		//////////////////// rowvec class ////////////////////

		rowvec::rowvec(const matrix& mat)
		{
			assert(mat.Rows() == 1);
			Real* v = mat.GetRow(0);
			Init(v);
		}
	}
}