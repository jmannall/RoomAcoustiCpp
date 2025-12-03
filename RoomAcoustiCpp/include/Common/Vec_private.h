/*
* @class Vec, Rowvec
*
* @brief Declaration of Vec and Rowvec classes
*
*/

#ifndef Common_Vec_private_h
#define Common_Vec_private_h

// C++ headers
#include <cassert>

// Common headers
#include "Common/Types.h"
#include "Common/Matrix.h"

#if MATRIX_LIBRARY == CUSTOM_FLAG
namespace RAC
{
	namespace Common
	{
		/**
		* Class that implements a column vector (n x 1)
		*
		* @details Inherits from Matrix
		*/
		template<typename T = Real>
		class Vec : public Matrix<T>
		{
		public:

			/**
			* Default constructor that initialises an empty Vec
			*/
			Vec() : Matrix<T>() {}

			/**
			* Constructor that initialises a Vec of zeros
			*
			* @param length The length of the vector
			*/
			Vec(const int length) : Matrix<T>(length, 1) {}

			/**
			* Constructor that initialises a Vec with data
			*
			* @param vector The input data to initialise the vector
			*/
			Vec(const std::vector<T>& vector) : Matrix<T>(static_cast<int>(vector.size()), 1) { Init(vector); }

			/**
			* @brief Default deconstructor
			*/
			~Vec() {};

			// Static factory: zeros
			static Vec Zero(const int length) { return Vec(length); }

			// Static factory: constant
			static Vec Constant(const int length, const T value) { return Vec(length, value); }

			/**
			* @return The length of the vector
			*/
			inline int Length() const { return this->data.rows; }

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
			Real Normal() const;

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
			T Sum() const;

			/**
			* @return The mean value of the vector
			*/
			inline T Mean() const { return Sum() / this->data.rows; }

			/**
			* @brief Access the vector at the specified index
			*
			* @param i The index of the value to return
			* @return The value at the specified index
			*/
			inline T operator()(const int i) const { return this->data(i, 0); }

			/**
			* @brief Access the vector at the specified index
			*
			* @param i The index of the value to return
			* @return A reference to the value at the specified index
			*/
			inline T& operator()(const int i) { return this->data(i, 0); }

			/**
			* @brief Assigns a Matrix to a Vec
			*/
			inline Vec operator=(const Matrix<T>& matrix)
			{
				Debug::Assert(matrix.Cols() == 1, "Matrix must have only one column");
				Matrix<T>::operator=(matrix);
				return *this;
			}

			/**
			 * @brief For eigen compatibility
			 */
			inline Vec& noalias()
			{
				return *this;
			}

		private:
			/**
			* Constructor that initialises a Rowvec with a given value
			*
			* @param length The length of the vector
			* @param value The value to initialise the vector with
			*/
			Vec(const int length, const T value) : Matrix<T>(length, 1)
			{
				for (int i = 0; i < length; ++i)
					this->data(i, 0) = value;
			}

			/**
			* @brief Initialise vector data from a std::vector
			*
			* @params vector Data to inialise the vector from
			*/
			void Init(const std::vector<T>& vector);
		};

		/**
		* Class that implements a row vector (1 x n)
		*
		* @details Inherits from Matrix
		*/
		template<typename T = Real>
		class Rowvec : public Matrix<T>
		{
		public:

			/**
			* Default constructor that initialises an empty Rowvec
			*/
			Rowvec() : Matrix<T>() {}

			/**
			* Constructor that initialises a Rowvec of zeros
			*
			* @param length The length of the vector
			*/
			Rowvec(const int length) : Matrix<T>(1, length) {}

			/**
			* Constructor that initialises a Rowvec with data
			*
			* @param vector The input data to initialise the vector
			*/
			Rowvec(const std::vector<T>& vector) : Matrix<T>(1, static_cast<int>(vector.size())) { Init(vector); }

			/**
			* @brief Default deconstructor
			*/
			~Rowvec() {}

			// Static factory: zeros
			static Rowvec Zero(const int length) { return Rowvec(length); }

			// Static factory: constant
			static Rowvec Constant(const int length, const T value) { return Rowvec(length, value); }

			/**
			* @return The length of the vector
			*/
			inline int Length() const { return this->data.cols; }

			T Sum() const;

			inline T Mean() const { return Sum() / this->data.cols; }

			/**
			* @brief Access the vector at the specified index
			*
			* @param i The index of the value to return
			* @return The value at the specified index
			*/
			inline T operator()(const int i) const { return this->data(0, i); }

			/**
			* @brief Access the vector at the specified index
			*
			* @param i The index of the value to return
			* @return A reference to the value at the specified index
			*/
			inline T& operator()(const int i) { return this->data(0, i); }

			/**
			* @brief Assigns a Matrix to a Rowvec
			*/
			inline Rowvec operator=(const Matrix<T>& matrix)
			{
				Debug::Assert(matrix.Rows() == 1, "Matrix must have only one row");
				Matrix<T>::operator=(matrix);
				return *this;
			}

		private:
			/**
			* Constructor that initialises a Rowvec with a given value
			*
			* @param length The length of the vector
			* @param value The value to initialise the vector with
			*/
			Rowvec(const int length, const T value) : Matrix<T>(1, length)
			{
				for (int i = 0; i < length; ++i)
					this->data(0, i) = value;
			}

			/**
			* @brief Initialise vector data from a std::vector
			*
			* @params vector Data to inialise the vector from
			*/
			void Init(const std::vector<T>& vec);
		};

		/**
		* @brief Calculates the dot product of two vectors
		*/
		Real Dot(const Vec<>& u, const Vec<>& v);

		/**
		* @brief Calculates the sum of the element-wise product of three vectors
		*/
		Real ThreeWayDot(const Vec<>& u, const Vec<>& v, const Vec<>& w);
	}
}

#endif // Common_Vec_private_h
#endif // MATRIX_LIBRARY == CUSTOM_FLAG