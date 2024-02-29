///*
//* \class Buffer, BuffferF, FIRFilter, IIRFilter, HighShelf, LowPass, TransDF2, Band, LinkwitzRiley, BandPass, ParametricEQ
//* 
//* \brief Base audio components
//* 
//*/
//
//#ifndef Common_AudioManager_h
//#define Common_AudioManager_h
//
//// C++ headers
//#if defined(_WINDOWS)
///* Microsoft C/C++-compatible compiler */
//#include <intrin.h>
//#endif
//#include <vector>
//
//// Common headers
//#include "Common/Definitions.h"
//#include "Common/Types.h"
//#include "Common/Coefficients.h"
//
//// DSP headers
//#include "DSP/Buffer.h"
//#include "DSP/FIRFilter.h"
//
//namespace UIE
//{
//	namespace Common
//	{
//		inline void Lerp(Real& start, const Real& end, const Real factor)
//		{
//			if (end - EPS < start && start < end + EPS)
//			{
//				start = end;
//				return;
//			}
//#if(_WINDOWS)
//			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
//#elif(_ANDROID)
//			unsigned m_savedCSR = getStatusWord();
//			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
//			setStatusWord(m_savedCSR | (1 << 24));
//#endif
//			start *= 1.0 - factor;
//			start += end * factor;
//
//#if(_WINDOWS)
//			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
//#elif(_ANDROID)
//
//			m_savedCSR = getStatusWord();
//			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
//			setStatusWord(m_savedCSR | (0 << 24));
//#endif
//		}
//
//		inline void Lerp(Buffer& start, const Buffer& end, const Real factor)
//		{
//#if(_WINDOWS)
//			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
//#elif(_ANDROID)
//
//			unsigned m_savedCSR = getStatusWord();
//			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
//			setStatusWord(m_savedCSR | (1 << 24));
//#endif
//			size_t len = start.Length();
//
//			if (len % 8 != 0)
//			{
//				for (int i = 0; i < len; i++)
//				{
//					start[i] *= (1.0 - factor);
//					start[i] += factor * end[i];
//				}
//			}
//			else
//			{
//				int i = 0;
//				while (i < len)
//				{
//					start[i] *= (1.0 - factor);
//					start[i] += factor * end[i];
//					i++;
//					start[i] *= (1.0 - factor);
//					start[i] += factor * end[i];
//					i++;
//					start[i] *= (1.0 - factor);
//					start[i] += factor * end[i];
//					i++;
//					start[i] *= (1.0 - factor);
//					start[i] += factor * end[i];
//					i++;
//					start[i] *= (1.0 - factor);
//					start[i] += factor * end[i];
//					i++;
//					start[i] *= (1.0 - factor);
//					start[i] += factor * end[i];
//					i++;
//					start[i] *= (1.0 - factor);
//					start[i] += factor * end[i];
//					i++;
//					start[i] *= (1.0 - factor);
//					start[i] += factor * end[i];
//					i++;
//				}
//			}
//#if(_WINDOWS)
//			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
//#elif(_ANDROID)
//
//			m_savedCSR = getStatusWord();
//			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
//			setStatusWord(m_savedCSR | (0 << 24));
//#endif
//		}
//
//		enum class FilterShape
//		{
//			lpf,
//			hpf,
//			none
//		};
//
//		struct TransDF2Parameters
//		{
//			Real z[2];
//			Real p[2];
//			Real k;
//			TransDF2Parameters() : z{ 0.25, -0.99 }, p{ 0.99, -0.25 }, k(0.0) {};
//			TransDF2Parameters(Real _z, Real _p, Real _k) : z{ _z, _z }, p{ _p, _p }, k(_k) {};
//		};
//
//		class TransDF2 : public IIRFilter
//		{
//		public:
//			// TransDF2() : IIRFilter(2, 48000) { a[0] = 1.0; };
//			TransDF2(int fs) : IIRFilter(2, fs) { a[0] = 1.0; };
//			TransDF2(TransDF2Parameters zpk, int fs) : IIRFilter(2, fs) { a[0] = 1.0; UpdateParameters(zpk); };
//			TransDF2(Real fc, FilterShape shape, int fs) : IIRFilter(2, fs) { a[0] = 1.0; UpdateParameters(fc, shape); };
//			
//			void UpdateParameters(TransDF2Parameters zpk);
//			void UpdateParameters(Real fc, FilterShape shape);
//
//		private:
//			void UpdateLPF(Real fc);
//			void UpdateHPF(Real fc);
//		};
//
//		class Band : public IIRFilter
//		{
//		public:
//			Band(bool isLowBand, int fs) : IIRFilter(2, fs) { a[0] = 1.0; SetUpdatePointer(isLowBand); }
//			Band(Real fb, Real g, int m, int M, bool isLowBand, int fs) : IIRFilter(2, fs)
//			{ 
//				a[0] = 1.0;
//				SetUpdatePointer(isLowBand);
//				UpdateParameters(fb, g, m, M);
//			};
//
//			inline void SetUpdatePointer(bool isLowBand)
//			{
//				if (isLowBand)
//					UpdateBand = &Band::UpdateLowBand;
//				else
//					UpdateBand = &Band::UpdateHighBand;
//			};
//
//			inline void UpdateParameters(Real fb, Real g, int m, int M) { (this->*UpdateBand)(fb, g, m, M); };
//		private:
//			void UpdateLowBand(Real fb, Real g, int m, int M);
//			void UpdateHighBand(Real fb, Real g, int m, int M);
//
//			// Function pointer
//			void (Band::*UpdateBand)(Real fb, Real g, int m, int M);
//
//		};
//
//		//////////////////// Filterbanks ////////////////////
//
//		/*class LinkwitzRiley
//		{
//		public:
//			LinkwitzRiley(int fs);
//			LinkwitzRiley(Real fc0, Real fc1, Real fc2, int fs);
//			~LinkwitzRiley() {};
//
//			Real GetOutput(const Real input);
//			void UpdateParameters(Real gain[]);
//
//			Real fm[4];
//
//		private:
//			void InitFilters(int fs);
//			void CalcFM();
//
//			Real fc[3];
//			Real g[4];
//
//			std::vector<TransDF2> filters;
//		};*/
//
//		class BandPass
//		{
//		public:
//			// BandPass();
//			// BandPass(size_t order);
//			BandPass(const size_t order, const bool useLowBands, const int fs);
//			BandPass(const size_t order, const bool useLowBands, const Real fb, const Real g, const int fs);
//
//			void InitFilters(size_t order, bool useLowBands, int fs);
//			void UpdateParameters(Real fb, Real g);
//			Real GetOutput(const Real input);
//
//			inline void ClearBuffers()
//			{
//				for (int i = 0; i < numFilters; i++)
//					filters[i].ClearBuffers();
//			}
//
//		private:
//			int M;
//			FilterShape mShape;
//			size_t numFilters;
//			std::vector<Band> filters;
//
//			Real out;
//		};
//
//		class ParametricEQ
//		{
//		public:
//			ParametricEQ(size_t order, const Coefficients& fc, int fs);
//			ParametricEQ(size_t order, const Coefficients& fc, Coefficients& gain, int fs);
//
//			void UpdateParameters(Coefficients& gain);
//			Real GetOutput(const Real input);
//
//			inline void ClearBuffers()
//			{
//				for (int i = 0; i < numFilters; i++)
//					bands[i].ClearBuffers();
//			}
//		private:
//			void InitBands(const Coefficients& fc, int fs);
//
//			size_t mOrder;
//			size_t numFilters;
//			std::vector<BandPass> bands;
//			Real mGain;
//			Coefficients fb;
//			Coefficients g;
//			Real out;
//		};
//	}
//}
//
//#endif
//
//
//
