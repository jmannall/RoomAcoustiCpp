
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Common/Coefficients.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//////////////////// Assert templates ////////////////////

template<> static std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<RAC::Common::Coefficients<>>(const RAC::Common::Coefficients<>& t)
{
	std::string str = "Coefficients: ";
	for (int i = 0; i < t.Length(); i++)
		str += ToStr(t[i]) + ", ";
	std::wstring werror = std::wstring(str.begin(), str.end());
	return werror;
}

namespace RAC
{
	using namespace Common;

#pragma optimize("", off)

	TEST_CLASS(Coefficients_Class)
	{
	public:

		TEST_METHOD(InitArray)
		{
			const int n = 3;
			std::array<Real, n> arr = { REAL_CONST(2.0), REAL_CONST(3.0), REAL_CONST(5.0) };
			Coefficients<Real, n> c(arr);

			int x = 1;
			for (int i = 0; i < arr.size(); i++)
				Assert::AreEqual(arr[i], c[i], L"Error: Coefficients not initialised to vector");
		}

		TEST_METHOD(InitVector)
		{
			std::vector<Real> vec = { 2.0, 3.0, 5.0 };
			Coefficients<> c(vec);

			for (int i = 0; i < vec.size(); i++)
				Assert::AreEqual(vec[i], c[i], L"Error: Coefficients not initialised to vector");
		}

		TEST_METHOD(InitValue)
		{
			const int n = 3;
			Real value = REAL_CONST(3.0);
			Coefficients<Real, n> c(value);

			for (int i = 0; i < n; i++)
				Assert::AreEqual(value, c[i], L"Error: Coefficients not initialised to value");
		}

		TEST_METHOD(InitLength)
		{
			int length = 3;
			Coefficients<> c(length);

			for (int i = 0; i < length; i++)
				Assert::AreEqual(0.0, c[i], L"Error: Coefficients not initialised to zero");
		}
		
		void TestRealInit(Coefficients<Real, 3> input)
		{
			Real x = REAL_CONST(1.0);
		}

		TEST_METHOD(InitReal)
		{
			Real input = REAL_CONST(2.0);
			TestRealInit(input);
		}

		TEST_METHOD(Addition)
		{
			Coefficients<> c1 = Coefficients<>(std::vector<Real>({ REAL_CONST(2.0), REAL_CONST(3.0) }));
			Coefficients<> c2 = Coefficients<>(std::vector<Real>({ REAL_CONST(5.0), REAL_CONST(2.0) }));
			Coefficients<> out = Coefficients<>(std::vector<Real>({ REAL_CONST(7.0), REAL_CONST(5.0) }));

			Coefficients<> result = c1 + c2;

			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect addition");

			out = Coefficients<>(std::vector<Real>({ REAL_CONST(4.0), REAL_CONST(5.0) }));

			result = c1 + REAL_CONST(2.0);
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect factor addition");
		}

		TEST_METHOD(Subtraction)
		{
			Coefficients<> c1 = Coefficients<>(std::vector<Real>({ REAL_CONST(2.0), REAL_CONST(3.0) }));
			Coefficients<> c2 = Coefficients<>(std::vector<Real>({ REAL_CONST(5.0), REAL_CONST(2.0) }));
			Coefficients<> out = Coefficients<>(std::vector<Real>({ REAL_CONST(-3.0), REAL_CONST(1.0) }));

			Coefficients<> result = c1 - c2;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect subtraction");

			out = Coefficients<>(std::vector<Real>({ REAL_CONST(0.0), REAL_CONST(-1.0) }));
			
			result = REAL_CONST(2.0) - c1;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect factor subtraction");
		}

		TEST_METHOD(Multiplication)
		{
			Coefficients<> c1 = Coefficients<>(std::vector<Real>({ REAL_CONST(2.0), REAL_CONST(3.0) }));
			Coefficients<> c2 = Coefficients<>(std::vector<Real>({ REAL_CONST(5.0), REAL_CONST(2.0) }));
			Coefficients<> out = Coefficients<>(std::vector<Real>({ REAL_CONST(10.0), REAL_CONST(6.0) }));

			Coefficients<> result = c1 * c2;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect multiplication");

			out = Coefficients<>(std::vector<Real>({ REAL_CONST(4.0), REAL_CONST(6.0) }));

			result = c1 * REAL_CONST(2.0);
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect factor multiplication");
		}

		TEST_METHOD(Division)
		{
			Coefficients<> c1 = Coefficients<>(std::vector<Real>({ REAL_CONST(2.0), REAL_CONST(3.0) }));
			Coefficients<> c2 = Coefficients<>(std::vector<Real>({ REAL_CONST(5.0), REAL_CONST(2.0) }));
			Coefficients<> out = Coefficients<>(std::vector<Real>({ REAL_CONST(0.4), REAL_CONST(1.5) }));

			Coefficients<> result = c1 / c2;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect division");

			out = Coefficients<>(std::vector<Real>({ REAL_CONST(1.0), REAL_CONST(1.5) }));
			result = c1 / REAL_CONST(2.0);
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect factor division");
		}
	};

	TEST_CLASS(Absorption_Class)
	{
	public:

		TEST_METHOD(Addition)
		{
			std::vector<Real> a = { REAL_CONST(0.5), REAL_CONST(0.7) };
			std::vector<Real> b = { REAL_CONST(0.5), REAL_CONST(0.8) };

			Coefficients<> c1 = CalculateReflectance(a);
			Coefficients<> c2 = CalculateReflectance(b);

			Coefficients<> out = Coefficients<>(c1.Length());
			out[0] = sqrt(REAL_CONST(1.0) - a[0]) + sqrt(REAL_CONST(1.0) - b[0]);
			out[1] = sqrt(REAL_CONST(1.0) - a[1]) + sqrt(REAL_CONST(1.0) - b[1]);

			Coefficients<> result = c1 + c2;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect addition");

			Real x = REAL_CONST(2.0);
			out[0] = sqrt(REAL_CONST(1.0) - a[0]) + x;
			out[1] = sqrt(REAL_CONST(1.0) - a[1]) + x;

			result = c1 + x;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect factor addition");
		}

		TEST_METHOD(Subtraction)
		{
			std::vector<Real> a = { REAL_CONST(0.5), REAL_CONST(0.7) };
			std::vector<Real> b = { REAL_CONST(0.5), REAL_CONST(0.8) };

			Coefficients<> c1 = CalculateReflectance(a);
			Coefficients<> c2 = CalculateReflectance(b);

			Coefficients<> out = Coefficients<>(c1.Length());
			out[0] = sqrt(REAL_CONST(1.0) - a[0]) - sqrt(REAL_CONST(1.0) - b[0]);
			out[1] = sqrt(REAL_CONST(1.0) - a[1]) - sqrt(REAL_CONST(1.0) - b[1]);

			Coefficients<> result = c1 - c2;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect subtraction");

			Real x = REAL_CONST(2.0);
			out[0] = x - sqrt(REAL_CONST(1.0) - a[0]);
			out[1] = x - sqrt(REAL_CONST(1.0) - a[1]);

			result = x - c1;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect factor subtraction");
		}

		TEST_METHOD(Multiplication)
		{
			std::vector<Real> a = { REAL_CONST(0.5), REAL_CONST(0.7) };
			std::vector<Real> b = { REAL_CONST(0.5), REAL_CONST(0.8) };

			Coefficients<> c1 = CalculateReflectance(a);
			Coefficients<> c2 = CalculateReflectance(b);

			Coefficients<> out = Coefficients<>(c1.Length());
			out[0] = sqrt(REAL_CONST(1.0) - a[0]) * sqrt(REAL_CONST(1.0) - b[0]);
			out[1] = sqrt(REAL_CONST(1.0) - a[1]) * sqrt(REAL_CONST(1.0) - b[1]);

			Coefficients<> result = c1 * c2;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect multiplication");

			Real x = REAL_CONST(2.0);
			out[0] = sqrt(REAL_CONST(1.0) - a[0]) * x;
			out[1] = sqrt(REAL_CONST(1.0) - a[1]) * x;

			result = c1 * x;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect factor multiplication");
		}

		TEST_METHOD(Division)
		{
			std::vector<Real> a = { REAL_CONST(0.5), REAL_CONST(0.7) };
			std::vector<Real> b = { REAL_CONST(0.5), REAL_CONST(0.8) };

			Coefficients<> c1 = CalculateReflectance(a);
			Coefficients<> c2 = CalculateReflectance(b);

			Coefficients<> out = Coefficients<>(c1.Length());
			out[0] = sqrt(REAL_CONST(1.0) - a[0]) / sqrt(REAL_CONST(1.0) - b[0]);
			out[1] = sqrt(REAL_CONST(1.0) - a[1]) / sqrt(REAL_CONST(1.0) - b[1]);

			Coefficients<> result = c1 / c2;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect division");

			Real x = REAL_CONST(2.0);
			out[0] = sqrt(REAL_CONST(1.0) - a[0]) / x;
			out[1] = sqrt(REAL_CONST(1.0) - a[1]) / x;

			result = c1 / x;
			Assert::IsTrue(out.IsApprox(result), L"Error: Incorrect factor division");
		}
	};
#pragma optimize("", on)
}