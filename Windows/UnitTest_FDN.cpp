
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Spatialiser/FDN.h"

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
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>();
			const Real lerpFactor = config->GetLerpFactor();
			const std::vector<Absorption<>> reflectionGains(config->numReverbSources, gains);

			Vec dimensions({ 1.0, 1.5, 2.0 });
			FDN<> fdn(T60, dimensions, config);

			Matrix in(config->numReverbSources, config->numFrames);
			in.RandomUniformDistribution();

			std::vector<Buffer<>> out(config->numReverbSources, Buffer<>(config->numFrames));
			fdn.SetTargetReflectionFilters(reflectionGains);
			fdn.ProcessAudio(in, out, lerpFactor);

			for (int i = 0; i < config->numReverbSources; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < config->numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreNotEqual((Real)0.0, sum, L"ProcessAudio is zero");
			}

			fdn.Reset();
			in.Reset();

			fdn.ProcessAudio(in, out, lerpFactor);
			for (int i = 0; i < config->numReverbSources; i++)
			{
				for (int j = 0; j < config->numFrames; j++)
					Assert::AreEqual((Real)0.0, out[i][j], L"ProcessAudio not zero");
			}
		}

		TEST_METHOD(ReflectionFilters)
		{
			const Coefficients T60({ 0.1, 0.2, 0.3, 0.4 });
			const Absorption gains({ 1.0, 1.0, 1.0, 1.0 });
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>();
			const Real lerpFactor = config->GetLerpFactor();
			const std::vector<Absorption<>> reflectionGains(config->numReverbSources, gains);

			Vec dimensions({ 1.0, 1.5, 2.0 });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix in(config->numReverbSources, config->numFrames);
			in.RandomUniformDistribution();

			std::vector<Buffer<>> out(config->numReverbSources, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			for (int i = 0; i < config->numReverbSources; i++)
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
			const int numReverbSources = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(fs, numFrames, numReverbSources, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption gains({ 0.1, 0.05, 0.3, 0.25 });
			const std::vector<Absorption<>> reflectionGains(config->numReverbSources, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ RandomValue((Real)0.1, (Real)2.0), RandomValue((Real)0.1, (Real)5.0), RandomValue((Real)0.1, (Real)10.0) });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix in(config->numReverbSources, config->numFrames);
			for (int i = 0; i < config->numReverbSources; i++)
				in(i, 1) = 1.0;

			std::vector<Buffer<>> out(config->numReverbSources, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			// Analyze Output Decay
			Real decayTime = 0.0;
			for (int i = 0; i < config->numReverbSources; i++)
				decayTime += CalculateT60(out[i], config->numFrames, config->fs);
			decayTime /= config->numReverbSources;
			Assert::IsTrue(decayTime > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTime, (Real)0.02, L"Decay time does not match target RT60.");
		}

		TEST_METHOD(ProcessRandomOrthogonal)
		{
			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numReverbSources = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(fs, numFrames, numReverbSources, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption gains({ 0.1, 0.05, 0.3, 0.25 });
			const std::vector<Absorption<>> reflectionGains(config->numReverbSources, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix in(config->numReverbSources, config->numFrames);
			for (int i = 0; i < config->numReverbSources; i++)
				in(i, 1) = 1.0;

			std::vector<Buffer<>> out(config->numReverbSources, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			// Analyze Output Decay
			Real decayTime = 0.0;
			for (int i = 0; i < config->numReverbSources; i++)
				decayTime += CalculateT60(out[i], config->numFrames, config->fs);
			decayTime /= config->numReverbSources;
			Assert::IsTrue(decayTime > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTime, (Real)0.02, L"Decay time does not match target RT60.");
		}

		TEST_METHOD(ProcessHouseHolder)
		{
			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numReverbSources = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(fs, numFrames, numReverbSources, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption gains({ 0.1, 0.05, 0.3, 0.25 });
			const std::vector<Absorption<>> reflectionGains(config->numReverbSources, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec<> dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix in(config->numReverbSources, config->numFrames);
			for (int i = 0; i < config->numReverbSources; i++)
				in(i, 1) = 1.0;

			std::vector<Buffer<>> out(config->numReverbSources, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			// Analyze Output Decay
			Real decayTime = 0.0;
			for (int i = 0; i < config->numReverbSources; i++)
				decayTime += CalculateT60(out[i], config->numFrames, config->fs);
			decayTime /= config->numReverbSources;
			Assert::IsTrue(decayTime > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTime, (Real)0.02, L"Decay time does not match target RT60.");
		}

		TEST_METHOD(FeedbackMatrixIdentity)
		{
			const Real target = 0.56;
			const int fs = 48e3;
			const int numFrames = static_cast<int>(fs * target);
			const int numReverbSources = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(fs, numFrames, numReverbSources, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption reflectionGains({ 0.1, 0.05, 0.3, 0.25 });

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ 2.3, 1.5, 5.6 });
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(std::vector<Absorption<>>(config->numReverbSources, reflectionGains));

			Matrix in(config->numReverbSources, config->numFrames);
			in(0, 0) = 1.0;

			std::vector<Buffer<>> out(config->numReverbSources, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			Real sum = 0.0;
			for (int j = 0; j < config->numFrames; j++)
				sum += out[0][j] * out[0][j];
			Assert::AreNotEqual((Real)0.0, sum, L"Feedback matrix is not identity.");

			for (int i = 1; i < config->numReverbSources; i++)
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
			const int numReverbSources = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(fs, numFrames, numReverbSources, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption reflectionGains({ 0.1, 0.05, 0.3, 0.25 });

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ 2.3, 1.5, 5.6 });
			RandomOrthogonalFDN fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(std::vector<Absorption<>>(config->numReverbSources, reflectionGains));

			Matrix in(config->numReverbSources, config->numFrames);
			in(0, 0) = 1.0;

			std::vector<Buffer<>> out(config->numReverbSources, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			for (int i = 0; i < config->numReverbSources; i++)
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
			const int numReverbSources = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(fs, numFrames, numReverbSources, lerpFactor, Q, fBands);

			const Coefficients T60({ target, target, target, target });
			const Absorption reflectionGains({ 0.1, 0.05, 0.3, 0.25 });

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec dimensions({ 2.3, 1.5, 5.6 });
			HouseHolderFDN fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(std::vector<Absorption<>>(config->numReverbSources, reflectionGains));

			Matrix in(config->numReverbSources, config->numFrames);
			in(0, 0) = 1.0;

			std::vector<Buffer<>> out(config->numReverbSources, Buffer<>(config->numFrames));
			fdn.ProcessAudio(in, out, lerpFactor);

			for (int i = 0; i < config->numReverbSources; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < config->numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreNotEqual((Real)0.0, sum, L"Feedback matrix is not random orthogonal.");
			}
		}

		//TEST_METHOD(ProcessComplex)
		//{
		//	const Real target = RandomValue(0.1, 2.0);
		//	const int fs = 48e3;
		//	const int numFrames = static_cast<int>(fs * target * 1.2);
		//	const int numReverbSources = 8;
		//	const Real lerpFactor = 1.0;
		//	const Real Q = 0.98;
		//	const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
		//	const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(fs, numFrames, numReverbSources, lerpFactor, Q, fBands);

		//	const Coefficients T60({ target, target, target, target });
		//	const Absorption gains({ (Real)0.1, (Real)0.05, (Real)0.3, (Real)0.25 });

		//	// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
		//	Vec dimensions({ RandomValue(0.1, 2.0), RandomValue(0.1, 4.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) });
		//	// Vec dimensions({ 0.1, 0.2 });
		//	FDN<Complex> fdn(T60, dimensions, config);
		//	fdn.SetTimeDelay(0.001, fs);
		//	std::vector<Complex> in(2 * config->numFrames);
		//	in[2] = 1.0;
		//	in[3] = -2.0;

		//	Coefficients<> residues({ 1.0, 2.0, 3.0, -1.0, -2.0, -3.0, 0.0, 0.0 });
		//	fdn.SetTargetResidues(residues);
		//	std::vector<Buffer<Complex>> out(config->numReverbSources, Buffer<Complex>(config->numFrames));
		//	for (int i = 0; i < 1; i++)
		//		fdn.ProcessAudio(in, out, lerpFactor);

		//	fdn.Reset();
		//	fdn.ProcessAudio(in, out, lerpFactor);

		//	// Analyze Output Decay
		//	Real decayTime = 0.0;
		//	int count = 0;
		//	for (int i = 0; i < config->numReverbSources; i++)
		//	{
		//		if (residues[i] == 0.0)
		//		{
		//			for (int j = 0; j < numFrames; j++)
		//				Assert::IsTrue(out[i][j] == (Real)0.0, L"Output not zero.");
		//		}
		//		else
		//		{
		//			decayTime += CalculateT60(out[i], config->numFrames, config->fs);
		//			count++;
		//		}
		//	}

		//	decayTime /= count;;

		//	Assert::IsTrue(decayTime > (Real)0.0, L"Decay not detected.");
		//	Assert::AreEqual(target, decayTime, (Real)0.02, L"Decay time does not match target RT60.");
		//}
	};
#pragma optimize("", on)
}