
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Spatialiser/FDN.h"

#include "Spatialiser/Reverb.h"

#include "Common/RACProfiler.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

#pragma optimize("", off)

	Real CalculateT60(const Buffer<>& out, const int numSamples, const int fs)
	{
		Rowvec<> envelope(numSamples);

		// Calculate the decay curve (in dB) for each channel and find the time to reach -60 dB
		Buffer<> meanDecayCurve(numSamples);
		Real max = 0.0;
		Real cumSum = 0.0;
		for (int j = 0; j < numSamples; j++)
		{
			cumSum += out[j] * out[j];
			envelope(j) = cumSum;
		}
		envelope /= cumSum;
		for (int j = 0; j < numSamples; j++)
			envelope(j) = 1.0 - envelope(j);

		//// Find time to reach -60 dB on the mean decay curve
		Real targetDecay = std::pow(10.0f, -6.0f); // -60 dB corresponds to 1e-6 in linear scale
		for (int j = 0; j < numSamples; j++)
		{
			if (envelope(j) <= targetDecay)
				return static_cast<Real>(j) / fs;
		}
		return static_cast<Real>(numSamples) / fs;
	}

	TEST_CLASS(FDN_Class)
	{
	public:

		TEST_METHOD(Reset)
		{
			const Coefficients<> T60(std::vector<Real>({ 0.1, 0.2, 0.3, 0.4 }));
			const Coefficients<> gains(std::vector<Real>({ 0.87, 0.75, 0.81, 0.84 }));
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>();
			AudioData audioData(config);
			int numReverbSources = config->GetData().numReverbSources;
			int numFrames = config->GetData().numFrames;
			const std::vector<Coefficients<>> reflectionGains(numReverbSources, gains);

			Vec<> dimensions(std::vector<Real>({ (Real)1.0, (Real)1.5, (Real)2.0 }));
			FDN<> fdn(T60, dimensions, config);

			Matrix<> in(numReverbSources, numFrames);
			in.RandomUniformDistribution();

			std::vector<Buffer<>> out(numReverbSources, Buffer<>(numFrames));
			fdn.SetTargetReflectionFilters(reflectionGains);
			fdn.ProcessAudio(in, out, audioData);

			for (int i = 0; i < numReverbSources; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreNotEqual((Real)0.0, sum, L"ProcessAudio is zero");
			}

			config->FlagClearBuffers();
			audioData = AudioData(config);
			in.Reset();

			fdn.ProcessAudio(in, out, audioData);
			for (int i = 0; i < numReverbSources; i++)
			{
				for (int j = 0; j < numFrames; j++)
					Assert::AreEqual((Real)0.0, out[i][j], L"ProcessAudio not zero");
			}
		}

		TEST_METHOD(ReflectionFilters)
		{
			const Coefficients<> T60(std::vector<Real>({ 0.1, 0.2, 0.3, 0.4 }));
			const Coefficients<> gains(std::vector<Real>({ 1.0, 1.0, 1.0, 1.0 }));
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>();
			AudioData audioData(config);
			int numReverbSources = config->GetData().numReverbSources;
			int numFrames = config->GetData().numFrames;
			const std::vector<Coefficients<>> reflectionGains(numReverbSources, CalculateReflectance(gains));

			Vec<> dimensions(std::vector<Real>({ (Real)1.0, (Real)1.5, (Real)2.0 }));
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix<> in(numReverbSources, numFrames);
			in.RandomUniformDistribution();

			std::vector<Buffer<>> out(numReverbSources, Buffer<>(numFrames));
			fdn.ProcessAudio(in, out, audioData);

			for (int i = 0; i < numReverbSources; i++)
			{
				for (int j = 0; j < numFrames; j++)
					Assert::AreEqual((Real)0.0, out[i][j], L"Refelctions filters not zero");
			}
		}

		TEST_METHOD(ProcessIdentity)
		{
			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48000;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numReverbSources = 12;
			const int fdnSize = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const DSPData data(fs, numFrames, numReverbSources, fdnSize, lerpFactor, Q, fBands);
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(data);
			AudioData audioData(config);

			const Coefficients<> T60(std::vector<Real>({ target, target, target, target }));
			const Coefficients<> gains(std::vector<Real>({ 0.1, 0.05, 0.3, 0.25 }));
			const std::vector<Coefficients<>> reflectionGains(numReverbSources, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec<> dimensions(std::vector<Real>({ RandomValue((Real)0.1, (Real)2.0), RandomValue((Real)0.1, (Real)5.0), RandomValue((Real)0.1, (Real)10.0) }));
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix<> in = Matrix<>::Zero(numReverbSources, numFrames);
			for (int i = 0; i < numReverbSources; i++)
				in(i, 1) = 1.0;

			std::vector<Buffer<>> out(numReverbSources, Buffer<>(numFrames));
			fdn.ProcessAudio(in, out, audioData);

			// Analyze Output Decay
			Real decayTime = 0.0;
			for (int i = 0; i < numReverbSources; i++)
				decayTime += CalculateT60(out[i], numFrames, config->GetData().fs);
			decayTime /= numReverbSources;
			Assert::IsTrue(decayTime > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTime, (Real)0.10, L"Decay time does not match target RT60.");
		}

		TEST_METHOD(ProcessRandomOrthogonal)
		{
			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48000;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numReverbSources = 12;
			const int fdnSize = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const DSPData data(fs, numFrames, numReverbSources, fdnSize, lerpFactor, Q, fBands);
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(data);
			AudioData audioData(config);

			const Coefficients<> T60(std::vector<Real>({ target, target, target, target }));
			const Coefficients<> gains(std::vector<Real>({ 0.1, 0.05, 0.3, 0.25 }));
			const std::vector<Coefficients<>> reflectionGains(numReverbSources, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec<> dimensions(std::vector<Real>({ RandomValue(0.1, 2.0), RandomValue(0.1, 5.0), RandomValue(0.1, 10.0) }));
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix<> in = Matrix<>::Zero(numReverbSources, numFrames);
			for (int i = 0; i < numReverbSources; i++)
				in(i, 1) = 1.0;

			std::vector<Buffer<>> out(numReverbSources, Buffer<>(numFrames));
			fdn.ProcessAudio(in, out, audioData);

			// Analyze Output Decay
			Real decayTime = 0.0;
			for (int i = 0; i < numReverbSources; i++)
				decayTime += CalculateT60(out[i], numFrames, config->GetData().fs);
			decayTime /= numReverbSources;
			Assert::IsTrue(decayTime > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTime, (Real)0.03, L"Decay time does not match target RT60.");
		}

		TEST_METHOD(ProcessHouseHolder)
		{
			const Real target = RandomValue(0.1, 2.0);
			const int fs = 48000;
			const int numFrames = static_cast<int>(fs * target * 1.2);
			const int numReverbSources = 12;
			const int fdnSize = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const DSPData data(fs, numFrames, numReverbSources, fdnSize, lerpFactor, Q, fBands);
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(data);
			AudioData audioData(config);

			const Coefficients<> T60(std::vector<Real>({ target, target, target, target }));
			const Coefficients<> gains(std::vector<Real>({ 0.1, 0.05, 0.3, 0.25 }));
			const std::vector<Coefficients<>> reflectionGains(numReverbSources, gains);

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec<> dimensions(std::vector<Real>({ RandomValue(0.1f, 2.0f), RandomValue(0.1f, 5.0f), RandomValue(0.1f, 10.0f) }));
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(reflectionGains);

			Matrix<> in = Matrix<>::Zero(numReverbSources, numFrames);
			for (int i = 0; i < numReverbSources; i++)
				in(i, 1) = 1.0;

			std::vector<Buffer<>> out(numReverbSources, Buffer<>(numFrames));
			fdn.ProcessAudio(in, out, audioData);

			// Analyze Output Decay
			Real decayTime = 0.0;
			for (int i = 0; i < numReverbSources; i++)
				decayTime += CalculateT60(out[i], numFrames, config->GetData().fs);
			decayTime /= numReverbSources;
			Assert::IsTrue(decayTime > 0.0f, L"Decay not detected.");
			Assert::AreEqual(target, decayTime, (Real)0.02, L"Decay time does not match target RT60.");
		}

		TEST_METHOD(FeedbackMatrixIdentity)
		{
			const Real target = 0.56;
			const int fs = 48000;
			const int numFrames = static_cast<int>(fs * target);
			const int numReverbSources = 12;
			const int fdnSize = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const DSPData data(fs, numFrames, numReverbSources, fdnSize, lerpFactor, Q, fBands);
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(data);
			AudioData audioData(config);

			const Coefficients<> T60(std::vector<Real>({ target, target, target, target }));
			const Coefficients<> reflectionGains(std::vector<Real>({ 0.1, 0.05, 0.3, 0.25 }));

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec<> dimensions(std::vector<Real>({ 2.3, 1.5, 5.6 }));
			FDN<> fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(std::vector<Coefficients<>>(numReverbSources, reflectionGains));

			Matrix<> in = Matrix<>::Zero(numReverbSources, numFrames);
			in(0, 0) = 1.0;

			std::vector<Buffer<>> out(numReverbSources, Buffer<>(numFrames));
			fdn.ProcessAudio(in, out, audioData);

			Real sum = 0.0;
			for (int j = 0; j < numFrames; j++)
				sum += out[0][j] * out[0][j];
			Assert::AreNotEqual((Real)0.0, sum, L"Feedback matrix is not identity.");

			for (int i = 1; i < numReverbSources; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreEqual((Real)0.0, sum, L"Feedback matrix is not identity.");
			}
		}

		TEST_METHOD(FeedbackMatrixRandomOrthogonal)
		{
			const Real target = 0.56;
			const int fs = 48000;
			const int numFrames = static_cast<int>(fs * target);
			const int numReverbSources = 12;
			const int fdnSize = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const DSPData data(fs, numFrames, numReverbSources, fdnSize, lerpFactor, Q, fBands);
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(data);
			AudioData audioData(config);

			const Coefficients<> T60(std::vector<Real>({ target, target, target, target }));
			const Coefficients<> reflectionGains(std::vector<Real>({ 0.1, 0.05, 0.3, 0.25 }));

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec<> dimensions(std::vector<Real>({ 2.3, 1.5, 5.6 }));
			RandomOrthogonalFDN fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(std::vector<Coefficients<>>(numReverbSources, reflectionGains));

			Matrix<> in = Matrix<>::Zero(numReverbSources, numFrames);
			in(0, 0) = 1.0;

			std::vector<Buffer<>> out(numReverbSources, Buffer<>(numFrames));
			fdn.ProcessAudio(in, out, audioData);

			for (int i = 0; i < numReverbSources; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreNotEqual((Real)0.0, sum, L"Feedback matrix is not random orthogonal.");
			}
		}

		TEST_METHOD(FeedbackMatrixHouseHolder)
		{
			const Real target = 0.56;
			const int fs = 48000;
			const int numFrames = static_cast<int>(fs * target);
			const int numReverbSources = 12;
			const int fdnSize = 12;
			const Real lerpFactor = 1.0;
			const Real Q = 0.98;
			const std::vector<Real> fBands = { 500.0, 1000.0, 2000.0, 4000.0 };
			const DSPData data(fs, numFrames, numReverbSources, fdnSize, lerpFactor, Q, fBands);
			const std::shared_ptr<DSPConfig> config = std::make_shared<DSPConfig>(data);
			AudioData audioData(config);

			const Coefficients<> T60(std::vector<Real>({ target, target, target, target }));
			const Coefficients<> reflectionGains(std::vector<Real>({ 0.1, 0.05, 0.3, 0.25 }));

			// Long delay lines cause issues with the T60 estimation due to less frequent but larger drops in energy
			Vec<> dimensions(std::vector<Real>({ 2.3, 1.5, 5.6 }));
			HouseHolderFDN fdn(T60, dimensions, config);
			fdn.SetTargetReflectionFilters(std::vector<Coefficients<>>(numReverbSources, reflectionGains));

			Matrix<> in = Matrix<>::Zero(numReverbSources, numFrames);
			in(0, 0) = 1.0;

			std::vector<Buffer<>> out(numReverbSources, Buffer<>(numFrames));
			fdn.ProcessAudio(in, out, audioData);

			for (int i = 0; i < numReverbSources; i++)
			{
				Real sum = 0.0;
				for (int j = 0; j < numFrames; j++)
					sum += out[i][j] * out[i][j];
				Assert::AreNotEqual((Real)0.0, sum, L"Feedback matrix is not random orthogonal.");
			}
		}

		TEST_METHOD(DelayLineLengthAssignment)
		{
			const Real fs = 48000.0;
			const Real minDiffSeconds = 2e-4;
			const Real minLineSeconds = 3e-2;
			const Real maxLineSeconds = 3e-1;

			const int minDiff = static_cast<int>(minDiffSeconds * fs);
			const int minLine = static_cast<int>(minLineSeconds * fs);
			const int maxLine = static_cast<int>(maxLineSeconds * fs);

			for (int numFDNs = 1; numFDNs < 100; numFDNs *= 2)
			{
				for (int fdnSize = 6; fdnSize < 21; fdnSize++)
				{
					Matrix<int> delayLineLengthSets(numFDNs, fdnSize);
					Reverb::buildDelaySets(delayLineLengthSets, fs);

					for (int i = 0; i < numFDNs; i++)
					{
						for (int j = 0; j < fdnSize; j++)
						{
							Assert::AreNotEqual(0, delayLineLengthSets(i, j), L"Some delay line lengths were not initialized.");
							Assert::IsTrue(delayLineLengthSets(i, j) >= minLine, L"Some delay line lengths are too short.");
							Assert::IsTrue(delayLineLengthSets(i, j) <= maxLine, L"Some delay line lengths are too long.");
							// TODO: Check coprime pairs
							// TODO: Check pairs minDiff apart
							// TODO: Check no repeats overall
						}
					}
				}
			}
		}
	};
#pragma optimize("", on)
}