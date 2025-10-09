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
#include "Common/Vec_private.h"

#if MATRIX_LIBRARY == CUSTOM_FLAG
namespace RAC
{
	namespace Common
	{
		//////////////////// Vec ////////////////////

		static std::default_random_engine generator(100); // Seed the generator

		////////////////////////////////////////

		template<typename T>
		void Vec<T>::Init(const std::vector<T>& vector)
		{
			this->data.cols = 1;
			this->data.rows = static_cast<int>(vector.size());
			this->data.matrix = vector;
		}

		////////////////////////////////////////

		void Vec<Real>::RandomNormalDistribution()
		{
			std::normal_distribution<Real> distribution; // mean 0, standard deviation 1
			for (int i = 0; i < this->data.rows; i++)
				this->data(i, 0) = distribution(generator);
		}

		////////////////////////////////////////

		void Vec<Real>::RandomUniformDistribution()
		{
			std::uniform_real_distribution<Real> distribution; // a 0, b 1
			for (int i = 0; i < data.rows; i++)
				data(i, 0) = distribution(generator);
		}

		////////////////////////////////////////

		void Vec<Real>::RandomUniformDistribution(Real a, Real b)
		{
			std::uniform_real_distribution<Real> distribution(a, b);
			for (int i = 0; i < data.rows; i++)
				data(i, 0) = distribution(generator);
		}

		////////////////////////////////////////

		void Vec<Real>::Normalise()
		{
			Real normal = Normal();
			for (int i = 0; i < this->data.rows; i++)
				this->data(i, 0) = this->data(i, 0) / normal;
		}

		////////////////////////////////////////

		template<>
		Real Vec<Real>::Normal() const
		{
			Real magnitude = 0.0;
			for (int i = 0; i < this->data.rows; i++)
				magnitude += this->data(i, 0) * this->data(i, 0);
			return sqrt(magnitude);
		}

		////////////////////////////////////////

		void Vec<Real>::Max(const Real min)
		{
			for (int i = 0; i < data.rows; i++)
				data(i, 0) = std::max(min, data(i, 0));
		}

		////////////////////////////////////////

		void Vec<Real>::Min(const Real max)
		{
			for (int i = 0; i < data.rows; i++)
				data(i, 0) = std::min(max, data(i, 0));
		}

		////////////////////////////////////////

		template<typename T>
		T Vec<T>::Sum() const
		{
			T sum = T(0.0);
			for (int i = 0; i < this->data.cols; i++)
				sum += this->data(i, 0);
			return sum;
		}

		//////////////////// Rowvec ////////////////////

		template<typename T>
		void Rowvec<T>::Init(const std::vector<T>& vector)
		{
			this->data.rows = 1;
			this->data.cols = static_cast<int>(vector.size());
			this->data.matrix = vector;
		}

		////////////////////////////////////////

		template<typename T>
		T Rowvec<T>::Sum() const
		{
			T sum = T(0.0);
			for (int i = 0; i < this->data.cols; i++)
				sum += this->data(0, i);
			return sum;
		}

		Real Dot(const Vec<>& u, const Vec<>& v)
		{
			assert(u.Length() == v.Length());
			Real out = 0.0;
			for (int i = 0; i < u.Length(); ++i)
				out += u(i) * v(i);
			return out;
		}

		Real ThreeWayDot(const Vec<>& u, const Vec<>& v, const Vec<>& w)
		{
			assert(u.Length() == v.Length());
			assert(u.Length() == w.Length());
			Real out = 0.0;
			for (int i = 0; i < u.Length(); ++i)
				out += u(i) * v(i) * w(i);
			return out;
		}

		//////////////////// Instantiate ////////////////////

		// we don't implement/use every function, so disable the warning (we can't re-enable it since the warning is generated after the file is parsed)
		#ifdef _MSC_VER
		#pragma warning (disable : 4661)
		#endif

		template class Vec<int>;
		template class Vec<Real>;
		template class Vec<Complex>;

		template class Rowvec<int>;
		template class Rowvec<Real>;
		template class Rowvec<Complex>;

	}
}
#endif // MATRIX_LIBRARY == CUSTOM_FLAG