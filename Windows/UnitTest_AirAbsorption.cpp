
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Spatialiser/AirAbsorption.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

	std::vector<Real> ProcessAirAbsorption(const Real distance, const Real fs, const std::vector<Real>& input)
	{

		const Real c = REAL_CONST(331.5) + REAL_CONST(0.6) * REAL_CONST(20.0);

		Real alpha = REAL_CONST(7782.0);
		Real b0 = exp(((-distance * fs) / (c * alpha)));
		Real a1 = -(REAL_CONST(1.0) - b0);

		std::vector<Real> output(input.size(), REAL_CONST(0.0));

		output[0] = b0 * input[0];
		for (int i = 1; i < input.size(); i++)
			output[i] = b0 * input[i] - a1 * output[i - 1];

		return output;
	}

	TEST_CLASS(AirAbsorption_Class)
	{
	public:
		TEST_METHOD(Process)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);

			const Real distance = 50;
			AirAbsorption filter(distance, fs);

			std::vector<Real> input = { REAL_CONST(0.9), REAL_CONST(-0.1), REAL_CONST(0.2), REAL_CONST(0.0), REAL_CONST(-0.3), REAL_CONST(0.0), REAL_CONST(3.0), REAL_CONST(2.1), REAL_CONST(-0.22), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(-0.2) };
			std::vector<Real> output = ProcessAirAbsorption(distance, fs, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(IsInterpolating)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);

			const Real distance = 22;
			const Real newDistance = 31;

			AirAbsorption filter(distance, fs);
			filter.SetTargetDistance(newDistance);

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(1.2), REAL_CONST(0.2), REAL_CONST(0.1), REAL_CONST(-0.3), REAL_CONST(-0.2), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(-0.2) };
			std::vector<Real> output = ProcessAirAbsorption(distance, fs, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreNotEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ClearBuffers)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);

			const Real distance = 7;
			const Real newDistance = 34;

			AirAbsorption filter(distance, fs);

			const std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(-0.3), REAL_CONST(0.2), REAL_CONST(0.5), REAL_CONST(1.23), REAL_CONST(0.3), REAL_CONST(-0.4), REAL_CONST(0.2) };
			const std::vector<Real> output = ProcessAirAbsorption(distance, fs, input);

			for (int i = 0; i < 11; i++)
				filter.GetOutput(RandomValue(), lerpFactor);
			filter.ClearBuffers();

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

	};
}