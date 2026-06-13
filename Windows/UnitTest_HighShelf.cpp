
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/IIRFilter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

	std::vector<Real> ProcessHighShelf(const int fs, const Real fc, const Real gain, const std::vector<Real>& input)
	{
		const Real pi = REAL_CONST(3.141592653589793238462643383279502884197);

		const Real omega = cot(pi * fc / static_cast<Real>(fs));
		const Real sqrtG = sqrt(gain);

		const Real norm = REAL_CONST(1.0) / (REAL_CONST(1.0) + omega / sqrtG);
		const Real a1 = (1 - omega / sqrtG) * norm;

		const Real b0 = (1 + omega * sqrtG) * norm;
		const Real b1 = (1 - omega * sqrtG) * norm;

		std::vector<Real> output(input.size(), REAL_CONST(0.0));

		output[0] = b0 * input[0];
		for (int i = 1; i < input.size(); i++)
			output[i] = b0 * input[i] + b1 * input[i - 1] - a1 * output[i - 1];

		return output;
	}

	TEST_CLASS(HighShelf_Class)
	{
	public:

		TEST_METHOD(Default)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);

			const Real fc = REAL_CONST(1000.0);
			const Real gain = REAL_CONST(1.0);

			HighShelf filter(fs);

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.0), REAL_CONST(-0.3), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(-0.2) };
			std::vector<Real> output = ProcessHighShelf(fs, fc, gain, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ProcessAudio)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);

			const Real fc = REAL_CONST(500.0);
			const Real gain = REAL_CONST(0.77);

			HighShelf filter(fc, gain, fs);

			std::vector<Real> input = { REAL_CONST(0.9), REAL_CONST(-0.1), REAL_CONST(0.2), REAL_CONST(0.0), REAL_CONST(-0.3), REAL_CONST(0.0), REAL_CONST(3.0), REAL_CONST(2.1), REAL_CONST(-0.22), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(-0.2) };
			std::vector<Real> output = ProcessHighShelf(fs, fc, gain, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(IsInterpolating)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);
			const Real fc = REAL_CONST(500.0);
			const Real newFc = REAL_CONST(1000.0);
			const Real gain = REAL_CONST(0.2);
			const Real newGain = REAL_CONST(0.9);

			HighShelf filter(fc, gain, fs);

			filter.SetTargetParameters(newFc, newGain);

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(1.2), REAL_CONST(0.2), REAL_CONST(0.1), REAL_CONST(-0.3), REAL_CONST(-0.2), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(-0.2) };
			std::vector<Real> output = ProcessHighShelf(fs, fc, gain, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreNotEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ClearBuffers)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);
			const Real fc = 1700;
			const Real gain = REAL_CONST(0.1);

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.5), REAL_CONST(0.0), REAL_CONST(0.3), REAL_CONST(0.4), REAL_CONST(0.2) };
			std::vector<Real> output = ProcessHighShelf(fs, fc, gain, input);

			HighShelf filter(fc,gain, fs);

			for (int i = 0; i < 11; i++)
				filter.GetOutput(RandomValue(), lerpFactor);
			filter.ClearBuffers();

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}
	};
}