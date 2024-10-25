/*
* @brief Linear interpolation and approximate equality functions for various classes. Also handles flushing of denormals.
*
*/

#ifndef DSP_Interpolate_h
#define DSP_Interpolate_h

// C++ headers
#if defined(_WINDOWS)
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

		/**
		* Forces the CPU to flush denormals (cause performance issues in recursive filter structures)
		*/
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

		/**
		* Resets the CPU to use denormals
		*/
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

		/**
		* Performs a linear interpolation of two real values
		* 
		* @params start The current value
		* @params end The target value
		* @params factor The interpolation factor (must be between 0 and 1)
		* @return The result of the interpolation
		*/
		inline Real Lerp(Real start, const Real end, const Real factor)
		{
			assert(0.0 < factor && factor <= 1.0);

#ifdef PROFILE_DETAILED
			BeginLerp();
#endif			
			start *= 1.0 - factor;
			start += end * factor;
#ifdef PROFILE_DETAILED
			EndLerp();
#endif		
			return start;
		}

		/**
		* Performs a linear interpolation of two buffers classes
		*
		* @params start The current buffer to be interpolated
		* @params end The target buffer
		* @params factor The interpolation factor (must be between 0 and 1)
		*/
		inline void Lerp(Buffer& start, const Buffer& end, const Real factor)
		{
			assert(0.0 < factor && factor <= 1.0);

#ifdef PROFILE_DETAILED
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
#ifdef PROFILE_DETAILED
			EndLerp();
#endif		
		}

		/**
		* Performs a linear interpolation of two coefficient classes
		*
		* @params start The current coefficients to be interpolated
		* @params end The target coefficients
		* @params factor The interpolation factor (must be between 0 and 1)
		*/
		inline void Lerp(Coefficients& start, const Coefficients& end, const Real factor)
		{
			assert(0.0 < factor && factor <= 1.0);

#ifdef PROFILE_DETAILED
			BeginLerp();
#endif			
			size_t len = start.Length();
			for (int i = 0; i < len; i++)
			{
				start[i] *= (1.0 - factor);
				start[i] += factor * end[i];
			}
#ifdef PROFILE_DETAILED
			EndLerp();
#endif		
		}

		//////////////////// Equality functions ////////////////////

		/**
		* Checks for equality between two real values
		* 
		* @params a Value 1
		* @params b Value 2
		* @returns True if equal within the threshold EPS, false otherwise
		*/
		inline bool Equals(const Real a, const Real b)
		{
			if (a > b + EPS || a < b - EPS)
				return false;
			return true;
		}

		/**
		* Checks for equality between two coefficient classes
		*
		* @params u Coefficients 1
		* @params v Coefficients 2
		* @returns True if equal within the threshold EPS, false otherwise
		*/
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