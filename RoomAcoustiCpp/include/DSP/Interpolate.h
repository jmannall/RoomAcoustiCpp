/*
* @brief Linear interpolation and approximate equality functions for various classes. Also handles flushing of denormals.
*
*/

#ifndef DSP_Interpolate_h
#define DSP_Interpolate_h

// C++ headers
#if defined(__x86_64__) || defined(_M_X64)
// #include <intrin.h>
#include <xmmintrin.h>
#endif
#include <omp.h>

// Common headers
#include "Common/Coefficients.h"

// DSP headers
#include "DSP/Buffer.h"
#include "DSP/IIRFilter.h"

// Spatialiser headers
#include "Spatialiser/Types.h"

namespace RAC
{
	using namespace Spatialiser;
	namespace DSP
	{

		//////////////////// Android specific functions ////////////////////

#if defined(__aarch64__) || defined(__arm__)
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
#if defined(__aarch64__) || defined(__arm__)
			unsigned m_savedCSR = getStatusWord();
			// Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
			setStatusWord(m_savedCSR | (1 << 24));
#endif
#if defined(__x86_64__) || defined(_M_X64)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif
		}

		/**
		* Resets the CPU to use denormals
		*/
		inline void NoFlushDenormals()
		{
#if defined(__aarch64__) || defined(__arm__)
			unsigned m_savedCSR = getStatusWord();
			setStatusWord(m_savedCSR | (0 << 24));
#endif
#if defined(__x86_64__) || defined(_M_X64)
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
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
	
			start *= 1.0 - factor;
			start += end * factor;
			return start;
		}

		/**
		* Performs a linear interpolation of two buffers classes
		* @details if start is longer than end, the remaining samples are interpolated to zero.
		*
		* @params start The current buffer to be interpolated
		* @params end The target buffer (must be a multiple of 8 samples)
		* @params startLength The previous used length of the start buffer for interpolating old samples to zero
		* @params factor The interpolation factor (must be between 0 and 1)
		*/
		inline void Lerp(Buffer<>& start, const Buffer<>& end, const int startLength, const Real factor)
		{
			assert(0.0 < factor && factor <= 1.0);
			assert(end.Length() % 8 == 0);
			assert(start.Length() >= end.Length());
			assert(startLength <= start.Length());

			int len = end.Length();
			int i = 0;
			while(i < len) // Easier for compiler to vectorise ~1.6x faster
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
			for (int i = end.Length(); i < startLength; i++) // Interpolating to zero
				start[i] *= (1.0 - factor);
		}

		/**
		* Performs a linear interpolation of two coefficient classes
		*
		* @params start The current coefficients to be interpolated
		* @params end The target coefficients
		* @params factor The interpolation factor (must be between 0 and 1)
		*/
		template<typename T>
		inline void Lerp(Coefficients<T>& start, const Coefficients<T>& end, const Real factor)
		{
			assert(0.0 < factor && factor <= 1.0);	
			assert(start.Length() == end.Length());
			
			start *= (1.0 - factor);
			start += factor * end;
		}

		//////////////////// Equality functions ////////////////////

		/**
		* Checks for equality between two real values
		* 
		* @params a Value 1
		* @params b Value 2
		* @returns True if equal within the threshold EPS, false otherwise
		*/
		inline bool Equals(const Real a, const Real b, const Real threshold = EPS)
		{
			if (a > b + threshold || a < b - threshold)
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
		template<typename T>
		inline bool Equals(const Coefficients<T>& u, const Coefficients<T>& v, const Real threshold = EPS)
		{
			if (u.Length() != v.Length())
				return false;
			for (int i = 0; i < u.Length(); i++)
				if (u[i] > v[i] + threshold || u[i] < v[i] - threshold)
					return false;
			return true;
		}

		/**
		* Checks for equality between two buffers
		* 
		* @param u Buffer 1
		* @param v Buffer 2
		* @returns True if equal within the threshold EPS, false otherwise
		*/
		inline bool Equals(const Buffer<>& u, const Buffer<>& v, const int length, const Real threshold = EPS)
		{
			assert(v.Length() == length);
			assert(u.Length() >= length);

			for (int i = 0; i < length; i++)
				if (u[i] > v[i] + threshold || u[i] < v[i] - threshold)
					return false;
			for (int i = length; i < u.Length(); i++)
				if (u[i] > threshold || u[i] < -threshold)
					return false;
			return true;
		}
	}
}

#endif