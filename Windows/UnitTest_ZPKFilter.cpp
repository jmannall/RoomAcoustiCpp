
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/IIRFilter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

	std::vector<Real> ProcessZPKFilter(const Real z1, const Real z2, const Real p1, const Real p2, const Real k, const std::vector<Real>& input)
	{
		Real b0 = k;
		Real b1 = -k * (z1 + z2);
		Real b2 = k * z1 * z2;

		Real a1 = -(p1 + p2);
		Real a2 = p1 * p2;

		std::vector<Real> output(input.size(), 0.0);

		output[0] = b0 * input[0];
		output[1] = b0 * input[1] + b1 * input[0] - a1 * output[0];
		for (int i = 2; i < input.size(); i++)
			output[i] = b0 * input[i] + b1 * input[i - 1] + b2 * input[i - 2] - a1 * output[i - 1] - a2 * output[i - 2];

		return output;
	}

	TEST_CLASS(ZPKFilter_Class)
	{
	public:

		TEST_METHOD(Default)
		{
			const int fs = 48e3;
			const Real lerpFactor = 0.5;

			const Real z1 = 0.25;
			const Real z2 = -0.99;
			const Real p1 = 0.99;
			const Real p2 = -0.25;
			const Real k = 0.0;

			ZPKFilter filter(fs);

			std::vector<Real> input = { 1.0, 0.0, 0.2, 0.0, -0.3, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, -0.2 };
			std::vector<Real> output = ProcessZPKFilter(z1, z2, p1, p2, k, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ProcessAudio)
		{
			const int fs = 48e3;
			const Real lerpFactor = 0.5;

			const Real z1 = -0.1;
			const Real z2 = 0.2;
			const Real p1 = -0.99;
			const Real p2 = 0.43;
			const Real k = 0.5;
			Coefficients zpk = Coefficients<std::array<Real, 5>>({ z1, z2, p1, p2, k });

			ZPKFilter filter(zpk, fs);

			std::vector<Real> input = { 0.9, -0.1, 0.2, 0.0, -0.3, 0.0, 3.0, 2.1, -0.22, 2.0, 0.0, -0.2 };
			std::vector<Real> output = ProcessZPKFilter(z1, z2, p1, p2, k, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(IsInterpolating)
		{
			const int fs = 48e3;
			const Real lerpFactor = 0.5;
			
			const Real z1 = -0.1;
			const Real z2 = 0.2;
			const Real p1 = -0.99;
			const Real p2 = 0.43;
			const Real k = 0.5;
			const Coefficients zpk = Coefficients<std::array<Real, 5>>({ z1, z2, p1, p2, k });

			ZPKFilter filter(zpk, fs);

			const Real newZ1 = -0.3;
			const Real newZ2 = -0.7;
			const Real newP1 = -0.1;
			const Real newP2 = 0.2;
			const Real newK = 0.9;
			const Coefficients newZPK = Coefficients<std::array<Real, 5>>({ newZ1, newZ2, newP1, newP2, newK });

			filter.SetTargetParameters(newZPK);

			std::vector<Real> input = { 1.0, 1.2, 0.2, 0.1, -0.3, -0.2, 0.0, 0.0, 0.0, 2.0, 0.0, -0.2 };
			std::vector<Real> output = ProcessZPKFilter(z1, z2, p1, p2, k, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreNotEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ClearBuffers)
		{
			const int fs = 48e3;
			const Real lerpFactor = 0.5;
			
			const Real z1 = 0.8;
			const Real z2 = -0.2;
			const Real p1 = 0.79;
			const Real p2 = -0.22;
			const Real k = 0.2;

			std::vector<Real> input = { 1.0, 0.0, 0.2, 0.5, 0.0, 0.3, 0.4, 0.2 };
			std::vector<Real> output = ProcessZPKFilter(z1, z2, p1, p2, k, input);

			Coefficients zpk = Coefficients<std::array<Real, 5>>({ z1, z2, p1, p2, k });
			ZPKFilter filter(zpk, fs);

			for (int i = 0; i < 11; i++)
				filter.GetOutput(RandomValue(), lerpFactor);
			filter.ClearBuffers();

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}
	};
}