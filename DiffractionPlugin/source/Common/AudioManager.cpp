/*
*
*  \General audio components
*
*/

#include <iterator>
#include <cmath>
#include <cassert>

#include "Common/AudioManager.h"

namespace UIE
{
	namespace Common
	{

		//////////////////// Buffer ////////////////////

		Buffer::Buffer()
		{
			InitialiseBuffer(512);
		}

		Buffer::Buffer(int n)
		{
			InitialiseBuffer(n);
		}

		void Buffer::ResizeBuffer(size_t numSamples)
		{
			size_t capacity = mBuffer.capacity();
			if (capacity < numSamples)
			{
				mBuffer.reserve(numSamples);
				std::fill_n(std::back_inserter(mBuffer), numSamples - capacity, 0.0);
			}
			else
			{
				mBuffer.resize(numSamples);
			}
		}

		void Buffer::ResetBuffer()
		{
			std::fill(mBuffer.begin(), mBuffer.end(), 0.0);
		};

		void Buffer::InitialiseBuffer(int n)
		{
			mBuffer.reserve(n);
			std::fill_n(std::back_inserter(mBuffer), n, 0.0);
		}

		bool Buffer::Valid()
		{
			bool ret = true;
			int i = 0;
			while (ret && i < mBuffer.size())
			{
				ret = !std::isnan(mBuffer[i]);
				i++;
			}
			return ret;
		}

		bool BuffersEqual(Buffer& x, Buffer& y)
		{
			if (x.Length() != y.Length())
			{
				return false;
			}
			bool ret = true;
			int i = 0;
			while (ret && i < x.Length())
			{
				ret = x[i] == y[i];
				i++;
			}
			return ret;
		}

		void BufferF::ResizeBuffer(size_t numSamples)
		{
			size_t capacity = mBuffer.capacity();
			if (capacity < numSamples)
			{
				mBuffer.reserve(numSamples);
				std::fill_n(std::back_inserter(mBuffer), numSamples - capacity, 0.0);
			}
			else
			{
				mBuffer.resize(numSamples);
			}
		}

		//////////////////// FIR Filter ////////////////////

		Real FIRFilter::GetOutput(Real input)
		{
			x[count] = input;
			Real output = 0.0;
			size_t xLen = x.Length();
			if (irLen > xLen)
			{
				x.ResizeBuffer(irLen);
			}
			else if (irLen < xLen)
			{
				Buffer store = x;
				for (int i = 0; i < xLen; i++)
				{
					x[i] = store[(count + i) % xLen];
				}
				x.ResizeBuffer(irLen);
				count = 0;
			}
			for (int i = 0; i <= count; i++)
			{
				output += x[count - i] * mIr[i];
			}
			int n = irLen + count;
			for (int i = count + 1; i < irLen; i++)
			{
				output += x[n - i] * mIr[i];
			}

			count = (count + 1) % (int)irLen;
			return output;
		}

		//////////////////// IIR Filter ////////////////////

		Real IIRFilter::GetOutput(Real input)
		{
			x[0] = input;
			y[0] = 0.0;
			for (int i = 0; i < order; i++)
			{
				y[0] += b[i] * x[i] - a[i + 1] * y[i + 1];
			}
			y[0] += b[order] * x[order];

			for (int i = order; i > 0; i--)
			{
				x[i] = x[i - 1];
				y[i] = y[i - 1];
			}
			return y[0];
		}

		void IIRFilter::SetT(int fs)
		{
			T = 1.0 / (Real)fs;
		}

		void HighShelf::UpdateParameters(Real fc, Real g)
		{
			Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			Real sqrtG = sqrt(g);

			Real store = omega / sqrtG;
			a[0] = 1 + store; // a[0] isn't used in ProcessOutput
			a[1] = (1 - store) / a[0];

			store = omega * sqrtG;
			b[0] = (1 + store) / a[0];
			b[1] = (1 - store) / a[0];
		}

		void LowPass::UpdateParameters(Real fc)
		{
			Real K = PI_2 * fc * T;

			a[0] = K + 2; // a[0] isn't used in ProcessOutput
			a[1] = (K - 2) / a[0];

			b[0] = K / a[0];
			b[1] = K / a[0];
		}

		void TransDF2::UpdateParameters(TransDF2Parameters zpk)
		{
			b[0] = zpk.k;
			b[1] = -zpk.k * (zpk.z[0] + zpk.z[1]);
			b[2] = zpk.k * zpk.z[0] * zpk.z[1];

			a[1] = -(zpk.p[0] + zpk.p[1]);
			a[2] = zpk.p[0] * zpk.p[1];
		}

		void TransDF2::UpdateParameters(Real fc, FilterShape shape)
		{
			switch (shape)
			{
			case FilterShape::lpf:
				UpdateLPF(fc);
				break;
			case FilterShape::hpf:
				UpdateHPF(fc);
				break;
			}
		}

		void TransDF2::UpdateParameters(Real fb, Real g, int m, int M, FilterShape shape)
		{
			switch (shape)
			{
			case FilterShape::lbf:
				UpdateLBF(fb, g, m, M);
				break;
			case FilterShape::hbf:
				UpdateHBF(fb, g, m, M);
				break;
			}
		}

		void TransDF2::UpdateLPF(Real fc)
		{
			Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			Real omega_sq = pow(omega, 2.0);

			Real a0 = 1.0 / (1.0 + SQRT_2 * omega + omega_sq);
			b[0] = a0;
			b[1] = 2.0 * a0;
			b[2] = a0;

			a[1] = (2.0 - 2.0 * omega_sq) * a0;
			a[2] = (1.0 - SQRT_2 * omega + omega_sq) * a0;
		}

		void TransDF2::UpdateHPF(Real fc)
		{
			Real omega = cot(PI_1 * fc * T); // 2 * PI * fc * T / 2
			Real omega_sq = pow(omega, 2.0);

			Real a0 = 1.0 / (1.0 + SQRT_2 * omega + omega_sq);
			b[0] = omega_sq * a0;
			b[1] = -2.0 * omega_sq * a0;
			b[2] = omega_sq * a0;

			a[1] = (2.0 - 2.0 * omega_sq) * a0;
			a[2] = (1.0 - SQRT_2 * omega + omega_sq) * a0;
		}

		void TransDF2::UpdateLBF(Real fb, Real g, int m, int M)
		{
			Real K = tan(PI_1 * fb * T);
			Real K_2 = 2.0 * K;
			Real K_sq = pow(K, 2.0);
			Real K_sq_2 = 2.0 * K_sq;
			Real M_2 = 2.0 * M;
			Real V = pow(g, 1.0 / M) - 1.0;
			Real VK = V * K;
			Real VK_2 = 2.0 * VK;
			Real VK_sq = pow(VK, 2.0);

			Real alpha = (0.5 - (2.0 * m - 1.0) / (M_2)) * PI_1;
			Real cm = cos(alpha);
			Real K2cm = K_2 * cm;
			Real a0 = 1.0 / (1.0 + K2cm + K_sq);
			a[1] = (K_sq_2 - 2.0) * a0;
			a[2] = (1.0 - K2cm + K_sq) * a0;

			b[0] = a[0] + (VK_2 * (K + cm) + VK_sq) * a0;
			b[1] = a[1] + (VK_2 * K_2 + VK_sq * 2.0) * a0;
			b[2] = a[2] + (VK_2 * (K - cm) + VK_sq) * a0;
		}

		void TransDF2::UpdateHBF(Real fb, Real g, int m, int M)
		{
			Real K = tan(PI_1 * fb * T);
			Real K_2 = 2.0 * K;
			Real K_sq = pow(K, 2.0);
			Real K_sq_2 = 2.0 * K_sq;
			Real M_2 = 2.0 * M;
			Real V = pow(g, 1.0 / static_cast<Real>(M)) - 1.0;
			Real VK = V * K;
			Real VK_2 = 2.0 * VK;
			Real VK_sq = pow(VK, 2.0);

			Real alpha = (0.5 - (2.0 * static_cast<Real>(m) - 1.0) / M_2) * PI_1;
			Real cm = cos(alpha);
			Real K2cm = K_2 * cm;
			Real a0 = 1.0 / (1.0 + K2cm + K_sq);
			a[1] = (2.0 - K_sq_2) * a0;
			a[2] = (1.0 - K2cm + K_sq) * a0;

			b[0] = a[0] + (VK_2 * (K + cm) + VK_sq) * a0;
			b[1] = a[1] - (VK_2 * K_2 + VK_sq * 2.0) * a0;
			b[2] = a[2] + (VK_2 * (K - cm) + VK_sq) * a0;
		}

		//////////////////// Filterbanks ////////////////////

		LinkwitzRiley::LinkwitzRiley(int fs) : fc{ 176.0, 775.0, 3408.0 }, g{ 0.0, 0.0, 0.0, 0.0 }, filters()
		{
			InitFilters(fs);
			CalcFM();
		};

		LinkwitzRiley::LinkwitzRiley(Real fc0, Real fc1, Real fc2, int fs) : fc{ fc0, fc1, fc2 }, g{ 1.0, 1.0, 1.0, 1.0 }, filters()
		{
			InitFilters(fs);
			CalcFM();
		};

		void LinkwitzRiley::InitFilters(int fs)
		{
			TransDF2 lpFilter[3] = { TransDF2(fc[0], FilterShape(FilterShape::lpf), fs), TransDF2(fc[1], FilterShape(FilterShape::lpf), fs), TransDF2(fc[2], FilterShape(FilterShape::lpf), fs) };
			TransDF2 hpFilter[3] = { TransDF2(fc[0], FilterShape(FilterShape::hpf), fs), TransDF2(fc[1], FilterShape(FilterShape::hpf), fs), TransDF2(fc[2], FilterShape(FilterShape::hpf), fs) };

			filters[0] = lpFilter[1];
			filters[1] = lpFilter[1];
			filters[2] = lpFilter[2];
			filters[3] = lpFilter[2];
			filters[4] = hpFilter[2];
			filters[5] = hpFilter[2];
			filters[6] = lpFilter[0];
			filters[7] = lpFilter[0];
			filters[8] = hpFilter[0];
			filters[9] = hpFilter[0];
			filters[10] = hpFilter[1];
			filters[11] = hpFilter[1];
			filters[12] = lpFilter[0];
			filters[13] = lpFilter[0];
			filters[14] = hpFilter[0];
			filters[15] = hpFilter[0];
			filters[16] = lpFilter[2];
			filters[17] = lpFilter[2];
			filters[18] = hpFilter[2];
			filters[19] = hpFilter[2];
		}

		void LinkwitzRiley::CalcFM()
		{
			Real fMin;
			Real fMax;
			for (int i = 0; i < 4; i++)
			{
				if (i == 0)
					fMin = 20.0;
				else
					fMin = fc[i - 1];
				if (i == 3)
					fMax = 20000.0;
				else
					fMax = fc[i];

				fm[i] = sqrt(fMin * fMax);
			}
		}

		void LinkwitzRiley::UpdateParameters(Real gain[])
		{
			for (int i = 0; i < 4; i++)
				g[i] = *gain++;
		}

		Real LinkwitzRiley::GetOutput(const Real input)
		{
			Real mid[2];
			mid[0] = filters[1].GetOutput(filters[0].GetOutput(input));
			mid[1] = filters[11].GetOutput(filters[10].GetOutput(input));

			mid[0] = filters[3].GetOutput(filters[2].GetOutput(mid[0])) + filters[5].GetOutput(filters[4].GetOutput(mid[0]));
			mid[1] = filters[13].GetOutput(filters[12].GetOutput(mid[1])) + filters[15].GetOutput(filters[14].GetOutput(mid[1]));

			Real out[4];
			out[0] = g[0] * filters[7].GetOutput(filters[6].GetOutput(mid[0]));
			out[1] = g[1] * filters[9].GetOutput(filters[8].GetOutput(mid[0]));
			out[2] = g[2] * filters[17].GetOutput(filters[16].GetOutput(mid[1]));
			out[3] = g[3] * filters[19].GetOutput(filters[18].GetOutput(mid[1]));

			return out[0] + out[1] + out[2] + out[3];
		}

		BandPass::BandPass() : numFilters(0), M(0) {};

		BandPass::BandPass(size_t order)
		{
			InitFilters(order, 48000);
		};

		BandPass::BandPass(size_t order, int fs)
		{
			InitFilters(order, fs);
		};

		BandPass::BandPass(size_t order, FilterShape shape, Real fb, Real g, int fs)
		{
			InitFilters(order, fs);
			UpdateParameters(fb, g, shape);
		};

		void BandPass::InitFilters(int order, int fs)
		{
			numFilters = order / 2;
			assert(numFilters == order * 2); // order must be even
			M = order;
			filters.reserve(numFilters);
			std::fill_n(std::back_inserter(filters), numFilters, TransDF2(fs));
		}

		void BandPass::UpdateParameters(Real fb, Real g, FilterShape shape)
		{
			for (int i = 0; i < numFilters; i++)
			{
				filters[i].UpdateParameters(fb, g, i + 1, M, shape);
			}
		}

		Real BandPass::GetOutput(const Real input)
		{
			Real out = input;
			for (int i = 0; i < numFilters; i++)
			{
				out = filters[i].GetOutput(out);
			}
			return out;
		}

		ParametricEQ::ParametricEQ(size_t order) : mOrder(order), numFilters(4), mGain(0.0)
		{
			InitBands(48000);
		}

		ParametricEQ::ParametricEQ(size_t order, int fs) : mOrder(order), numFilters(4), mGain(0.0)
		{
			InitBands(fs);
		}

		ParametricEQ::ParametricEQ(size_t order, Real fc[], Real gain[], int fs) : mOrder(order), numFilters(4)
		{
			InitBands(fs);
			UpdateParameters(fc, gain);
		}

		void ParametricEQ::UpdateParameters(const Real fc[], Real gain[])
		{
			mGain = gain[numFilters];
			for (int i = 0; i < numFilters + 1; i++)
			{
				gain[i] = std::max(EPS, gain[i]); // Prevent division by zero
			}
			for (int i = 0; i < numFilters; i++)
			{
				fb[i] = fc[i] * sqrtf(fc[i + 1] / fc[i]);
				g[i] = gain[i] / gain[i + 1];
				bands[i].UpdateParameters(fb[i], g[i], FilterShape(FilterShape::lbf));
			}
		}

		Real ParametricEQ::GetOutput(const Real input)
		{
			Real out = input;
			for (int i = 0; i < numFilters; i++)
			{
				out = bands[i].GetOutput(out);
			}
			return mGain * out;
		}

		void ParametricEQ::InitBands(int fs)
		{
			for (int i = 0; i < numFilters; i++)
			{
				bands[i].InitFilters(mOrder, fs);
			}
		}
	}
}