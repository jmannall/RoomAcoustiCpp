#include <catch2/catch_all.hpp>

#include "DSP/GraphicEQ.h"
#include "UtilityFunctions.h"

TEST_CASE("Second script test") {
    REQUIRE(2 + 5 == 7);
    REQUIRE(3 * 2 == 6);
}

using namespace RAC::Common;
using namespace RAC::DSP;

TEST_CASE("Graphic equaliser")
{
	std::string filePath = "C:/Documents/GitHub/jmannall/RoomAcoustiCpp/UnitTestData/";
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
			REQUIRE(Round(outputData[i][j], 13) == Round(out[j], 13));
	}
}

TEST_CASE("Peaking filter")
{
	std::string filePath = "C:/Documents/GitHub/jmannall/RoomAcoustiCpp/UnitTestData/";
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
			REQUIRE(Round(outputData[i][j], 13) == Round(out[j], 13));
	}
}

TEST_CASE("Low-shelf filter")
{
	std::string filePath = "C:/Documents/GitHub/jmannall/RoomAcoustiCpp/UnitTestData/";
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
			REQUIRE(Round(outputData[i][j], 13) == Round(out[j], 13));
	}
}

TEST_CASE("High-shelf filter")
{
	std::string filePath = "C:/Documents/GitHub/jmannall/RoomAcoustiCpp/UnitTestData/";
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
			REQUIRE(Round(outputData[i][j], 13) == Round(out[j], 13));
	}
}