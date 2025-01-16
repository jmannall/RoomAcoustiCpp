
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
			Real target = 0.0;
			Real lerpFactor = 0.5;

			FlushDenormals();
			for (int i = 0; i < 10; i++)
				current = Lerp(current, target, lerpFactor);
			NoFlushDenormals();
			Assert::AreEqual(0.0, current, L"DenormalsFlushed");

			current = MIN_VALUE;
			for (int i = 0; i < 10; i++)
				current = Lerp(current, target, lerpFactor);

			Assert::AreNotEqual(0.0, current, L"NoDenormalsFlushed");
		}

		TEST_METHOD(Real_Type)
		{
			Real current = 1.0;
			Real target = 0.0;
			Real lerpFactor = 0.2;
			current = Lerp(current, target, lerpFactor);

			Assert::AreEqual(0.8, current, EPS, L"Wrong output");
			current = Lerp(current, target, lerpFactor);
			Assert::AreEqual(0.64, current, EPS, L"Wrong output");
		}

		TEST_METHOD(Buffer_Class)
		{
			std::vector<Real> start = { 1.0, 4.0, 3.0, 2.0 };
			Buffer current = Buffer(start);

			std::vector<Real> end = { 0.0, 2.0, 4.0, 2.0 };
			Buffer target = Buffer(end);
			Real lerpFactor = 0.2;
			Lerp(current, target, lerpFactor);

			{
				std::vector<Real> output = { 0.8, 3.6, 3.2, 2.0 };
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
			{
				std::vector<Real> output = { 0.64, 3.28, 3.36, 2.0 };
				Lerp(current, target, lerpFactor);
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
		}

		TEST_METHOD(Coefficients_Class)
		{
			std::vector<Real> start = { 1.0, 4.0, 3.0, 2.0 };
			Coefficients current = Coefficients(start);

			std::vector<Real> end = { 0.0, 2.0, 4.0, 2.0 };
			Coefficients target = Coefficients(end);
			Real lerpFactor = 0.2;
			Lerp(current, target, lerpFactor);

			{
				std::vector<Real> output = { 0.8, 3.6, 3.2, 2.0 };
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
			{
				std::vector<Real> output = { 0.64, 3.28, 3.36, 2.0 };
				Lerp(current, target, lerpFactor);
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
		}
	};
#pragma optimize("", on)
}