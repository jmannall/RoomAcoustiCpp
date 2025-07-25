
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Spatialiser/FDN.h"

#include "Spatialiser/ComplexFDN.h"

#include "Common/RACProfiler.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

#pragma optimize("", off)

	Real CalculateT60(const Buffer<>& out, const int numSamples, const int fs)
	{
		Rowvec envelope(numSamples);

		// Calculate the decay curve (in dB) for each channel and find the time to reach -60 dB
		Buffer<> meanDecayCurve(numSamples);
		Real max = 0.0;
		Real cumSum = 0.0;
		for (int j = 0; j < numSamples; j++)
		{
			cumSum += out[j] * out[j];
			envelope[j] = cumSum;
		}
		envelope /= cumSum;
		for (int j = 0; j < numSamples; j++)
			envelope[j] = 1.0 - envelope[j];

		//// Find time to reach -60 dB on the mean decay curve
		Real targetDecay = std::pow(10.0f, -6.0f); // -60 dB corresponds to 1e-6 in linear scale
		for (int j = 0; j < numSamples; j++)
		{
			if (envelope[j] <= targetDecay)
				return static_cast<Real>(j) / fs;
		}
	}

	TEST_CLASS(FDN_Class)
	{
	public:

		TEST_METHOD(Reset)
		{
			const Coefficients T60({ 0.1, 0.2, 0.3, 0.4 });
			const Absorption gains({ 0.87, 0.75, 0.81, 0.84 });
			const std::shared_ptr<Config> config = std::make_shared<Config>();
			const Real lerpFactor = config->GetLerpFactor();
			const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

			Vec dimensions({ 1.0, 1.5, 2.0 });
			FDN<> fdn(T60, dimensions, config);

			Matrix in(config->numLateReverbChannels, config->numFrames);
			in.RandomUniformDistribution();

			std::vector<Buffer<>> out(config->numLateReverbChannels, Buffer<>(config->numFrames));
			fdn.SetTargetReflectionFilters(reflectionGains);
			fdn.ProcessAudio(in, out, lerpFactor);

			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < config->numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreNotEqual((Real)0.0, sum, L"ProcessAudio is zero");
			}

			fdn.Reset();
			in.Reset();

			fdn.ProcessAudio(in, out, lerpFactor);
			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				for (int j = 0; j < config->numFrames; j++)
					Assert::AreEqual((Real)0.0, out[i][j], L"ProcessAudio not zero");
			}
		}

		TEST_METHOD(ReflectionFilters)
		{
			const Coefficients T60({ 0.1, 0.2, 0.3, 0.4 });
			const Absorption gains({ 1.0, 1.0, 1.0, 1.0 });
			const std::shared_ptr<Config> config = std::make_shared<Config>();
			const Real lerpFactor = config->GetLerpFactor();
			const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

			Vec dimensions({ 1.0, 1.5, 2.0 });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix in(config->numLateReverbChannels, config->numFrames);
			in.RandomUniformDistribution();

			std::vector<Buffer<>> out(config->numLateReverbChannels, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				for (int j = 0; j < config->numFrames; j++)
					Assert::AreEqual((Real)0.0, out[i][j], L"Refelctions filters not zero");
			}
		}

		TEST_METHOD(ProcessIdentity)
		{
			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numLateReverbChannels = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption gains({ 0.1, 0.05, 0.3, 0.25 });
			const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ RandomValue((Real)0.1, (Real)2.0), RandomValue((Real)0.1, (Real)5.0), RandomValue((Real)0.1, (Real)10.0) });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix in(config->numLateReverbChannels, config->numFrames);
			for (int i = 0; i < config->numLateReverbChannels; i++)
				in[i][1] = 1.0;

			std::vector<Buffer<>> out(config->numLateReverbChannels, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			// Analyze Output Decay
			Real decayTime = 0.0;
			for (int i = 0; i < config->numLateReverbChannels; i++)
				decayTime += CalculateT60(out[i], config->numFrames, config->fs);
			decayTime /= config->numLateReverbChannels;
			Assert::IsTrue(decayTime > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTime, (Real)0.02, L"Decay time does not match target RT60.");
		}

		TEST_METHOD(ProcessRandomOrthogonal)
		{
			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numLateReverbChannels = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption gains({ 0.1, 0.05, 0.3, 0.25 });
			const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix in(config->numLateReverbChannels, config->numFrames);
			for (int i = 0; i < config->numLateReverbChannels; i++)
				in[i][1] = 1.0;

			std::vector<Buffer<>> out(config->numLateReverbChannels, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			// Analyze Output Decay
			Real decayTime = 0.0;
			for (int i = 0; i < config->numLateReverbChannels; i++)
				decayTime += CalculateT60(out[i], config->numFrames, config->fs);
			decayTime /= config->numLateReverbChannels;
			Assert::IsTrue(decayTime > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTime, (Real)0.02, L"Decay time does not match target RT60.");
		}

		TEST_METHOD(ProcessHouseHolder)
		{
			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numLateReverbChannels = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption gains({ 0.1, 0.05, 0.3, 0.25 });
			const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec<> dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix in(config->numLateReverbChannels, config->numFrames);
			for (int i = 0; i < config->numLateReverbChannels; i++)
				in[i][1] = 1.0;

			std::vector<Buffer<>> out(config->numLateReverbChannels, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			// Analyze Output Decay
			Real decayTime = 0.0;
			for (int i = 0; i < config->numLateReverbChannels; i++)
				decayTime += CalculateT60(out[i], config->numFrames, config->fs);
			decayTime /= config->numLateReverbChannels;
			Assert::IsTrue(decayTime > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTime, (Real)0.02, L"Decay time does not match target RT60.");
		}

		TEST_METHOD(FeedbackMatrixIdentity)
		{
			const Real target = 0.56;
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target);
			const int numLateReverbChannels = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption reflectionGains({ 0.1, 0.05, 0.3, 0.25 });

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ 2.3, 1.5, 5.6 });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(std::vector<Absorption<>>(config->numLateReverbChannels, reflectionGains));

			Matrix in(config->numLateReverbChannels, config->numFrames);
			in[0][0] = 1.0;

			std::vector<Buffer<>> out(config->numLateReverbChannels, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			Real sum = 0.0;
			for (int j = 0; j < config->numFrames; j++)
				sum += out[0][j] * out[0][j];
			Assert::AreNotEqual((Real)0.0, sum, L"Feedback matrix is not identity.");

			for (int i = 1; i < config->numLateReverbChannels; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < config->numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreEqual((Real)0.0, sum, L"Feedback matrix is not identity.");
			}
		}

		TEST_METHOD(FeedbackMatrixRandomOrthogonal)
		{
			const Real target = 0.56;
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target);
			const int numLateReverbChannels = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption reflectionGains({ 0.1, 0.05, 0.3, 0.25 });

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ 2.3, 1.5, 5.6 });
			RandomOrthogonalFDN fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(std::vector<Absorption<>>(config->numLateReverbChannels, reflectionGains));

			Matrix in(config->numLateReverbChannels, config->numFrames);
			in[0][0] = 1.0;

			std::vector<Buffer<>> out(config->numLateReverbChannels, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < config->numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreNotEqual((Real)0.0, sum, L"Feedback matrix is not random orthogonal.");
			}
		}

		TEST_METHOD(FeedbackMatrixHouseHolder)
		{
			const Real target = 0.56;
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target);
			const int numLateReverbChannels = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption reflectionGains({ 0.1, 0.05, 0.3, 0.25 });

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ 2.3, 1.5, 5.6 });
			HouseHolderFDN fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(std::vector<Absorption<>>(config->numLateReverbChannels, reflectionGains));

			Matrix in(config->numLateReverbChannels, config->numFrames);
			in[0][0] = 1.0;

			std::vector<Buffer<>> out(config->numLateReverbChannels, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < config->numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreNotEqual((Real)0.0, sum, L"Feedback matrix is not random orthogonal.");
			}
		}

		TEST_METHOD(ProcessReal)
		{
			Profiler::Instance().SetOutputFile("RealProfile2.csv", true);

			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numLateReverbChannels = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption gains({ 0.1, 0.05, 0.3, 0.25 });
			const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
			FDN<Real> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix<Real> in(config->numLateReverbChannels, config->numFrames);
			for (int i = 0; i < config->numLateReverbChannels; i++)
				in[i][1] = 1.0;

			std::vector<Buffer<Real>> out(config->numLateReverbChannels, Buffer<Real>(config->numFrames));

			for (int i = 0; i < 1000; i++)
				fdn.ProcessAudio(in, out, lerpFactor);

			fdn.Reset();
			fdn.ProcessAudio(in, out, lerpFactor);
			std::vector<Buffer<Real>> outReal(config->numLateReverbChannels, Buffer<Real>(config->numFrames));
			std::vector<Buffer<Real>> outImaginary(config->numLateReverbChannels, Buffer<Real>(config->numFrames));

			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				for (int j = 0; j < config->numFrames; j++)
				{
					outReal[i][j] = out[i][j];
					outImaginary[i][j] = out[i][j];

					Assert::AreEqual(outReal[i][j], outImaginary[i][j], L"Incorrect ratio between real and imaginary parts.");
				}
			}

			// Analyze Output Decay
			Real decayTimeReal = 0.0;
			Real decayTimeImaginay = 0.0;
			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				decayTimeReal += CalculateT60(outReal[i], config->numFrames, config->fs);
				decayTimeImaginay += CalculateT60(outReal[i], config->numFrames, config->fs);
			}
			decayTimeReal /= config->numLateReverbChannels;
			decayTimeImaginay /= config->numLateReverbChannels;

			Assert::IsTrue(decayTimeReal > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTimeReal, (Real)0.02, L"Decay time does not match target RT60.");

			Assert::IsTrue(decayTimeImaginay > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTimeImaginay, (Real)0.02, L"Decay time does not match target RT60.");
			Profiler::Instance().SetOutputFile("RealProfile.csv", false);
		}

		TEST_METHOD(ProcessComplex)
		{
			Profiler::Instance().SetOutputFile("ComplexProfile2.csv", true);

			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numLateReverbChannels = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption gains({ (Real)0.1, (Real)0.05, (Real)0.3, (Real)0.25 });
			const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
			FDN<Complex> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix<Complex> in(config->numLateReverbChannels, config->numFrames);
			for (int i = 0; i < config->numLateReverbChannels; i++)
				in[i][1] = Complex(1.0, 2.0);

			std::vector<Buffer<Complex>> out(config->numLateReverbChannels, Buffer<Complex>(config->numFrames));

			{
				for (int i = 0; i < 1000; i++)
					fdn.ProcessAudio(in, out, lerpFactor);
			}

			fdn.Reset();
			fdn.ProcessAudio(in, out, lerpFactor);
			std::vector<Buffer<Real>> outReal(config->numLateReverbChannels, Buffer<Real>(config->numFrames));
			std::vector<Buffer<Real>> outImaginary(config->numLateReverbChannels, Buffer<Real>(config->numFrames));

			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				for (int j = 0; j < config->numFrames; j++)
				{
					outReal[i][j] = out[i][j].real();
					outImaginary[i][j] = out[i][j].imag();

					Assert::AreEqual((Real)2.0 * outReal[i][j], outImaginary[i][j], L"Incorrect ratio between real and imaginary parts.");
				}
			}

			// Analyze Output Decay
			Real decayTimeReal = 0.0;
			Real decayTimeImaginay = 0.0;
			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				decayTimeReal += CalculateT60(outReal[i], config->numFrames, config->fs);
				decayTimeImaginay += CalculateT60(outReal[i], config->numFrames, config->fs);
			}
			decayTimeReal /= config->numLateReverbChannels;
			decayTimeImaginay /= config->numLateReverbChannels;

			Assert::IsTrue(decayTimeReal > (Real)0.0, L"Decay not detected.");
			Assert::AreEqual(target, decayTimeReal, (Real)0.02, L"Decay time does not match target RT60.");

			Assert::IsTrue(decayTimeImaginay > (Real)0.0, L"Decay not detected.");
			Assert::AreEqual(target, decayTimeImaginay, (Real)0.02, L"Decay time does not match target RT60.");
			Profiler::Instance().SetOutputFile("ComplexProfile.csv", false);

		}

		TEST_METHOD(ProcessComplexPair)
		{
			Profiler::Instance().SetOutputFile("ComplexPairProfile2.csv", true);

			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numLateReverbChannels = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption gains({ (Real)0.1, (Real)0.05, (Real)0.3, (Real)0.25 });
			const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
			FDN<ComplexPair> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix<ComplexPair> in(config->numLateReverbChannels, config->numFrames);
			for (int i = 0; i < config->numLateReverbChannels; i++)
				in[i][1] = ComplexPair(1.0, 2.0);

			std::vector<Buffer<ComplexPair>> out(config->numLateReverbChannels, Buffer<ComplexPair>(config->numFrames));


			for (int i = 0; i < 1000; i++)
				fdn.ProcessAudio(in, out, lerpFactor);

			fdn.Reset();
			fdn.ProcessAudio(in, out, lerpFactor);
			std::vector<Buffer<Real>> outReal(config->numLateReverbChannels, Buffer<Real>(config->numFrames));
			std::vector<Buffer<Real>> outImaginary(config->numLateReverbChannels, Buffer<Real>(config->numFrames));

			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				for (int j = 0; j < config->numFrames; j++)
				{
					outReal[i][j] = out[i][j].real;
					outImaginary[i][j] = out[i][j].imag;

					Assert::AreEqual((Real)2.0 * outReal[i][j], outImaginary[i][j], L"Incorrect ratio between real and imaginary parts.");
				}
			}

			// Analyze Output Decay
			Real decayTimeReal = 0.0;
			Real decayTimeImaginay = 0.0;
			for (int i = 0; i < config->numLateReverbChannels; i++)
			{
				decayTimeReal += CalculateT60(outReal[i], config->numFrames, config->fs);
				decayTimeImaginay += CalculateT60(outReal[i], config->numFrames, config->fs);
			}
			decayTimeReal /= config->numLateReverbChannels;
			decayTimeImaginay /= config->numLateReverbChannels;

			Assert::IsTrue(decayTimeReal > (Real)0.0, L"Decay not detected.");
			Assert::AreEqual(target, decayTimeReal, (Real)0.02, L"Decay time does not match target RT60.");

			Assert::IsTrue(decayTimeImaginay > (Real)0.0, L"Decay not detected.");
			Assert::AreEqual(target, decayTimeImaginay, (Real)0.02, L"Decay time does not match target RT60.");
			Profiler::Instance().SetOutputFile("ComplexPairProfile.csv", false);
		}

		TEST_METHOD(ProcessInterleaved)
		{
			Profiler::Instance().SetOutputFile("ProfileFDNsInterleaved.csv", true);
			for (int i = 0; i < 1000; i++)
			{
				{
					const Real target = RandomValue(0.1, 2.0);
					const int fs = 48e3;
					const int numFrames = static_cast<int>(fs * target * 1.2);
					const int numLateReverbChannels = 12;
					const Real lerpFactor = 1.0;
					const Real Q = 0.98;
					const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
					const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

					const Coefficients T60({ target, target, target, target });
					const Absorption gains({ 0.1, 0.05, 0.3, 0.25 });
					const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

					// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
					Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
					FDN<Real> fdn(T60, dimensions, config);
					fdn.SetTargetReflectionFilters(reflectionGains);

					Matrix<Real> in(config->numLateReverbChannels, config->numFrames);
					for (int i = 0; i < config->numLateReverbChannels; i++)
						in[i][1] = 1.0;

					std::vector<Buffer<Real>> out(config->numLateReverbChannels, Buffer<Real>(config->numFrames));

					fdn.ProcessAudio(in, out, lerpFactor);
				}
				{

					const Real target = RandomValue(0.1, 2.0);
					const int fs = 48e3;
					const int numFrames = static_cast<int>(fs * target * 1.2);
					const int numLateReverbChannels = 12;
					const Real lerpFactor = 1.0;
					const Real Q = 0.98;
					const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
					const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

					const Coefficients T60({ target, target, target, target });
					const Absorption gains({ (Real)0.1, (Real)0.05, (Real)0.3, (Real)0.25 });
					const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

					// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
					Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
					FDN<Complex> fdn(T60, dimensions, config);
					fdn.SetTargetReflectionFilters(reflectionGains);

					Matrix<Complex> in(config->numLateReverbChannels, config->numFrames);
					for (int i = 0; i < config->numLateReverbChannels; i++)
						in[i][1] = Complex(1.0, 2.0);

					std::vector<Buffer<Complex>> out(config->numLateReverbChannels, Buffer<Complex>(config->numFrames));

					fdn.ProcessAudio(in, out, lerpFactor);
				}
				{
					const Real target = RandomValue(0.1, 2.0);
					const int fs = 48e3;
					const int numFrames = static_cast<int>(fs * target * 1.2);
					const int numLateReverbChannels = 12;
					const Real lerpFactor = 1.0;
					const Real Q = 0.98;
					const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
					const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

					const Coefficients T60({ target, target, target, target });
					const Absorption gains({ (Real)0.1, (Real)0.05, (Real)0.3, (Real)0.25 });
					const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

					// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
					Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
					FDN<ComplexPair> fdn(T60, dimensions, config);
					fdn.SetTargetReflectionFilters(reflectionGains);

					Matrix<ComplexPair> in(config->numLateReverbChannels, config->numFrames);
					for (int i = 0; i < config->numLateReverbChannels; i++)
						in[i][1] = ComplexPair(1.0, 2.0);

					std::vector<Buffer<ComplexPair>> out(config->numLateReverbChannels, Buffer<ComplexPair>(config->numFrames));

					fdn.ProcessAudio(in, out, lerpFactor);
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			Profiler::Instance().SetOutputFile("ProfileFDNsInterleaved.csv", false);
		}

		TEST_METHOD(ProcessAll)
		{
			Profiler::Instance().SetOutputFile("ProfileFDNs2.csv", true);

			{
				const Real target = RandomValue(0.1, 2.0);
				const int fs = 48e3;
				const int numFrames = static_cast<int>(fs * target * 1.2);
				const int numLateReverbChannels = 12;
				const Real lerpFactor = 1.0;
				const Real Q = 0.98;
				const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
				const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

				const Coefficients T60({ target, target, target, target });
				const Absorption gains({ 0.1, 0.05, 0.3, 0.25 });
				const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

				// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
				Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
				FDN<Real> fdn(T60, dimensions, config);
				fdn.SetTargetReflectionFilters(reflectionGains);

				Matrix<Real> in(config->numLateReverbChannels, config->numFrames);
				for (int i = 0; i < config->numLateReverbChannels; i++)
					in[i][1] = 1.0;

				std::vector<Buffer<Real>> out(config->numLateReverbChannels, Buffer<Real>(config->numFrames));

				for (int i = 0; i < 1000; i++)
					fdn.ProcessAudio(in, out, lerpFactor);
			}
			{

				const Real target = RandomValue(0.1, 2.0);
				const int fs = 48e3;
				const int numFrames = static_cast<int>(fs * target * 1.2);
				const int numLateReverbChannels = 12;
				const Real lerpFactor = 1.0;
				const Real Q = 0.98;
				const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
				const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

				const Coefficients T60({ target, target, target, target });
				const Absorption gains({ (Real)0.1, (Real)0.05, (Real)0.3, (Real)0.25 });
				const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

				// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
				Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
				FDN<Complex> fdn(T60, dimensions, config);
				fdn.SetTargetReflectionFilters(reflectionGains);

				Matrix<Complex> in(config->numLateReverbChannels, config->numFrames);
				for (int i = 0; i < config->numLateReverbChannels; i++)
					in[i][1] = Complex(1.0, 2.0);

				std::vector<Buffer<Complex>> out(config->numLateReverbChannels, Buffer<Complex>(config->numFrames));

				for (int i = 0; i < 1000; i++)
					fdn.ProcessAudio(in, out, lerpFactor);
			}
			{
				const Real target = RandomValue(0.1, 2.0);
				const int fs = 48e3;
				const int numFrames = static_cast<int>(fs * target * 1.2);
				const int numLateReverbChannels = 12;
				const Real lerpFactor = 1.0;
				const Real Q = 0.98;
				const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
				const std::shared_ptr<Config> config = std::make_shared<Config>(fs, numFrames, numLateReverbChannels, lerpFactor, Q, fBands);

				const Coefficients T60({ target, target, target, target });
				const Absorption gains({ (Real)0.1, (Real)0.05, (Real)0.3, (Real)0.25 });
				const std::vector<Absorption<>> reflectionGains(config->numLateReverbChannels, gains);

				// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
				Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
				FDN<ComplexPair> fdn(T60, dimensions, config);
				fdn.SetTargetReflectionFilters(reflectionGains);

				Matrix<ComplexPair> in(config->numLateReverbChannels, config->numFrames);
				for (int i = 0; i < config->numLateReverbChannels; i++)
					in[i][1] = ComplexPair(1.0, 2.0);

				std::vector<Buffer<ComplexPair>> out(config->numLateReverbChannels, Buffer<ComplexPair>(config->numFrames));

				for (int i = 0; i < 1000; i++)
					fdn.ProcessAudio(in, out, lerpFactor);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			Profiler::Instance().SetOutputFile("ProfileFDNs2.csv", false);
		}

		/*TEST_METHOD(ResetReflection)
		{
			const int delay = 73;
			const Coefficients T60({ 0.1, 0.2, 0.3, 0.4 });
			const Coefficients reflectionGains({ 0.87, 0.75, 0.81, 0.84 });
			const Config config = Config();

			FDNChannel channel(delay, T60, config);
			channel.SetTargetReflectionFilter(reflectionGains);

			const int numFrames = 256;
			Buffer<> in(numFrames);
			Buffer<> out(numFrames);
			for (int i = 0; i < numFrames; i++)
				in[i] = channel.GetOutput(rand(), config->lerpFactor);
			channel.ProcessOutput(in, out, numFrames, config->lerpFactor);
			channel.Reset();

			in.Reset();
			channel.ProcessOutput(in, out, numFrames, config->lerpFactor);
			for (int i = 0; i < numFrames; i++)
				Assert::AreEqual((Real)0.0, out[i], L"ProcessOutput not zero");
		}

		TEST_METHOD(ProcessReflection)
		{
			const int delay = 7;
			const Coefficients T60({ 0.1, 0.2, 0.3, 0.4 });
			const Real target = 0.6;
			const Coefficients reflectionGains({ target, target, target, target });
			Config config = Config();
			config->lerpFactor = 0.5;

			FDNChannel channel(delay, T60, config);
			channel.SetTargetReflectionFilter(reflectionGains);

			const int numFrames = 256;
			Buffer<> in(numFrames);
			in[0] = 1.0;
			Buffer<> out(numFrames);

			for (int i = 0; i < 10; i++)
				channel.ProcessOutput(in, out, numFrames, config->lerpFactor);
			in[0] = 1.0;
			channel.ProcessOutput(in, out, numFrames, config->lerpFactor);
			Assert::AreEqual(target, out[0], 10e-16, L"Reflection filter incorrect");
		}

		TEST_METHOD(ProcessAbsorption)
		{
			const int delay = 100;
			const Coefficients T60({ 0.2, 0.2, 0.2, 0.2 });
			Config config = Config();
			config->lerpFactor = 0.5;
			const Real target = std::pow(10.0, (-60.0 * (static_cast<Real>(delay) / static_cast<Real>(config->fs)) / 20.0) / T60[0]);

			FDNChannel channel(delay, T60, config);

			channel.GetOutput(1.0, config->lerpFactor);
			for (int i = 1; i < delay; i++)
				channel.GetOutput(0.0, config->lerpFactor);
			Real out = channel.GetOutput(0.0, config->lerpFactor);
			Assert::AreEqual(target, out, 10e-16, L"Absorption filter incorrect");
		}

		TEST_METHOD(UpdateT60)
		{
			const int delay = 100;
			const Coefficients T60({ 0.2, 0.2, 0.2, 0.2 });
			const Coefficients newT60({ 0.6, 0.6, 0.6, 0.6 });
			Config config = Config();
			config->lerpFactor = 0.5;
			const Real target = std::pow(10.0, (-60.0 * (static_cast<Real>(delay) / static_cast<Real>(config->fs)) / 20.0) / newT60[0]);

			FDNChannel channel(delay, T60, config);
			channel.SetTargetT60(newT60);

			for (int i = 0; i < 1e3; i++)
				channel.GetOutput(0.0, config->lerpFactor);
			channel.GetOutput(1.0, config->lerpFactor);
			for (int i = 1; i < delay; i++)
				channel.GetOutput(0.0, config->lerpFactor);
			Real out = channel.GetOutput(0.0, config->lerpFactor);
			Assert::AreEqual(target, out, 10e-16, L"Absorption filter not interpolating");
		}*/
	};
#pragma optimize("", on)
}