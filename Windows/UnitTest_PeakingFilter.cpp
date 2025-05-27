
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/IIRFilter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

	std::vector<Real> ProcessPeakingFilter(const int fs, const Real fc, const Real Q, const Real gain, const std::vector<Real>& input)
	{
		const Real pi = 3.141592653589793238462643383279502884197;

		const Real omega = 2.0 * pi * fc / static_cast<Real>(fs);
		const Real cosOmega = cos(omega);
		const Real alpha = sin(omega) / (2.0 * Q);

		const Real A = sqrt(gain);

		const Real norm = 1.0 / (1.0 + alpha / A);
		const Real a1 = -2.0 * cosOmega * norm;
		const Real a2 = (1.0 - alpha / A) * norm;

		const Real b0 = (1.0 + alpha * A) * norm;
		const Real b1 = -2.0 * cosOmega * norm;
		const Real b2 = (1.0 - alpha * A) * norm;

		std::vector<Real> output(input.size(), 0.0);

		output[0] = b0 * input[0];
		output[1] = b0 * input[1] + b1 * input[0] - a1 * output[0];
		for (int i = 2; i < input.size(); i++)
			output[i] = b0 * input[i] + b1 * input[i - 1] + b2 * input[i - 2] - a1 * output[i - 1] - a2 * output[i - 2];

		return output;
	}

	TEST_CLASS(PeakingFilter_Class)
	{
	public:

		TEST_METHOD(Default)
		{
			const int fs = 48e3;
			const Real lerpFactor = 0.5;

			const Real fc = 1000.0;
			const Real Q = 0.98;
			const Real gain = 1.0;

			PeakingFilter filter(fc, Q, fs);

			std::vector<Real> input = { 1.0, 0.0, 0.2, 0.0, -0.3, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, -0.2 };
			std::vector<Real> output = ProcessPeakingFilter(fs, fc, Q, gain, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ProcessAudio)
		{
			const int fs = 48e3;
			const Real lerpFactor = 0.5;

			const Real fc = 500.0;
			const Real Q = 0.5;
			const Real gain = 0.77;

			PeakingFilter filter(fc, gain, Q, fs);

			std::vector<Real> input = { 0.9, -0.1, 0.2, 0.0, -0.3, 0.0, 3.0, 2.1, -0.22, 2.0, 0.0, -0.2 };
			std::vector<Real> output = ProcessPeakingFilter(fs, fc, Q, gain, input);
			
			for (int i = 0; i < output.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(IsInterpolating)
		{
			const int fs = 48e3;
			const Real lerpFactor = 0.5;
			const Real fc = 500.0;
			const Real Q = 0.89;
			const Real gain = 0.2;
			const Real newGain = 0.9;

			PeakingFilter filter(fc, gain, Q, fs);

			filter.SetTargetGain(newGain);

			std::vector<Real> input = { 1.0, 1.2, 0.2, 0.1, -0.3, -0.2, 0.0, 0.0, 0.0, 2.0, 0.0, -0.2 };
			std::vector<Real> output = ProcessPeakingFilter(fs, fc, Q, gain, input);
			;
			for (int i = 0; i < output.size(); i++)
				Assert::AreNotEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(ClearBuffers)
		{
			const int fs = 48e3;
			const Real lerpFactor = 0.5;
			const Real fc = 1700;
			const Real Q = 1.3;
			const Real gain = 0.1;

			std::vector<Real> input = { 1.0, 0.0, 0.2, 0.5, 0.0, 0.3, 0.4, 0.2 };
			std::vector<Real> output = ProcessPeakingFilter(fs, fc, Q, gain, input);

			PeakingFilter filter(fc, gain, Q, fs);

			for (int i = 0; i < 11; i++)
				filter.GetOutput(rand(), lerpFactor);
			filter.ClearBuffers();

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i], lerpFactor), EPS, L"Wrong output");
		}

		TEST_METHOD(Process)
		{
			const Real lerpFactor = 0.5;

			// std::string filePath = _SOLUTIONDIR;
			auto inputData = Parse2Dcsv<double>(filePath + "peakingFilterInput.csv");
			auto outputData = Parse2Dcsv<double>(filePath + "peakingFilterOutput.csv");

			std::vector<Real> fc(inputData[0]);
			std::vector<Real> g(inputData[1]);

			Real Q = 0.98;
			int fs = 48e3;
			int numFrames = 256;
			Buffer out = Buffer(numFrames);
			Buffer in = Buffer(numFrames);
			in[0] = 1.0;

			int numTests = fc.size();
			std::vector<Coefficients> gains = std::vector<Coefficients>(numTests, Coefficients(5));
			for (int i = 0; i < numTests; i++)
			{
				PeakingFilter peakingFilter = PeakingFilter(fc[i], g[i], Q, fs);

				for (int i = 0; i < numFrames; ++i)
					out[i] = peakingFilter.GetOutput(in[i], lerpFactor);

				// AppendBufferToCSV(filePath + "peakingFilterOutput.csv", out);

				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Test: " + ToStr(i) + ", Incorrect Sample : " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], out[j], 10e-16, werrorchar);
				}
			}
		}
	};
}