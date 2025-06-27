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

// Spatialiser headers
#include "Spatialiser/FDN.h"
#include "Spatialiser/Globals.h"

// Unity headers
#include "Unity/UnityInterface.h"
#include "Unity/Profiler.h"
#include "Unity/Debug.h"

namespace RAC
{
	using namespace Common;
	using namespace Unity;
	namespace Spatialiser
	{

		//////////////////// FDN Channel class ////////////////////

		////////////////////////////////////////

		Real FDNChannel::GetOutput(const Real input, const Real lerpFactor)
		{
			if (clearBuffers.load())
			{
				mBuffer.Reset();
				clearBuffers.store(false);
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

		FDN::FDN(const Coefficients<>& T60, const Vec& dimensions, const std::shared_ptr<Config> config, const Matrix& matrix) : x(config->numLateReverbChannels),
			y(config->numLateReverbChannels), feedbackMatrix(matrix)
		{
			assert(T60 > 0);
			
			std::vector<int> delayLengths = CalculateTimeDelay(dimensions, config->numLateReverbChannels, config->fs);
			mChannels.reserve(config->numLateReverbChannels);
			for (int i = 0; i < config->numLateReverbChannels; i++)
				mChannels.push_back(std::make_unique<FDNChannel>(delayLengths[i], T60, config));
		}

		////////////////////////////////////////

		void FDN::SetTargetT60(const Coefficients<>& T60)
		{
			for (int i = 0; i < mChannels.size(); i++)
				mChannels[i]->SetTargetT60(T60);
		}

		////////////////////////////////////////

		std::vector<int> FDN::CalculateTimeDelay(const Vec& dimensions, const int numLateReverbChannels, const int fs)
		{
			assert(dimensions.Rows() >  0);

			Vec t = Vec(numLateReverbChannels);
			std::vector<int> delays = std::vector<int>(numLateReverbChannels);
			if (dimensions.Rows() > 0)
			{
				Real idx = static_cast<Real>(numLateReverbChannels) / static_cast<Real>(dimensions.Rows());

				assert(dimensions.Rows() <= numLateReverbChannels);
				assert(idx == floor(idx)); // length of dimensions must be a multiple of mNumChannels

				t.RandomUniformDistribution(-0.1, 0.1f);
				t *= dimensions.Mean();

				int k = 0;
				for (int j = 0; j < numLateReverbChannels / idx; ++j)
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

				for (int i = 0; i < numLateReverbChannels; i++)
					delays[i] = static_cast<int>(round(t[i] * static_cast<Real>(fs)));
				if (!IsSetMutuallyPrime(delays))
					MakeSetMutuallyPrime(delays);
			}
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

		void FDN::ProcessAudio(const Matrix& data, std::vector<Buffer>& outputBuffers, const Real lerpFactor)
		{
#ifdef PROFILE_AUDIO_THREAD
			BeginFDN();
#endif					
			if (clearBuffers.load())
			{
				x.Reset();
				y.Reset();
				clearBuffers.store(false);
			}

			FlushDenormals();

			// Process feedback loop
			for (int i = 0; i < data.Cols(); i++)
			{
				for (int j = 0; j < mChannels.size(); j++)
				{
					if (isnan(x[j]))
						Debug::Log("X was nan", Colour::Red);
					y[j] = mChannels[j]->GetOutput(x[j] + data[j][i], lerpFactor);
					outputBuffers[j][i] = y[j];
					if (isnan(y[j]))
						Debug::Log("Y was nan", Colour::Red);
				}
				ProcessMatrix();
			}

			// Process output filters
			for (int i = 0; i < mChannels.size(); i++)
				mChannels[i]->ProcessOutput(outputBuffers[i], outputBuffers[i], lerpFactor);
			
			NoFlushDenormals();
#ifdef PROFILE_AUDIO_THREAD
			EndFDN();
#endif
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
