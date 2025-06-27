
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
			m = new Attenuate(paths[1]);
			break;
		case DiffractionModel::lowPass:
			m = new LPF(paths[1], fs);
			break;
		case DiffractionModel::btm:
			m = new BTM(paths[1], fs);
			break;
		case DiffractionModel::udfa:
			m = new UDFA(paths[1], fs);
			break;
		case DiffractionModel::udfai:
			m = new UDFAI(paths[1], fs);
			break;
		case DiffractionModel::nnSmall:
			m = new NNSmall(paths[1]);
			break;
		case DiffractionModel::nnBest:
			m = new NNBest(paths[1]);
			break;
		case DiffractionModel::utd:
			m = new UTD(paths[1], fs);
			break;
		}

		for (int j = 0; j < 1e4; j++)
		{
			for (int i = 0; i < paths.size(); i++)
				m->SetTargetParameters(paths[i]);
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
			m = new Attenuate(paths[1]);
			break;
		case DiffractionModel::lowPass:
			m = new LPF(paths[1], fs);
			break;
		case DiffractionModel::btm:
			m = new BTM(paths[1], fs);
			break;
		case DiffractionModel::udfa:
			m = new UDFA(paths[1], fs);
			break;
		case DiffractionModel::udfai:
			m = new UDFAI(paths[1], fs);
			break;
		case DiffractionModel::nnSmall:
			m = new NNSmall(paths[1]);
			break;
		case DiffractionModel::nnBest:
			m = new NNBest(paths[1]);
			break;
		case DiffractionModel::utd:
			m = new UTD(paths[1], fs);
			break;
		}

		int numFrames = 2048;
		Buffer in = Buffer(numFrames);
		in[0] = 1.0;
		Buffer out = Buffer(2 * numFrames);
		Real lerp = 2.0 / static_cast<Real>(numFrames);

		for (int i = 0; i < paths.size(); i++)
		{
			m->SetTargetParameters(paths[i]);
			for (int j = 0; j < 1e3; j++)
			{
				m->ProcessAudio(in, out, lerp);
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

	TEST_CLASS(Attenuate_Class)
	{
	public:

		TEST_METHOD(Shadowed)
		{
			Real zW = 7.0;
			Real tW = 270.0;
			Real tS = 15.0;
			Real tR = 250.0;
			Real rS = 1.0;
			Real rR = 2.0;
			Real zS = 1.0;
			Real zR = 2.0;

			Real lerpFactor = 0.1;
			int numFrames = 1024;
			Path path = CreatePath(rS, rR, tS, tR, tW, zS, zR, zW);
			Attenuate attenuate(path);

			Buffer in(numFrames);
			for (int i = 0; i < numFrames; i++)
				in[i] = RandomValue();
			Buffer output(numFrames);
			attenuate.ProcessAudio(in, output, lerpFactor);

			for (int i = 0; i < numFrames; i++)
			{
				std::string error = "Failed Sample: " + ToStr(i);
				std::wstring werror = std::wstring(error.begin(), error.end());
				const wchar_t* werrorchar = werror.c_str();
				Assert::AreEqual(in[i], output[i], werrorchar);
			}
		}

		TEST_METHOD(NonShadowed)
		{
			Real zW = 7.0;
			Real tW = 270.0;
			Real tS = 30.0;
			Real tR = 90.0;
			Real rS = 1.0;
			Real rR = 2.0;
			Real zS = 1.0;
			Real zR = 2.0;

			Real lerpFactor = 0.1;
			int numFrames = 1024;
			Path path = CreatePath(rS, rR, tS, tR, tW, zS, zR, zW);
			Attenuate attenuate(path);

			Buffer in(numFrames);
			for (int i = 0; i < numFrames; i++)
				in[i] = RandomValue();
			Buffer output(numFrames);
			attenuate.ProcessAudio(in, output, lerpFactor);

			for (int i = 0; i < numFrames; i++)
			{
				std::string error = "Failed Sample: " + ToStr(i);
				std::wstring werror = std::wstring(error.begin(), error.end());
				const wchar_t* werrorchar = werror.c_str();
				Assert::AreEqual(0.0, output[i], werrorchar);
			}
		}
	};

	TEST_CLASS(LPF_Class)
	{
	public:

		TEST_METHOD(Shadowed)
		{
			Real zW = 7.0;
			Real tW = 270.0;
			Real tS = 15.0;
			Real tR = 250.0;
			Real rS = 1.0;
			Real rR = 2.0;
			Real zS = 1.0;
			Real zR = 2.0;

			Real lerpFactor = 0.1;
			int numFrames = 1024;
			int fs = 48e3;
			Path path = CreatePath(rS, rR, tS, tR, tW, zS, zR, zW);
			LPF lpf(path, fs);

			Buffer in(numFrames);
			for (int i = 0; i < numFrames; i++)
				in[i] = RandomValue();
			Buffer output(numFrames);
			lpf.ProcessAudio(in, output, lerpFactor);

			Real fc = 1000.0;
			LowPass1 filter(fc, fs);

			for (int i = 0; i < numFrames; i++)
			{
				std::string error = "Failed Sample: " + ToStr(i);
				std::wstring werror = std::wstring(error.begin(), error.end());
				const wchar_t* werrorchar = werror.c_str();
				Assert::AreEqual(filter.GetOutput(in[i], lerpFactor), output[i], werrorchar);
			}
		}

		TEST_METHOD(NonShadowed)
		{
			Real zW = 7.0;
			Real tW = 270.0;
			Real tS = 30.0;
			Real tR = 90.0;
			Real rS = 1.0;
			Real rR = 2.0;
			Real zS = 1.0;
			Real zR = 2.0;

			Real lerpFactor = 0.1;
			int numFrames = 1024;
			int fs = 48e3;
			Path path = CreatePath(rS, rR, tS, tR, tW, zS, zR, zW);
			LPF lpf(path, fs);

			Buffer in(numFrames);
			for (int i = 0; i < numFrames; i++)
				in[i] = RandomValue();
			Buffer output(numFrames);
			lpf.ProcessAudio(in, output, lerpFactor);

			for (int i = 0; i < numFrames; i++)
			{
				std::string error = "Failed Sample: " + ToStr(i);
				std::wstring werror = std::wstring(error.begin(), error.end());
				const wchar_t* werrorchar = werror.c_str();
				Assert::AreEqual(0.0, output[i], werrorchar);
			}
		}
	};

	TEST_CLASS(BTM_Class)
	{
	public:

		TEST_METHOD(Process)
		{
			int fs = 48000;
			std::string filePath = _SOLUTIONDIR;
			auto data = Parse2Dcsv<double>(filePath + "UnitTestData\\diffractionPaths.csv");
			auto target = Parse2Dcsv<double>(filePath + "UnitTestData\\BTM.csv");

			const int numFrames = target[0].size();
			const int numTests = target.size();
			Real lerpFactor = 0.1;

			Buffer in(numFrames);
			in[0] = 1.0;
			std::vector<Buffer> outputs(numTests, Buffer(numFrames));

			std::vector<Real> zW(data[0]);
			std::vector<Real> tW(data[1]);
			std::vector<Real> tS(data[2]);
			std::vector<Real> tR(data[3]);
			std::vector<Real> rS(data[4]);
			std::vector<Real> rR(data[5]);
			std::vector<Real> zS(data[6]);
			std::vector<Real> zR(data[7]);

			for (int i = 0; i < numTests; i++)
			{
				Path path = CreatePath(rS[i], rR[i], tS[i], tR[i], tW[i], zS[i], zR[i], zW[i]);
				BTM btm = BTM(path, fs);
				btm.ProcessAudio(in, outputs[i], lerpFactor);
			}

			for (int i = 0; i < numTests; i++)
			{
				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Failed Test: " + ToStr(i) + ", Sample: " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Real tolerance = target[i][j] * 1e-6;
					Assert::AreEqual(target[i][j], outputs[i][j], tolerance, werrorchar);
					Assert::AreEqual(target[i][j], outputs[i][j], 1e-8, werrorchar);
				}
			}
		}
	};

	TEST_CLASS(UTD_Class)
	{
	public:

		TEST_METHOD(Process)
		{
			int fs = 48000;
			std::string filePath = _SOLUTIONDIR;
			auto data = Parse2Dcsv<double>(filePath + "UnitTestData\\diffractionPaths.csv");
			auto target = Parse2Dcsv<double>(filePath + "UnitTestData\\UTD.csv");

			const int numFrames = target[0].size();
			const int numTests = target.size();
			Real lerpFactor = 0.1;

			Buffer in(numFrames);
			in[0] = 1.0;
			std::vector<Buffer> outputs(numTests, Buffer(numFrames));

			std::vector<Real> zW(data[0]);
			std::vector<Real> tW(data[1]);
			std::vector<Real> tS(data[2]);
			std::vector<Real> tR(data[3]);
			std::vector<Real> rS(data[4]);
			std::vector<Real> rR(data[5]);
			std::vector<Real> zS(data[6]);
			std::vector<Real> zR(data[7]);

			for (int i = 0; i < numTests; i++)
			{
				Path path = CreatePath(rS[i], rR[i], tS[i], tR[i], tW[i], zS[i], zR[i], zW[i]);
				UTD utd = UTD(path, fs);
				utd.ProcessAudio(in, outputs[i], lerpFactor);
			}

			for (int i = 0; i < numTests; i++)
			{
				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Failed Test: " + ToStr(i) + ", Sample: " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Real tolerance = target[i][j] * 0.1;
					Assert::AreEqual(target[i][j], outputs[i][j], tolerance, werrorchar);
					Assert::AreEqual(target[i][j], outputs[i][j], 1e-5, werrorchar);
				}
			}
		}
	};

	TEST_CLASS(UDFAI_Class)
	{
	public:

		TEST_METHOD(Process)
		{
			int fs = 48000;
			std::string filePath = _SOLUTIONDIR;
			auto data = Parse2Dcsv<double>(filePath + "UnitTestData\\diffractionPaths.csv");
			auto target = Parse2Dcsv<double>(filePath + "UnitTestData\\UDFAI.csv");

			const int numFrames = target[0].size();
			const int numTests = target.size();
			Real lerpFactor = 0.1;

			Buffer in(numFrames);
			in[0] = 1.0;
			std::vector<Buffer> outputs(numTests, Buffer(numFrames));

			std::vector<Real> zW(data[0]);
			std::vector<Real> tW(data[1]);
			std::vector<Real> tS(data[2]);
			std::vector<Real> tR(data[3]);
			std::vector<Real> rS(data[4]);
			std::vector<Real> rR(data[5]);
			std::vector<Real> zS(data[6]);
			std::vector<Real> zR(data[7]);

			for (int i = 0; i < numTests; i++)
			{
				Path path = CreatePath(rS[i], rR[i], tS[i], tR[i], tW[i], zS[i], zR[i], zW[i]);
				UDFAI udfai = UDFAI(path, fs);
				// udfai.SetTargetParameters(path);
				udfai.ProcessAudio(in, outputs[i], lerpFactor);
			}

			for (int i = 0; i < numTests; i++)
			{
				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Failed Test: " + ToStr(i) + ", Sample: " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Real tolerance = target[i][j] * 0.1;
					Assert::AreEqual(target[i][j], outputs[i][j], tolerance, werrorchar);
					Assert::AreEqual(target[i][j], outputs[i][j], 1e-5, werrorchar);
				}
			}
		}
	};

	TEST_CLASS(UDFA_Class)
	{
	public:

		TEST_METHOD(Process)
		{
			int fs = 48000;
			std::string filePath = _SOLUTIONDIR;
			auto data = Parse2Dcsv<double>(filePath + "UnitTestData\\diffractionPaths.csv");
			auto target = Parse2Dcsv<double>(filePath + "UnitTestData\\UDFA.csv");

			const int numFrames = target[0].size();
			const int numTests = target.size();
			Real lerpFactor = 0.1;

			Buffer in(numFrames);
			in[0] = 1.0;
			std::vector<Buffer> outputs(numTests, Buffer(numFrames));

			std::vector<Real> zW(data[0]);
			std::vector<Real> tW(data[1]);
			std::vector<Real> tS(data[2]);
			std::vector<Real> tR(data[3]);
			std::vector<Real> rS(data[4]);
			std::vector<Real> rR(data[5]);
			std::vector<Real> zS(data[6]);
			std::vector<Real> zR(data[7]);

			for (int i = 0; i < numTests; i++)
			{
				Path path = CreatePath(rS[i], rR[i], tS[i], tR[i], tW[i], zS[i], zR[i], zW[i]);
				UDFA udfa = UDFA(path, fs);
				// udfai.SetTargetParameters(path);
				udfa.ProcessAudio(in, outputs[i], lerpFactor);
			}

			for (int i = 0; i < numTests; i++)
			{
				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Failed Test: " + ToStr(i) + ", Sample: " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Real tolerance = target[i][j] * 0.1;
					Assert::AreEqual(target[i][j], outputs[i][j], tolerance, werrorchar);
					Assert::AreEqual(target[i][j], outputs[i][j], 1e-5, werrorchar);
				}
			}
		}
	};

	TEST_CLASS(NNBest_Class)
	{
	public:

		TEST_METHOD(Process)
		{
			int fs = 48000;
			std::string filePath = _SOLUTIONDIR;
			auto data = Parse2Dcsv<double>(filePath + "UnitTestData\\diffractionPaths.csv");
			auto target = Parse2Dcsv<double>(filePath + "UnitTestData\\NNBest.csv");

			const int numFrames = target[0].size();
			const int numTests = target.size();
			Real lerpFactor = 0.1;

			Buffer in(numFrames);
			in[0] = 1.0;
			std::vector<Buffer> outputs(numTests, Buffer(numFrames));

			std::vector<Real> zW(data[0]);
			std::vector<Real> tW(data[1]);
			std::vector<Real> tS(data[2]);
			std::vector<Real> tR(data[3]);
			std::vector<Real> rS(data[4]);
			std::vector<Real> rR(data[5]);
			std::vector<Real> zS(data[6]);
			std::vector<Real> zR(data[7]);

			for (int i = 0; i < numTests; i++)
			{
				Path path = CreatePath(rS[i], rR[i], tS[i], tR[i], tW[i], zS[i], zR[i], zW[i]);
				NNBest nnBest = NNBest(path);
				nnBest.ProcessAudio(in, outputs[i], lerpFactor);
			}

			for (int i = 0; i < numTests; i++)
			{
				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Incorrect Test: " + ToStr(i) + ", Incorrect Sample: " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Real tolerance = target[i][j] * 0.1;
					if (abs(target[i][j]) > 1e-40)
						Assert::AreEqual(target[i][j], outputs[i][j], tolerance, werrorchar);
					Assert::AreEqual(target[i][j], outputs[i][j], 1e-5, werrorchar);
				}
			}
		}
	};

	TEST_CLASS(NNSmall_Class)
	{
	public:

		TEST_METHOD(Process)
		{
			int fs = 48000;
			std::string filePath = _SOLUTIONDIR;
			auto data = Parse2Dcsv<double>(filePath + "UnitTestData\\diffractionPaths.csv");
			auto target = Parse2Dcsv<double>(filePath + "UnitTestData\\NNSmall.csv");

			const int numFrames = target[0].size();
			const int numTests = target.size();
			Real lerpFactor = 0.1;

			Buffer in(numFrames);
			in[0] = 1.0;
			std::vector<Buffer> outputs(numTests, Buffer(numFrames));

			std::vector<Real> zW(data[0]);
			std::vector<Real> tW(data[1]);
			std::vector<Real> tS(data[2]);
			std::vector<Real> tR(data[3]);
			std::vector<Real> rS(data[4]);
			std::vector<Real> rR(data[5]);
			std::vector<Real> zS(data[6]);
			std::vector<Real> zR(data[7]);

			for (int i = 0; i < numTests; i++)
			{
				Path path = CreatePath(rS[i], rR[i], tS[i], tR[i], tW[i], zS[i], zR[i], zW[i]);
				NNSmall nnSmall = NNSmall(path);
				nnSmall.ProcessAudio(in, outputs[i], lerpFactor);
			}

			for (int i = 0; i < numTests; i++)
			{
				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Incorrect Test: " + ToStr(i) + ", Incorrect Sample: " + ToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Real tolerance = target[i][j] * 0.1;
					if (abs(target[i][j]) > 1e-40)
						Assert::AreEqual(target[i][j], outputs[i][j], tolerance, werrorchar);
					Assert::AreEqual(target[i][j], outputs[i][j], 1e-5, werrorchar);
				}
			}
		}
	};
#pragma optimize("", on)
}