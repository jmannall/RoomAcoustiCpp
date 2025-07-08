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
#include <assert.h>

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"

namespace RAC
{
	using namespace Common;
	namespace DSP
	{
		/**
		* @brief Class that stores a resizeable buffer of T values.
		* 
		* @details Used to store audio buffers and impulse responses
		*/
		template <typename T = Real>
		class Buffer
		{
		public:

			/**
			* @brief Default constructor that initialises buffer with 1 sample.
			*/
			Buffer() : mBuffer(1, 0.0) {};

			/**
			* @brief Constructor that initialises the buffer with a specified number of samples
			*
			* @param length The number of samples to initialise the buffer with.
			*/
			Buffer(const int length) : mBuffer(length, 0.0) {};

			/**
			* Constructor that initialises the buffer with a vector of T values.
			*
			* @param vec The vector of T values to initialise the buffer with.
			*/
			Buffer(const std::vector<T>& vector) : mBuffer(vector) {};

			/**
			* @brief Default deconstructor.
			*/
			~Buffer() {};

			/**
			* @brief Sets all samples in the buffer to 0.
			*/
			inline void Reset() { std::fill(mBuffer.begin(), mBuffer.end(), 0.0); };

			/**
			* @brief Returns the length of the buffer.
			* 
			* @return The length of the buffer
			*/
			inline size_t Length() const { return mBuffer.size(); }

			/**
			* @brief Resizes the buffer to a specified number of samples.
			* @details If the new size is larger than the current size, the new samples are initialised to 0.
			* If it is smaller, the buffer is truncated.
			* 
			* @param numSamples The number of samples to resize the buffer to.
			*/
			inline void ResizeBuffer(const size_t numSamples)
			{
				assert(numSamples >= 0);

				size_t size = Length();
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

			/**
			* @brief Checks if the buffer is valid.
			* @details A buffer is valid if none of the values are nan.
			* 
			* @return False if the buffer contains nan values, true otherwise
			*/
			bool Valid()
			{
				for (int i = 0; i < Length(); i++)
					if (std::isnan(mBuffer[i]))
						return false;
				return true;
			}

			/**
			* @brief Access the buffer at the specified index
			*
			* @param i The index of the value to return
			* @return A reference to the value at the specified index
			*/
			inline T& operator[](const int i) { return mBuffer[i]; };

			/**
			* @brief Access the buffer at the specified index
			*
			* @param i The index of the value to return
			* @return The value at the specified index
			*/
			inline T operator[](const int i) const { assert(i < mBuffer.size()); return mBuffer[i]; };

			/**
			* @brief Multiplies each sample in the buffer by a scalar value.
			*/
			inline Buffer operator*=(const T a)
			{
				for (T& sample : mBuffer)
					sample *= a;
				return *this;
			}

			/**
			* @brief Adds a scalar value to each sample in the buffer.
			*/
			inline Buffer operator+=(const T a)
			{
				for (T& sample : mBuffer)
					sample += a;
				return *this;
			}

			/**
			* @brief Adds another buffer to this buffer.
			*/
			inline Buffer operator+=(const Buffer& x)
			{
				for (int i = 0; i < mBuffer.size(); i++)
					mBuffer[i] += x[i];
				return *this;
			}

			inline auto begin() { return mBuffer.begin(); }

			inline auto end() { return mBuffer.end(); }

			inline const auto begin() const { return mBuffer.begin(); }

			inline const auto end() const { return mBuffer.end(); }

		private:

			std::vector<T> mBuffer;		// Buffer data
		};

		//////////////////// Buffer operator overloads ////////////////////

		/**
		* @brief Performs an element-wise comparison
		* @return True if their samples are equal, false otherwise
		*/
		template <typename T = Real>
		inline bool operator==(const Buffer<T>& a, const Buffer<T>& b)
		{
			if (a.Length() != b.Length())
				return false;
			for (int i = 0; i < a.Length(); i++)
				if (a[i] != b[i])
					return false;
			return true;
		}
	}
}
#endif