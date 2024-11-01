/*
* @class vec, rowvec
*
* @brief Declaration of vec and rowvec classes
*
*/

#ifndef Common_Vec_h
#define Common_Vec_h

// C++ headers
#include <cassert>
#include <random>

// Common headers
#include "Common/Types.h"
#include "Common/Matrix.h"

static std::default_random_engine generator (100);

namespace RAC
{
	namespace Common
	{

		//////////////////// vec class ////////////////////
		
		class Vec : public Matrix
		{
		public:

			// Load and Destroy
			Vec() : Matrix() {}
			Vec(const int& len) : Matrix(len, 1) {}
			Vec(const std::vector<Real>& vec) : Matrix(static_cast<int>(vec.size()), 1) { Init(vec); }
			Vec(Matrix& mat);
			~Vec() {};

			void Init(const std::vector<Real>& vec);

			// Distributions
			void RandomNormalDistribution();
			void RandomUniformDistribution();
			void RandomUniformDistribution(Real a, Real b);

			void Normalise();

			// Getters
			Real CalculateNormal() const;
			Real Mean() const;

			inline void Max(const Real min)
			{ 
				for (int i = 0; i < rows; i++)
					data[i][0] = std::max(min, data[i][0]);
			}

			inline Real Sum() const
			{
				Real sum = 0.0;
				for (int i = 0; i < cols; i++)
					sum += data[i][0];
				return sum;
			}

			// Operators
			inline Real operator[](const int i) const { return data[i][0]; }
			inline Real& operator[](const int i) { return data[i][0]; }

			inline Vec operator=(const Matrix& mat)
			{
				assert(mat.Cols() == 1);
				cols = mat.Cols();

				if (rows != mat.Rows())
					data.resize(mat.Cols(), std::vector<Real>(1, 0.0));

				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						data[i][j] = mat.GetEntry(i, j);
					}
				}
				return *this;
			}

		private:
		};

		//////////////////// rowvec class ////////////////////

		class Rowvec : public Matrix
		{
		public:

			// Load and Destroy
			Rowvec() : Matrix() {}
			Rowvec(const int& c) : Matrix(1, c) {}
			Rowvec(const std::vector<Real>& vec) : Matrix(1, vec.size()) { Init(vec); }
			Rowvec(const Matrix& mat);
			~Rowvec() {}

			void Init(const std::vector<Real>& vec);

			inline Real Sum() const
			{
				Real sum = 0.0;
				for (int i = 0; i < cols; i++)
					sum += data[0][i];
				return sum;
			}


			// Operators
			inline Real operator[](const int i) const { return data[0][i]; }
			inline Real& operator[](const int i) { return data[0][i]; }

			inline Rowvec operator=(const Matrix& mat)
			{
				assert(mat.Rows() == 1);
				rows = mat.Rows();

				if (cols != mat.Cols())
					data[0].resize(mat.Cols());
				
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						data[i][j] = mat.GetEntry(i, j);
					}
				}
				return *this;
			}
		};

	}
}

#endif