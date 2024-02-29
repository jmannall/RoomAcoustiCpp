#pragma once

#ifndef DSP_Interpolate_h
#define DSP_Interpolate_h

// C++ headers
#if defined(_WINDOWS)
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#endif

// Common headers
#include "Common/Coefficients.h"

// DSP headers
#include "DSP/Buffer.h"

namespace UIE
{
	namespace DSP
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

		inline void Lerp(Coefficients& start, const Coefficients& end, const Real factor)
		{
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#elif(_ANDROID)

			unsigned m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (1 << 24));
#endif
			size_t len = start.Length();
			for (int i = 0; i < len; i++)
			{
				start[i] *= (1.0 - factor);
				start[i] += factor * end[i];
			}
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
#elif(_ANDROID)

			m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (0 << 24));
#endif
		}
	}
}

#endif