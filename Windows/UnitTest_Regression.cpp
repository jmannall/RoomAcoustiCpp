
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include <ranges>

#include "Spatialiser/Interface.h"
#include "Unity/Debug.h"
#include "DSP/Interpolate.h"

#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"

#include <cmath>
#include <limits>
#include "CppUnitTest.h"
#include <windows.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace matlab::engine;
using namespace matlab::data;

namespace RAC
{
	using namespace Spatialiser;

#pragma optimize("", off)

	bool lateReverbCompleted = false;
	int iemCounter = 0;
	int sourceID = -1;

	struct IRResult
	{
		float impulseResponse;
		float frequencyResponse;
		float energyDecay;
	};

	using namespace Microsoft::VisualStudio::CppUnitTestFramework;

	bool AreFloatsEqual(double a, double b)
	{
		// Handle exact equality
		if (a == b) return true;

		double relativeTolerance = 1e-3;

		// Compute absolute and relative errors
		double diff = std::abs(a - b);
		double scale = std::max(std::abs(a), std::abs(b));

		// Use a mix of absolute and relative tolerance
		return diff <= (relativeTolerance * scale) || diff <= std::numeric_limits<double>::epsilon();
	}

	std::shared_ptr<MATLABEngine> LoadMATLAB()
	{
		HMODULE hMatlabEngine = LoadLibrary(L"C:\\Program Files\\MATLAB\\R2024a\\extern\\bin\\win64\\libMatlabEngine.dll");
		HMODULE hMatlabDataArray = LoadLibrary(L"C:\\Program Files\\MATLAB\\R2024a\\extern\\bin\\win64\\libMatlabDataArray.dll");

		if (!hMatlabEngine || !hMatlabDataArray) {
			std::cerr << "Failed to load MATLAB DLLs." << std::endl;
			return nullptr;
		}

		// Start MATLAB engine synchronously
		std::shared_ptr<MATLABEngine> matlabPtr = startMATLAB();

		// Add function file path to MATLAB
		std::u16string path = std::u16string(filePath.begin(), filePath.end());
		matlabPtr->eval(u"addpath('" + path + u"');");

		return matlabPtr;
	}

	IRResult AssessImpulseResponse(std::shared_ptr<MATLABEngine> matlabPtr, std::vector<float> currentIR, std::vector<float> targetIR, float fs)
	{
		// Create MATLAB data arrays
		ArrayFactory factory;
		TypedArray<float> matCurrentIR = factory.createArray({ 1, currentIR.size()}, currentIR.begin(), currentIR.end());
		TypedArray<float> matTargetIR = factory.createArray({ 1, targetIR.size()}, targetIR.begin(), targetIR.end());
		TypedArray<float> matFs = factory.createScalar(fs);

		std::vector<Array> args({ matCurrentIR, matTargetIR, matFs });
		Array results = matlabPtr->feval(u"AssessImpulseResponseSimilarity", args);

		return { results[0], results[1], results[2] };
	}

	void OnIEMCompleted(int id)
	{
		if (id == -1)
			lateReverbCompleted = true;
		else if (id == sourceID)
			iemCounter++;
	}

	std::vector<IRResult> RunScene(std::string scene, IEMConfig iemConfig, DiffractionModel diffractionModel)
	{
		std::vector<float> data = Parse2Dcsv<float>(filePath + "ImpulseResponses\\" + scene + "\\RAC\\Data.csv")[0];

		std::vector<std::vector<float>> sourceData = Parse2Dcsv<float>(filePath + "Scenes\\" + scene + "\\Sources.csv");
		std::vector<std::vector<std::string>> sourceNames = Parse2Dcsv<std::string>(filePath + "Scenes\\" + scene + "\\SourceNames.csv");

		std::vector<std::vector<float>> listenerData = Parse2Dcsv<float>(filePath + "Scenes\\" + scene + "\\Listeners.csv");
		std::vector<std::vector<std::string>> listenerNames = Parse2Dcsv<std::string>(filePath + "Scenes\\" + scene + "\\ListenerNames.csv");

		std::vector<std::vector<float>> verticesData = Parse2Dcsv<float>(filePath + "Scenes\\" + scene + "\\Vertices.csv");
		std::vector<std::vector<float>> absorptionData = Parse2Dcsv<float>(filePath + "Scenes\\" + scene + "\\Absorption.csv");

		Coefficients fBands = Coefficients(std::vector<double>(data.begin() + 7, data.end()));
		std::shared_ptr<Config> config = std::make_shared<Config>(data[0], data[1], data[2], data[3], data[4], fBands, diffractionModel, SpatialisationMode::none);

		std::shared_ptr<MATLABEngine> matlabPtr = LoadMATLAB();

		std::vector<IRResult> results;

		Init(config);
		UpdateIEMConfig(iemConfig);
		UpdateImpulseResponseMode(true);
		UpdateReverbTime(ReverbFormula::Sabine);
		bool filesLoaded = false;

		std::vector<size_t> wallIDs(verticesData.size());
		Vertices vertices;
		size_t absorptionIdx = 1;
		Absorption absorption = { std::vector<Real>(absorptionData[0].begin() + 1, absorptionData[0].end()) };

		size_t numWalls = verticesData.size();
		for (int i = 0; i < numWalls; i++)
		{
			vertices[0] = Vec3(verticesData[i][0], verticesData[i][1], verticesData[i][2]);
			vertices[1] = Vec3(verticesData[i][3], verticesData[i][4], verticesData[i][5]);
			vertices[2] = Vec3(verticesData[i][6], verticesData[i][7], verticesData[i][8]);

			if (absorptionIdx < absorptionData.size() && i == absorptionData[absorptionIdx][0])
			{
				absorption = { std::vector<Real>(absorptionData[absorptionIdx].begin() + 1, absorptionData[absorptionIdx].end()) };
				absorptionIdx++;
			}

			wallIDs[i] = InitWall(vertices, absorption);
		}

		InitLateReverb(145.0f, Vec({ 8.444f, 6.038f, 2.988f }), FDNMatrix::randomOrthogonal);
		UpdatePlanesAndEdges();

		std::vector<Real> input(config->numFrames);

		RegisterIEMCallback(OnIEMCompleted);

		std::vector<std::string> irFiles = ListDirectoryFiles(filePath + "ImpulseResponses\\" + scene + "\\RAC");

		std::vector<int> fileIndices(sourceNames.size());
		for (int i = 0; i < sourceNames.size(); i++)
		{
			for (int j = 0; j < irFiles.size(); j++)
			{
				if (irFiles[j].find(sourceNames[i][0]) != std::string::npos)
				{
					fileIndices[i] = j;
					break; // Stop searching once a match is found
				}
			}
		}

		for (int i = 0; i < sourceData.size(); i++)
		{
			int fileIdx = fileIndices[i];
			if (irFiles[fileIdx].find("Data.csv") != std::string::npos)
				continue;

			SpatialisationMode mode = SpatialisationMode::none;
			if (irFiles[fileIdx].find("Quality") != std::string::npos)
			{
				mode = SpatialisationMode::quality;
				if (!filesLoaded)
					filesLoaded = LoadSpatialisationFiles(data[6], { filePath + "Kemar_HRTF_ITD_48000_3dti-hrtf.3dti-hrtf", filePath + "NearFieldCompensation_ILD_48000.3dti-ild", filePath + "HRTF_ILD_48000.3dti-ild" });
			}
			else if (irFiles[fileIdx].find("Performance") != std::string::npos)
			{
				mode = SpatialisationMode::performance;
				if (!filesLoaded)
					filesLoaded = LoadSpatialisationFiles(data[6], { filePath + "Kemar_HRTF_ITD_48000_3dti-hrtf.3dti-hrtf", filePath + "NearFieldCompensation_ILD_48000.3dti-ild", filePath + "HRTF_ILD_48000.3dti-ild" });
			}
			UpdateSpatialisationMode(mode);

			std::vector<std::vector<float>> irData = Parse2Dcsv<float>(irFiles[fileIdx]);

			Vec3 sourcePosition = Vec3(sourceData[i][0], sourceData[i][1], sourceData[i][2]);
			Vec4 sourceOrientation = Vec4(sourceData[i][3], sourceData[i][4], sourceData[i][5], sourceData[i][6]);

			int fileCounter = 0;

			for (int j = 0; j < listenerData.size(); j++)
			{
				if (mode == SpatialisationMode::quality && listenerNames[j][0].find("Quality") == std::string::npos)
					continue;

				if (mode == SpatialisationMode::performance && listenerNames[j][0].find("Performance") == std::string::npos)
					continue;

				if (mode == SpatialisationMode::none && listenerNames[j][0].find("None") == std::string::npos)
					continue;

				int numBuffers = irData[fileCounter].size() / (2 * config->numFrames);
				if (mode == SpatialisationMode::none)
					numBuffers *= 2;

				Vec3 listenerPosition = Vec3(listenerData[j][0], listenerData[j][1], listenerData[j][2]);
				Vec4 listenerOrientation = Vec4(listenerData[j][3], listenerData[j][4], listenerData[j][5], listenerData[j][6]);
				UpdateListener(listenerPosition, listenerOrientation);
				sourceID = InitSource();
				UpdateSource(sourceID, sourcePosition, sourceOrientation);

				std::vector<float> output(irData[fileCounter].size());
				float* outputPtr = nullptr;

				lateReverbCompleted = false;
				iemCounter = 0;
				while (!lateReverbCompleted || iemCounter < 2) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }

				input[0] = 0.0;
				SubmitAudio(sourceID, input);

				input[0] = 1.0;

				int count = 0;
				for (int k = 0; k < numBuffers; k++)
				{
					SubmitAudio(sourceID, input);

					input[0] = 0.0;
					GetOutput(&outputPtr);

					for (int l = 0; l < config->numFrames; l++)
					{
						output[count++] = outputPtr[2 * l];
						if (mode != SpatialisationMode::none)
							output[count++] = outputPtr[2 * l + 1];
					}
				}
				results.push_back(AssessImpulseResponse(matlabPtr, irData[fileCounter], output, config->fs));

				RemoveSource(sourceID);
				fileCounter++;
			}
		}

		Exit();

		return results;
	}

	TEST_CLASS(Regression_Tests)
	{
	public:

		TEST_METHOD(CR2)
		{
			std::string scene = "CR2 small room (seminar room)";
			IEMConfig iemConfig(DirectSound::check, 3, 3, 1, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::btm);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				/*Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");*/
			}
		}

		TEST_METHOD(RS1)
		{
			std::string scene = "RS1 single reflection (infinite plate)";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::btm);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS2)
		{
			std::string scene = "RS2 single reflection (finite plate)";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::btm);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS3)
		{
			std::string scene = "RS3 multiple reflection (parallel finite plates)";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::btm);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS4)
		{
			std::string scene = "RS4 single reflection (reflector array)";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::btm);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS5)
		{
			std::string scene = "RS5 diffraction (infinite wedge)";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::btm);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS5_LPF)
		{
			std::string scene = "RS5 LPF";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::lowPass);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS5_NNBest)
		{
			std::string scene = "RS5 NNBest";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::nnBest);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS5_NNSmall)
		{
			std::string scene = "RS5 NNSmall";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::nnSmall);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS5_UDFA)
		{
			std::string scene = "RS5 UDFA";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::udfa);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS5_UDFAI)
		{
			std::string scene = "RS5 UDFAI";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::udfai);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS5_UTD)
		{
			std::string scene = "RS5 UTD";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::utd);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}

		TEST_METHOD(RS6)
		{
			std::string scene = "RS6 diffraction (finite body)";
			IEMConfig iemConfig(DirectSound::check, 6, 6, 6, true, 0.0);

			std::vector<IRResult> results = RunScene(scene, iemConfig, DiffractionModel::btm);
			for (IRResult result : results)
			{
				Assert::IsTrue(result.energyDecay < 3.0f, L"Energy Decay");
				Assert::IsTrue(result.frequencyResponse < 2.0f, L"Frequency Response");
				Assert::IsTrue(result.impulseResponse > 0.9f, L"Impulse Response");
			}
		}
	};
	
#pragma optimize("", on)
}