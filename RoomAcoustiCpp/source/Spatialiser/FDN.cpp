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

		template class FDNChannel<Real>;
		template class FDNChannel<Complex>;

		////////////////////////////////////////

		template<typename T>
		T FDNChannel<T>::GetOutput(const T input, const Real lerpFactor)
		{
			if (idx >= mBuffer.Length())
				idx = 0;
			// T out = mAbsorptionFilter.GetOutput(mBuffer[idx], lerpFactor);
			// TODO: Interpolate absorption if needed? - is this only changed on a full reset?
			T out = absorption.load(std::memory_order_acquire) * mBuffer[idx];
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
			for (int i = 0; i < numbers.Rows(); ++i)
			{
				for (int j = i + 1; j < numbers.Rows(); ++j)
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
			for (int i = 0; i < numbers.Rows(); ++i)
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
			for (int i = 0; i < numbers.Rows(); ++i)
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

		template<>
		FDN<Real>::FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix) : x(dspConfig->GetData().fdnSize),
			y(dspConfig->GetData().fdnSize), feedbackMatrix(matrix), mT60(nullptr), inputData(nullptr)
		{
			assert(T60 > 0);

#if MATRIX_LIBRARY == EIGEN_FLAG
			x.SetConstant(0.0);
			y.SetConstant(0.0);
#endif

			// For the purpose of `powerNormalization`, see notes in FDN_private.h
			powerNormalization = 0.0;

			int fdnSize = dspConfig->GetData().fdnSize;
			Vec<int> delayLengths = CalculateTimeDelay(dimensions, fdnSize, dspConfig->GetData().fs);
			mChannels.reserve(fdnSize);
			for (int i = 0; i < fdnSize; i++)
			{
				mChannels.push_back(std::make_unique<FDNChannel<Real>>(delayLengths(i), T60, dspConfig));
				powerNormalization += static_cast<Real>(delayLengths(i));
			}

			powerNormalization = std::sqrt(powerNormalization / static_cast<Real>(fdnSize));
		}

		////////////////////////////////////////

		// TODO: Work out how to remove obsolete constructors for each type
		template<>
		FDN<Real>::FDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix) : x(dspConfig->GetData().fdnSize),
			y(dspConfig->GetData().fdnSize), feedbackMatrix(matrix), mT60(nullptr), inputData(nullptr)
		{
			assert(T60 > 0);

#if MATRIX_LIBRARY == EIGEN_FLAG
			x.SetConstant(0.0);
			y.SetConstant(0.0);
#endif

			// For the purpose of `powerNormalization`, see notes in FDN_private.h
			powerNormalization = 0.0;

			int fdnSize = dspConfig->GetData().fdnSize;
			mChannels.reserve(fdnSize);
			for (int i = 0; i < fdnSize; i++)
			{
				mChannels.push_back(std::make_unique<FDNChannel<Real>>(delayLengths(i), T60, dspConfig));
				powerNormalization += static_cast<Real>(delayLengths(i));
			}

			powerNormalization = std::sqrt(powerNormalization / static_cast<Real>(fdnSize));
		}

		////////////////////////////////////////

		template<>
		FDN<Complex>::FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix) : x(dspConfig->GetData().fdnSize),
			y(dspConfig->GetData().fdnSize), feedbackMatrix(matrix), ravesResidues(dspConfig->GetData().numReverbSources), mT60(0.0), inputData(dspConfig->GetData().numFrames), enabled(false)
		{
			assert(T60 > 0);

#if MATRIX_LIBRARY == EIGEN_FLAG
			x.SetConstant(0.0);
			y.SetConstant(0.0);
#endif

			// For the purpose of `powerNormalization`, see notes in FDN_private.h
			powerNormalization = 0.0;

			int fdnSize = dspConfig->GetData().fdnSize;
			Vec<int> delayLengths = CalculateTimeDelay(dimensions, fdnSize, dspConfig->GetData().fs);
			mChannels.reserve(fdnSize);
			for (int i = 0; i < fdnSize; i++)
			{
				mChannels.push_back(std::make_unique<FDNChannel<Complex>>(delayLengths(i), T60, dspConfig));
				powerNormalization += static_cast<Real>(delayLengths(i));
			}

			powerNormalization = std::sqrt(powerNormalization / static_cast<Real>(fdnSize));
		}

		////////////////////////////////////////

		// TODO: Check power normalization with different FDN size and numReverbSources
		template<>
		FDN<Complex>::FDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix) : x(dspConfig->GetData().fdnSize),
			y(dspConfig->GetData().fdnSize), feedbackMatrix(matrix), ravesResidues(dspConfig->GetData().numReverbSources), mT60(T60), inputData(dspConfig->GetData().numFrames), enabled(false)
		{
			assert(T60 > 0);

#if MATRIX_LIBRARY == EIGEN_FLAG
			x.SetConstant(0.0);
			y.SetConstant(0.0);
#endif

			// For the purpose of `powerNormalization`, see notes in FDN_private.h
			powerNormalization = 0.0;

			int fdnSize = dspConfig->GetData().fdnSize;
			mChannels.reserve(fdnSize);
			for (int i = 0; i < fdnSize; i++)
			{
				mChannels.push_back(std::make_unique<FDNChannel<Complex>>(delayLengths(i), T60, dspConfig));
				powerNormalization += static_cast<Real>(delayLengths(i));
			}

			powerNormalization = std::sqrt(powerNormalization / static_cast<Real>(fdnSize));
		}

		////////////////////////////////////////

		template<typename T>
		void FDN<T>::SetTargetT60(const Coefficients<>& T60)
		{
			for (int i = 0; i < mChannels.size(); i++)
				mChannels[i]->SetTargetT60(T60);
		}

		////////////////////////////////////////

		template<typename T>
		Vec<int> FDN<T>::CalculateTimeDelay(const Vec<>& dimensions, const int fdnSize, const int fs)
		{
			assert(dimensions.Rows() >  0);

			Vec<> t(fdnSize);
			Vec<int> delays = Vec<int>::Constant(fdnSize, 1);
			if (dimensions.Rows() > 0)
			{

				assert(dimensions.Rows() <= fdnSize);

				// gives [-1:1], divide by 10 to get [-0.1:0.1]
				t.RandomUniformDistribution();
				t *= dimensions.Mean() / (Real)10.0;

				int k = 0;
				while (k < fdnSize)
				{
					for (int i = 0; i < dimensions.Rows(); ++i)
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
			// TODO: Check this when using eigen.
			for (int j = 0; j < feedbackMatrix.Cols(); ++j)
			{
				x(j) = 0.0;
				for (int k = 0; k < feedbackMatrix.Rows(); ++k)
					x(j) += y(k) * feedbackMatrix(k, j);
			}
		}
		
		////////////////////////////////////////

		template class RandomOrthogonalFDN<Real>;
		template class RandomOrthogonalFDN<Complex>;

		template<typename T>
		Matrix<> RandomOrthogonalFDN<T>::InitMatrix(const size_t numChannels)
		{
			Matrix<> matrix = Matrix<>::Zero(numChannels, numChannels);

#if MATRIX_LIBRARY == EIGEN_FLAG
			matrix.Col(0).RandomUniformDistribution().Normalise();
#elif MATRIX_LIBRARY == CUSTOM_FLAG
			Vec<> vector(numChannels);
			// [-1:1] uniform distribution
			 vector.RandomUniformDistribution();
			 vector.Normalise();
			 matrix.AddColumn(vector.Col(0), 0);
#endif

			Real tol = 0.000001;
			for (int j = 1; j < numChannels; ++j)
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
					Matrix<> section(numChannels, j);

					for (int i = 0; i < numChannels; ++i)
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
	}
}
