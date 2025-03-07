
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Diffraction/Models.h"
#include "Diffraction/Path.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser::Diffraction;

#pragma optimize("", off)

	Path CreatePath(Real rS, Real rR, Real tS, Real tR, Real tW, Real zS, Real zR, Real zW)
	{
		tS = Deg2Rad(tS);
		tR = Deg2Rad(tR);
		tW = Deg2Rad(tW);

		Vec3 base = Vec3(0.0, 0.0, 0.0);
		Vec3 top = Vec3(0.0, zW, 0.0);
		Vec3 normal1 = Vec3(sin(tW), 0.0, -cos(tW));
		Vec3 normal2 = Vec3(0.0, 0.0, 1.0);

		Edge e = Edge(base, top, normal1, normal2, 0, 1, 0, 1);

		Vec3 s = Vec3(rS * cos(tS), zS, rS * sin(tS));
		Vec3 r = Vec3(rR * cos(tR), zR, rR * sin(tR));

		return Path(s, r, e);
	}

	void UpdateDiffractionModel(DiffractionModel model)
	{
		int fs = 48000;
		std::string filePath = _SOLUTIONDIR;
		auto data = Parse2Dcsv<double>(filePath + "UnitTestData\\diffractionPaths.csv");

		std::vector<Real> zW(data[0]);
		std::vector<Real> tW(data[1]);
		std::vector<Real> tS(data[2]);
		std::vector<Real> tR(data[3]);
		std::vector<Real> rS(data[4]);
		std::vector<Real> rR(data[5]);
		std::vector<Real> zS(data[6]);
		std::vector<Real> zR(data[7]);

		std::vector<Path> paths;

		Buffer ir = Buffer(0);
		for (int i = 0; i < zW.size(); i++)
		{
			paths.emplace_back(CreatePath(rS[i], rR[i], tS[i], tR[i], tW[i], zS[i], zR[i], zW[i]));
		}

		Model* m = nullptr;

		switch (model)
		{
		case DiffractionModel::attenuate:
			m = new Attenuate(&paths[1]);
			break;
		case DiffractionModel::lowPass:
			m = new LPF(&paths[1], fs);
			break;
		case DiffractionModel::btm:
			m = new BTM(&paths[1], fs);
			break;
		case DiffractionModel::udfa:
			m = new UDFA(&paths[1], fs);
			break;
		case DiffractionModel::udfai:
			m = new UDFAI(&paths[1], fs);
			break;
		case DiffractionModel::nnSmall:
			m = new NNSmall(&paths[1]);
			break;
		case DiffractionModel::nnBest:
			m = new NNBest(&paths[1]);
			break;
		case DiffractionModel::utd:
			m = new UTD(&paths[1], fs);
			break;
		}

		for (int j = 0; j < 1e4; j++)
		{
			for (int i = 0; i < paths.size(); i++)
			{
				m->UpdatePath(&paths[i]);
				m->UpdateParameters();
			}
		}
	}

	void ProcessDiffractionModel(DiffractionModel model)
	{
		int fs = 48000;
		std::string filePath = _SOLUTIONDIR;
		auto data = Parse2Dcsv<double>(filePath + "UnitTestData\\diffractionPaths.csv");

		std::vector<Real> zW(data[0]);
		std::vector<Real> tW(data[1]);
		std::vector<Real> tS(data[2]);
		std::vector<Real> tR(data[3]);
		std::vector<Real> rS(data[4]);
		std::vector<Real> rR(data[5]);
		std::vector<Real> zS(data[6]);
		std::vector<Real> zR(data[7]);

		std::vector<Path> paths;

		Buffer ir = Buffer(0);
		for (int i = 0; i < zW.size(); i++)
		{
			paths.emplace_back(CreatePath(rS[i], rR[i], tS[i], tR[i], tW[i], zS[i], zR[i], zW[i]));
		}

		Model* m = nullptr;

		switch (model)
		{
		case DiffractionModel::attenuate:
			m = new Attenuate(&paths[1]);
			break;
		case DiffractionModel::lowPass:
			m = new LPF(&paths[1], fs);
			break;
		case DiffractionModel::btm:
			m = new BTM(&paths[1], fs);
			break;
		case DiffractionModel::udfa:
			m = new UDFA(&paths[1], fs);
			break;
		case DiffractionModel::udfai:
			m = new UDFAI(&paths[1], fs);
			break;
		case DiffractionModel::nnSmall:
			m = new NNSmall(&paths[1]);
			break;
		case DiffractionModel::nnBest:
			m = new NNBest(&paths[1]);
			break;
		case DiffractionModel::utd:
			m = new UTD(&paths[1], fs);
			break;
		}

		int numFrames = 2048;
		Buffer in = Buffer(numFrames);
		in[0] = 1.0;
		Buffer out = Buffer(2 * numFrames);
		Real lerp = 2.0 / static_cast<Real>(numFrames);

		for (int i = 0; i < paths.size(); i++)
		{
			m->UpdatePath(&paths[i]);
			for (int j = 0; j < 1e3; j++)
			{
				m->ProcessAudio(in, out, numFrames, lerp);
			}
		}
	}

	TEST_CLASS(DiffractionModel_Classes_Timing)
	{
	public:

		TEST_METHOD(UpdateAttenuate) { UpdateDiffractionModel(DiffractionModel::attenuate); }

		TEST_METHOD(UpdateLPF) { UpdateDiffractionModel(DiffractionModel::lowPass); }

		TEST_METHOD(UpdateBTM) { UpdateDiffractionModel(DiffractionModel::btm); }

		TEST_METHOD(UpdateUDFA) { UpdateDiffractionModel(DiffractionModel::udfa); }

		TEST_METHOD(UpdateUDFAI) { UpdateDiffractionModel(DiffractionModel::udfai); }

		TEST_METHOD(UpdateNNSmall) { UpdateDiffractionModel(DiffractionModel::nnSmall); }

		TEST_METHOD(UpdateNNBest) { UpdateDiffractionModel(DiffractionModel::nnBest); }

		TEST_METHOD(UpdateUTD) { UpdateDiffractionModel(DiffractionModel::utd); }

		TEST_METHOD(ProcessAttenuate) { ProcessDiffractionModel(DiffractionModel::attenuate); }

		TEST_METHOD(ProcessLPF) { ProcessDiffractionModel(DiffractionModel::lowPass); }

		TEST_METHOD(ProcessBTM) { ProcessDiffractionModel(DiffractionModel::btm); }

		TEST_METHOD(ProcessUDFA) { ProcessDiffractionModel(DiffractionModel::udfa); }

		TEST_METHOD(ProcessUDFAI) { ProcessDiffractionModel(DiffractionModel::udfai); }

		TEST_METHOD(ProcessNNSmall) { ProcessDiffractionModel(DiffractionModel::nnSmall); }

		TEST_METHOD(ProcessNNBest) { ProcessDiffractionModel(DiffractionModel::nnBest); }

		TEST_METHOD(ProcessUTD) { ProcessDiffractionModel(DiffractionModel::utd); }
	};

	TEST_CLASS(DiffractionModel_Classes)
	{
	public:

		TEST_METHOD(BTM_Class)
		{
			int fs = 48000;
			std::string filePath = _SOLUTIONDIR;
			auto data = Parse2Dcsv<double>(filePath + "UnitTestData\\diffractionPaths.csv");

			std::vector<Real> zW(data[0]);
			std::vector<Real> tW(data[1]);
			std::vector<Real> tS(data[2]);
			std::vector<Real> tR(data[3]);
			std::vector<Real> rS(data[4]);
			std::vector<Real> rR(data[5]);
			std::vector<Real> zS(data[6]);
			std::vector<Real> zR(data[7]);

			Buffer ir = Buffer(0);
			for (int i = 0; i < zW.size(); i++)
			{
				Path path = CreatePath(rS[i], rR[i], tS[i], tR[i], tW[i], zS[i], zR[i], zW[i]);
				BTM btm = BTM(&path, fs);
				btm.AddIr(ir);
			}

			auto csv = Parse1Dcsv<double>(filePath + "UnitTestData\\btm.csv");
			for (int i = 0; i < ir.Length(); i++)
			{
				std::string error = "Incorrect Sample: " + ToStr(i);
				std::wstring werror = std::wstring(error.begin(), error.end());
				const wchar_t* werrorchar = werror.c_str();
				Real tolerance = csv[i] * 0.1;
				Assert::AreEqual(csv[i], ir[i], tolerance, werrorchar);
				Assert::AreEqual(csv[i], ir[i], 0.00006, werrorchar);
			}
		}
	};
#pragma optimize("", on)
}