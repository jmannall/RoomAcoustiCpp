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
			vec() : matrix(), length(1)	{ Init(); }
			vec(const int& len) : matrix(len, 1), length(len) { Init(); }
			vec(const Real* vec, const int& len) : matrix(len, 1), length(len) { Init(vec); }
			vec(const matrix& mat);
			~vec() {};

			// Distributions
			void RandomNormalDistribution();
			void RandomUniformDistribution();
			void RandomUniformDistribution(Real a, Real b);

			void Normalise();

			// Getters
			Real CalculateNormal() const;
			Real Mean() const;

			// Operators
			inline Real& operator[] (const int& i) const { return e[i][0]; }

			inline vec operator=(const matrix& mat)
			{
				assert(mat.Cols() == 1);

				rows = mat.Rows();
				cols = mat.Cols();
				Init();
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

			// Member Variables
			int length;
		};

		//////////////////// rowvec class ////////////////////

		class rowvec : public matrix
		{
		public:

			// Load and Destroy
			rowvec() : matrix() { Init(); }
			rowvec(const int& c) : matrix(1, c) { Init(); }
			rowvec(const Real* vec, const int& c) : matrix(1, c) { Init(vec); }
			rowvec(const matrix& mat);
			~rowvec() {}

			// Operators
			inline Real& operator[] (const int& i) const { return e[0][i]; }

			inline rowvec operator=(const matrix& mat)
			{
				assert(mat.Rows() == 1);

				rows = mat.Rows();
				cols = mat.Cols();
				Init();
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