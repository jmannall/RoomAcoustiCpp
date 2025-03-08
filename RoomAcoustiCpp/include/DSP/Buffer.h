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
		* @brief Class that stores a resizeable buffer of Real values.
		* 
		* @details Used to store audio buffers and impulse responses
		*/
		class Buffer
		{
		public:

			/**
			* @brief Default constructor that initialises buffer with 1 sample.
			*/
			Buffer() { ResizeBuffer(1); };

			/**
			* @brief Constructor that initialises the buffer with a specified number of samples
			*
			* @param length The number of samples to initialise the buffer with.
			*/
			Buffer(const int length) { ResizeBuffer(length); };

			/**
			* Constructor that initialises the buffer with a vector of Real values.
			*
			* @param vec The vector of Real values to initialise the buffer with.
			*/
			Buffer(const std::vector<Real>& vector) : mBuffer(vector) {};

			/**
			* @brief Default deconstructor.
			*/
			~Buffer() {};

			/**
			* @brief Resets all samples in the buffer to 0.
			*/
			inline void ResetBuffer() { std::fill(mBuffer.begin(), mBuffer.end(), 0.0); };

			/**
			* @brief Returns the length of the buffer.
			* 
			* @return The length of the buffer
			*/
			inline int Length() const { return static_cast<int>(mBuffer.size()); }

			/**
			* @brief Resizes the buffer to a specified number of samples.
			* @details If the new size is larger than the current size, the new samples are initialised to 0.
			* 
			* @param numSamples The number of samples to resize the buffer to.
			*/
			void ResizeBuffer(const int numSamples);

			/**
			* @brief Checks if the buffer is valid.
			* @details A buffer is valid if none of the values are nan.
			* 
			* @return False if the buffer contains nan values, true otherwise
			*/
			bool Valid();

			/**
			* Returns the buffer as a vector of Real values.
			*/
			std::vector<Real> GetBuffer() { return mBuffer; }

			/**
			* @brief Access the buffer at the specified index
			*
			* @param i The index of the value to return
			* @return A reference to the value at the specified index
			*/
			inline Real& operator[](const int i) { return mBuffer[i]; };

			/**
			* @brief Access the buffer at the specified index
			*
			* @param i The index of the value to return
			* @return The value at the specified index
			*/
			inline Real operator[](const int i) const { assert(i < mBuffer.size()); return mBuffer[i]; };

			/**
			* @brief Multiplies each sample in the buffer by a scalar value.
			*/
			inline Buffer operator*=(const Real a)
			{
				for (Real& sample : mBuffer)
					sample *= a;
				return *this;
			}

			/**
			* @brief Adds a scalar value to each sample in the buffer.
			*/
			inline Buffer operator+=(const Real a)
			{
				for (Real& sample : mBuffer)
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

			std::vector<Real> mBuffer;		// Buffer data
		};

		//////////////////// Buffer operator overloads ////////////////////

		/**
		* @brief Performs an element-wise comparison
		* @return True if their samples are equal, false otherwise
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
		* @brief Class that stores a resizeable buffer of float values
		*
		* @details Used to return an audio buffer of float values
		*/
		class BufferF
		{
		public:

			/**
			* @brief Default constructor that initialises buffer with 1 sample.
			*/
			BufferF() { ResizeBuffer(1); };

			/**
			* @brief Constructor that initialises the buffer with a specified number of samples
			*
			* @param length The number of samples to initialise the buffer with.
			*/
			BufferF(const int length) { ResizeBuffer(length); };

			/**
			* @brief Default deconstructor.
			*/
			~BufferF() {};

			/**
			* @brief Access the buffer at the specified index
			*
			* @param i The index of the value to return
			* @return The value at the specified index
			*/
			inline float& operator[](const int& i) { return mBuffer[i]; };

			/**
			* @brief Access the buffer at the specified index
			*
			* @param i The index of the value to return
			* @return A reference to the value at the specified index
			*/
			inline float operator[](const int& i) const { return mBuffer[i]; };

			/**
			* @brief Casts the values of a Buffer to a BufferF
			*/
			inline BufferF& operator=(const Buffer& b)
			{
				ResizeBuffer(b.Length());
				for (int i = 0; i < b.Length(); i++)
					mBuffer[i] = static_cast<float>(b[i]);
				return *this;
			}

			/**
			* @brief Returns the length of the buffer.
			*/
			inline int Length() const { return static_cast<int>(mBuffer.size()); }

		private:

			/**
			* @brief Resizes the buffer to a specified number of samples.
			* @details If the new size is larger than the current size, the new samples are initialised to 0.
			*
			* @param numSamples The number of samples to resize the buffer to.
			*/
			void ResizeBuffer(const int numSamples);

			std::vector<float> mBuffer;		// Buffer data
		};
	}
}
#endif