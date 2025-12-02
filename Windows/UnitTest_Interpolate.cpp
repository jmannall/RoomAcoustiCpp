
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/Interpolate.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

#pragma optimize("", off)

	TEST_CLASS(Interpolate_Functions)
	{
	public:

		TEST_METHOD(Denormals)
		{
			Real current = MIN_VALUE;
			Real target = REAL_CONST(0.0);
			Real lerpFactor = REAL_CONST(0.5);

			FlushDenormals();
			for (int i = 0; i < 10; i++)
				current = Lerp(current, target, lerpFactor);
			NoFlushDenormals();
			Assert::AreEqual(REAL_CONST(0.0), current, L"DenormalsFlushed");

			current = MIN_VALUE;
			for (int i = 0; i < 10; i++)
				current = Lerp(current, target, lerpFactor);

			Assert::AreNotEqual(REAL_CONST(0.0), current, L"NoDenormalsFlushed");
		}

		TEST_METHOD(Real_Type)
		{
			Real current = REAL_CONST(1.0);
			Real target = REAL_CONST(0.0);
			Real lerpFactor = REAL_CONST(0.2);
			current = Lerp(current, target, lerpFactor);

			Assert::AreEqual(REAL_CONST(0.8), current, EPS, L"Wrong output");
			current = Lerp(current, target, lerpFactor);
			Assert::AreEqual(REAL_CONST(0.64), current, EPS, L"Wrong output");
		}

		TEST_METHOD(Buffer_Class)
		{
			std::vector<Real> start = { REAL_CONST(1.0), REAL_CONST(4.0), REAL_CONST(3.0), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0) };
			Buffer<> current(start);

			std::vector<Real> end = { REAL_CONST(0.0), REAL_CONST(2.0), REAL_CONST(4.0), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0) };
			Buffer<> target(end);
			Real lerpFactor = REAL_CONST(0.2);
			Lerp(current, target, 4, lerpFactor);

			{
				std::vector<Real> output = { REAL_CONST(0.8), REAL_CONST(3.6), REAL_CONST(3.2), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0) };
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
			{
				std::vector<Real> output = { REAL_CONST(0.64), REAL_CONST(3.28), REAL_CONST(3.36), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0) };
				Lerp(current, target, 4, lerpFactor);
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
		}

		TEST_METHOD(Coefficients_Class)
		{
			std::vector<Real> start = { REAL_CONST(1.0), REAL_CONST(4.0), REAL_CONST(3.0), REAL_CONST(2.0) };
			Coefficients<> current = Coefficients<>(start);

			std::vector<Real> end = { REAL_CONST(0.0), REAL_CONST(2.0), REAL_CONST(4.0), REAL_CONST(2.0) };
			Coefficients<> target = Coefficients<>(end);
			Real lerpFactor = REAL_CONST(0.2);
			Lerp(current, target, lerpFactor);

			{
				std::vector<Real> output = { REAL_CONST(0.8), REAL_CONST(3.6), REAL_CONST(3.2), REAL_CONST(2.0) };
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
			{
				std::vector<Real> output = { REAL_CONST(0.64), REAL_CONST(3.28), REAL_CONST(3.36), REAL_CONST(2.0) };
				Lerp(current, target, lerpFactor);
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
		}
	};
#pragma optimize("", on)
}