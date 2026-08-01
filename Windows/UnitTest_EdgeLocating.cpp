
#include "CppUnitTest.h"

#include "UtilityFunctions.h"

#include "Spatialiser/EdgeLocating.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

	TEST_CLASS(EdgeLocating)
	{
	public:
		TEST_METHOD(Generic)
		{
			
		}

		TEST_METHOD(Parallel)
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

	};
}