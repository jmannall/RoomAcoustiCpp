/*
*
*  \vec class
*
*/

#ifndef Common_Vec_h
#define Common_Vec_h

#include <cassert>
#include <random>

#include "Common/Types.h"
#include "Common/Matrix.h"

static std::default_random_engine generator (100);

namespace UIE
{
	namespace Common
	{

		//////////////////// vec class ////////////////////
		
		class vec : public matrix
		{
		public:

			// Load and Destroy
			vec() : matrix() {}
			vec(const int& len) : matrix(len, 1) {}
			vec(const std::vector<Real>& vec) : matrix(static_cast<int>(vec.size()), 1) { Init(vec); }
			vec(matrix& mat);
			~vec() {};

			void Init(const std::vector<Real>& vec);

			// Distributions
			void RandomNormalDistribution();
			void RandomUniformDistribution();
			void RandomUniformDistribution(Real a, Real b);

			void Normalise();

			// Getters
			Real CalculateNormal() const;
			Real Mean() const;

			inline void AddEntry(const Real& in, const int& i) { e[i][0] = in; }
			inline void IncreaseEntry(const Real& in, const int& i) { e[i][0] += in; }

			inline void Max(const Real& min)
			{ 
				for (int i = 0; i < rows; i++)
					e[i][0] = std::max(min, e[i][0]);
			}

			inline Real GetEntry(const int& i) const { return e[i][0]; }

			inline Real Sum() const
			{
				Real sum = 0.0;
				for (int i = 0; i < cols; i++)
					sum += e[i][0];
				return sum;
			}

			// Operators
			inline vec operator=(const matrix& mat)
			{
				assert(mat.Cols() == 1);
				cols = mat.Cols();

				if (rows != mat.Rows())
					e.resize(mat.Cols(), std::vector<Real>(1, 0.0));

				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						e[i][j] = mat.GetEntry(i, j);
					}
				}
				return *this;
			}

		private:
		};

		//////////////////// rowvec class ////////////////////

		class rowvec : public matrix
		{
		public:

			// Load and Destroy
			rowvec() : matrix() {}
			rowvec(const int& c) : matrix(1, c) {}
			rowvec(const std::vector<Real>& vec, const int& c) : matrix(1, c) { Init(vec); }
			rowvec(const matrix& mat);
			~rowvec() {}

			void Init(const std::vector<Real>& vec);

			inline void AddEntry(const Real& in, const int& i) { e[0][i] = in; }
			inline void IncreaseEntry(const Real& in, const int& i) { e[0][i] += in; }

			inline Real GetEntry(const int& i) const { return e[0][i]; }

			inline Real Sum() const
			{
				Real sum = 0.0;
				for (int i = 0; i < cols; i++)
					sum += e[0][i];
				return sum;
			}


			// Operators
			inline rowvec operator=(const matrix& mat)
			{
				assert(mat.Rows() == 1);
				rows = mat.Rows();

				if (cols != mat.Cols())
					e[0].resize(mat.Cols());
				
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						e[i][j] = mat.GetEntry(i, j);
					}
				}
				return *this;
			}
		};

	}
}

#endif