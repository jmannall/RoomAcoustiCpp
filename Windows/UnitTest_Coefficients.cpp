
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Common/Coefficients.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//////////////////// Assert templates ////////////////////

template<> static std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<RAC::Common::Coefficients<std::vector<double>>>(const RAC::Common::Coefficients<std::vector<double>>& t)
{
	std::string str = "Coefficients: ";
	for (int i = 0; i < t.Length(); i++)
		str += ToStr(t[i]) + ", ";
	std::wstring werror = std::wstring(str.begin(), str.end());
	return werror;
}

template<> static std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<RAC::Common::Absorption<std::vector<double>>>(const RAC::Common::Absorption<std::vector<double>>& t)
{
	std::string str = "Absorption: ";
	for (int i = 0; i < t.Length(); i++)
		str += ToStr(t[i]) + ", ";
	str += "Area: " + ToStr(t.mArea);
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

		TEST_METHOD(Addition)
		{
			Coefficients c1 = Coefficients(std::vector<Real>({ 2.0, 3.0 }));
			Coefficients c2 = Coefficients(std::vector<Real>({ 5.0, 2.0 }));
			Coefficients out = Coefficients(std::vector<Real>({7.0, 5.0}));
			Assert::AreEqual(out, c1 + c2, L"Error: Incorrect addition");

			out = Coefficients(std::vector<Real>({ 4.0, 5.0 }));
			Assert::AreEqual(out, c1 + 2.0, L"Error: Incorrect factor addition");
		}

		TEST_METHOD(Subtraction)
		{
			Coefficients c1 = Coefficients(std::vector<Real>({ 2.0, 3.0 }));
			Coefficients c2 = Coefficients(std::vector<Real>({ 5.0, 2.0 }));
			Coefficients out = Coefficients(std::vector<Real>({ -3.0, 1.0 }));

			Assert::AreEqual(out, c1 - c2, L"Error: Incorrect subtraction");

			out = Coefficients(std::vector<Real>({ 0.0, -1.0 }));
			Assert::AreEqual(out, 2.0 - c1, L"Error: Incorrect factor subtraction");
		}

		TEST_METHOD(Multiplication)
		{
			Coefficients c1 = Coefficients(std::vector<Real>({ 2.0, 3.0 }));
			Coefficients c2 = Coefficients(std::vector<Real>({ 5.0, 2.0 }));
			Coefficients out = Coefficients(std::vector<Real>({ 10.0, 6.0 }));

			Assert::AreEqual(out, c1 * c2, L"Error: Incorrect multiplication");

			out = Coefficients(std::vector<Real>({ 4.0, 6.0 }));
			Assert::AreEqual(out, c1 * 2.0, L"Error: Incorrect factor multiplication");
		}

		TEST_METHOD(Division)
		{
			Coefficients c1 = Coefficients(std::vector<Real>({ 2.0, 3.0 }));
			Coefficients c2 = Coefficients(std::vector<Real>({ 5.0, 2.0 }));
			Coefficients out = Coefficients(std::vector<Real>({ 0.4, 1.5 }));

			Assert::AreEqual(out, c1 / c2, L"Error: Incorrect division");

			out = Coefficients(std::vector<Real>({ 1.0, 1.5 }));
			Assert::AreEqual(out, c1 / 2.0, L"Error: Incorrect factor division");
		}
	};

	TEST_CLASS(Absorption_Class)
	{
	public:

		TEST_METHOD(Addition)
		{
			std::vector<Real> a = { 0.5, 0.7 };
			std::vector<Real> b = { 0.5, 0.8 };

			Absorption c1 = Absorption(a);
			Absorption c2 = Absorption(b);
			c1.mArea = 2.0;
			c2.mArea = 5.0;

			Absorption out = Absorption(c1.Length());
			out[0] = sqrt(1.0 - a[0]) + sqrt(1.0 - b[0]);
			out[1] = sqrt(1.0 - a[1]) + sqrt(1.0 - b[1]);
			out.mArea = 7.0;
			Assert::AreEqual(out, c1 + c2, L"Error: Incorrect addition");

			Real x = 2.0;
			out[0] = sqrt(1.0 - a[0]) + x;
			out[1] = sqrt(1.0 - a[1]) + x;
			out.mArea = 2.0;
			Assert::AreEqual(out, c1 + x, L"Error: Incorrect factor addition");
		}

		TEST_METHOD(Subtraction)
		{
			std::vector<Real> a = { 0.5, 0.7 };
			std::vector<Real> b = { 0.5, 0.8 };

			Absorption c1 = Absorption(a);
			Absorption c2 = Absorption(b);
			c1.mArea = 2.0;
			c2.mArea = 5.0;

			Absorption out = Absorption(c1.Length());
			out[0] = sqrt(1.0 - a[0]) - sqrt(1.0 - b[0]);
			out[1] = sqrt(1.0 - a[1]) - sqrt(1.0 - b[1]);
			out.mArea = -3.0;
			Assert::AreEqual(out, c1 - c2, L"Error: Incorrect subtraction");

			Real x = 2.0;
			out[0] = x - sqrt(1.0 - a[0]);
			out[1] = x - sqrt(1.0 - a[1]);
			out.mArea = 2.0;
			Assert::AreEqual(out, x - c1, L"Error: Incorrect factor subtraction");
		}

		TEST_METHOD(Multiplication)
		{
			std::vector<Real> a = { 0.5, 0.7 };
			std::vector<Real> b = { 0.5, 0.8 };

			Absorption c1 = Absorption(a);
			Absorption c2 = Absorption(b);
			c1.mArea = 2.0;
			c2.mArea = 5.0;

			Absorption out = Absorption(c1.Length());
			out[0] = sqrt(1.0 - a[0]) * sqrt(1.0 - b[0]);
			out[1] = sqrt(1.0 - a[1]) * sqrt(1.0 - b[1]);
			out.mArea = 2.0;
			Assert::AreEqual(out, c1 * c2, L"Error: Incorrect multiplication");

			Real x = 2.0;
			out[0] = sqrt(1.0 - a[0]) * x;
			out[1] = sqrt(1.0 - a[1]) * x;
			out.mArea = 2.0;
			Assert::AreEqual(out, c1 * x, L"Error: Incorrect factor multiplication");
		}

		TEST_METHOD(Division)
		{
			std::vector<Real> a = { 0.5, 0.7 };
			std::vector<Real> b = { 0.5, 0.8 };

			Absorption c1 = Absorption(a);
			Absorption c2 = Absorption(b);
			c1.mArea = 2.0;
			c2.mArea = 5.0;

			Absorption out = Absorption(c1.Length());
			out[0] = sqrt(1.0 - a[0]) / sqrt(1.0 - b[0]);
			out[1] = sqrt(1.0 - a[1]) / sqrt(1.0 - b[1]);
			out.mArea = 2.0;
			Assert::AreEqual(out, c1 / c2, L"Error: Incorrect division");

			Real x = 2.0;
			out[0] = sqrt(1.0 - a[0]) / x;
			out[1] = sqrt(1.0 - a[1]) / x;
			out.mArea = 2.0;
			Assert::AreEqual(out, c1 / x, L"Error: Incorrect factor division");
		}
	};
#pragma optimize("", on)
}