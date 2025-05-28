
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
			const std::array<Real, 4> gain({ 0.0, 0.0, 0.0, 0.0 });
			const int fs = 48e3;
			const Real lerpFactor = 0.5;

			LinkwitzRiley filter = LinkwitzRiley(gain, fs);

			Real out = filter.GetOutput(1.0, lerpFactor);
			Assert::IsTrue(out == 0.0, L"Not zero");
		}

		// Need example LR filter responses.
		//TEST_METHOD(IsInterpolating)
		//{
		//	const std::array<Real, 4> gain({ 0.4, 0.4, 0.4, 0.4 });
		//	const int fs = 48e3;
		//	const Real lerpFactor = 0.5;

		//	LinkwitzRiley filter = LinkwitzRiley(gain, fs);

		//	//filter.SetTargetGains(std::array<Real, 4>({ 0.7, 0.7, 0.7, 0.7 }));

		//	Real out = filter.GetOutput(1.0, lerpFactor);
		//	Assert::AreNotEqual(0.4, out, 10e-16, L"Wrong output");
		//}

		TEST_METHOD(ClearBuffers)
		{
			const std::array<Real, 4> gain({ 0.7, 0.8, 0.5, 0.65 });
			const int fs = 48e3;
			const Real lerpFactor = 0.5;

			LinkwitzRiley filter = LinkwitzRiley(gain, fs);

			for (int i = 0; i < 20; i++)
				filter.GetOutput(rand(), lerpFactor);

			filter.ClearBuffers();

			Real out = filter.GetOutput(0.0, lerpFactor);
			Assert::AreEqual(0.0, out, L"Wrong output");
		}
	};
#pragma optimize("", on)
}