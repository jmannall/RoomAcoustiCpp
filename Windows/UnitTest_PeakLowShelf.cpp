
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/IIRFilter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

	std::vector<Real> ProcessPeakLowShelf(const int fs, const Real fc, const Real Q, const Real gain, const std::vector<Real>& input)
	{
		const Real pi = REAL_CONST(3.141592653589793238462643383279502884197);

		const Real omega = REAL_CONST(2.0) * pi * fc / static_cast<Real>(fs);
		const Real cosOmega = cos(omega);
		const Real alpha = sin(omega) / (REAL_CONST(2.0) * Q);

		const Real A = sqrt(gain);

		const Real norm = REAL_CONST(1.0) / ((A + 1) + (A - 1) * cosOmega + REAL_CONST(2.0) * sqrt(A) * alpha);
		const Real a1 = -REAL_CONST(2.0) * ((A - 1) + (A + 1) * cosOmega) * norm;
		const Real a2 = ((A + 1) + (A - 1) * cosOmega - REAL_CONST(2.0) * sqrt(A) * alpha) * norm;

		const Real b0 = A * ((A + 1) - (A - 1) * cosOmega + REAL_CONST(2.0) * sqrt(A) * alpha) * norm;
		const Real b1 = REAL_CONST(2.0) * A * ((A - 1) - (A + 1) * cosOmega) * norm;
		const Real b2 = A * ((A + 1) - (A - 1) * cosOmega - REAL_CONST(2.0) * sqrt(A) * alpha) * norm;

		std::vector<Real> output(input.size(), REAL_CONST(0.0));

		output[0] = b0 * input[0];
		output[1] = b0 * input[1] + b1 * input[0] - a1 * output[0];
		for (int i = 2; i < input.size(); i++)
			output[i] = b0 * input[i] + b1 * input[i - 1] + b2 * input[i - 2] - a1 * output[i - 1] - a2 * output[i - 2];

		return output;
	}

	TEST_CLASS(PeakLowShelf_Class)
	{
	public:

		TEST_METHOD(Default)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);

			const Real fc = REAL_CONST(1000.0);
			const Real Q = REAL_CONST(0.98);
			const Real gain = REAL_CONST(1.0);

			PeakLowShelf filter(fc, Q, fs);

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.0), REAL_CONST(-0.3), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(-0.2) };
			std::vector<Real> output = ProcessPeakLowShelf(fs, fc, Q, gain, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ProcessAudio)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);

			const Real fc = REAL_CONST(500.0);
			const Real Q = REAL_CONST(0.5);
			const Real gain = REAL_CONST(0.77);

			PeakLowShelf filter(fc, gain, Q, fs);

			std::vector<Real> input = { REAL_CONST(0.9), REAL_CONST(-0.1), REAL_CONST(0.2), REAL_CONST(0.0), REAL_CONST(-0.3), REAL_CONST(0.0), REAL_CONST(3.0), REAL_CONST(2.1), REAL_CONST(-0.22), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(-0.2) };
			std::vector<Real> output = ProcessPeakLowShelf(fs, fc, Q, gain, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS_TEST_MEDIUM, L"Wrong output");
		}

		TEST_METHOD(IsInterpolating)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);
			const Real fc = REAL_CONST(500.0);
			const Real Q = REAL_CONST(0.89);
			const Real gain = REAL_CONST(0.2);
			const Real newGain = REAL_CONST(0.9);

			PeakLowShelf filter(fc, gain, Q, fs);

			filter.SetTargetGain(newGain);

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(1.2), REAL_CONST(0.2), REAL_CONST(0.1), REAL_CONST(-0.3), REAL_CONST(-0.2), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(2.0), REAL_CONST(0.0), REAL_CONST(-0.2) };
			std::vector<Real> output = ProcessPeakLowShelf(fs, fc, Q, gain, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreNotEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ClearBuffers)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(0.5);
			const Real fc = 1700;
			const Real Q = REAL_CONST(1.3);
			const Real gain = REAL_CONST(0.1);

			std::vector<Real> input = { REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(0.2), REAL_CONST(0.5), REAL_CONST(0.0), REAL_CONST(0.3), REAL_CONST(0.4), REAL_CONST(0.2) };
			std::vector<Real> output = ProcessPeakLowShelf(fs, fc, Q, gain, input);

			PeakLowShelf filter(fc, gain, Q, fs);

			for (int i = 0; i < 11; i++)
				filter.GetOutput(RandomValue(), lerpFactor);
			filter.ClearBuffers();

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(Process)
		{
			const Real lerpFactor = REAL_CONST(0.5);

			// std::string filePath = _SOLUTIONDIR;
			auto inputData = Parse2Dcsv<Real>(filePath + "peakingFilterInput.csv");
			auto outputData = Parse2Dcsv<Real>(filePath + "highShelfFilterOutput.csv");

			std::vector<Real> fc(inputData[0]);
			std::vector<Real> g(inputData[1]);

			Real Q = REAL_CONST(0.98);
			int fs = 48000;
			int numFrames = 256;
			Buffer<> out(numFrames);
			Buffer<> in = Buffer<>::Zero(numFrames);
			in[0] = REAL_CONST(1.0);

			int numTests = ToInt( fc.size() );
			std::vector<Coefficients<>> gains = std::vector<Coefficients<>>(numTests, Coefficients<>(5));
			for (int i = 0; i < numTests; i++)
			{
				PeakHighShelf highShelfFilter = PeakHighShelf(fc[i], g[i], Q, fs);

				for (int i = 0; i < numFrames; ++i)
					out[i] = highShelfFilter.GetOutput(in[i], lerpFactor);

				// AppendBufferToCSV(filePath + "UnitTestData\\highShelfFilterOutput.csv", out);

				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Test: " + ToStr(i) + ", Incorrect Sample : " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], out[j], EPS_TEST_LOW, werrorchar);		// the SSE filters require this to be low due to ordering
				}
			}
		}
	};
}