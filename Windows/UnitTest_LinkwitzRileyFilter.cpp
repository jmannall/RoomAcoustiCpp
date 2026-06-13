
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/LinkwitzRileyFilter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

#pragma optimize("", off)

	TEST_CLASS(LinkwitzRileyFilter_Class)
	{
	public:

		TEST_METHOD(Zero)
		{
			const Coefficients<Real, 4> gain(std::array<Real, 4>({ REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0) }));
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);

			LinkwitzRiley filter = LinkwitzRiley(gain, fs);

			Real out = filter.GetOutput(REAL_CONST(1.0), lerpFactor);
			Assert::IsTrue(out == REAL_CONST(0.0), L"Not zero");
		}

		// Need example LR filter responses.
		//TEST_METHOD(IsInterpolating)
		//{
		//	const std::array<Real, 4> gain({ REAL_CONST(0.4), REAL_CONST(0.4), REAL_CONST(0.4), REAL_CONST(0.4) });
		//	const int fs = 48000;
		//	const Real lerpFactor = REAL_CONST(0.5);

		//	LinkwitzRiley filter = LinkwitzRiley(gain, fs);

		//	//filter.SetTargetGains(std::array<Real, 4>({ REAL_CONST(0.7), REAL_CONST(0.7), REAL_CONST(0.7), REAL_CONST(0.7) }));

		//	Real out = filter.GetOutput(REAL_CONST(1.0), lerpFactor);
		//	Assert::AreNotEqual(REAL_CONST(0.4), out, REAL_CONST(1e-16), L"Wrong output");
		//}

		TEST_METHOD(ClearBuffers)
		{
			const Coefficients<Real, 4> gain(std::array<Real, 4>({ REAL_CONST(0.7), REAL_CONST(0.8), REAL_CONST(0.5), REAL_CONST(0.65) }));
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);

			LinkwitzRiley filter = LinkwitzRiley(gain, fs);

			for (int i = 0; i < 20; i++)
			{
				Real out = filter.GetOutput(RandomValue(), lerpFactor);
				Assert::AreNotEqual(REAL_CONST(0.0), out, L"Output is zero");
			}

			filter.ClearBuffers();

			Real out = filter.GetOutput(0.0, lerpFactor);
			Assert::AreEqual(REAL_CONST(0.0), out, L"Output not zero");
		}
	};
#pragma optimize("", on)
}