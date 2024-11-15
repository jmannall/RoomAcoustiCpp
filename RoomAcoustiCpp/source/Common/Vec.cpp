/*
* @class Vec, Rowvec
*
* @brief Declaration of Vec and Rowvec classes
*
*/

// C++ headers
#include <cmath>
#include <random>

// Common headers
#include "Common/Vec.h"

namespace RAC
{
	namespace Common
	{
		//////////////////// Vec ////////////////////

		static std::default_random_engine generator(100); // Seed the generator

		////////////////////////////////////////

		void Vec::Init(const std::vector<Real>& vector)
		{
			rows = static_cast<int>(vector.size());
			data.resize(rows, std::vector<Real>(1, 0.0));
			for (int i = 0; i < rows; i++)
				data[i][0] = vector[i];
		}

		////////////////////////////////////////

		void Vec::RandomNormalDistribution()
		{
			std::normal_distribution<Real> distribution; // mean 0, standard deviation 1
			for (int i = 0; i < rows; i++)
				data[i][0] = distribution(generator);
		}

		////////////////////////////////////////

		void Vec::RandomUniformDistribution()
		{
			std::uniform_real_distribution<Real> distribution; // a 0, b 1
			for (int i = 0; i < rows; i++)
				data[i][0] = distribution(generator);
		}

		////////////////////////////////////////

		void Vec::RandomUniformDistribution(Real a, Real b)
		{
			std::uniform_real_distribution<Real> distribution(a, b);
			for (int i = 0; i < rows; i++)
				data[i][0] = distribution(generator);
		}

		////////////////////////////////////////

		void Vec::Normalise()
		{
			Real normal = CalculateNormal();
			for (int i = 0; i < rows; i++)
				data[i][0] = data[i][0] / normal;
		}

		Real Vec::CalculateNormal() const
		{
			Real magnitude = 0.0;
			for (int i = 0; i < rows; i++)
				magnitude += data[i][0] * data[i][0];
			return sqrt(magnitude);
		}

		////////////////////////////////////////

		void Vec::Max(const Real min)
		{
			for (int i = 0; i < rows; i++)
				data[i][0] = std::max(min, data[i][0]);
		}

		void Vec::Min(const Real max)
		{
			for (int i = 0; i < rows; i++)
				data[i][0] = std::min(max, data[i][0]);
		}

		////////////////////////////////////////

		Real Vec::Sum() const
		{
			Real sum = 0.0;
			for (int i = 0; i < cols; i++)
				sum += data[i][0];
			return sum;
		}

		//////////////////// Rowvec ////////////////////

		////////////////////////////////////////

		void Rowvec::Init(const std::vector<Real>& vector)
		{
			cols = static_cast<int>(vector.size());
			data[0] = vector;
		}

		////////////////////////////////////////

		Real Rowvec::Sum() const
		{
			Real sum = 0.0;
			for (int i = 0; i < cols; i++)
				sum += data[0][i];
			return sum;
		}
	}
}