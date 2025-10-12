/*
* @class FDN, Channel
*
* @brief Declaration of FDN and Channel classes
*
*/

// C++ headers
#include <mutex>
//#include <xmmintrin.h>
#include <cmath>
#include <numeric>  // For std::gcd

#include "Common/RACProfiler.h"

// Spatialiser headers
#include "Spatialiser/FDN_private.h"
#include "Spatialiser/Globals.h"

// Unity headers
#include "Unity/Debug.h"

namespace RAC
{
	using namespace Common;
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// FDN Channel class ////////////////////

		////////////////////////////////////////

		template<>
		Real FDNChannel<Real>::GetOutput(const Real input, const Real lerpFactor)
		{
			if (idx >= mBuffer.Length())
				idx = 0;
			Real out = mAbsorptionFilter.GetOutput(mBuffer[idx], lerpFactor);
			mBuffer[idx] = input;
			++idx;
			return out;
		}

		////////////////////////////////////////

		template<>
		Complex FDNChannel<Complex>::GetOutput(const Complex input, const Real lerpFactor)
		{
			if (idx >= mBuffer.Length())
				idx = 0;
			// TODO: Interpolate absorption if needed? - is this only changed on a full reset?
			Complex out = mAbsorptionFilter.load(std::memory_order_acquire) * mBuffer[idx];
			mBuffer[idx] = input;
			++idx;
			return out;
		}

		//////////////////// FDN class ////////////////////

		template class FDN<Real>;
		template class FDN<Complex>;
		
		////////////////////////////////////////

		template<typename T>
		bool FDN<T>::IsSetMutuallyPrime(const Vec<int>& numbers)
		{
			for (int i = 0; i < numbers.Length(); ++i)
			{
				for (int j = i + 1; j < numbers.Length(); ++j)
				{
					if (std::gcd(numbers(i), numbers(j)) != 1)
						return false;
				}
			}
			return true;
		}

		////////////////////////////////////////

		template<typename T>
		bool FDN<T>::IsEntryMutuallyPrime(const Vec<int>& numbers, int idx)
		{
			for (int i = 0; i < numbers.Length(); ++i)
			{
				if (i == idx)
					continue;
				if (std::gcd(numbers(i), numbers(idx)) != 1)
					return false;
			}
			return true;
		}

		////////////////////////////////////////

		template<typename T>
		void FDN<T>::MakeSetMutuallyPrime(Vec<int>& numbers)
		{
			for (int i = 0; i < numbers.Length(); ++i)
			{
				int limit = static_cast<int>(round(0.1 * numbers(i)));
				for (int adjustment = 0; adjustment <= limit; ++adjustment)
				{
					int original = numbers(i);
					bool found = false;

					for (int sign : {-1, 1})
					{
						numbers(i) = original + sign * adjustment;
						if (IsEntryMutuallyPrime(numbers, i))
						{
							found = true;
							break;
						}
					}

					if (found)
						break;
					numbers(i) = original;
				}
			}
		}

		////////////////////////////////////////

		template<typename T>
		Vec<int> FDN<T>::CalculateTimeDelay(const Vec<>& dimensions, const int fdnSize, const int fs)
		{
			assert(dimensions.Length() >  0);

			Vec<> t(fdnSize);
			Vec<int> delays = Vec<int>::Constant(fdnSize, 1);
			if (dimensions.Length() > 0)
			{

				assert(dimensions.Length() <= fdnSize);

				// gives [-1:1], divide by 10 to get [-0.1:0.1]
				t.RandomUniformDistribution();
				t *= dimensions.Mean() / (Real)10.0;

				int k = 0;
				while (k < fdnSize)
				{
					for (int i = 0; i < dimensions.Length(); ++i)
					{
						if (k >= fdnSize)
							break;
						assert(dimensions(i) > 0.0);
						t(k) += dimensions(i);
						++k;
					}
				}
				t *= INV_SPEED_OF_SOUND;
				t.Max((Real)1.0 / static_cast<Real>(fs));

				for (int i = 0; i < fdnSize; i++)
					delays(i) = static_cast<int>(round(t(i) * static_cast<Real>(fs)));
				if (!IsSetMutuallyPrime(delays))
					MakeSetMutuallyPrime(delays);
			}
			return delays;
		}

		////////////////////////////////////////

		template <>
		void FDN<Real>::ProcessAudio(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData)
		{
			PROFILE_FDN
			if (audioData.clearBuffers)
				Reset();

			// Process feedback loop
			for (int i = 0; i < data.Cols(); i++)
			{
				for (int j = 0; j < mChannels.size(); j++)
					y(j) = mChannels[j]->GetOutput(x(j) + data(j, i), audioData.lerpFactor);

				for (int j = 0; j < outputBuffers.size(); j++)
					outputBuffers[j][i] = y(j);

				ProcessMatrix();
			}

			// Process output filters
			for (int i = 0; i < mChannels.size(); i++)
				mChannels[i]->ProcessOutput(outputBuffers[i], outputBuffers[i], audioData.lerpFactor);
		}

		////////////////////////////////////////

		template<>
		void FDN<Complex>::ProcessAudio(std::vector<Buffer<>>& outputBuffers, const AudioData& audioData)
		{
			if (!enabled.load(std::memory_order_acquire))
				return;

			PROFILE_FDN
			if (audioData.clearBuffers)
				Reset();

			// Process feedback loop
			for (int i = 0; i < outputBuffers[0].Length(); i++)
			{
				if (precedingDelayCursor >= precedingDelayBuffer.Length())
					precedingDelayCursor = 0;
				for (int j = 0; j < mChannels.size(); j++)
					y(j) = mChannels[j]->GetOutput(x(j) + precedingDelayBuffer[precedingDelayCursor], audioData.lerpFactor);

				for (int j = 0; j < outputBuffers.size(); j++)
					outputBuffers[j][i] += ravesResidues[j].GetOutput(y(j), audioData.lerpFactor);

				ProcessMatrix();
				// For the purpose of `powerNormalization`, see notes in FDN_private.h
				precedingDelayBuffer[precedingDelayCursor] = inputData(i) * powerNormalization;
				++precedingDelayCursor;
			}
		}

		////////////////////////////////////////

		template<typename T>
		void FDN<T>::ProcessSquare()
		{
#if MATRIX_LIBRARY == EIGEN_FLAG
			// TODO: Check this when using eigen.
			x = feedbackMatrix * y;
#else
			for (int j = 0; j < feedbackMatrix.Cols(); ++j)
			{
				x(j) = 0.0;
				for (int k = 0; k < feedbackMatrix.Rows(); ++k)
					x(j) += y(k) * feedbackMatrix(k, j);
			}
#endif
		}
		
		////////////////////////////////////////

		template class RandomOrthogonalFDN<Real>;
		template class RandomOrthogonalFDN<Complex>;

		template<typename T>
		Matrix<> RandomOrthogonalFDN<T>::InitMatrix(const size_t numChannels)
		{
      const int numChannelsI = ToInt(numChannels);
			Matrix<> matrix = Matrix<>::Zero(numChannelsI, numChannelsI);

#if MATRIX_LIBRARY == EIGEN_FLAG
			matrix.Col(0).RandomUniformDistribution().Normalise();
#elif MATRIX_LIBRARY == CUSTOM_FLAG
			Vec<> vector(numChannelsI);
			// [-1:1] uniform distribution
			 vector.RandomUniformDistribution();
			 vector.Normalise();
			 matrix.AddColumn(vector.Col(0), 0);
#endif

			Real tol = 0.000001;
			for (int j = 1; j < numChannelsI; ++j)
			{
				Real norm = 0;
				while (norm < tol)
				{
#if MATRIX_LIBRARY == EIGEN_FLAG
					// TODO: Check this behaves as expected.
					// vector.RandomUniformDistribution();
					matrix.Col(j).RandomUniformDistribution(); 

					Matrix<> section = matrix.leftCols(j);

					matrix.Col(j) -= section * (section.Transposed() * matrix.Col(j));
					norm = matrix.Col(j).Normal();
#elif MATRIX_LIBRARY == CUSTOM_FLAG
					vector.RandomUniformDistribution();
					Matrix<> section(numChannelsI, j);

					for (int i = 0; i < numChannelsI; ++i)
					{
						for (int k = 0; k < j; k++)
							section(i, k) = matrix(i, k);
					}

					vector -= section * (section.Transposed() * vector);
					norm = vector.Normal();
#endif
				}
#if MATRIX_LIBRARY == EIGEN_FLAG
				matrix.Col(j) /= norm;
#elif MATRIX_LIBRARY == CUSTOM_FLAG
				vector /= norm;
				matrix.AddColumn(vector.Col(0), j);
#endif
			}
			return matrix;
		}

		//////////////////// Instantiate ////////////////////

		// we don't implement/use every function, so disable the warning (we can't re-enable it since the warning is generated after the file is parsed)
		#ifdef _MSC_VER
		#pragma warning (disable : 4661)
		#endif

		template class FDNChannel<Real>;
		template class FDNChannel<Complex>;


	}
}
