
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

		TEST_METHOD(Invalid)
		{
			const Coefficients gain(std::vector<Real>(5, 0.0));
			const Coefficients fc({ 250.0, 500.0, 1000.0, 2000.0, 4000.0 });
			const Real Q = 0.98;
			const int fs = 48e3;

			GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);

			const int numFrames = 256;
			const Real lerpFactor = 0.5;

			Buffer in(numFrames);
			in[0] = 1.0;
			Buffer out(numFrames);

			eq.ProcessAudio(in, out, numFrames, lerpFactor);

			Assert::IsTrue(out[0] == 0.0, L"Not zero");
			Assert::IsFalse(std::isnan(out[0]), L"Return is nan");

			eq.SetTargetGains(std::vector<Real>(5, 1.0));
			eq.ProcessAudio(in, out, numFrames, lerpFactor);

			Assert::IsFalse(out[1] == 0.0, L"Filter stuck at zeros");
		}

		TEST_METHOD(Process)
		{
			// std::string filePath = _SOLUTIONDIR;
			auto inputData = Parse2Dcsv<double>(filePath + "graphicEQInput.csv");
			auto outputData = Parse2Dcsv<double>(filePath + "graphicEQOutput.csv");

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
			for (int i = 1; i < numTests; i++)
			{
				Buffer out = Buffer(numFrames);

				Coefficients gain = Coefficients({ g0[i], g1[i], g2[i], g3[i], g4[i] });
				GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);
				eq.ProcessAudio(in, out, numFrames, lerpFactor);

				// AppendBufferToCSV(filePath + "graphicEQOutput.csv", out);

				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Test: " + ToStr(i) + ", Incorrect Sample : " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], out[j], 10e-16, werrorchar);
				}
			}
		}	

		TEST_METHOD(IsInterpolating)
		{
			const Coefficients gain(std::vector<Real>(5, 0.7));
			const Coefficients fc({ 250.0, 500.0, 1000.0, 2000.0, 4000.0 });
			const Real Q = 0.98;
			const int fs = 48e3;

			const Real lerpFactor = 0.5;


			GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);

			Real out = eq.GetOutput(1.0, lerpFactor);
			Assert::AreEqual(0.7, out, 10e-16, L"Wrong output");

			eq.SetTargetGains(std::vector<Real>(5, 0.3));

			out = eq.GetOutput(1.0, lerpFactor);
			Assert::AreNotEqual(0.7, out, 0.1, L"Is not interpolating");
		}

		TEST_METHOD(ClearBuffers)
		{
			const Coefficients gain({0.3, 0.4, 0.25, 0.21, 0.4});
			const Coefficients fc({ 250.0, 500.0, 1000.0, 2000.0, 4000.0 });
			const Real Q = 0.98;
			const int fs = 48e3;

			const Real lerpFactor = 0.5;

			GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);

			for (int i = 0; i < 11; i++)
				eq.GetOutput(rand(), lerpFactor);

			eq.ClearBuffers();

			Real out = eq.GetOutput(0.0, lerpFactor);

			Assert::AreEqual(0.0, out, L"Wrong output");
		}

		TEST_METHOD(NegativeGain)
		{
			const Coefficients gain({ -0.8, -0.4, -0.15, -0.83, -0.75 });
			const Coefficients fc({ 250.0, 500.0, 1000.0, 2000.0, 4000.0 });
			const Real Q = 0.98;
			const int fs = 48e3;

			const Real lerpFactor = 0.5;

			GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);
			Real out = eq.GetOutput(1.0, lerpFactor);

			Assert::AreEqual(0.0, out, L"Wrong output");

			const int numFrames = 256;
			Buffer in(numFrames);
			Buffer outBuffer(numFrames);
			outBuffer[0] = 1.0;
			in[0] = 1.0;
			eq.ProcessAudio(in, outBuffer, numFrames, lerpFactor);
			Assert::AreEqual(0.0, outBuffer[0], L"Output buffer not zeroed");

		}

		TEST_METHOD(IsZero)
		{
			const Coefficients gain(std::vector<Real>(5, 0.7));
			const Coefficients fc({ 250.0, 500.0, 1000.0, 2000.0, 4000.0 });
			const Real Q = 0.98;
			const int fs = 48e3;

			const Real lerpFactor = 0.5;

			GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);

			bool isZero = eq.SetTargetGains(std::vector<Real>(5, 0.0));
			Assert::IsFalse(isZero, L"True when not zero");

			eq.GetOutput(RandomValue(), lerpFactor);
			isZero = eq.SetTargetGains(std::vector<Real>(5, 0.0));
			Assert::IsFalse(isZero, L"True when not zero");

			for (int i = 0; i < 1e3; i++)
				eq.GetOutput(RandomValue(), lerpFactor);

			isZero = eq.SetTargetGains(std::vector<Real>(5, 0.0));
			Assert::IsTrue(isZero, L"False when zero");
		}
	};
#pragma optimize("", on)
}