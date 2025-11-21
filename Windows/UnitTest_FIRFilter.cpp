
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
			const Buffer<> longIR(std::vector<Real>({ 1.0, 0.5, -3.0, 0.2, 0.7, -0.13, 0.2, 2.1, -1.2, 0.48, 0.1, -0.35 }));

			FIRFilter filter = FIRFilter(longIR, 16);

			Assert::AreEqual((Real)1.0, filter.GetOutput(1.0, lerpFactor), L"Init wrong");

			const Buffer<> shortIR(std::vector<Real>({ -0.9, 0.3, 0.33, -0.1, -0.4, 0.6 }));
			filter.SetTargetIR(shortIR);

			for (int i = 0; i < 1000; i++)
				filter.GetOutput(0.0, lerpFactor);

			std::vector<Real> input = { 1.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			std::vector<Real> output = { -0.9, 0.3, 0.33 - 1.8, -0.1 + 0.6, -0.4 + 0.66, 0.6 - 0.2, -0.8, 1.2, 0.0 };

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), L"Wrong output");
		}

		TEST_METHOD(IncreaseSize)
		{
			const Real lerpFactor = 0.5;
			const Buffer<> shortIR(std::vector<Real>({ 0.9, 0.5, 0.0, 0.2 }));

			FIRFilter filter(shortIR, 16);

			Assert::AreEqual((Real)1.8, filter.GetOutput(2.0, lerpFactor), L"Init wrong");

			const Buffer<> longIR(std::vector<Real>({ 1.3, -0.5, 0.15, 0.78, -0.2, -1.0, 0.1, 0.9, 1.3, 2.3 }));
			filter.SetTargetIR(longIR);

			for (int i = 0; i < 1000; i++)
				filter.GetOutput(0.0, lerpFactor);

			std::vector<Real> input = { 1.0, 0.0, 2.0, 0.0, 0.0, 0.0 };
			std::vector<Real> output = { 1.3, -0.5, 0.15 + 2.6, 0.78 - 1.0, -0.2 + 0.3, -1.0 + 1.56, 0.1 - 0.4, 0.9 - 2.0, 1.3 + 0.2, 2.3 + 1.8, 2.6, 4.6, 0.0 };

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), L"Wrong output");
		}

		TEST_METHOD(ProcessAudio)
		{
			const Real lerpFactor = 0.5;
			const Buffer<> ir(std::vector<Real>({ 1.0, -0.5, 0.0, 0.2, 0.3, 0.0, 0.7, 0.1 }));

			std::vector<Real> input = { 1.0, 0.0, 0.2, 0.5, 0.0, 0.3, 0.4, 0.2 };
			std::vector<Real> output = { 1.0, -0.5, 0.2, 0.6, 0.05, 0.34, 1.11, 0.25 };

			FIRFilter filter = FIRFilter(ir, 8);

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ClearInputLine)
		{
			const Real lerpFactor = 0.5;
			const Buffer<> ir(std::vector<Real>({ 1.0, -0.5, 0.0, 0.2, 0.3, 0.0, 0.7, 0.1 }));

			std::vector<Real> input = { 1.0, 0.0, 0.2, 0.5, 0.0, 0.3, 0.4, 0.2 };
			std::vector<Real> output = { 1.0, -0.5, 0.2, 0.6, 0.05, 0.34, 1.11, 0.25 };

			FIRFilter filter(ir, 8);

			for (int i = 0; i < 11; i++)
				filter.GetOutput(RandomValue(), lerpFactor);
			filter.ClearBuffers();

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(IsInterpolating)
		{
			const Real lerpFactor = 0.5;
			const Buffer<> ir(std::vector<Real>({ 1.0, -0.5, 0.0, 0.2, 0.3, 0.0, 0.7, 0.1 }));

			std::vector<Real> input = { 1.0, 0.0, 0.2, 0.5, 0.0, 0.3, 0.4, 0.2 };
			std::vector<Real> output = { 1.0, -0.5, 0.2, 0.6, 0.05, 0.34, 1.11, 0.25 };

			FIRFilter filter(ir, 8);

			const Buffer<> irNew(std::vector<Real>({ -1.0, 0.5, 0.0, -0.2, -0.3, 0.0, -0.7, -0.1 }));
			filter.SetTargetIR(irNew);

			for (int i = 0; i < input.size(); i++)
				Assert::AreNotEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(IrTooLong)
		{
			const Real lerpFactor = 0.5;
			const Buffer<> ir(std::vector<Real>({ 1.0, 0.5, 0.0, 0.2, 0.3, 0.0, 0.7, 0.1, 4.0, 3.2, 5.1 }));

			std::vector<Real> input = { 1.0, 0.0, 0.2, 0.5, 0.0, 0.3, 0.4, 0.2 };

			FIRFilter filter(ir, 8);

			Assert::IsFalse(filter.IsValid(), L"Filter should be invalid due to to IR length");

			// target needs to be set in construction
			filter.SetTargetIR(ir);
			Assert::IsFalse(filter.IsValid(), L"Filter should still be invalid due to IR length");
		}
	};
}