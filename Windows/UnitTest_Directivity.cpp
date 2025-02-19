
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Spatialiser/Directivity.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

#pragma optimize("", off)

	TEST_CLASS(Directivity_Class)
	{
	public:

		TEST_METHOD(GenelecDirectivity)
		{
			auto inputData = Parse2Dcsv(filePath + "genelecDirectivityInput.csv");
			auto outputData = Parse2Dcsv(filePath + "genelecDirectivityOutput.csv");
			auto inputFreq = Parse2Dcsv(filePath + "directivityFreq.csv");

			std::vector<Real> theta(inputData[0]);
			std::vector<Real> phi(inputData[1]);
			std::vector<Real> freq(inputFreq[0]);

			int numTests = theta.size();
			for (int i = 1; i < numTests; i++)
			{
				Absorption directivity = GENELEC.Response(freq, theta[i], phi[i]);

				for (int j = 0; j < freq.size(); j++)
				{
					std::string error = "Test: " + IntToStr(i) + ", Incorrect Frequency : " + DoubleToStr(freq[j]);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], directivity[j], 10e-15, werrorchar);
				}
			}
		}
	};
#pragma optimize("", on)
}