
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/GraphicEQ.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

#pragma optimize("", off)

	TEST_CLASS(GraphicEQ_Class)
	{
	public:

		TEST_METHOD(InvalidGraphicEQ)
		{
			Coefficients gain = Coefficients(std::vector<Real>(5, 0.0));
			Coefficients fc = Coefficients({ 250.0, 500.0, 1000.0, 2000.0, 4000.0 });
			Real Q = 0.98;
			int fs = 48e3;

			GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);

			int numFrames = 256;
			Real lerpFactor = 1.0 / static_cast<Real>(fs);
			Buffer in = Buffer(numFrames);
			in[0] = 1.0;
			Buffer out = Buffer(numFrames);
			eq.ProcessAudio(in, out, numFrames, lerpFactor);

			eq.SetGain(std::vector<Real>(5, 1.0));
			eq.ProcessAudio(in, out, numFrames, lerpFactor);

			Assert::IsFalse(std::isnan(out[0]), L"Filter stuck as invalid");
			Assert::IsFalse(out[1] == 0.0, L"Filter at zero");
		}

		TEST_METHOD(ProcessGraphicEQ)
		{
			// std::string filePath = _SOLUTIONDIR;
			auto inputData = Parse2Dcsv(filePath + "graphicEQInput.csv");
			auto outputData = Parse2Dcsv(filePath + "graphicEQOutput.csv");

			std::vector<Real> g0(inputData[0]);
			std::vector<Real> g1(inputData[1]);
			std::vector<Real> g2(inputData[2]);
			std::vector<Real> g3(inputData[3]);
			std::vector<Real> g4(inputData[4]);

			Coefficients fc = Coefficients({ 250.0, 500.0, 1000.0, 2000.0, 4000.0 });
			Real Q = 0.98;
			int fs = 48e3;
			int numFrames = 256;
			Real lerpFactor = 0.0;
			Buffer in = Buffer(numFrames);
			in[0] = 1.0;

			int numTests = g0.size();
			std::vector<Coefficients> gains = std::vector<Coefficients>(numTests, Coefficients(5));
			for (int i = 0; i < numTests; i++)
			{
				Buffer out = Buffer(numFrames);

				Coefficients gain = Coefficients({ g0[i], g1[i], g2[i], g3[i], g4[i] });
				GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);
				eq.ProcessAudio(in, out, numFrames, lerpFactor);

				// AppendBufferToCSV(filePath + "graphicEQOutput.csv", out);

				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Test: " + IntToStr(i) + ", Incorrect Sample : " + IntToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], out[j], 10e-16, werrorchar);
				}
			}
		}

		TEST_METHOD(ProcessPeakingFilter)
		{
			// std::string filePath = _SOLUTIONDIR;
			auto inputData = Parse2Dcsv(filePath + "peakingFilterInput.csv");
			auto outputData = Parse2Dcsv(filePath + "peakingFilterOutput.csv");

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
					out[i] = peakingFilter.GetOutput(in[i]);

				// AppendBufferToCSV(filePath + "peakingFilterOutput.csv", out);

				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Test: " + IntToStr(i) + ", Incorrect Sample : " + IntToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], out[j], 10e-16, werrorchar);
				}
			}
		}

		TEST_METHOD(ProcessLowShelfFilter)
		{
			// std::string filePath = _SOLUTIONDIR;
			auto inputData = Parse2Dcsv(filePath + "peakingFilterInput.csv");
			auto outputData = Parse2Dcsv(filePath + "lowShelfFilterOutput.csv");

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
				PeakLowShelf lowShelfFilter = PeakLowShelf(fc[i], g[i], Q, fs);

				for (int i = 0; i < numFrames; ++i)
					out[i] = lowShelfFilter.GetOutput(in[i]);

				// AppendBufferToCSV(filePath + "lowShelfFilterOutput.csv", out);

				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Test: " + IntToStr(i) + ", Incorrect Sample : " + IntToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], out[j], 10e-16, werrorchar);
				}
			}
		}

		TEST_METHOD(ProcessHighShelfFilter)
		{
			// std::string filePath = _SOLUTIONDIR;
			auto inputData = Parse2Dcsv(filePath + "peakingFilterInput.csv");
			auto outputData = Parse2Dcsv(filePath + "highShelfFilterOutput.csv");

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
				PeakHighShelf highShelfFilter = PeakHighShelf(fc[i], g[i], Q, fs);

				for (int i = 0; i < numFrames; ++i)
					out[i] = highShelfFilter.GetOutput(in[i]);

				// AppendBufferToCSV(filePath + "UnitTestData\\highShelfFilterOutput.csv", out);

				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Test: " + IntToStr(i) + ", Incorrect Sample : " + IntToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], out[j], 10e-16, werrorchar);
				}
			}
		}
	};
#pragma optimize("", on)
}