/*
* @class Buffer, BufferF
*
* @brief Declaration of Buffer and BufferF classes
*
*/

#ifndef DSP_Buffer_h
#define DSP_Buffer_h

// C++ headers
#include <vector>

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"

namespace RAC
{
	using namespace Common;
	namespace DSP
	{

		/**
		* Class that stores a resizeable buffer of Real values.
		* 
		* @details Used to store audio buffers and impulse responses
		*/
		class Buffer
		{
		public:

			/**
			* Default constructor that initialises buffer with 1 sample.
			*/
			Buffer() { ResizeBuffer(1); };

			/**
			* Constructor that initialises the buffer with a specified number of samples
			*
			* @param n The number of samples to initialise the buffer with.
			*/
			Buffer(const size_t n) { ResizeBuffer(n); };

			/**
			* Constructor that initialises the buffer with a vector of Real values.
			*
			* @param vec The vector of Real values to initialise the buffer with.
			*/
			Buffer(const std::vector<Real>& vec) : mBuffer(vec) {};

			/**
			* Default deconstructor.
			*/
			~Buffer() {};

			/**
			* Resets all samples in the buffer to 0.
			*/
			inline void ResetBuffer() { std::fill(mBuffer.begin(), mBuffer.end(), 0.0); };

			/**
			* Returns the length of the buffer.
			*/
			inline size_t Length() const { return mBuffer.size(); }

			/**
			* Resizes the buffer to a specified number of samples.
			* 
			* @details If the new size is larger than the current size, the new samples are initialised to 0.
			* 
			* @param numSamples The number of samples to resize the buffer to.
			*/
			void ResizeBuffer(const int numSamples);

			/**
			* Checks if the buffer is valid.
			* 
			* @details A buffer is valid if none of the values are nan.
			*/
			bool Valid();

			/**
			* Returns the buffer as a vector of Real values.
			*/
			std::vector<Real> GetBuffer() { return mBuffer; }

			//////////////////// Member Operators ////////////////////

			/**
			* Returns the sample at a specified index.
			*
			* @param i The index of the sample to return.
			*/
			inline Real& operator[](const int i) { return mBuffer[i]; };
			inline Real operator[](const int i) const { return mBuffer[i]; };

			/**
			* Multiplies each sample in the buffer by a scalar value.
			*/
			inline Buffer operator*=(const Real x)
			{
				for (Real& sample : mBuffer)
					sample*= x;
				return *this;
			}

			/**
			* Adds a scalar value to each sample in the buffer.
			*/
			inline Buffer operator+=(const Real x)
			{
				for (Real& sample : mBuffer)
					sample += x;
				return *this;
			}

			/**
			* Adds another buffer to this buffer.
			*/
			inline Buffer operator+=(const Buffer& x)
			{
				for (int i = 0; i < mBuffer.size(); i++)
					mBuffer[i] += x[i];
				return *this;
			}

		private:

			//////////////////// Member Variables ////////////////////

			/**
			* Buffer
			*/
			std::vector<Real> mBuffer;	// The buffer of Real values
		};

		//////////////////// Buffer Operators ////////////////////

		/**
		* Compares two buffers to check if they are equal.
		*
		* @details Two buffers are equal if they have the same length and all of their samples are equal.
		*/
		inline bool operator==(const Buffer& a, const Buffer& b)
		{
			if (a.Length() != b.Length())
				return false;
			for (int i = 0; i < a.Length(); i++)
				if (a[i] != b[i])
					return false;
			return true;
		}

		/**
		* Class that stores a resizeable buffer of float values
		*
		* @details Used to return an audio buffer of float values
		*/
		class BufferF
		{
		public:

			/**
			* Default constructor that initialises buffer with 1 sample.
			*/
			BufferF() { ResizeBuffer(1); };

			/**
			* Constructor that initialises the buffer with a specified number of samples
			*
			* @param n The number of samples to initialise the buffer with.
			*/
			BufferF(const size_t n) { ResizeBuffer(n); };

			/**
			* Default deconstructor.
			*/
			~BufferF() {};

			//////////////////// Member Operators ////////////////////

			/**
			* Returns the sample at a specified index.
			*
			* @param i The index of the sample to return.
			*/
			inline float& operator[](const int& i) { return mBuffer[i]; };
			inline float operator[](const int& i) const { return mBuffer[i]; };

			/**
			* Casts the values of a Buffer to a BufferF
			*/
			inline BufferF& operator=(const Buffer& b)
			{
				ResizeBuffer(b.Length());
				for (int i = 0; i < b.Length(); i++)
					mBuffer[i] = static_cast<float>(b[i]);
				return *this;
			}

			/**
			* Returns the length of the buffer.
			*/
			inline size_t Length() const { return mBuffer.size(); }

		private:

			/**
			* Resizes the buffer to a specified number of samples.
			*
			* @details If the new size is larger than the current size, the new samples are initialised to 0.
			*
			* @param numSamples The number of samples to resize the buffer to.
			*/
			void ResizeBuffer(const size_t numSamples);

			//////////////////// Member Variables ////////////////////

			/**
			* Buffer
			*/
			std::vector<float> mBuffer; // The buffer of float values
		};
	}
}
#endif