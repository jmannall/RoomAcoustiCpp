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
			if (clearBuffers.load(std::memory_order_acquire))
			{
				mBuffer.Reset();
				clearBuffers.store(false, std::memory_order_release);
			}

			if (idx >= mBuffer.Length())
				idx = 0;
			T out = mAbsorptionFilter.GetOutput(mBuffer[idx], lerpFactor);
			mBuffer[idx] = input;
			++idx;
			return out;
		}

		//////////////////// FDN class ////////////////////

		template class FDN<Real>;
		template class FDN<Complex>;
		
		////////////////////////////////////////

		template<typename T>
		bool FDN<T>::IsSetMutuallyPrime(const std::vector<int>& numbers)
		{
			for (int i = 0; i < numbers.size(); ++i)
			{
				for (int j = i + 1; j < numbers.size(); ++j)
				{
					if (std::gcd(numbers[i], numbers[j]) != 1)
						return false;
				}
			}
			return true;
		}

		////////////////////////////////////////

		template<typename T>
		bool FDN<T>::IsEntryMutuallyPrime(const std::vector<int>& numbers, int idx)
		{
			for (int i = 0; i < numbers.size(); ++i)
			{
				if (i == idx)
					continue;
				if (std::gcd(numbers[i], numbers[idx]) != 1)
					return false;
			}
			return true;
		}

		////////////////////////////////////////

		template<typename T>
		void FDN<T>::MakeSetMutuallyPrime(std::vector<int>& numbers)
		{
			for (int i = 0; i < numbers.size(); ++i)
			{
				int limit = static_cast<int>(round(0.1 * numbers[i]));
				for (int adjustment = 0; adjustment <= limit; ++adjustment)
				{
					int original = numbers[i];
					bool found = false;

					for (int sign : {-1, 1})
					{
						numbers[i] = original + sign * adjustment;
						if (IsEntryMutuallyPrime(numbers, i))
						{
							found = true;
							break;
						}
					}

					if (found)
						break;
					numbers[i] = original;
				}
			}
		}

		////////////////////////////////////////

		template<>
		FDN<Real>::FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<Config> config, const Matrix<>& matrix) : x(config->numReverbSources),
			y(config->numReverbSources), feedbackMatrix(matrix)
		{
			assert(T60 > 0);
			
			std::vector<int> delayLengths = CalculateTimeDelay(dimensions, config->numReverbSources, config->fs);
			mChannels.reserve(config->numReverbSources);
			for (int i = 0; i < config->numReverbSources; i++)
				mChannels.push_back(std::make_unique<FDNChannel<Real>>(delayLengths[i], T60, config));
		}

		////////////////////////////////////////

		template<>
		FDN<Complex>::FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<Config> config, const Matrix<>& matrix) : x(config->numReverbSources),
			y(config->numReverbSources), feedbackMatrix(matrix), ravesResiduals(config->numReverbSources)
		{
			assert(T60 > 0);

			std::vector<int> delayLengths = CalculateTimeDelay(dimensions, config->numReverbSources, config->fs);
			mChannels.reserve(config->numReverbSources);
			for (int i = 0; i < config->numReverbSources; i++)
				mChannels.push_back(std::make_unique<FDNChannel<Complex>>(delayLengths[i], T60, config));
			SetTimeDelay(10.0 / SPEED_OF_SOUND, config->fs); // Time delay for 10m path propagation
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
		std::vector<int> FDN<T>::CalculateTimeDelay(const Vec<>& dimensions, const int numReverbSources, const int fs)
		{
			assert(dimensions.Rows() >  0);

			Vec t = Vec(numReverbSources);
			std::vector<int> delays = std::vector<int>(numReverbSources);
			if (dimensions.Rows() > 0)
			{
				Real idx = static_cast<Real>(numReverbSources) / static_cast<Real>(dimensions.Rows());

				assert(dimensions.Rows() <= numReverbSources);
				assert(idx == floor(idx)); // length of dimensions must be a multiple of mNumChannels

				t.RandomUniformDistribution(-0.1, 0.1f);
				t *= dimensions.Mean();

				int k = 0;
				for (int j = 0; j < numReverbSources / idx; ++j)
				{
					assert(dimensions[j] > 0.0);
					for (int i = 0; i < idx; ++i)
					{
						t[k] += dimensions[j];
						++k;
					}
				}
				t *= INV_SPEED_OF_SOUND;
				t.Max(1.0 / static_cast<Real>(fs));

				for (int i = 0; i < numReverbSources; i++)
					delays[i] = static_cast<int>(round(t[i] * static_cast<Real>(fs)));
				if (!IsSetMutuallyPrime(delays))
					MakeSetMutuallyPrime(delays);
			}
			return delays;
		}

		////////////////////////////////////////

		template <>
		void FDN<Real>::ProcessAudio(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const Real lerpFactor)
		{
			PROFILE_FDN
			if (clearBuffers.load(std::memory_order_acquire))
			{
				x.Reset();
				y.Reset();
				clearBuffers.store(false, std::memory_order_release);
			}

			// Process feedback loop
			for (int i = 0; i < data.Cols(); i++)
			{
				for (int j = 0; j < mChannels.size(); j++)
				{
					y[j] = mChannels[j]->GetOutput(x[j] + data[j][i], lerpFactor);
					outputBuffers[j][i] = y[j];
				}
				ProcessMatrix();
			}

			// Process output filters
			for (int i = 0; i < mChannels.size(); i++)
				mChannels[i]->ProcessOutput(outputBuffers[i], outputBuffers[i], lerpFactor);
		}

		////////////////////////////////////////

		template<>
		void FDN<Complex>::ProcessAudio(std::vector<Buffer<>>& outputBuffers, const Real lerpFactor)
		{
			PROFILE_FDN
			if (clearBuffers.load(std::memory_order_acquire))
			{
				x.Reset();
				y.Reset();
				clearBuffers.store(false, std::memory_order_release);
			}

			// Process feedback loop
			for (int i = 0; i < outputBuffers[0].Length(); i++)
			{
				if (idx >= delayBuffer.Length())
					idx = 0;
				for (int j = 0; j < mChannels.size(); j++)
				{
					y[j] = mChannels[j]->GetOutput(x[j] + delayBuffer[idx], lerpFactor);
					outputBuffers[j][i] += ravesResiduals[j].GetOutput(y[j], lerpFactor);
				}
				ProcessMatrix();
				delayBuffer[idx] = inputData[i];
				++idx;
			}
		}

		////////////////////////////////////////

		template<typename T>
		void FDN<T>::ProcessSquare()
		{
			for (int j = 0; j < feedbackMatrix.Cols(); ++j)
			{
				x[j] = 0.0;
				for (int k = 0; k < feedbackMatrix.Rows(); ++k)
					x[j] += y[k] * feedbackMatrix[k][j];
			}
		}
		
		////////////////////////////////////////

		template class RandomOrthogonalFDN<Real>;
		template class RandomOrthogonalFDN<Complex>;

		template<typename T>
		Matrix<> RandomOrthogonalFDN<T>::InitMatrix(const size_t numChannels)
		{
			Matrix<> matrix = Matrix<>(numChannels, numChannels);

			Vec<> vector = Vec<>(numChannels);
			vector.RandomUniformDistribution(-1.0, 1.0);
			vector.Normalise();
			matrix.AddColumn(vector.GetColumn(0), 0);

			Real tol = 0.000001;
			for (int j = 1; j < numChannels; ++j)
			{
				Real norm = 0;
				while (norm < tol)
				{
					vector.RandomUniformDistribution(-1.0, 1.0);

					Matrix section = Matrix(numChannels, j);

					for (int i = 0; i < numChannels; ++i)
					{
						for (int k = 0; k < j; k++)
							section[i][k] = matrix[i][k];
						std::vector<Real> test = matrix[i];
						Real x = test[0];
						Real y = matrix[i][0];
					}

					vector -= section * (section.Transpose() * vector);
					norm = vector.CalculateNormal();
				}
				vector /= norm;
				matrix.AddColumn(vector.GetColumn(0), j);
			}
			return matrix;
		}
	}
}
