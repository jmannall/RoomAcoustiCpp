
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/FIRFilter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

#pragma optimize("", off)

	TEST_CLASS(FIRFilter_Class)
	{
	public:

		TEST_METHOD(DecreaseSize)
		{
			Buffer impulseResponse = Buffer({ 1.0, 0.5, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 });
			FIRFilter filter = FIRFilter(impulseResponse);

			filter.GetOutput(1.0);
			filter.GetOutput(2.0);
			Buffer impulseResponse2 = Buffer({ 1.0, 0.5, 0.0, 0.2 });
			filter.SetImpulseResponse(impulseResponse2);

			std::vector<Real> input = { 1.0, 0.0, 2.0, 0.0, 0.0, 0.0 };
			std::vector<Real> output = { 2.0, 0.7, 2.4, 1.2, 0.0, 0.4 };

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i]), L"Wrong output");
		}

		TEST_METHOD(IncreaseSize)
		{
			Buffer impulseResponse = Buffer({ 1.0, 0.5, 0.0, 0.2 });
			FIRFilter filter = FIRFilter(impulseResponse);

			filter.GetOutput(1.0);
			filter.GetOutput(2.0);
			Buffer impulseResponse2 = Buffer({ 1.0, 0.5, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 });
			filter.SetImpulseResponse(impulseResponse2);

			std::vector<Real> input = { 1.0, 0.0, 2.0, 0.0, 0.0, 0.0 };
			std::vector<Real> output = { 2.0, 0.7, 2.4, 1.2, 0.0, 0.4 };

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i]), L"Wrong output");
		}

		TEST_METHOD(ProcessAudio)
		{
			std::vector<Real> ir = { 1.0, 0.5, 0.0, 0.2, 0.3, 0.0, 0.7, 0.1 };
			std::vector<Real> in = { 1.0, 0.0, 0.2, 0.5, 0.0, 0.3, 0.4, 0.2 };

			std::vector<Real> out = { 1.0, 0.5, 0.2, 0.8, 0.55, 0.34, 1.41, 0.65 };
			Buffer impulseResponse = Buffer(ir);
			FIRFilter filter = FIRFilter(impulseResponse);

			for (int i = 0; i < out.size(); i++)
				Assert::AreEqual(out[i], filter.GetOutput(in[i]), EPS, L"Wrong output");
		}
	};
#pragma optimize("", on)
}