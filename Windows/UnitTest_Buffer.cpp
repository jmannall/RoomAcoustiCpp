
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/Buffer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

	TEST_CLASS(Buffer_Class)
	{
	public:

		std::vector<Real> CreateRandomVector(const size_t length)
		{
			std::vector<Real> vec(length, 0.0);
			for (int i = 0; i < length; i++)
				vec[i] = rand();
			return vec;
		}

		TEST_METHOD(Default)
		{
			const size_t length = 1;
			const Buffer buffer = Buffer();
			Assert::AreEqual(length, buffer.Length(), L"Buffer not initialised correctly");
			for (int i = 0; i < buffer.Length(); i++)
				Assert::AreEqual(0.0, buffer[i], L"Buffer not initialised to zero");
		}

		TEST_METHOD(InitLength)
		{
			const size_t length = 256;
			const Buffer buffer(length);
			Assert::AreEqual(buffer.Length(), length, L"Buffer not initialised correctly");
			for (int i = 0; i < buffer.Length(); i++)
				Assert::AreEqual(0.0, buffer[i], L"Buffer not initialised to zero");
		}

		TEST_METHOD(InitVector)
		{
			const size_t length = 256;
			const std::vector<Real> vec = CreateRandomVector(length);

			const Buffer buffer(vec);
			Assert::AreEqual(buffer.Length(), length, L"Buffer not initialised correctly");
			for (int i = 0; i < buffer.Length(); i++)
				Assert::AreEqual(vec[i], buffer[i], L"Buffer not initialised to correct values");
		}

		TEST_METHOD(Reset)
		{
			const size_t length = 128;
			const std::vector<Real> vec = CreateRandomVector(length);
			Buffer buffer(vec);

			buffer.Reset();
			for (int i = 0; i < length; i++)
				Assert::AreEqual(0.0, buffer[i], L"Buffer not reset to zero");
		}

		TEST_METHOD(Resize)
		{
			const size_t length = 89;
			const std::vector<Real> vec = CreateRandomVector(length);
			Buffer buffer(vec);
			
			const size_t newLength = 245;
			buffer.ResizeBuffer(newLength);
			Assert::AreEqual(buffer.Length(), newLength, L"Buffer not resized correctly");
			for (int i = length; i < buffer.Length(); i++)
				Assert::AreEqual(0.0, buffer[i], L"New values not initialised to zero");

			buffer.ResizeBuffer(length);
			Assert::AreEqual(buffer.Length(), length, L"Buffer not resized correctly");
			for (int i = length; i < buffer.Length(); i++)
				Assert::AreEqual(vec[i], buffer[i], L"Buffer values have changed");
		}

		TEST_METHOD(Valid)
		{
			const size_t length = 512;
			Buffer buffer(length);
			Assert::IsTrue(buffer.Valid(), L"Buffer not valid");
			for (int i = 0; i < length; i++)
				buffer[i] = rand();
			Assert::IsTrue(buffer.Valid(), L"Buffer not valid");
			buffer[3] = NAN;
			Assert::IsFalse(buffer.Valid(), L"Buffer valid");
		}

		TEST_METHOD(Access)
		{
			const size_t length = 512;
			const std::vector<Real> vec = CreateRandomVector(length);
			Buffer buffer(vec);
			for (int i = 0; i < length; i++)
				Assert::AreEqual(vec[i], buffer[i], L"Incorrect buffer access");

			std::vector<Real> newVec = CreateRandomVector(length);
			for (int i = 0; i < length; i++)
				buffer[i] = newVec[i];
			for (int i = 0; i < length; i++)
				Assert::AreEqual(newVec[i], buffer[i], L"Incorrect buffer access");
		}

		TEST_METHOD(Assign)
		{
			const size_t length = 512;
			const std::vector<Real> vec = CreateRandomVector(length);
			Buffer buffer(length);
			
			for (int i = 0; i < length; i++)
				buffer[i] = vec[i];

			for (int i = 0; i < length; i++)
				Assert::AreEqual(vec[i], buffer[i], L"Incorrect buffer assignment");
		}

		TEST_METHOD(Equality)
		{
			const size_t length = 512;
			std::vector<Real> vec = CreateRandomVector(length);
			Buffer buffer1(vec);
			Buffer buffer2(vec);
			Assert::IsTrue(buffer1 == buffer2, L"Buffers not equal");

			vec[56] = vec[56] / 5.2; // Change one value
			Buffer buffer3(vec);
			Assert::IsFalse(buffer1 == buffer3, L"Buffers are equal");
		}

		TEST_METHOD(Iterators)
		{
			const size_t length = 512;
			const std::vector<Real> vec = CreateRandomVector(length);
			Buffer buffer(vec);
			size_t i = 0;
			for (const auto& sample : buffer)
			{
				Assert::AreEqual(vec[i], sample, L"Incorrect sample in iterator");
				i++;
			}
			i = 0;
			for (auto it = buffer.begin(); it != buffer.end(); ++it)
			{
				Assert::AreEqual(vec[i], *it, L"Incorrect sample in iterator");
				i++;
			}
		}

		TEST_METHOD(Multiply)
		{
			const size_t length = 512;
			const std::vector<Real> vec = CreateRandomVector(length);
			Buffer buffer(vec);
			const Real scalar = 2.0;
			buffer *= scalar;
			for (int i = 0; i < length; i++)
				Assert::AreEqual(vec[i] * scalar, buffer[i], L"Incorrect sample after multiplication");
		}

		TEST_METHOD(Addition)
		{
			const size_t length = 512;
			std::vector<Real> vec = CreateRandomVector(length);
			Buffer buffer(vec);
			const Real scalar = 2.0;
			buffer += scalar;
			for (int i = 0; i < length; i++)
				Assert::AreEqual(vec[i] + scalar, buffer[i], L"Incorrect sample after addition");
		}

		TEST_METHOD(Combine)
		{
			const size_t length = 512;
			const std::vector<Real> vec1 = CreateRandomVector(length);
			Buffer buffer1(vec1);

			const std::vector<Real> vec2 = CreateRandomVector(length);
			Buffer buffer2(vec2);
			buffer1 += buffer2;
			for (int i = 0; i < length; i++)
				Assert::AreEqual(vec1[i] + vec2[i], buffer1[i], L"Incorrect sample after combine");
		}
	};
}