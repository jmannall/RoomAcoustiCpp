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

		Channel::Channel(const int delayLength, const Coefficients& T60, const Config& config) : mConfig(config), mAbsorptionFilter(config.frequencyBands, config.Q, config.fs), idx(0)
		{
			InitDelay(delayLength);
			InitAbsorption(T60);
		}

		////////////////////////////////////////

		Real Channel::GetOutput(const Real input)
		{
			if (idx >= mBuffer.Length())
				idx = 0;
			Real out = mAbsorptionFilter.GetOutput(mBuffer[idx]);
			mAbsorptionFilter.UpdateParameters(mConfig.lerpFactor);
			mBuffer[idx] = input;
			++idx;
			return out;
		}

		//////////////////// FDN utility functions ////////////////////

		////////////////////////////////////////

		bool IsSetMutuallyPrime(const std::vector<int>& numbers)
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

		bool IsEntryMutuallyPrime(const std::vector<int>& numbers, int idx) {
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

		void MakeSetMutuallyPrime(std::vector<int>& numbers) {
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

		//////////////////// FDN class ////////////////////

		////////////////////////////////////////

		FDN::FDN(const Coefficients& T60, const Vec& dimensions, const Config& config) : mConfig(config), x(config.numFDNChannels), y(config.numFDNChannels), feedbackMatrix(config.numFDNChannels, config.numFDNChannels)
		{
			std::vector<int> delayLengths = CalculateTimeDelay(dimensions);
			mChannels.reserve(mConfig.numFDNChannels);
			for (int i = 0; i < mConfig.numFDNChannels; i++)
				mChannels.push_back(Channel(delayLengths[i], T60, mConfig));
			InitFDNMatrix(FDNMatrix::householder);
		}

		////////////////////////////////////////

		void FDN::UpdateT60(const Coefficients& T60)
		{
			for (int i = 0; i < mConfig.numFDNChannels; i++)
				mChannels[i].UpdateAbsorption(T60);
		}

		////////////////////////////////////////

		void FDN::UpdateDelayLines(const Vec& dimensions)
		{
			std::vector<int> delayLengths = CalculateTimeDelay(dimensions);
			for (int i = 0; i < mConfig.numFDNChannels; i++)
				mChannels[i].UpdateDelayLine(delayLengths[i]);
		}

		////////////////////////////////////////

		std::vector<int> FDN::CalculateTimeDelay(const Vec& dimensions)
		{
			assert(dimensions.Rows() >  0);

			Vec t = Vec(mConfig.numFDNChannels);
			std::vector<int> delays = std::vector<int>(mConfig.numFDNChannels);
			if (dimensions.Rows() > 0)
			{
				Real idx = static_cast<Real>(mConfig.numFDNChannels) / static_cast<Real>(dimensions.Rows());

				assert(dimensions.Rows() <= mConfig.numFDNChannels);
				assert(idx == floor(idx)); // length of dimensions must be a multiple of mNumChannels

				t.RandomUniformDistribution(-0.1, 0.1f);
				t *= dimensions.Mean();

				int k = 0;
				for (int j = 0; j < mConfig.numFDNChannels / idx; ++j)
				{
					for (int i = 0; i < idx; ++i)
					{
						t[k] += dimensions[j];
						++k;
					}
				}
				t *= INV_SPEED_OF_SOUND;
				t.Max(1.0 / mConfig.fs);

				for (int i = 0; i < mConfig.numFDNChannels; i++)
					delays[i] = static_cast<int>(round(t[i] * mConfig.fs));
				if (!IsSetMutuallyPrime(delays))
					MakeSetMutuallyPrime(delays);
			}
			return delays;
		}

		////////////////////////////////////////

		void FDN::InitRandomOrthogonal()
		{
			feedbackMatrix = Matrix(mConfig.numFDNChannels, mConfig.numFDNChannels);

			Vec vector = Vec(mConfig.numFDNChannels);
			vector.RandomUniformDistribution(-1.0, 1.0);
			vector.Normalise();
			feedbackMatrix.AddColumn(vector.GetColumn(0), 0);

			Real tol = 0.000001;
			for (int j = 1; j < mConfig.numFDNChannels; ++j)
			{
				Real norm = 0;
				while (norm < tol)
				{
					vector.RandomUniformDistribution(-1.0, 1.0);

					Matrix section = Matrix(mConfig.numFDNChannels, j);

					for (int i = 0; i < mConfig.numFDNChannels; ++i)
					{
						for (int k = 0; k < j; k++)
							section[i][k] = feedbackMatrix[i][k];
						std::vector<Real> test = feedbackMatrix[i];
						Real x = test[0];
						Real y = feedbackMatrix[i][0];
					}

					vector -= section * (section.Transpose() * vector);
					norm = vector.CalculateNormal();
				}
				vector /= norm;
				feedbackMatrix.AddColumn(vector.GetColumn(0), j);
			}
		}

		////////////////////////////////////////

		void FDN::ProcessOutput(const std::vector<Real>& data, const Real gain)
		{
#ifdef PROFILE_DETAILED
			BeginFDNChannel();
#endif

			int i = 0;
			for (Channel& channel : mChannels)
			{
				if (isnan(x[i]))
					Debug::Log("X was nan", Colour::Red);
				y[i] = channel.GetOutput(x[i] + data[i]);
				if (isnan(y[i]))
					Debug::Log("Y was nan", Colour::Red);
				++i;
			}
			// y *= gain;
#ifdef PROFILE_DETAILED
			EndFDNChannel();
			BeginFDNMatrix();
#endif
			ProcessMatrix();
#ifdef PROFILE_DETAILED
			EndFDNMatrix();
#endif
		}

		////////////////////////////////////////

		void FDN::ProcessSquare()
		{
			Real sum = 0.0;
			for (int j = 0; j < feedbackMatrix.Cols(); ++j)
			{
				sum = 0.0;
				for (int k = 0; k < feedbackMatrix.Rows(); ++k)
					sum += y[k] * feedbackMatrix[k][j];
				x[j] = sum;
			}
		}
	}
}
