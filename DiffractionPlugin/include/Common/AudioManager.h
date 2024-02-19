/*
*
*  \General audio components
*
*/

#ifndef Common_AudioManager_h
#define Common_AudioManager_h

// C++ headers
#if defined(_WINDOWS)
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#endif
#include <vector>

// Common headers
#include "Common/Definitions.h"
#include "Common/Types.h"

#include "Spatialiser/Main.h"

namespace UIE
{
	namespace Common
	{

		inline void Lerp(Real& start, const Real& end, const Real factor)
		{
			if (end - EPS < start && start < end + EPS)
			{
				start = end;
				return;
			}
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#elif(_ANDROID)

			unsigned m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (1 << 24));
#endif
			start *= 1.0 - factor;
			start += end * factor;

#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
#elif(_ANDROID)

			m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (0 << 24));
#endif
		}

		//////////////////// Buffer class ////////////////////

		class Buffer
		{
		public:
			Buffer() { InitialiseBuffer(1); };
			Buffer(int n) { InitialiseBuffer(n); };
			Buffer(const std::vector<Real>& vec) { mBuffer = vec; };
			~Buffer() {};

			inline Real& operator[](const int& i) { return mBuffer[i]; };
			inline Real operator[](const int& i) const { return mBuffer[i]; };

			void ResetBuffer();
			void ResizeBuffer(size_t numSamples);
			inline size_t Length() const { return mBuffer.size(); }
			bool Valid();

			std::vector<Real> GetBuffer() { std::vector<Real> buffer(mBuffer.begin(), mBuffer.end()); return buffer; }

			inline Buffer operator*=(Real x)
			{
				for (Real& sample : mBuffer)
					sample *= x;
				return *this;
			}

			inline Buffer operator+=(Real x)
			{
				for (Real& sample : mBuffer)
					sample += x;
				return *this;
			}

			inline Buffer operator+=(Buffer& x)
			{
				for (int i = 0; i < mBuffer.size(); i++)
					mBuffer[i] += x[i];
				return *this;
			}

		private:
			void InitialiseBuffer(int n);

			std::vector<Real> mBuffer;
		};

		inline void Lerp(Buffer& start, const Buffer& end, const Real factor)
		{
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#elif(_ANDROID)

			unsigned m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (1 << 24));
#endif
			size_t len = start.Length();

			if (len % 8 != 0)
			{
				for (int i = 0; i < len; i++)
				{
					start[i] *= (1.0 - factor);
					start[i] += factor * end[i];
				}
			}
			else
			{
				int i = 0;
				while (i < len)
				{
					start[i] *= (1.0 - factor);
					start[i] += factor * end[i];
					i++;
					start[i] *= (1.0 - factor);
					start[i] += factor * end[i];
					i++;
					start[i] *= (1.0 - factor);
					start[i] += factor * end[i];
					i++;
					start[i] *= (1.0 - factor);
					start[i] += factor * end[i];
					i++;
					start[i] *= (1.0 - factor);
					start[i] += factor * end[i];
					i++;
					start[i] *= (1.0 - factor);
					start[i] += factor * end[i];
					i++;
					start[i] *= (1.0 - factor);
					start[i] += factor * end[i];
					i++;
					start[i] *= (1.0 - factor);
					start[i] += factor * end[i];
					i++;
				}
			}
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
#elif(_ANDROID)

			m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (0 << 24));
#endif
		}

		bool BuffersEqual(Buffer& x, Buffer& y);

		class BufferF
		{
		public:
			BufferF() {};
			~BufferF() {};

			inline float& operator[](const int& i) { return mBuffer[i]; };

			inline void operator=(Buffer& b)
			{
				ResizeBuffer(b.Length());
				for (int i = 0; i < b.Length(); i++)
					mBuffer[i] = static_cast<float>(b[i]);
			}

		private:
			void ResizeBuffer(size_t numSamples);

			std::vector<float> mBuffer;
		};

		//////////////////// FIR Filter ////////////////////

		class FIRFilter
		{
		public:
			FIRFilter(Buffer& ir) : irLen(1), mIr(), count(ir.Length() - 1), n(0), inputLine(), store() { SetImpulseResponse(ir); };
			~FIRFilter() {};

			Real GetOutput(Real input);
			
			inline void Resize(size_t len)
			{
				if (len % 8 != 0)
					len += (8 - len % 8);
				if (len != irLen)
				{
					if (len > irLen)
						IncreaseSize(len);
					else
						DecreaseSize(len);
					irLen = len;
				}
			}

			inline void SetImpulseResponse(const Buffer& ir)
			{ 
				Resize(ir.Length());
				for (int i = 0; i < ir.Length(); i++)
					mIr[i] = ir[i];
			}

		private:

			inline void IncreaseSize(size_t len)
			{
				inputLine.ResizeBuffer(len);
				mIr.ResizeBuffer(len);
			}

			inline void DecreaseSize(size_t len)
			{
				store = inputLine;
				int index = count;
				for (int i = 0; i < irLen; i++)
				{
					inputLine[i] = store[index++];
					if (index >= irLen) { index = 0; }
				}
				inputLine.ResizeBuffer(len);
				mIr.ResizeBuffer(len);
				count = 0;
			}

			Buffer mIr;
			Buffer inputLine;
			Buffer store;
			int count;
			int n;
			size_t irLen;
		};

		//////////////////// IIR Filter ////////////////////

		class IIRFilter
		{
		public:
			IIRFilter(int _order, int fs) : order(_order), T(1.0 / static_cast<Real>(fs)), b(order + 1), a(order + 1), x(order + 1), y(order + 1) {};
			~IIRFilter() {};

			Real GetOutput(Real input);
			void SetT(int fs);

			inline void ClearBuffers() { x.ResetBuffer(); y.ResetBuffer(); }

		protected:
			int order;
			Real T;
			Buffer b;
			Buffer a;
			Buffer x;
			Buffer y;
		};

		enum class FilterShape
		{
			lpf,
			hpf,
			lbf,
			hbf
		};

		class HighShelf : public IIRFilter
		{
		public:
			HighShelf() : IIRFilter(1, 48000) {};
			HighShelf(int fs) : IIRFilter(1, fs) {};
			HighShelf(Real fc, Real g, int fs) : IIRFilter(1, fs) { UpdateParameters(fc, g); };
			void UpdateParameters(Real fc, Real g);

		private:
		};

		class LowPass : public IIRFilter
		{
		public:
			LowPass() : IIRFilter(1, 48000) {};
			LowPass(int fs) : IIRFilter(1, fs) {};
			LowPass(Real fc, int fs) : IIRFilter(1, fs) { UpdateParameters(fc); };
			void UpdateParameters(Real fc);

		private:
		};

		struct TransDF2Parameters
		{
			Real z[2];
			Real p[2];
			Real k;
			TransDF2Parameters() : z{ 0.25, -0.99 }, p{ 0.99, -0.25 }, k(0.0) {};
			TransDF2Parameters(Real _z, Real _p, Real _k) : z{ _z, _z }, p{ _p, _p }, k(_k) {};
		};

		class TransDF2 : public IIRFilter
		{
		public:
			TransDF2() : IIRFilter(2, 48000) { a[0] = 1.0; };
			TransDF2(int fs) : IIRFilter(2, fs) { a[0] = 1.0; };
			TransDF2(TransDF2Parameters zpk, int fs) : IIRFilter(2, fs) { a[0] = 1.0; UpdateParameters(zpk); };
			TransDF2(Real fc, FilterShape shape, int fs) : IIRFilter(2, fs) { a[0] = 1.0; UpdateParameters(fc, shape); };
			TransDF2(Real fb, Real g, int m, int M, FilterShape shape, int fs) : IIRFilter(2, fs) { a[0] = 1.0; UpdateParameters(fb, g, m, M, shape); };
			void UpdateParameters(TransDF2Parameters zpk);
			void UpdateParameters(Real fc, FilterShape shape);
			void UpdateParameters(Real fb, Real g, int m, int M, FilterShape shape);

		private:
			void UpdateLPF(Real fc);
			void UpdateHPF(Real fc);
			void UpdateLBF(Real fb, Real g, int m, int M);
			void UpdateHBF(Real fb, Real g, int m, int M);
		};

		//////////////////// Filterbanks ////////////////////

		class LinkwitzRiley
		{
		public:
			LinkwitzRiley(int fs);
			LinkwitzRiley(Real fc0, Real fc1, Real fc2, int fs);
			~LinkwitzRiley() {};

			Real GetOutput(const Real input);
			void UpdateParameters(Real gain[]);

			Real fm[4];

		private:
			void InitFilters(int fs);
			void CalcFM();

			Real fc[3];
			Real g[4];

			TransDF2 filters[20];
		};

		class BandPass
		{
		public:
			BandPass();
			BandPass(size_t order);
			BandPass(size_t order, int fs);
			BandPass(size_t order, FilterShape shape, Real fb, Real g, int fs);

			void InitFilters(int order, int fs);
			void UpdateParameters(Real fb, Real g, FilterShape shape);
			Real GetOutput(const Real input);

			inline void ClearBuffers()
			{
				for (int i = 0; i < numFilters; i++)
					filters[i].ClearBuffers();
			}

		private:
			int M;
			size_t numFilters;
			std::vector<TransDF2> filters;

			Real out;
		};

		class ParametricEQ
		{
		public:
			ParametricEQ(size_t order);
			ParametricEQ(size_t order, int fs);
			ParametricEQ(size_t order, Real fc[], Real gain[], int fs);

			void UpdateParameters(const Real fc[], Real gain[]);
			Real GetOutput(const Real input);

			inline void ClearBuffers()
			{
				for (int i = 0; i < numFilters; i++)
					bands[i].ClearBuffers();
			}
		private:
			void InitBands(int fs);

			size_t mOrder;
			size_t numFilters;
			BandPass bands[4];
			Real mGain;
			Real fb[4];
			Real g[4];
			Real out;
		};
	}
}

#endif



