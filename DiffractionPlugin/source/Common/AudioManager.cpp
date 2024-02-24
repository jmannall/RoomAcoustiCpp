/*
*
*  \General audio components
*
*/

// C++ headers
#include <iterator>
#include <cmath>
#include <cassert>

// Common headers
#include "Common/AudioManager.h"
#include "Common/Coefficients.h"

namespace UIE
{
	namespace Common
	{

		//////////////////// Buffer ////////////////////

		void Buffer::ResizeBuffer(size_t numSamples)
		{

			size_t size = mBuffer.size();
			if (size == numSamples)
				return;
			if (size < numSamples)
			{
				mBuffer.reserve(numSamples);
				mBuffer.insert(mBuffer.end(), numSamples - size, 0.0);
			}
			else
				mBuffer.resize(numSamples);
		}

		void Buffer::ResetBuffer()
		{
			std::fill(mBuffer.begin(), mBuffer.end(), 0.0);
		};

		void Buffer::InitialiseBuffer(int n)
		{
			mBuffer.reserve(n);
			mBuffer.insert(mBuffer.end(), n, 0.0);
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
			size_t size = mBuffer.size();
			if (size < numSamples)
			{
				mBuffer.reserve(numSamples);
				mBuffer.insert(mBuffer.end(), numSamples - size, 0.0);
			}
			else
			{
				mBuffer.resize(numSamples);
			}
		}

		//////////////////// FIR Filter ////////////////////

		Real FIRFilter::GetOutput(Real input)
		{
			inputLine[count] = input;
			Real output = 0.0;
			int index = count;

			if (irLen % 8 != 0)
			{
				for (int i = 0; i < irLen; i++)
				{
					output += mIr[i] * inputLine[index++];
					if (index >= irLen) { index = 0; }
				}
			}
			else
			{
				// This is easier for the compiler to vectorise
				Real result_a = 0.0;
				Real result_b = 0.0;
				Real result_c = 0.0;
				Real result_d = 0.0;
				Real result_e = 0.0;
				Real result_f = 0.0;
				Real result_g = 0.0;
				Real result_h = 0.0;
				int i = 0;
				while (i < irLen)
				{
					if (index < (irLen - 8))
					{
						result_a += mIr[i++] * inputLine[index++];
						result_b += mIr[i++] * inputLine[index++];
						result_c += mIr[i++] * inputLine[index++];
						result_d += mIr[i++] * inputLine[index++];
						result_e += mIr[i++] * inputLine[index++];
						result_f += mIr[i++] * inputLine[index++];
						result_g += mIr[i++] * inputLine[index++];
						result_h += mIr[i++] * inputLine[index++];
					}
					else
					{
						for (int k = 0; k < 8; k++)
						{
							output += mIr[i++] * inputLine[index++];
							if (index >= irLen) { index = 0; }
						}
					}
				}
				output += result_a + result_b + result_c + result_d + result_e + result_f + result_g + result_h;
			}
			if (--count < 0) { count = irLen - 1; }
			return output;
		}

		//////////////////// IIR Filter ////////////////////

		Real IIRFilter::GetOutput(Real input)
		{
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#elif(_ANDROID)

			unsigned m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (1 << 24));
#endif
			Real v = input;
			Real output = 0.0;
			for (int i = 1; i <= order; i++)
			{
				v += y[i - 1] * (-a[i]);
				output += y[i - 1] * b[i];
			}

			for (int i = order; i >= 1; i--)
				y[i] = y[i - 1];

			y[0] = v;
			output += v * b[0];
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
#elif(_ANDROID)

			m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (0 << 24));
#endif
			return output;
		}

		void IIRFilter::SetT(int fs)
		{
			T = 1.0 / static_cast<Real>(fs);
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
			Real omega_sq = omega * omega;

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
			Real omega_sq = omega * omega;

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
			Real K_sq = K * K;
			Real K_sq_2 = 2.0 * K_sq;
			Real V = pow(g, 1.0 / M) - 1.0;
			Real VK = V * K;
			Real VK_2 = 2.0 * VK;
			Real VK_sq = VK * VK;

			Real alpha = (0.5 - (2.0 * m - 1.0) / (2.0 * M)) * PI_1;
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
			Real K_sq = K * K;
			Real K_sq_2 = 2.0 * K_sq;
			Real M_2 = 2.0 * M;
			Real V = pow(g, 1.0 / static_cast<Real>(M)) - 1.0;
			Real VK = V * K;
			Real VK_2 = 2.0 * VK;
			Real VK_sq = VK * VK;

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

		BandPass::BandPass() : numFilters(0), M(0), out(0.0) {};

		BandPass::BandPass(size_t order) : out(0.0)
		{
			InitFilters(order, 48000);
		};

		BandPass::BandPass(size_t order, int fs) : out(0.0)
		{
			InitFilters(order, fs);
		};

		BandPass::BandPass(size_t order, FilterShape shape, Real fb, Real g, int fs) : out(0.0)
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
			filters = std::vector<TransDF2>(numFilters, TransDF2(fs));
		}

		void BandPass::UpdateParameters(Real fb, Real g, FilterShape shape)
		{
			for (int i = 0; i < numFilters; i++)
				filters[i].UpdateParameters(fb, g, i + 1, M, shape);
		}

		Real BandPass::GetOutput(const Real input)
		{
			out = input;
			for (TransDF2& filter : filters)
				out = filter.GetOutput(out);

			return out;
		}

		ParametricEQ::ParametricEQ(size_t order, const Coefficients& fc, int fs) : mOrder(order), numFilters(fc.Length() - 1), fb(numFilters), g(numFilters), mGain(0.0), out(0.0)
		{
			InitBands(fc, fs);
		}

		ParametricEQ::ParametricEQ(size_t order, const Coefficients& fc, Coefficients& gain, int fs) : mOrder(order), numFilters(fc.Length() - 1), fb(numFilters), g(numFilters), out(0.0)
		{
			InitBands(fc, fs);
			UpdateParameters(gain);
		}

		void ParametricEQ::UpdateParameters(Coefficients& gain)
		{
			mGain = gain[numFilters];
			for (int i = 0; i < numFilters + 1; i++)
				gain[i] = std::max(EPS, gain[i]); // Prevent division by zero

			for (int i = 0; i < numFilters; i++)
			{
				g[i] = gain[i] / gain[i + 1];
				bands[i].UpdateParameters(fb[i], g[i], FilterShape(FilterShape::lbf));
			}
		}

		Real ParametricEQ::GetOutput(const Real input)
		{
			out = input;
			for (BandPass& band : bands)
				out = band.GetOutput(out);
			out *= mGain;
			return out;
		}

		void ParametricEQ::InitBands(const Coefficients& fc, int fs)
		{
			bands = std::vector<BandPass>(numFilters, BandPass(mOrder, fs));
			for (int i = 0; i < numFilters; i++)
				fb[i] = fc[i] * sqrt(fc[i + 1] / fc[i]);
		}
	}
}