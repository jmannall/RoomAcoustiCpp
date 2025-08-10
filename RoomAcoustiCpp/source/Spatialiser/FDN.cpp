/*
* @class FDN, Channel
*
* @brief Declaration of FDN and Channel classes
*
*/

// C++ headers
#include <mutex>
#include <cmath>
#include <numeric>  // For std::gcd

#include "Common/RACProfiler.h"

// Spatialiser headers
#include "Spatialiser/FDN.h"
#include "Spatialiser/Globals.h"

// Unity headers
#include "Unity/Debug.h"

namespace RAC
{
	using namespace Common;
#ifdef USE_UNITY_DEBUG
	using namespace Unity;
#endif
	namespace Spatialiser
	{

		//////////////////// FDN Channel class ////////////////////

		////////////////////////////////////////

		Real FDNChannel::GetOutput(const Real input, const Real lerpFactor)
		{
			if (clearBuffers.load(std::memory_order_acquire))
			{
				mBuffer.Reset();
				clearBuffers.store(false, std::memory_order_release);
			}

			if (idx >= mBuffer.Length())
				idx = 0;
			Real out = mAbsorptionFilter.GetOutput(mBuffer[idx], lerpFactor);
			mBuffer[idx] = input;
			++idx;
			return out;
		}

		//////////////////// FDN class ////////////////////

		////////////////////////////////////////

		bool FDN::IsSetMutuallyPrime(const std::vector<int>& numbers)
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

		bool FDN::IsEntryMutuallyPrime(const std::vector<int>& numbers, int idx)
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

		void FDN::MakeSetMutuallyPrime(std::vector<int>& numbers)
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

		FDN::FDN(const Coefficients<>& T60, const Vec& dimensions, const std::shared_ptr<Config> config, const Matrix& matrix) : x(config->numReverbSources),
			y(config->numReverbSources), feedbackMatrix(matrix)
		{
			assert(T60 > 0);
			
			std::vector<int> delayLengths = CalculateTimeDelay(dimensions, config->numReverbSources, config->fs);
			mChannels.reserve(config->numReverbSources);
			for (int i = 0; i < config->numReverbSources; i++)
				mChannels.push_back(std::make_unique<FDNChannel>(static_cast<size_t>(delayLengths[i]), T60, config));
		}

		////////////////////////////////////////

		void FDN::SetTargetT60(const Coefficients<>& T60)
		{
			for (int i = 0; i < mChannels.size(); i++)
				mChannels[i]->SetTargetT60(T60);
		}

		////////////////////////////////////////

		std::vector<int> FDN::CalculateTimeDelay(const Vec& dimensions, const int numChannels, const int fs)
		{
			assert(dimensions.Rows() >  0);

			Vec t = Vec(numChannels);
			std::vector<int> delays = std::vector<int>(numChannels);

			t.RandomUniformDistribution(-0.1, 0.1);
			t *= dimensions.Mean();

			int k = 0;
			while (k < numChannels)
			{
				for (int i = 0; i < dimensions.Rows(); ++i)
				{
					if (k >= numChannels)
						break;
					assert(dimensions[i] > 0.0);
					t[k] += dimensions[i];
					++k;
				}
			}
			t *= INV_SPEED_OF_SOUND;

			for (int i = 0; i < numChannels; i++)
				delays[i] = static_cast<int>(round(t[i] * static_cast<Real>(fs)));
			if (!IsSetMutuallyPrime(delays))
				MakeSetMutuallyPrime(delays);
			for (int& delay : delays)
				delay = std::max(delay, 1);  // Ensure no zero or less delays
			return delays;
		}

		////////////////////////////////////////

		Matrix RandomOrthogonalFDN::InitMatrix(const size_t numChannels)
		{
			Matrix matrix = Matrix(numChannels, numChannels);

			Vec vector = Vec(numChannels);
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

		////////////////////////////////////////

		void FDN::ProcessAudio(const Matrix& data, std::vector<Buffer<>>& outputBuffers, const Real lerpFactor)
		{
			PROFILE_FDN		
			if (clearBuffers.load(std::memory_order_acquire))
			{
				x.Reset();
				y.Reset();
				clearBuffers.store(false, std::memory_order_release);
			}

			FlushDenormals();

			// Process feedback loop
			for (int i = 0; i < data.Cols(); i++)
			{
				for (int j = 0; j < mChannels.size(); j++)
				{
#ifdef USE_UNITY_DEBUG
					if (isnan(x[j]))
						Debug::Log("X was nan", Colour::Red);
#endif
					y[j] = mChannels[j]->GetOutput(x[j] + data[j][i], lerpFactor);
					outputBuffers[j][i] = y[j];
#ifdef USE_UNITY_DEBUG
					if (isnan(y[j]))
						Debug::Log("Y was nan", Colour::Red);
#endif
				}
				ProcessMatrix();
			}

			// Process output filters
			for (int i = 0; i < mChannels.size(); i++)
				mChannels[i]->ProcessOutput(outputBuffers[i], outputBuffers[i], lerpFactor);
			
			NoFlushDenormals();
		}

		////////////////////////////////////////

		void FDN::ProcessSquare()
		{
			for (int j = 0; j < feedbackMatrix.Cols(); ++j)
			{
				x[j] = 0.0;
				for (int k = 0; k < feedbackMatrix.Rows(); ++k)
					x[j] += y[k] * feedbackMatrix[k][j];
			}
		}
	}
}
