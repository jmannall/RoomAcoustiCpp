/*
* @class Vec, Rowvec
*
* @brief Declaration of Vec and Rowvec classes
*
*/

#ifndef Common_Vec_h
#define Common_Vec_h

// C++ headers
#include <cassert>

// Common headers
#include "Common/Types.h"
#include "Common/Matrix.h"

namespace RAC
{
	namespace Common
	{

		/**
		* Class that implements a column vector (n x 1)
		*
		* @details Inherits from Matrix
		*/
		class Vec : public Matrix
		{
		public:

			/**
			* Default constructor that initialises an empty Vec
			*/
			Vec() : Matrix() {}

			/**
			* Constructor that initialises a Vec of zeros
			* 
			* @param length The length of the vector
			*/
			Vec(const int& length) : Matrix(length, 1) {}

			/**
			* Constructor that initialises a Vec with data
			*
			* @param vector The input data to initialise the vector
			*/
			Vec(const std::vector<Real>& vector) : Matrix(static_cast<int>(vector.size()), 1) { Init(vector); }

			/**
			* @brief Default deconstructor
			*/
			~Vec() {};

			/**
			* @brief Randomly fills the vector with values from a normal distribution
			*/
			void RandomNormalDistribution();

			/**
			* @brief Randomly fills the vector with values from a uniform distribution between 0 and 1
			*/
			void RandomUniformDistribution();

			/**
			* @brief Randomly fills the vector with values from a uniform distribution
			* 
			* @param a The lower bound of the distribution
			* @param b The upper bound of the distribution
			*/
			void RandomUniformDistribution(Real a, Real b);

			/**
			* @brief Normalises the vector
			*/
			void Normalise();

			/**
			* @return The normal of the vector
			*/
			Real CalculateNormal() const;

			/**
			* @brief Applies the max function to each element in the vector
			* 
			* @param min The minimum value
			*/
			void Max(const Real min);

			/**
			* @brief Applies the min function to each element in the vector
			*
			* @param max The maximum value
			*/
			void Min(const Real max);

			/**
			* @return The sum of every element in the vector
			*/
			Real Sum() const;

			/**
			* @return The mean value of the vector
			*/
			inline Real Mean() const { return Sum() / rows; }

			/**
			* @brief Access the vector at the specified index
			*
			* @param i The index of the value to return
			* @return The value at the specified index
			*/
			inline Real operator[](const int i) const { return data[i][0]; }

			/**
			* @brief Access the vector at the specified index
			*
			* @param i The index of the value to return
			* @return A reference to the value at the specified index
			*/
			inline Real& operator[](const int i) { return data[i][0]; }

			/**
			* @brief Assigns a Matrix to a Vec
			*/
			inline Vec operator=(const Matrix& matrix)
			{
				assert(matrix.Cols() == 1);
				Matrix::operator=(matrix);
				return *this;
			}

		private:

			/**
			* @brief Initialise vector data from a std::vector
			*
			* @params vector Data to inialise the vector from
			*/
			void Init(const std::vector<Real>& vector);
		};

		/**
		* Class that implements a row vector (1 x n)
		*
		* @details Inherits from Matrix
		*/
		class Rowvec : public Matrix
		{
		public:

			/**
			* Default constructor that initialises an empty Rowvec
			*/
			Rowvec() : Matrix() {}

			/**
			* Constructor that initialises a Rowvec of zeros
			* 
			* @param length The length of the vector
			*/
			Rowvec(const int& length) : Matrix(1, length) {}

			/**
			* Constructor that initialises a Rowvec with data
			*
			* @param vector The input data to initialise the vector
			*/
			Rowvec(const std::vector<Real>& vector) : Matrix(1, static_cast<int>(vector.size())) { Init(vector); }

			/**
			* @brief Default deconstructor
			*/
			~Rowvec() {}

			Real Sum() const;

			/**
			* @brief Access the vector at the specified index
			*
			* @param i The index of the value to return
			* @return The value at the specified index
			*/
			inline Real operator[](const int i) const { return data[0][i]; }

			/**
			* @brief Access the vector at the specified index
			*
			* @param i The index of the value to return
			* @return A reference to the value at the specified index
			*/
			inline Real& operator[](const int i) { return data[0][i]; }

			/**
			* @brief Assigns a Matrix to a Rowvec
			*/
			inline Rowvec operator=(const Matrix& matrix)
			{
				assert(mat.Rows() == 1);
				Matrix::operator=(matrix);
				return *this;
			}

		private:

			/**
			* @brief Initialise vector data from a std::vector
			*
			* @params vector Data to inialise the vector from
			*/
			void Init(const std::vector<Real>& vec);
		};

	}
}

#endif