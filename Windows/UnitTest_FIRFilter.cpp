
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/FIRFilter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

	TEST_CLASS(FIRFilter_Class)
	{
	public:

		TEST_METHOD(DecreaseSize)
		{
			const Real lerpFactor = 0.5;
			const Buffer<> longIR(std::vector<Real>({ REAL_CONST(1.0), REAL_CONST(0.5), REAL_CONST(-3.0), REAL_CONST(0.2), REAL_CONST(0.7), REAL_CONST(-0.13), REAL_CONST(0.2), REAL_CONST(2.1), REAL_CONST(-1.2), REAL_CONST(0.48), REAL_CONST(0.1), REAL_CONST(-0.35) }));

			FIRFilter filter = FIRFilter(longIR, 16);

			Assert::AreEqual((Real)1.0, filter.GetOutput(1.0, lerpFactor), L"Init wrong");

			const Buffer<> shortIR(std::vector<Real>({ REAL_CONST(-0.9), REAL_CONST(0.3), REAL_CONST(0.33), REAL_CONST(-0.1), REAL_CONST(-0.4), REAL_CONST(0.6) }));
			filter.SetTargetIR(shortIR);

			for (int i = 0; i < 1000; i++)
				filter.GetOutput(0.0, lerpFactor);

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0) };
			std::vector<Real> output = { REAL_CONST(-0.9), REAL_CONST(0.3), REAL_CONST(0.33) - REAL_CONST(1.8), REAL_CONST(-0.1) + REAL_CONST(0.6), REAL_CONST(-0.4) + REAL_CONST(0.66), REAL_CONST(0.6) - REAL_CONST(0.2), REAL_CONST(-0.8), REAL_CONST(1.2), REAL_CONST(0.0) };

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), L"Wrong output");
		}

		TEST_METHOD(IncreaseSize)
		{
			const Real lerpFactor = 0.5;
			const Buffer<> shortIR(std::vector<Real>({ REAL_CONST(0.9), REAL_CONST(0.5), REAL_CONST(0.0), REAL_CONST(0.2) }));

			FIRFilter filter(shortIR, 16);

			Assert::AreEqual((Real)1.8, filter.GetOutput(2.0, lerpFactor), L"Init wrong");

			const Buffer<> longIR(std::vector<Real>({ REAL_CONST(1.3), REAL_CONST(-0.5), REAL_CONST(0.15), REAL_CONST(0.78), REAL_CONST(-0.2), REAL_CONST(-1.0), REAL_CONST(0.1), REAL_CONST(0.9), REAL_CONST(1.3), REAL_CONST(2.3) }));
			filter.SetTargetIR(longIR);

			for (int i = 0; i < 1000; i++)
				filter.GetOutput(0.0, lerpFactor);

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0) };
			std::vector<Real> output = { REAL_CONST(1.3), REAL_CONST(-0.5), REAL_CONST(0.15) + REAL_CONST(2.6), REAL_CONST(0.78) - REAL_CONST(1.0), REAL_CONST(-0.2) + REAL_CONST(0.3), REAL_CONST(-1.0) + REAL_CONST(1.56), REAL_CONST(0.1) - REAL_CONST(0.4), REAL_CONST(0.9) - REAL_CONST(2.0), REAL_CONST(1.3) + REAL_CONST(0.2), REAL_CONST(2.3) + REAL_CONST(1.8), REAL_CONST(2.6), REAL_CONST(4.6), REAL_CONST(0.0) };

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), L"Wrong output");
		}

		TEST_METHOD(ProcessAudio)
		{
			const Real lerpFactor = REAL_CONST(0.5);
			const Buffer<> ir(std::vector<Real>({ REAL_CONST(1.0), REAL_CONST(-0.5), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.3), REAL_CONST(0.0), REAL_CONST(0.7), REAL_CONST(0.1) }));

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.5), REAL_CONST(0.0), REAL_CONST(0.3), REAL_CONST(0.4), REAL_CONST(0.2) };
			std::vector<Real> output = { REAL_CONST(1.0), REAL_CONST(-0.5), REAL_CONST(0.2), REAL_CONST(0.6), REAL_CONST(0.05), REAL_CONST(0.34), REAL_CONST(1.11), REAL_CONST(0.25) };

			FIRFilter filter = FIRFilter(ir, 8);

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ClearInputLine)
		{
			const Real lerpFactor = REAL_CONST(0.5);
			const Buffer<> ir(std::vector<Real>({ REAL_CONST(1.0), REAL_CONST(-0.5), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.3), REAL_CONST(0.0), REAL_CONST(0.7), REAL_CONST(0.1) }));

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.5), REAL_CONST(0.0), REAL_CONST(0.3), REAL_CONST(0.4), REAL_CONST(0.2) };
			std::vector<Real> output = { REAL_CONST(1.0), REAL_CONST(-0.5), REAL_CONST(0.2), REAL_CONST(0.6), REAL_CONST(0.05), REAL_CONST(0.34), REAL_CONST(1.11), REAL_CONST(0.25) };

			FIRFilter filter(ir, 8);

			for (int i = 0; i < 11; i++)
				filter.GetOutput(RandomValue(), lerpFactor);
			filter.ClearBuffers();

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(IsInterpolating)
		{
			const Real lerpFactor = REAL_CONST(0.5);
			const Buffer<> ir(std::vector<Real>({ REAL_CONST(1.0), REAL_CONST(-0.5), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.3), REAL_CONST(0.0), REAL_CONST(0.7), REAL_CONST(0.1) }));

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.5), REAL_CONST(0.0), REAL_CONST(0.3), REAL_CONST(0.4), REAL_CONST(0.2) };
			std::vector<Real> output = { REAL_CONST(1.0), REAL_CONST(-0.5), REAL_CONST(0.2), REAL_CONST(0.6), REAL_CONST(0.05), REAL_CONST(0.34), REAL_CONST(1.11), REAL_CONST(0.25) };

			FIRFilter filter(ir, 8);

			const Buffer<> irNew(std::vector<Real>({ REAL_CONST(-1.0), REAL_CONST(0.5), REAL_CONST(0.0), REAL_CONST(-0.2), REAL_CONST(-0.3), REAL_CONST(0.0), REAL_CONST(-0.7), REAL_CONST(-0.1) }));
			filter.SetTargetIR(irNew);

			for (int i = 0; i < input.size(); i++)
				Assert::AreNotEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(IrTooLong)
		{
			const Real lerpFactor = REAL_CONST(0.5);
			const Buffer<> ir(std::vector<Real>({ REAL_CONST(1.0), REAL_CONST(0.5), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.3), REAL_CONST(0.0), REAL_CONST(0.7), REAL_CONST(0.1), REAL_CONST(4.0), REAL_CONST(3.2), REAL_CONST(5.1) }));

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.5), REAL_CONST(0.0), REAL_CONST(0.3), REAL_CONST(0.4), REAL_CONST(0.2) };

			FIRFilter filter(ir, 8);

			Assert::IsFalse(filter.IsValid(), L"Filter should be invalid due to to IR length");

			// target needs to be set in construction
			filter.SetTargetIR(ir);
			Assert::IsFalse(filter.IsValid(), L"Filter should still be invalid due to IR length");
		}
	};
}