
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "DSP/OctaveBandFilter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace DSP;

	TEST_CLASS(OctaveBand_Class)
	{
	public:

		TEST_METHOD(ProcessAudio)
		{
			const int fs = 48000; // Sampling frequency
			const Real lerpFactor = REAL_CONST(1.0);

			// Coefficients<> gains({ REAL_CONST(0.3), REAL_CONST(0.8), REAL_CONST(0.65), REAL_CONST(0.9), REAL_CONST(0.1), REAL_CONST(0.05), REAL_CONST(0.4) });
			Coefficients<> gains(std::vector<Real>({ REAL_CONST(0.3), REAL_CONST(0.8), REAL_CONST(0.55), REAL_CONST(0.05) }));
			Coefficients<> frequencies(std::vector<Real>({ REAL_CONST(2e3), REAL_CONST(4e3), REAL_CONST(8e3), REAL_CONST(16e3) }));
			int numFrequencyBands = ToInt(frequencies.Length());
			OctaveBand filter = OctaveBand(frequencies, fs);

			std::vector<Real> target = { REAL_CONST(-3.2418096154715189e-10), REAL_CONST(6.9712509556169127e-25), REAL_CONST(3.5104324663071749e-09), REAL_CONST(-2.5222222901527198e-24), REAL_CONST(-8.4120071847774052e-09), REAL_CONST(-2.7707363124279565e-24), REAL_CONST(-6.4626830411486335e-09), REAL_CONST(2.0708923227680026e-23), REAL_CONST(4.3847942395212278e-09), REAL_CONST(-1.8727724410357487e-07), REAL_CONST(-3.6686424357081627e-07), REAL_CONST(4.0217776173310949e-22), REAL_CONST(9.7516024049674181e-07), REAL_CONST(2.0279541240305100e-06), REAL_CONST(2.1361880756320357e-06), REAL_CONST(-1.6097157804109926e-21), REAL_CONST(-3.5929272019840226e-06), REAL_CONST(-4.8595621267396538e-06), REAL_CONST(-2.4569797144299550e-06), REAL_CONST(-1.4667803678562170e-21), REAL_CONST(1.9330910023201963e-07), REAL_CONST(-3.7334501806799786e-06), REAL_CONST(-6.0360915733423490e-06), REAL_CONST(1.1985892718153569e-20), REAL_CONST(8.0501280769038435e-06), REAL_CONST(5.0511313945011273e-07), REAL_CONST(-4.2662392876418910e-05), REAL_CONST(-1.0818885227449929e-04), REAL_CONST(-1.7256726537625135e-04), REAL_CONST(-1.8997515524966815e-04), REAL_CONST(-1.3356280800328008e-04), REAL_CONST(2.1681909656963195e-19), REAL_CONST(2.1733648048555192e-04), REAL_CONST(5.1072140988077093e-04), REAL_CONST(8.6538142279256368e-04), REAL_CONST(1.1715359770184184e-03), REAL_CONST(1.3230964804721436e-03), REAL_CONST(1.1936339170534696e-03), REAL_CONST(7.3899624296031681e-04), REAL_CONST(-8.0181617978282504e-19), REAL_CONST(-9.6798367481103252e-04), REAL_CONST(-2.0632536045776359e-03), REAL_CONST(-3.1938517602984342e-03), REAL_CONST(-3.9788735772973826e-03), REAL_CONST(-4.1589138084615045e-03), REAL_CONST(-3.6593212126752794e-03), REAL_CONST(-2.3441400173670815e-03), REAL_CONST(1.7818888649869454e-18), REAL_CONST(3.3143560238833481e-03), REAL_CONST(6.9923381648344758e-03), REAL_CONST(9.8445787014842870e-03), REAL_CONST(1.0529327977948302e-02), REAL_CONST(9.0039869362184225e-03), REAL_CONST(5.5968290794902184e-03), REAL_CONST(2.8746586978076845e-03), REAL_CONST(-1.5699167265245384e-18), REAL_CONST(-6.8732620319994844e-03), REAL_CONST(-7.1991576025509079e-03), REAL_CONST(-2.7802081699781021e-03), REAL_CONST(-3.8815036102503364e-02), REAL_CONST(-1.0818537758617623e-01), REAL_CONST(-9.4730582180478445e-02), REAL_CONST(3.8512145400356919e-02), REAL_CONST(4.2499999999999993e-01), REAL_CONST(3.8512145400356912e-02), REAL_CONST(-9.4730582180478473e-02), REAL_CONST(-1.0818537758617626e-01), REAL_CONST(-3.8815036102503364e-02), REAL_CONST(-2.7802081699781021e-03), REAL_CONST(-7.1991576025509079e-03), REAL_CONST(-6.8732620319994827e-03), REAL_CONST(-1.5699167265245384e-18), REAL_CONST(2.8746586978076845e-03), REAL_CONST(5.5968290794902193e-03), REAL_CONST(9.0039869362184260e-03), REAL_CONST(1.0529327977948302e-02), REAL_CONST(9.8445787014842835e-03), REAL_CONST(6.9923381648344758e-03), REAL_CONST(3.3143560238833476e-03), REAL_CONST(1.7818888649869457e-18), REAL_CONST(-2.3441400173670815e-03), REAL_CONST(-3.6593212126752785e-03), REAL_CONST(-4.1589138084615045e-03), REAL_CONST(-3.9788735772973826e-03), REAL_CONST(-3.1938517602984342e-03), REAL_CONST(-2.0632536045776359e-03), REAL_CONST(-9.6798367481103274e-04), REAL_CONST(-8.0181617978282484e-19), REAL_CONST(7.3899624296031691e-04), REAL_CONST(1.1936339170534691e-03), REAL_CONST(1.3230964804721436e-03), REAL_CONST(1.1715359770184184e-03), REAL_CONST(8.6538142279256357e-04), REAL_CONST(5.1072140988077082e-04), REAL_CONST(2.1733648048555189e-04), REAL_CONST(2.1681909656963197e-19), REAL_CONST(-1.3356280800328005e-04), REAL_CONST(-1.8997515524966810e-04), REAL_CONST(-1.7256726537625137e-04), REAL_CONST(-1.0818885227449929e-04), REAL_CONST(-4.2662392876418910e-05), REAL_CONST(5.0511313945011284e-07), REAL_CONST(8.0501280769038418e-06), REAL_CONST(1.1985892718153566e-20), REAL_CONST(-6.0360915733423516e-06), REAL_CONST(-3.7334501806799778e-06), REAL_CONST(1.9330910023202116e-07), REAL_CONST(-1.4667803678562185e-21), REAL_CONST(-2.4569797144299554e-06), REAL_CONST(-4.8595621267396538e-06), REAL_CONST(-3.5929272019840217e-06), REAL_CONST(-1.6097157804109926e-21), REAL_CONST(2.1361880756320357e-06), REAL_CONST(2.0279541240305100e-06), REAL_CONST(9.7516024049674181e-07), REAL_CONST(4.0217776173310954e-22), REAL_CONST(-3.6686424357081638e-07), REAL_CONST(-1.8727724410357487e-07), REAL_CONST(4.3847942395212204e-09), REAL_CONST(2.0708923227680026e-23), REAL_CONST(-6.4626830411486269e-09), REAL_CONST(-2.7707363124279568e-24), REAL_CONST(-8.4120071847774069e-09), REAL_CONST(-2.5222222901527198e-24), REAL_CONST(3.5104324663071749e-09), REAL_CONST(6.9712509556169127e-25), REAL_CONST(-3.2418096154715189e-10) };

			std::vector<Real> output(target.size(), REAL_CONST(0.0)); // Initialize output vector with zeros
			std::vector<std::vector<Real>> outputs(numFrequencyBands, std::vector<Real>(127)); // Initialize output vector with zeros
			const Buffer<>& out = filter.GetOutput(1.0, lerpFactor); // Get the output from the filter
			Buffer<> bands = out;
			for (int j = 0; j < numFrequencyBands; j++)
			{
				bands[j] *= gains[j];
				outputs[j][0] = bands[j];
				output[0] += bands[j];
			}

			Assert::AreEqual(target[0], output[0], EPS_TEST_MEDIUM, L"Wrong output");
			for (int i = 1; i < target.size(); i++)
			{
				const Buffer<>& out = filter.GetOutput(0.0, lerpFactor); // Get the output from the filter
				Buffer<> bands = out;
				for (int j = 0; j < numFrequencyBands; j++)
				{
					bands[j] *= gains[j];
					outputs[j][i] = bands[j];
					output[i] += bands[j];
				}
				Assert::AreEqual(target[i], output[i], EPS_TEST_MEDIUM, L"Wrong output");
			}
		}

		TEST_METHOD(CombineBands)
		{
			const int fs = 48000; // Sampling frequency
			const Real lerpFactor = REAL_CONST(1.0);

			Coefficients<> frequencies(std::vector<Real>({ REAL_CONST(1e3), REAL_CONST(2e3), REAL_CONST(4e3), REAL_CONST(8e3), REAL_CONST(16e3) }));
			Coefficients<> shorterFrequencies(std::vector<Real>({ REAL_CONST(1e3), REAL_CONST(2e3), REAL_CONST(4e3) }));

			int numFrequencyBands = ToInt(frequencies.Length());
			OctaveBand filter = OctaveBand(frequencies, fs);

			int numSamples = 128;
			std::vector<std::vector<Real>> allBands(frequencies.Length(), std::vector<Real>(numSamples)); // Initialize output vector with zeros
			
			auto output = filter.GetOutput(1.0, lerpFactor); // Get the output from the filter
			for (int i = 0; i < frequencies.Length(); i++)
				 allBands[i][0] = output[i];
			for (int j = 1; j < numSamples; j++)
			{
				output = filter.GetOutput(0.0, lerpFactor);
				for (int i = 0; i < frequencies.Length(); i++)
					allBands[i][j] = output[i];
			}

			OctaveBand shorterFilter = OctaveBand(shorterFrequencies, fs);

			std::vector<std::vector<Real>> bands(shorterFrequencies.Length(), std::vector<Real>(numSamples)); // Initialize output vector with zeros

			output = shorterFilter.GetOutput(1.0, lerpFactor); // Get the output from the filter
			for (int i = 0; i < shorterFrequencies.Length(); i++)
				bands[i][0] = output[i];
			for (int j = 1; j < numSamples; j++)
			{
				output = shorterFilter.GetOutput(0.0, lerpFactor);
				for (int i = 0; i < shorterFrequencies.Length(); i++)
					bands[i][j] = output[i];
			}

			std::vector<Real> combinedBands(numSamples, 0.0);
			for (int i = 0; i < numSamples; i++)
			{
				for (int j = 0; j < frequencies.Length() - shorterFrequencies.Length() + 1; j++)
					combinedBands[i] += allBands[j][i];
			}

			for (int i = 0; i < numSamples; i++)
				Assert::AreEqual(bands[0][i], combinedBands[i], EPS_TEST_ACCURATE, L"Wrong output");
		}

		TEST_METHOD(FrequencyIndexing)
		{
			const int fs = 48000; // Sampling frequency
			const Real lerpFactor = REAL_CONST(1.0);

			Coefficients<> frequencies(std::vector<Real>({ REAL_CONST(31.25), REAL_CONST(62.5), REAL_CONST(125.0), REAL_CONST(250.0), REAL_CONST(500.0), REAL_CONST(1e3), REAL_CONST(2e3), REAL_CONST(4e3), REAL_CONST(8e3), REAL_CONST(16e3) }));
			Coefficients<> mixedFrequencies(std::vector<Real>({ REAL_CONST(8e3), REAL_CONST(500.0), REAL_CONST(2e3), REAL_CONST(16e3), REAL_CONST(4e3), REAL_CONST(1e3) }));
			Coefficients<> shortFrequencies(std::vector<Real>({ REAL_CONST(1e3), REAL_CONST(2e3), REAL_CONST(4e3) }));
			std::vector<int> freqIndices = { 3, 1, 4, 2, 0 };

			OctaveBand filter = OctaveBand(frequencies, fs);
			OctaveBand mixedFilter = OctaveBand(mixedFrequencies, fs);
			OctaveBand shortFilter = OctaveBand(shortFrequencies, fs);

			const Buffer<>& output1 = filter.GetOutput(REAL_CONST(1.0), lerpFactor);
			Assert::AreEqual((int)frequencies.Length(), (int)output1.Length(), L"Wrong number of bands");

			Assert::AreEqual(9, filter.GetBandIndex(0), L"Wrong 31Hz");
			Assert::AreEqual(8, filter.GetBandIndex(1), L"Wrong 62kHz");
			Assert::AreEqual(7, filter.GetBandIndex(2), L"Wrong 125Hz");
			Assert::AreEqual(6, filter.GetBandIndex(3), L"Wrong 250Hz");
			Assert::AreEqual(5, filter.GetBandIndex(4), L"Wrong 500Hz");
			Assert::AreEqual(4, filter.GetBandIndex(5), L"Wrong 1kHz");
			Assert::AreEqual(3, filter.GetBandIndex(6), L"Wrong 2kHz");
			Assert::AreEqual(2, filter.GetBandIndex(7), L"Wrong 4kHz");
			Assert::AreEqual(1, filter.GetBandIndex(8), L"Wrong 8kHz");
			Assert::AreEqual(0, filter.GetBandIndex(9), L"Wrong 16kHz");

			const Buffer<>& output2 = mixedFilter.GetOutput(1.0, lerpFactor);
			Assert::AreEqual((int)mixedFrequencies.Length(), (int)output2.Length(), L"Wrong number of bands (mixed)");

			Assert::AreEqual(1, mixedFilter.GetBandIndex(0), L"Wrong 500 Hz (mixed)");
			Assert::AreEqual(5, mixedFilter.GetBandIndex(1), L"Wrong 1kHz (mixed)");
			Assert::AreEqual(3, mixedFilter.GetBandIndex(2), L"Wrong 2kHz (mixed)");
			Assert::AreEqual(0, mixedFilter.GetBandIndex(3), L"Wrong 4kHz (mixed)");
			Assert::AreEqual(2, mixedFilter.GetBandIndex(4), L"Wrong 8kHz (mixed)");
			Assert::AreEqual(4, mixedFilter.GetBandIndex(5), L"Wrong 16kHz (mixed)");

			const Buffer<>& output3 = shortFilter.GetOutput(REAL_CONST(1.0), lerpFactor);
			Assert::AreEqual((int)shortFrequencies.Length(), (int)output3.Length(), L"Wrong number of bands (short)");

			Assert::AreEqual(2, shortFilter.GetBandIndex(0), L"Wrong 1kHz (short)");
			Assert::AreEqual(1, shortFilter.GetBandIndex(1), L"Wrong 2kHz (short)");
			Assert::AreEqual(0, shortFilter.GetBandIndex(2), L"Wrong 4kHz (short)");
		}

		TEST_METHOD(SingleBand)
		{
			const int fs = 48000;
			const Real lerpFactor = REAL_CONST(1.0);

			Coefficients<> frequencies = Coefficients<>::Constant(1, REAL_CONST(2e3));
			OctaveBand filter = OctaveBand(frequencies, fs);
			Assert::IsFalse(filter.IsValid(), L"Filter should be invalid due to single band");
		}
	};
}