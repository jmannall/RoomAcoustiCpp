
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"
#include "DSP/DelayLine.h"
#include "Common/Types.h"
#include "Common/Complex.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

template <>
inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<RAC::Common::Complex>(const RAC::Common::Complex& value)
{
	if (value.imag() == 0.0)
		return std::format(L"%f", value.real());
	return std::format(L"%f+%fi", value.real(), value.imag());
}


namespace RAC
{
#pragma optimize("", off)
	using namespace DSP;

	TEST_CLASS(DelayLine_Class)
	{
	public:
		TEST_METHOD(DelayLineInitialise)
		{
			DelayLine<Complex> uninitialised(0);
			Assert::IsFalse(uninitialised.IsValid());

			uninitialised = DelayLine<Complex>(17);
			Assert::IsTrue(uninitialised.IsValid());
		}

		TEST_METHOD(DelayLineBasic)
		{
			constexpr int LineSize = 10;
			DelayLine<Complex> basicLine(LineSize);

			for (int index = 0; index < LineSize; ++index)
			{
				Complex notUsed;
				const Complex newValue(index, index + 100.0);
				basicLine.GetOutput(newValue, notUsed);
			}

			for (int index = 0; index < LineSize; ++index)
			{
				const int expectedIndex = index;
				const int newIndex = index + LineSize;

				Complex oldValue;
				const Complex newValue(newIndex, newIndex + 100.0);
				basicLine.GetOutput(newValue, oldValue);
				Assert::AreEqual(Complex(expectedIndex, expectedIndex + 100.0), oldValue);
			}

			for (int index = 0; index < LineSize; ++index)
			{
				const int expectedIndex = index + LineSize;
				const int newIndex = index + 2*LineSize;

				Complex oldValue;
				const Complex newValue(newIndex, newIndex + 100.0);
				basicLine.GetOutput(newValue, oldValue);
				Assert::AreEqual(Complex(expectedIndex, expectedIndex + 100.0), oldValue);
			}
		}

		TEST_METHOD(DelayLineAlignment)
		{
			constexpr int LineSize = 2;
			DelayLine<Complex> basicLine(LineSize);

			for (int index = 0; index < LineSize; ++index)
			{
				const Complex newValue(index, index + 100.0);
				Complex notUsed;
				basicLine.GetOutput(newValue, notUsed);
			}

			// on same cases, the Complex class must be aligned. This makes sure that RAC_ALIGN_COMPLEX is working but adding random other
			// variables on the stack
			char dummy1 = '\a';
			Complex item0;
			Complex newItem = Complex(2.0, 102.0);
			basicLine.GetOutput(newItem, item0);
			Assert::AreEqual(Complex(0.0, 100.0), item0);

			int dummy2 = 3;
			Complex item1;
			newItem = Complex(3.0, 103.0);
			basicLine.GetOutput(newItem, item1);
			Assert::AreEqual(Complex(1.0, 101.0), item1);

			char dummy3[17] = "Hi";
			Complex item2;
			newItem = Complex(4.0, 104.0);
			basicLine.GetOutput(newItem, item2);
			Assert::AreEqual(Complex(2.0, 102.0), item2);

			// use them to make sure that they don't get optimised away
			Assert::AreEqual('\a', dummy1);
			Assert::AreEqual(3, dummy2);
			Assert::AreEqual('H', dummy3[0]);
		}

	};
#pragma optimize("", on)
}