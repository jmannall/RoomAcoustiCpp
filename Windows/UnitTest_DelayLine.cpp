
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
				RAC_ALIGN_COMPLEX const Complex newValue(index, index + 100.0);
				RAC_ALIGN_COMPLEX Complex notUsed;
				basicLine.GetOutput(newValue, notUsed);
			}

			for (int index = 0; index < LineSize; ++index)
			{
				const int expectedIndex = index;
				const int newIndex = index + LineSize;

				RAC_ALIGN_COMPLEX const Complex newValue(newIndex, newIndex + 100.0);
				RAC_ALIGN_COMPLEX Complex oldValue;
				basicLine.GetOutput(newValue, oldValue);
				Assert::AreEqual(Complex(expectedIndex, expectedIndex + 100.0), oldValue);
			}

			for (int index = 0; index < LineSize; ++index)
			{
				const int expectedIndex = index + LineSize;
				const int newIndex = index + 2*LineSize;

				RAC_ALIGN_COMPLEX const Complex newValue(newIndex, newIndex + 100.0);
				RAC_ALIGN_COMPLEX Complex oldValue;
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
				RAC_ALIGN_COMPLEX const Complex newValue(index, index + 100.0);
				RAC_ALIGN_COMPLEX Complex notUsed;
				basicLine.GetOutput(newValue, notUsed);
			}

			// on same cases, the Complex class must be aligned. This makes sure that RAC_ALIGN_COMPLEX is working but adding random other
			// variables on the stack
			char dummy1 = '\a';
			RAC_ALIGN_COMPLEX Complex item0;
			RAC_ALIGN_COMPLEX Complex newValue(2.0, 102.0);
			basicLine.GetOutput(newValue, item0);
			Assert::AreEqual(Complex(0.0, 100.0), item0);

			int dummy2 = 3;
			RAC_ALIGN_COMPLEX Complex item1;
			newValue = Complex(3.0, 103.0);
			basicLine.GetOutput(newValue, item1);
			Assert::AreEqual(Complex(1.0, 101.0), item1);

			char dummy3[17] = "Hi";
			RAC_ALIGN_COMPLEX Complex item2;
			newValue = Complex(340, 104.0);
			basicLine.GetOutput(newValue, item2);
			Assert::AreEqual(Complex(2.0, 102.0), item2);

			// use them to make sure that they don't get optimised away
			Assert::AreEqual('\a', dummy1);
			Assert::AreEqual(3, dummy2);
			Assert::AreEqual('H', dummy3[0]);
		}

	};
#pragma optimize("", on)
}