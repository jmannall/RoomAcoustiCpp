
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
			const Coefficients<> gain = Coefficients<>::Constant(5, REAL_CONST(0.0));
			const Coefficients<> fc(std::vector<Real>({ REAL_CONST(250.0), REAL_CONST(500.0), REAL_CONST(1000.0), REAL_CONST(2000.0), REAL_CONST(4000.0) }));
			const Real Q = REAL_CONST(0.98);
			const int fs = 48000;

			GraphicEQ<> eq = GraphicEQ<>(gain, fc, Q, fs);

			const int numFrames = 256;
			const Real lerpFactor = REAL_CONST(0.5);

			Buffer<> in = Buffer<>::Zero(numFrames);
			in[0] = REAL_CONST(1.0);
			Buffer<> out(numFrames);

			eq.ProcessAudio(in, out, lerpFactor);

			Assert::AreEqual(REAL_CONST(0.0), out[0], EPS_TEST_ACCURATE, L"Not zero");
			Assert::IsFalse(std::isnan(out[0]), L"Return is nan");

			eq.SetTargetGains(std::vector<Real>(5, REAL_CONST(1.0)));
			eq.ProcessAudio(in, out, lerpFactor);

			Assert::AreNotEqual(REAL_CONST(0.0), out[0], EPS_TEST_ACCURATE, L"Filter stuck at zeros");
		}

		TEST_METHOD(Process)
		{
			// std::string filePath = _SOLUTIONDIR;
			auto inputData = Parse2Dcsv<Real>(filePath + "graphicEQInput.csv");
			auto outputData = Parse2Dcsv<Real>(filePath + "graphicEQOutput.csv");

			std::vector<Real> g0(inputData[0]);
			std::vector<Real> g1(inputData[1]);
			std::vector<Real> g2(inputData[2]);
			std::vector<Real> g3(inputData[3]);
			std::vector<Real> g4(inputData[4]);

			const Coefficients<> fc(std::vector<Real>({ REAL_CONST(250.0), REAL_CONST(500.0), REAL_CONST(1000.0), REAL_CONST(2000.0), REAL_CONST(4000.0) }));

			Real Q = REAL_CONST(0.98);
			int fs = 48000;
			int numFrames = 256;
			Real lerpFactor = REAL_CONST(0.0);
			Buffer<> in = Buffer<>::Zero(numFrames);
			in[0] = 1.0;

			int numTests = ToInt(g0.size());
			for (int i = 1; i < numTests; i++)
			{
				Buffer<> out(numFrames);

				Coefficients<> gain = Coefficients<>(std::vector<Real>({ g0[i], g1[i], g2[i], g3[i], g4[i] }));
				GraphicEQ<> eq = GraphicEQ<>(gain, fc, Q, fs);
				eq.ProcessAudio(in, out, lerpFactor);

				// AppendBufferToCSV(filePath + "graphicEQOutput.csv", out);

				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Test: " + ToStr(i) + ", Incorrect Sample : " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], out[j], EPS_TEST_LOW, werrorchar);
				}
			}
		}	

		TEST_METHOD(IsInterpolating)
		{
			const Coefficients<> gain = Coefficients<>::Constant(5, REAL_CONST(0.7));
			const Coefficients<> fc(std::vector<Real>({ REAL_CONST(250.0), REAL_CONST(500.0), REAL_CONST(1000.0), REAL_CONST(2000.0), REAL_CONST(4000.0) }));
			const Real Q = REAL_CONST(0.98);
			const int fs = 48000;

			const Real lerpFactor = REAL_CONST(0.5);


			GraphicEQ<> eq(gain, fc, Q, fs);

			Real out = eq.GetOutput(1.0, lerpFactor);
			Assert::AreEqual(REAL_CONST(0.7), out, EPS_TEST_ACCURATE, L"Wrong output");

			eq.SetTargetGains(std::vector<Real>(5, REAL_CONST(0.3)));

			out = eq.GetOutput(REAL_CONST(1.0), lerpFactor);
			Assert::AreNotEqual(REAL_CONST(0.7), out, REAL_CONST(0.1), L"Is not interpolating");
		}

		TEST_METHOD(ClearBuffers)
		{
			const Coefficients<> gain(std::vector<Real>({REAL_CONST(0.3), REAL_CONST(0.4), REAL_CONST(0.25), REAL_CONST(0.21), REAL_CONST(0.4)}));
			const Coefficients<> fc(std::vector<Real>({ REAL_CONST(250.0), REAL_CONST(500.0), REAL_CONST(1000.0), REAL_CONST(2000.0), REAL_CONST(4000.0) }));
			const Real Q = REAL_CONST(0.98);
			const int fs = 48000;

			const Real lerpFactor = REAL_CONST(0.5);

			GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);

			for (int i = 0; i < 11; i++)
				eq.GetOutput(RandomValue(), lerpFactor);

			eq.ClearBuffers();

			Real out = eq.GetOutput(0.0, lerpFactor);

			Assert::AreEqual((Real)0.0, out, L"Wrong output");
		}

		TEST_METHOD(NegativeGain)
		{
			const Coefficients<> gain(std::vector({ REAL_CONST(-0.8), REAL_CONST(-0.4), REAL_CONST(-0.15), REAL_CONST(-0.83), REAL_CONST(-0.75) }));
			const Coefficients<> fc(std::vector<Real>({ REAL_CONST(250.0), REAL_CONST(500.0), REAL_CONST(1000.0), REAL_CONST(2000.0), REAL_CONST(4000.0) }));
			const Real Q = REAL_CONST(0.98);
			const int fs = 48000;

			const Real lerpFactor = REAL_CONST(0.5);

			GraphicEQ<> eq = GraphicEQ<>(gain, fc, Q, fs);
			Real out = eq.GetOutput(1.0, lerpFactor);

			Assert::AreEqual(REAL_CONST(0.0), out, L"Wrong output");

			const int numFrames = 256;
			Buffer<> in = Buffer<>(numFrames);
			Buffer<> outBuffer(numFrames);
			outBuffer[0] = REAL_CONST(1.0);
			in[0] = REAL_CONST(1.0);
			eq.ProcessAudio(in, outBuffer, lerpFactor);
			Assert::AreEqual(REAL_CONST(0.0), outBuffer[0], L"Output buffer not zeroed");

		}

		TEST_METHOD(IsZero)
		{
			const Coefficients<> gain = Coefficients<>::Constant(5, REAL_CONST(0.7));
			const Coefficients<> fc(std::vector<Real>({ REAL_CONST(250.0), REAL_CONST(500.0), REAL_CONST(1000.0), REAL_CONST(2000.0), REAL_CONST(4000.0) }));
			const Real Q = REAL_CONST(0.98);
			const int fs = 48000;

			const Real lerpFactor = REAL_CONST(0.5);

			GraphicEQ<> eq = GraphicEQ<>(gain, fc, Q, fs);

			bool isZero = eq.SetTargetGains(std::vector<Real>(5, REAL_CONST(0.0)));
			Assert::IsFalse(isZero, L"True when not zero");

			eq.GetOutput(RandomValue(), lerpFactor);
			isZero = eq.SetTargetGains(std::vector<Real>(5, REAL_CONST(0.0)));
			Assert::IsFalse(isZero, L"True when not zero");

			for (int i = 0; i < 1e3; i++)
				eq.GetOutput(RandomValue(), lerpFactor);

			isZero = eq.SetTargetGains(std::vector<Real>(5, REAL_CONST(0.0)));
			Assert::IsTrue(isZero, L"False when zero");
		}

		TEST_METHOD(SingleGain)
		{
			Real startGain = REAL_CONST(0.7);
			Real endGain = REAL_CONST(0.5);
			const Coefficients<> gain = Coefficients<>::Constant(1, startGain);
			const Coefficients<> fc = Coefficients<>::Constant(1, REAL_CONST(500.0));
			const Real Q = REAL_CONST(0.98);
			const int fs = 48000;

			const Real lerpFactor = REAL_CONST(0.5);

			GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);

			eq.SetTargetGains(std::vector<Real>(1, endGain));
			Real input = RandomValue();
			Real out = eq.GetOutput(input, lerpFactor);

			Real currentGain = lerpFactor * endGain + (REAL_CONST(1.0) - lerpFactor) * startGain;
			Assert::AreEqual(input * currentGain, out, EPS_TEST_ACCURATE, L"Incorrect gain");
		}
	};
#pragma optimize("", on)
}