/*
* @brief Linear interpolation and approximate equality functions for various classes. Also handles flushing of denormals.
*
*/

#ifndef DSP_Interpolate_h
#define DSP_Interpolate_h

// C++ headers
#if defined(_WINDOWS)
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#endif
#include <omp.h>

// Common headers
#include "Common/Coefficients.h"

// DSP headers
#include "DSP/Buffer.h"
#include "DSP/IIRFilter.h"

// Spatialiser headers
#include "Spatialiser/Types.h"

// Unity headers
#include "Unity/UnityInterface.h"

namespace RAC
{
	using namespace Spatialiser;
	namespace DSP
	{

		//////////////////// Android specific functions ////////////////////

#if(_ANDROID)
		inline int getStatusWord()
		{
			int result;
			asm volatile("mrs %[result], FPCR" : [result] "=r" (result));
			return result;
	}

		inline void setStatusWord(int a)
		{
			asm volatile("msr FPCR, %[src]" : : [src] "r" (a));
		}
#endif

		//////////////////// Flush denormals ////////////////////

		inline void FlushDenormals()
		{
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#elif(_ANDROID)
			unsigned m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (1 << 24));
#endif
		}

		inline void NoFlushDenormals()
		{
#if(_WINDOWS)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
#elif(_ANDROID)
			unsigned m_savedCSR = getStatusWord();
			setStatusWord(m_savedCSR | (0 << 24));
#endif
		}

		//////////////////// Interpolation functions ////////////////////

		inline Real Lerp(Real start, const Real end, const Real factor)
		{
#ifdef PROFILE_AUDIO_THREAD
			BeginLerp();
#endif			
			start *= 1.0 - factor;
			start += end * factor;
#ifdef PROFILE_AUDIO_THREAD
			EndLerp();
#endif		
			return start;
		}

		inline void Lerp(Buffer& start, const Buffer& end, const Real factor)
		{
#ifdef PROFILE_AUDIO_THREAD
			BeginLerp();
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
#ifdef PROFILE_AUDIO_THREAD
			EndLerp();
#endif		
		}

		inline void Lerp(Coefficients& start, const Coefficients& end, const Real factor)
		{
#ifdef PROFILE_AUDIO_THREAD
			BeginLerp();
#endif			
			size_t len = start.Length();
			for (int i = 0; i < len; i++)
			{
				start[i] *= (1.0 - factor);
				start[i] += factor * end[i];
			}
#ifdef PROFILE_AUDIO_THREAD
			EndLerp();
#endif		
		}

		//////////////////// Equality functions ////////////////////

		inline bool Equals(const Real a, const Real b)
		{
			if (a > b + EPS || a < b - EPS)
				return false;
			return true;
		}

		inline bool Equals(const Coefficients& u, const Coefficients& v)
		{
			if (u.Length() != v.Length())
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] > v[i] + EPS || u[i] < v[i] - EPS)
					return false;
			return true;
		}
	}
}

#endif