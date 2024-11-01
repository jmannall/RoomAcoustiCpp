/*
*
*  \vec class
*
*/

#include <cmath>

#include "Common/Vec.h"

namespace RAC
{
	namespace Common
	{
		//////////////////// vec class ////////////////////

		Vec::Vec(Matrix& mat)
		{
			assert(mat.Cols() == 1);
			Init(mat.GetColumn(0));
		}

		void Vec::Init(const std::vector<Real>& vec)
		{
			assert(rows == vec.size());
			for (int i = 0; i < rows; i++)
				data[i][0] = vec[i];
		}

		// Distributions
		void Vec::RandomNormalDistribution()
		{
			std::normal_distribution<Real> distribution; // mean 0, standard deviation 1
			for (int i = 0; i < rows; i++)
			{
				data[i][0] = distribution(generator);
			}
		}

		void Vec::RandomUniformDistribution()
		{
			std::uniform_real_distribution<Real> distribution; // a 0, b 1
			for (int i = 0; i < rows; i++)
			{
				data[i][0] = distribution(generator);
			}
		}

		void Vec::RandomUniformDistribution(Real a, Real b)
		{
			std::uniform_real_distribution<Real> distribution(a, b);
			for (int i = 0; i < rows; i++)
			{
				data[i][0] = distribution(generator);
			}
		}

		void Vec::Normalise()
		{
			Real norm = CalculateNormal();
			for (int i = 0; i < rows; i++)
			{
				data[i][0] = data[i][0] / norm;
			}
		}

		// Getters
		Real Vec::CalculateNormal() const
		{
			Real mag = 0.0;
			for (int i = 0; i < rows; i++)
			{
				mag += data[i][0] * data[i][0];
			}
			return sqrt(mag);
		}

		Real Vec::Mean() const
		{
			Real out = 0;
			for (int i = 0; i < rows; i++)
			{
				out += data[i][0];
			}
			return out / rows;
		}

		//////////////////// rowvec class ////////////////////

		Rowvec::Rowvec(const Matrix& mat)
		{
			assert(mat.Rows() == 1);
			Init(mat.GetRow(0));
		}

		void Rowvec::Init(const std::vector<Real>& vec)
		{
			assert(cols == vec.size());
			data[0] = vec;
		}
	}
}