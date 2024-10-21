#include "CppUnitTest.h"
#define NOMINMAX
#include <Windows.h>
#include <fstream>

#include "BinauralSpatializer/Core.h"

#include "Diffraction/Models.h"
#include "Diffraction/Path.h"

#include "Spatialiser/Types.h"
#include "Spatialiser/Edge.h"
#include "Spatialiser/Wall.h"
#include "Spatialiser/Reverb.h"
#include "Spatialiser/VirtualSource.h"
#include "Spatialiser/Source.h"

#include "Unity/Debug.h"

#include "Common/Types.h"
#include "Common/Definitions.h"
#include "Common/Vec3.h"
#include "Common/ScopedTimer.h"

#include "DSP/Interpolate.h"
#include "DSP/ParametricEQ.h"
#include "DSP/GraphicEQ.h"

#include <iostream>
#include <chrono>
#include "omp.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft
{
	namespace VisualStudio
	{
		namespace CppUnitTestFramework
		{
			template<> static std::wstring ToString<RAC::Common::Coefficients>(const RAC::Common::Coefficients& t)
			{
				std::string str = "Coefficients: ";
				for (int i = 0; i < t.Length(); i++)
					str += RAC::Unity::RealToStr(t[i]) + ", ";
				std::wstring werror = std::wstring(str.begin(), str.end());
				return werror;
			}

			template<> static std::wstring ToString<RAC::Common::Absorption>(const RAC::Common::Absorption& t)
			{
				std::string str = "Absorption: ";
				for (int i = 0; i < t.Length(); i++)
					str += RAC::Unity::RealToStr(t[i]) + ", ";
				str += "Area: " + RAC::Unity::RealToStr(t.mArea);
				std::wstring werror = std::wstring(str.begin(), str.end());
				return werror;
			}
		}
	}
}

namespace RAC
{
	using namespace Common;
	using namespace Spatialiser;
	using namespace Spatialiser::Diffraction;
	using namespace DSP;

#pragma optimize("", off)

	//////////////////// Create functions ////////////////////

	Path CreatePath(Real rS, Real rR, Real tS, Real tR, Real tW, Real zS, Real zR, Real zW)
	{
		tS = Deg2Rad(tS);
		tR = Deg2Rad(tR);
		tW = Deg2Rad(tW);

		vec3 base = vec3(0.0, 0.0, 0.0);
		vec3 top = vec3(0.0, zW, 0.0);
		vec3 normal1 = vec3(sin(tW), 0.0, -cos(tW));
		vec3 normal2 = vec3(0.0, 0.0, 1.0);

		Edge e = Edge(base, top, normal1, normal2, 0, 1, 0, 1);

		vec3 s = vec3(rS * cos(tS), zS, rS * sin(tS));
		vec3 r = vec3(rR * cos(tR), zR, rR * sin(tR));

		return Path(s, r, e);
	}

	Binaural::CCore CreateCore(int fs)
	{
		int numFrames = 1024;
		int hrtfResamplingStep = 5;

		Binaural::CCore core = Binaural::CCore();

		// Set dsp settings
		core.SetAudioState({ fs, numFrames });
		core.SetHRTFResamplingStep(hrtfResamplingStep);

		// Create listener
		shared_ptr<Binaural::CListener> mListener = core.CreateListener();
		return core;
	}

	void RemoveCore(Binaural::CCore core)
	{
		core.RemoveListener();
	}

	//void CreateShoebox()
	//{
	//	Absorption absorbtion = Absorption(0.7f, 0.7f, 0.7f, 0.7f, 0.7f);
	//	vec3 pos = vec3(7.0f, 3.0f, 5.0f);
	//	
	//	CreateShoebox(pos, absorbtion);
	//}
	//	
	//void CreateShoebox(const vec3& pos, Absorption absorbtion)
	//{
	//	int numVert = 4;
	//	
	//	float vert1[] = { 0.0f, pos.y, 0.0f,
	//					pos.x, pos.y, 0.0f,
	//					pos.x, pos.y, pos.z,
	//					0.0f, pos.y, pos.z };
	//	vec3 normal1 = vec3(0.0f, -1.0f, 0.0f);
	//	size_t wID1 = InitWall(normal1, &vert1[0], (size_t)numVert, absorbtion, ReverbWall::posZ);
	//	
	//	float vert2[] = { pos.x, 0.0f, 0.0f,
	//					0.0f, 0.0f, 0.0f,
	//					0.0f, 0.0f, pos.z,
	//					pos.x, 0.0f, pos.z };
	//	vec3 normal2 = vec3(0.0f, 1.0f, 0.0f);
	//	size_t wID2 = InitWall(normal2, &vert2[0], (size_t)numVert, absorbtion, ReverbWall::negZ);
	//	
	//	float vert3[] = { pos.x, 0.0f, pos.z,
	//					pos.x, pos.y, pos.z,
	//					pos.x, pos.y, 0.0f,
	//					pos.x, 0.0f, 0.0f };
	//	vec3 normal3 = vec3(-1.0f, 0.0f, 0.0f);
	//	size_t wID3 = InitWall(normal3, &vert3[0], (size_t)numVert, absorbtion, ReverbWall::posX);
	//	
	//	float vert4[] = { 0.0f, 0.0f, 0.0f,
	//					0.0f, pos.y, 0.0f,
	//					0.0f, pos.y, pos.z,
	//					0.0f, 0.0f, pos.z };
	//	vec3 normal4 = vec3(1.0f, 0.0f, 0.0f);
	//	size_t wID4 = InitWall(normal4, &vert4[0], (size_t)numVert, absorbtion, ReverbWall::negX);
	//	
	//	float vert5[] = { 0.0f, 0.0f, 0.0f,
	//					pos.x, 0.0f, 0.0f,
	//					pos.x, pos.y, 0.0f,
	//					0.0f, pos.y, 0.0f };
	//	vec3 normal5 = vec3(0.0f, 0.0f, 1.0f);
	//	size_t wID5 = InitWall(normal5, &vert5[0], (size_t)numVert, absorbtion, ReverbWall::negY);
	//	
	//	float vert6[] = { 0.0f, pos.y, pos.z,
	//					pos.x, pos.y, pos.z,
	//					pos.x, 0.0f, pos.z,
	//					0.0f, 0.0f, pos.z };
	//	vec3 normal6 = vec3(0.0f, 0.0f, -1.0f);
	//	size_t wID6 = InitWall(normal6, &vert6[0], (size_t)numVert, absorbtion, ReverbWall::posY);
	//}
	//	
	//void CreateLaboratory()
	//{
	//	int numVert = 4;
	//	
	//	Absorption carpetOnConcrete = Absorption(0.06f, 0.15f, 0.4f, 0.6f, 0.6f);
	//	Absorption concrete = Absorption(0.01f, 0.02f, 0.02f, 0.02f, 0.03f);
	//	Absorption windowGlass = Absorption(0.2f, 0.2f, 0.1f, 0.07f, 0.04f);
	//	Absorption gypsum = Absorption(0.1f, 0.05f, 0.04f, 0.07f, 0.1f);
	//	Absorption plywood = Absorption(0.3f, 0.1f, 0.1f, 0.1f, 0.1f);
	//	Absorption plasterSprayed = Absorption(0.7f, 0.6f, 0.7f, 0.7f, 0.5f);
	//	
	//	vec3 pos = vec3(4.97f, 3.0f, 4.12f);
	//	
	//	float vert1[] = { 0.0f, pos.y, 0.0f,
	//					pos.x, pos.y, 0.0f,
	//					pos.x, pos.y, pos.z,
	//					0.0f, pos.y, pos.z };
	//	vec3 normal1 = vec3(0.0f, -1.0f, 0.0f); // concrete
	//	size_t wID1 = InitWall(normal1, &vert1[0], (size_t)numVert, concrete, ReverbWall::posZ);
	//	
	//	float vert2[] = { pos.x, 0.0f, 0.0f,
	//					0.0f, 0.0f, 0.0f,
	//					0.0f, 0.0f, pos.z,
	//					pos.x, 0.0f, pos.z };
	//	vec3 normal2 = vec3(0.0f, 1.0f, 0.0f); //carpet
	//	size_t wID2 = InitWall(normal2, &vert2[0], (size_t)numVert, carpetOnConcrete, ReverbWall::negZ);
	//	
	//	float vert3[] = { pos.x, 0.0f, pos.z,
	//					pos.x, pos.y, pos.z,
	//					pos.x, pos.y, 0.0f,
	//					pos.x, 0.0f, 0.0f };
	//	vec3 normal3 = vec3(-1.0f, 0.0f, 0.0f); //window glass
	//	size_t wID3 = InitWall(normal3, &vert3[0], (size_t)numVert, windowGlass, ReverbWall::posX);
	//	
	//	float vert4[] = { 0.0f, 0.0f, 0.0f,
	//					0.0f, pos.y, 0.0f,
	//					0.0f, pos.y, pos.z,
	//					0.0f, 0.0f, pos.z };
	//	vec3 normal4 = vec3(1.0f, 0.0f, 0.0f); //plywood
	//	size_t wID4 = InitWall(normal4, &vert4[0], (size_t)numVert, plywood, ReverbWall::negX);
	//	
	//	float vert5[] = { 0.0f, 0.0f, 0.0f,
	//					pos.x, 0.0f, 0.0f,
	//					pos.x, pos.y, 0.0f,
	//					0.0f, pos.y, 0.0f };
	//	vec3 normal5 = vec3(0.0f, 0.0f, 1.0f); //gypsum
	//	size_t wID5 = InitWall(normal5, &vert5[0], (size_t)numVert, gypsum, ReverbWall::negY);
	//	
	//	float vert6[] = { 0.0f, pos.y, pos.z,
	//					pos.x, pos.y, pos.z,
	//					pos.x, 0.0f, pos.z,
	//					0.0f, 0.0f, pos.z };
	//	vec3 normal6 = vec3(0.0f, 0.0f, -1.0f); //plaster
	//	size_t wID6 = InitWall(normal6, &vert6[0], (size_t)numVert, plasterSprayed, ReverbWall::posY);
	//}

	//////////////////// Parse csv files ////////////////////

	std::vector<Real> Parse1Dcsv(std::string filePath)
	{
		std::ifstream  data(filePath);
		std::string line;
		std::vector<Real> parsedCsv;
		while (std::getline(data, line))
		{
			std::stringstream lineStream(line);
			std::string cell;
			std::vector<Real> parsedRow;
			std::getline(lineStream, cell, ',');
			parsedCsv.push_back(StrToReal(cell));
		}
		return parsedCsv;
	}

	std::vector<std::vector<Real> > Parse2Dcsv(std::string filePath)
	{
		std::ifstream  data(filePath);
		std::string line;
		std::vector<std::vector<Real> > parsedCsv;
		while (std::getline(data, line))
		{
			std::stringstream lineStream(line);
			std::string cell;
			std::vector<Real> parsedRow;
			while (std::getline(lineStream, cell, ','))
			{
				parsedRow.push_back(StrToReal(cell));
			}

			parsedCsv.push_back(parsedRow);
		}
		return parsedCsv;
	}

	void AppendBufferToCSV(const std::string& filename, const Buffer& data) {
		// Open the file in append mode
		std::ofstream file(filename, std::ios::app);

		// Check if the file is open
		if (!file.is_open()) {
			std::cerr << "Error: Could not open file " << filename << std::endl;
			return;
		}

		int decimalPlaces = 18;

		// Set the decimal precision for floating point numbers
		file << std::fixed << std::setprecision(decimalPlaces);

		// Write the vector data to the file as a CSV row
		for (size_t i = 0; i < data.Length(); ++i) {
			file << data[i];
			// Add a comma after every element except the last one
			if (i != data.Length() - 1) {
				file << ",";
			}
		}
		// End the row by adding a newline
		file << "\n";

		// Close the file
		file.close();
	}

	//////////////////// Diffraction Model Tests ////////////////////

	TEST_CLASS(DiffractionModels)
	{
	public:

		TEST_METHOD(BtmModel)
		{
			int fs = 48000;
			std::string filePath = _SOLUTIONDIR;
			auto data = Parse2Dcsv(filePath + "UnitTestData\\diffractionPaths.csv");

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

			auto csv = Parse1Dcsv(filePath + "UnitTestData\\btm.csv");
			for (int i = 0; i < ir.Length(); i++)
			{
				std::string error = "Incorrect Sample: " + Unity::IntToStr(i);
				std::wstring werror = std::wstring(error.begin(), error.end());
				const wchar_t* werrorchar = werror.c_str();
				Real tolerance = csv[i] * 0.1;
				Assert::AreEqual(csv[i], ir[i], tolerance, werrorchar);
				Assert::AreEqual(csv[i], ir[i], 0.00006, werrorchar);
			}
		}

		void UpdateDiffractionModel(DiffractionModel model)
		{
			int fs = 48000;
			std::string filePath = _SOLUTIONDIR;
			auto data = Parse2Dcsv(filePath + "UnitTestData\\diffractionPaths.csv");

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
			auto data = Parse2Dcsv(filePath + "UnitTestData\\diffractionPaths.csv");

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

		TEST_METHOD(UpdateAttenuate)
		{
			UpdateDiffractionModel(DiffractionModel::attenuate);
		}

		TEST_METHOD(UpdateLPF)
		{
			UpdateDiffractionModel(DiffractionModel::lowPass);
		}

		TEST_METHOD(UpdateBTM)
		{
			UpdateDiffractionModel(DiffractionModel::btm);
		}

		TEST_METHOD(UpdateUDFA)
		{
			UpdateDiffractionModel(DiffractionModel::udfa);
		}

		TEST_METHOD(UpdateUDFAI)
		{
			UpdateDiffractionModel(DiffractionModel::udfai);
		}

		TEST_METHOD(UpdateNNSmall)
		{
			UpdateDiffractionModel(DiffractionModel::nnSmall);
		}

		TEST_METHOD(UpdateNNBest)
		{
			UpdateDiffractionModel(DiffractionModel::nnBest);
		}

		TEST_METHOD(UpdateUTD)
		{
			UpdateDiffractionModel(DiffractionModel::utd);
		}


		TEST_METHOD(ProcessAttenuate)
		{
			ProcessDiffractionModel(DiffractionModel::attenuate);
		}

		TEST_METHOD(ProcessLPF)
		{
			ProcessDiffractionModel(DiffractionModel::lowPass);
		}

		TEST_METHOD(ProcessBTM)
		{
			ProcessDiffractionModel(DiffractionModel::btm);
		}

		TEST_METHOD(ProcessUDFA)
		{
			ProcessDiffractionModel(DiffractionModel::udfa);
		}

		TEST_METHOD(ProcessUDFAI)
		{
			ProcessDiffractionModel(DiffractionModel::udfai);
		}

		TEST_METHOD(ProcessNNSmall)
		{
			ProcessDiffractionModel(DiffractionModel::nnSmall);
		}

		TEST_METHOD(ProcessNNBest)
		{
			ProcessDiffractionModel(DiffractionModel::nnBest);
		}

		TEST_METHOD(ProcessUTD)
		{
			ProcessDiffractionModel(DiffractionModel::utd);
		}
	};

	void TestReverbSource(const ReverbSource& rS, const Absorption& t)
	{
		//Absorption a = rS.GetAbsorption();
		Real tolerance = 0.0001;
		// Assert::AreEqual(a.mArea, t.mArea, tolerance, L"Incorrect area");

		for (int i = 0; i < 5; i++)
		{
			std::string error = "Incorrect Absorption: " + Unity::IntToStr(i);
			std::wstring werror = std::wstring(error.begin(), error.end());
			const wchar_t* werrorchar = werror.c_str();
			//Assert::AreEqual(t[i], a[i], tolerance, werrorchar);
		}
	}

#include <omp.h>
#include <iostream>

	TEST_CLASS(Test_OMP)
	{
	public:

		void test_openmp() {
#pragma omp parallel
			{
				int thread_id = omp_get_thread_num();
				std::cout << "Hello from thread " << thread_id << std::endl;
			}
		}

		TEST_METHOD(OMP)
		{
			std::ofstream out("OMP.txt");
			std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			// Test with multiple threads
			omp_set_num_threads(8);
			test_openmp();

			// Test with a single thread
			omp_set_num_threads(1);
			test_openmp();

			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
		}


	};

	TEST_CLASS(Test_ProcessAudio)
	{
	public:

		Source CreateSource(Binaural::CCore core, Config config)
		{
			vec3 sPosition = vec3(1.0, 1.6, 1.0);
			vec3 lPosition = vec3(0.0, 1.6, 0.0);

			vec3 normal = vec3(0.0, 1.0, 0.0);
			std::vector<Real> in = { 0.3, 0.15, 0.2, 0.18, 0.1 };
			std::vector<Real> vData = { -10.0, 0.0, -10.0, 0.0, 0.0, 10.0, 10.0, 0.0, -10.0 };
			Absorption absorption = Absorption(in);
			Wall wall = Wall(normal, &vData[0], absorption);
			Plane plane = Plane(0, wall);

			core.CreateListener();
			CTransform lTransform;
			lTransform.SetPosition(CVector3(lPosition.x, lPosition.y, lPosition.z));
			core.GetListener()->SetListenerTransform(lTransform);

			Source source = Source(&core, config);

			VirtualSourceDataMap vSources;
			VirtualSourceData vSource = VirtualSourceData(5);
			vSource.SetPreviousPlane(vec4(plane.GetD(), plane.GetNormal()));

			vSource.Valid();
			vSource.AddPlaneID(0);

			vec3 position;
			plane.ReflectPointInPlane(position, sPosition);
			vSource.SetTransform(position);

			Absorption& abs = vSource.GetAbsorptionRef();
			abs = absorption;

			vSource.SetDistance(lPosition);
			vSource.Visible(false);

			for (int i = 0; i < 10; i++)
			{
				VirtualSourceData v = vSource;
				v.AddPlaneID(i);
				vSources.insert_or_assign(v.GetKey(), v);
			}

			source.UpdateData(true, vSources);
			source.Update(sPosition, vec4(0.0, 0.0, 0.0, 1.0), (sPosition - lPosition).Length());

			return source;
		}

		TEST_METHOD(Tree)
		{
			Config config;
			config.fs = 48000;
			Binaural::CCore core = CreateCore(config.fs);

			Source source = CreateSource(core, config);

			size_t numFrames = 2048;
			size_t numFDNChannels = 12;
			Buffer mInputBuffer = Buffer(numFrames);
			Buffer mOutputBuffer = Buffer(2 * numFrames); // Stereo output buffer
			matrix mReverbInput = matrix(numFrames, numFDNChannels);

			mInputBuffer[0] = 1.0;

			source.ProcessAudio(mInputBuffer, mReverbInput, mOutputBuffer);
			source.ProcessAudio(mInputBuffer, mReverbInput, mOutputBuffer);
		}

		// Seems to be an issue with the omp and test environment.
		TEST_METHOD(Parallel)
		{
			std::ofstream out("Parallel.txt");
			std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			Config config;
			config.fs = 48000;
			Binaural::CCore core = CreateCore(config.fs);

			Source source = CreateSource(core, config);

			size_t numFrames = 2048;
			size_t numFDNChannels = 12;
			Buffer mInputBuffer = Buffer(numFrames);
			Buffer mOutputBuffer = Buffer(2 * numFrames); // Stereo output buffer
			matrix mReverbInput = matrix(numFrames, numFDNChannels);

			mInputBuffer[0] = 1.0;

			source.ProcessAudioParallel(mInputBuffer, mReverbInput, mOutputBuffer);
			source.ProcessAudioParallel(mInputBuffer, mReverbInput, mOutputBuffer);

			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
		}

		TEST_METHOD(Compare)
		{
			Config config;
			config.fs = 48000;
			Binaural::CCore core = CreateCore(config.fs);

			Source source = CreateSource(core, config);

			size_t numFrames = 2048;
			size_t numFDNChannels = 12;
			Buffer mInputBuffer = Buffer(numFrames);
			Buffer mOutputBuffer = Buffer(2 * numFrames); // Stereo output buffer
			matrix mReverbInput = matrix(numFrames, numFDNChannels);

			mInputBuffer[0] = 1.0;

			source.ProcessAudioParallel(mInputBuffer, mReverbInput, mOutputBuffer);
			source.ProcessAudio(mInputBuffer, mReverbInput, mOutputBuffer);

			Source source2 = CreateSource(core, config);

			Buffer mOutputBuffer2 = Buffer(2 * numFrames); // Stereo output buffer

			source2.ProcessAudio(mInputBuffer, mReverbInput, mOutputBuffer2);
			source2.ProcessAudio(mInputBuffer, mReverbInput, mOutputBuffer2);
			
			for (int i = 0; i < mOutputBuffer.Length(); i++)
			{
				std::string error = "Incorrect Sample: " + Unity::IntToStr(i);
				std::wstring werror = std::wstring(error.begin(), error.end());
				const wchar_t* werrorchar = werror.c_str();
				Assert::AreEqual(mOutputBuffer[i], mOutputBuffer2[i], 0.0001, werrorchar);
			}
		}
	};
	
	TEST_CLASS(Test_Coefficients)
	{
	public:

		TEST_METHOD(Operators)
		{
			std::vector<Real> a = { 2.0, 3.0 };
			std::vector<Real> b = { 5.0, 2.0 };

			Coefficients c1 = Coefficients(a);
			Coefficients c2 = Coefficients(b);

			Coefficients out = Coefficients(c1.Length());
			out[0] = 7.0;
			out[1] = 5.0;
			Assert::AreEqual(out, c1 + c2, L"Error: Incorrect addition");

			out[0] = -3.0;
			out[1] = 1.0;
			Assert::AreEqual(out, c1 - c2, L"Error: Incorrect subtraction");

			out[0] = 10.0;
			out[1] = 6.0;
			Assert::AreEqual(out, c1 * c2, L"Error: Incorrect multiplication");

			out[0] = 0.4;
			out[1] = 1.5;
			Assert::AreEqual(out, c1 / c2, L"Error: Incorrect division");

			Real x = 2.0;

			out[0] = 4.0;
			out[1] = 5.0;
			Assert::AreEqual(out, c1 + x, L"Error: Incorrect factor addition");

			out[0] = 0.0;
			out[1] = -1.0;
			Assert::AreEqual(out, x - c1, L"Error: Incorrect factor subtraction");

			out[0] = 4.0;
			out[1] = 6.0;
			Assert::AreEqual(out, c1 * x, L"Error: Incorrect factor multiplication");

			out[0] = 1.0;
			out[1] = 1.5;
			Assert::AreEqual(out, c1 / x, L"Error: Incorrect factor division");
		}
	};

	TEST_CLASS(Test_Absorption)
	{
	public:

		TEST_METHOD(Operators)
		{
			std::vector<Real> a = { 0.5, 0.7 };
			std::vector<Real> b = { 0.5, 0.8 };

			Absorption c1 = Absorption(a);
			Absorption c2 = Absorption(b);
			c1.mArea = 2.0;
			c2.mArea = 5.0;

			Absorption out = Absorption(c1.Length());
			out[0] = sqrt(1.0 - a[0]) + sqrt(1.0 - b[0]);
			out[1] = sqrt(1.0 - a[1]) + sqrt(1.0 - b[1]);
			out.mArea = 7.0;
			Absorption c = c1 + c2;
			Assert::AreEqual(out, c1 + c2, L"Error: Incorrect addition");

			out[0] = sqrt(1.0 - a[0]) - sqrt(1.0 - b[0]);
			out[1] = sqrt(1.0 - a[1]) - sqrt(1.0 - b[1]);
			out.mArea = -3.0;
			Assert::AreEqual(out, c1 - c2, L"Error: Incorrect subtraction");

			out[0] = sqrt(1.0 - a[0]) * sqrt(1.0 - b[0]);
			out[1] = sqrt(1.0 - a[1]) * sqrt(1.0 - b[1]);
			out.mArea = 2.0;
			Assert::AreEqual(out, c1 * c2, L"Error: Incorrect multiplication");

			out[0] = sqrt(1.0 - a[0]) / sqrt(1.0 - b[0]);
			out[1] = sqrt(1.0 - a[1]) / sqrt(1.0 - b[1]);
			Assert::AreEqual(out, c1 / c2, L"Error: Incorrect division");

			Real x = 2.0;

			out[0] = sqrt(1.0 - a[0]) + x;
			out[1] = sqrt(1.0 - a[1]) + x;
			Assert::AreEqual(out, c1 + x, L"Error: Incorrect factor addition");

			out[0] = x - sqrt(1.0 - a[0]);
			out[1] = x - sqrt(1.0 - a[1]);
			Assert::AreEqual(out, x - c1, L"Error: Incorrect factor subtraction");

			out[0] = sqrt(1.0 - a[0]) * x;
			out[1] = sqrt(1.0 - a[1]) * x;
			Assert::AreEqual(out, c1 * x, L"Error: Incorrect factor multiplication");

			out[0] = sqrt(1.0 - a[0]) / x;
			out[1] = sqrt(1.0 - a[1]) / x;
			Assert::AreEqual(out, c1 / x, L"Error: Incorrect factor division");
		}
	};

	//////////////////// FDN Tests ////////////////////

	TEST_CLASS(FDNModel)
	{
	public:

		TEST_METHOD(ReflectionFilters)
		{
			Config config;
			config.fs = 48000;

			Binaural::CCore core = CreateCore(config.fs);

			ReverbSource reverbSource(&core, config);

			std::vector<Real> in = { 0.7, 0.7, 0.7, 0.7, 0.7 };
			Absorption absorption1(in);
			absorption1.mArea = 5.0;

			in = { 0.5, 0.5, 0.3, 0.3, 0.4 };
			Absorption absorption2(in);
			absorption2.mArea = 2.0;

			in = { 0.1, 0.2, 0.25, 0.1, 0.2 };
			Absorption absorption3(in);
			absorption3.mArea = 3.0;

			//Absorption test = absorption2;
			reverbSource.UpdateReflectionFilter(absorption1);
			TestReverbSource(reverbSource, absorption1);

			reverbSource.UpdateReflectionFilter(absorption2);
			in = { 9.0 / 14.0, 9.0 / 14.0, 41.0 / 70.0, 41.0 / 70.0, 43.0 / 70.0 };
			Absorption test(in);
			test.mArea = 7.0;
			TestReverbSource(reverbSource, test);

			reverbSource.UpdateReflectionFilter(absorption3);
			in = { 0.48, 0.51, 0.485, 0.44, 0.49 };
			test.Update(in);
			test.mArea = 10.0;
			TestReverbSource(reverbSource, test);

			absorption1.mArea = -absorption1.mArea;
			reverbSource.UpdateReflectionFilter(absorption1);
			in = { 0.26, 0.32, 0.27, 0.18, 0.28 };
			test.Update(in);
			test.mArea = 5.0;
			TestReverbSource(reverbSource, test);

			absorption2.mArea = -absorption2.mArea;
			reverbSource.UpdateReflectionFilter(absorption2);
			TestReverbSource(reverbSource, absorption3);

			absorption3.mArea = -absorption3.mArea;
			reverbSource.UpdateReflectionFilter(absorption3);
			in = { 0.0, 0.0, 0.0, 0.0, 0.0 };
			test.Update(in);
			test.mArea = 0.0;
			TestReverbSource(reverbSource, test);

			RemoveCore(core);
		}

		TEST_METHOD(FDNChannel)
		{
			Config config;
			Real t = 0.1;
			Coefficients T60 = Coefficients(5, 1.0);
			Channel channel = Channel(t, T60, config);
		}

		//TEST_METHOD(Parellise)
		//{
		//	Config config;
		//	Real t = 0.1;
		//	Coefficients T60 = Coefficients(5, 1.0);
		//	Channel channel = Channel(t, T60, config);

		//	int a = 12;
		//	rowvec y1 = rowvec(a);
		//	rowvec y2 = rowvec(a);

		//	rowvec x = rowvec(a);
		//	std::vector<Real> data = std::vector<Real>(a, 1.0);

		//	std::ofstream out("RunTime.txt");
		//	std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
		//	std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

		//	ScopedTimer timer;


		//	std::default_random_engine gen(200);
		//	std::uniform_real_distribution<Real> distribution(-1.0, 1.0);
		//	for (int i = 0; i < a; i++)
		//	{
		//		x.AddEntry(distribution(gen), i);
		//	}

		//	int repeats = 2048;
		//	std::vector<Channel> mChannels = std::vector<Channel>(a, Channel(t, T60, config));
		//	int i = 0;
		//	timer.Start();

		//	for (int k = 0; k < repeats; k++)
		//	{
		//		i = 0;
		//		for (auto& channel : mChannels)
		//		{
		//			y1.AddEntry(channel.GetOutput(x.GetEntry(i) + data[i]), i);
		//			i++;
		//		}
		//		for (int i = 0; i < a; i++)
		//		{
		//			x.AddEntry(distribution(gen), i);
		//		}
		//	}

		//	timer.Stop("Serial");
		//	timer.Start();

		//	for (int k = 0; k < repeats; k++)
		//	{
		//		i = 0;
		//		for (auto& channel : mChannels)
		//		{
		//			y2.AddEntry(channel.GetOutput(x.GetEntry(i) + data[i]), i);
		//			i++;
		//		}
		//		for (int i = 0; i < a; i++)
		//		{
		//			x.AddEntry(distribution(gen), i);
		//		}
		//	}
		//	timer.Stop("Parrallel");


		//	std::cout.rdbuf(coutbuf); //reset to standard output again

		//	/*for (int i = 0; i < a; i++)
		//		Assert::AreEqual(y1.GetEntry(i), y2.GetEntry(i), L"Mismatch results");*/

		//}

	};

	//////////////////// Matrix Tests ////////////////////

	TEST_CLASS(MatrixTests)
	{
	public:

		TEST_METHOD(Init)
		{
			const int rows = 5;
			const int cols = 4;

			matrix m = matrix(rows, cols);

			Real x = 1.0;

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					m[i][j] = x;
					Assert::AreEqual(x, m.GetEntry(i, j), L"Error: Add entry");
					m[i][j] += 1.0;
					x += 1.0;
					Assert::AreEqual(x, m.GetEntry(i, j), L"Error: Increase entry");
				}
			}

			matrix mat = matrix(m.Data());

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					Assert::AreEqual(m.GetEntry(i, j), mat.GetEntry(i, j), L"Error: Init from vectors");
				}
			}

			m.Reset();

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					Real test = m.GetEntry(i, j);
					Assert::AreEqual(0.0, test, L"Error: Reset");
				}
			}
		}

//		TEST_METHOD(Parellise)
//		{
//			const int a = 2000;
//			const int b = 2000;
//
//			matrix mat = matrix(a, b);
//			rowvec x1 = rowvec(a);
//			rowvec x2 = rowvec(a);
//			rowvec y = rowvec(a);
//
//			std::default_random_engine gen(50);
//			std::uniform_real_distribution<Real> distribution(-1.0, 1.0);
//			for (int i = 0; i < a; i++)
//			{
//				y.AddEntry(distribution(gen), i);
//			}
//
//			for (int i = 0; i < a; i++)
//			{
//				for (int j = 0; j < b; j++)
//				{
//					mat.AddEntry(distribution(gen), i, j);
//				}
//			}
//
//			std::ofstream out("RunTime4.txt");
//			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
//			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
//
//			int maxThreads = omp_get_max_threads();
//
//			cout << "Max threads: " << maxThreads << "\n";
//			ScopedTimer timer;
//
//			for (int i = 0; i < 10; i++)
//			{
//				Real sum = 0.0;
//
//				x1.Reset();
//				timer.Start();
//
//				for (int j = 0; j < a; ++j)
//				{
//					sum = 0.0;
//					for (int k = 0; k < b; ++k)
//					{
//						sum += y.GetEntry(k) * mat.GetEntry(k, j);
//						//x1.IncreaseEntry(y.GetEntry(k) * mat.GetEntry(k, j), j);
//					}
//					x1.AddEntry(sum, j);
//				}
//
//				timer.Stop("Serial");
//				sum = 0.0;
//
//				x2.Reset();
//				timer.Start();
//
//#pragma omp parallel for num_threads(omp_get_max_threads()) private (sum)
//				for (int j = 0; j < a; ++j)
//				{
//					sum = 0.0;
//					for (int k = 0; k < b; ++k)
//					{
//						sum += y.GetEntry(k) * mat.GetEntry(k, j);
//						//x2.IncreaseEntry(y.GetEntry(k) * mat.GetEntry(k, j), j);
//					}
//					x2.AddEntry(sum, j);
//				}
//
//				timer.Stop("Parallel");
//			}
//			std::cout.rdbuf(coutbuf); //reset to standard output again
//
//			for (int i = 0; i < a; i++)
//				Assert::AreEqual(x1.GetEntry(i), x2.GetEntry(i), L"Mismatch results");
//		}

		TEST_METHOD(Multiply)
		{
			const int a = 2;
			const int b = 3;

			matrix x = matrix(a, b);
			matrix y = matrix(b, a);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
				{
					x[i][j] = 1.0;
					y[j][i] = 1.0;
				}
			}

			x[0][0] = 2.0;
			x[0][2] = 5.0;
			x[1][1] = 3.0;
			y[1][0] = 4.0;
			y[0][1] = 3.0;

			matrix z = x * y;

			Assert::AreEqual(11.0, z.GetEntry(0, 0), L"Error (0, 0)");
			Assert::AreEqual(12.0, z.GetEntry(0, 1), L"Error (0, 1)");
			Assert::AreEqual(14.0, z.GetEntry(1, 0), L"Error (1, 0)");
			Assert::AreEqual(7.0, z.GetEntry(1, 1), L"Error (1, 1)");

			z *= 2.0;

			Assert::AreEqual(22.0, z.GetEntry(0, 0), L"Error 2 (0, 0)");
			Assert::AreEqual(24.0, z.GetEntry(0, 1), L"Error 2 (0, 1)");
			Assert::AreEqual(28.0, z.GetEntry(1, 0), L"Error 2 (1, 0)");
			Assert::AreEqual(14.0, z.GetEntry(1, 1), L"Error 2 (1, 1)");

			matrix w = z * 2.0;

			Assert::AreEqual(44.0, w.GetEntry(0, 0), L"Error 3 (0, 0)");
			Assert::AreEqual(48.0, w.GetEntry(0, 1), L"Error 3 (0, 1)");
			Assert::AreEqual(56.0, w.GetEntry(1, 0), L"Error 3 (1, 0)");
			Assert::AreEqual(28.0, w.GetEntry(1, 1), L"Error 3 (1, 1)");
		}

		TEST_METHOD(Add)
		{
			const int a = 2;
			const int b = 3;

			matrix x = matrix(a, b);
			matrix y = matrix(a, b);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
				{
					x[i][j] = 1.0;
					y[i][j] = 1.0;
				}
			}

			x[0][0] = 2.0;
			x[0][2] = 5.0;
			x[1][1] = 3.0;
			y[1][0] = 4.0;
			y[0][1] = 3.0;
			y[0][0] = 7.0;

			matrix z = x + y;

			Assert::AreEqual(9.0, z.GetEntry(0, 0), L"Error (0, 0)");
			Assert::AreEqual(4.0, z.GetEntry(0, 1), L"Error (0, 1)");
			Assert::AreEqual(6.0, z.GetEntry(0, 2), L"Error (0, 2)");
			Assert::AreEqual(5.0, z.GetEntry(1, 0), L"Error (1, 0)");
			Assert::AreEqual(4.0, z.GetEntry(1, 1), L"Error (1, 1)");
			Assert::AreEqual(2.0, z.GetEntry(1, 2), L"Error (1, 2)");
		}

		TEST_METHOD(Negative)
		{
			const int a = 2;
			const int b = 3;

			matrix x = matrix(a, b);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
				{
					x[i][j] = 1.0;
				}
			}

			matrix y = -x;

			Assert::AreEqual(-1.0, y.GetEntry(0, 0), L"Error (0, 0)");
			Assert::AreEqual(-1.0, y.GetEntry(0, 1), L"Error (0, 1)");
			Assert::AreEqual(-1.0, y.GetEntry(0, 2), L"Error (0, 2)");
			Assert::AreEqual(-1.0, y.GetEntry(1, 0), L"Error (1, 0)");
			Assert::AreEqual(-1.0, y.GetEntry(1, 1), L"Error (1, 1)");
			Assert::AreEqual(-1.0, y.GetEntry(1, 2), L"Error (1, 2)");
		}

		TEST_METHOD(Comparison)
		{
			const int a = 2;
			const int b = 3;

			matrix x = matrix(a, b);
			matrix y = matrix(a, b);

			Assert::AreEqual(true, x == y, L"Match");

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
				{
					x[i][j] = 1.0;
				}
			}

			Assert::AreEqual(false, x == y, L"No match");
		}
	};

	//////////////////// DSP Tests ////////////////////

	TEST_CLASS(Test_DSP)
	{
	public:

		TEST_METHOD(FIRResize)
		{
			std::vector<Real> ir = { 1.0, 0.5, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			Buffer impulseResponse = Buffer(ir);
			FIRFilter filter = FIRFilter(impulseResponse);

			filter.GetOutput(1.0);
			filter.GetOutput(2.0);
			std::vector<Real> ir2 = { 1.0, 0.5, 0.0, 0.2 };
			Buffer impulseResponse2 = Buffer(ir2);
			filter.SetImpulseResponse(impulseResponse2);

			std::vector<Real> input = { 1.0, 0.0, 2.0, 0.0, 0.0, 0.0 };
			std::vector<Real> output = { 2.0, 0.7, 2.4, 1.2, 0.0, 0.4 };

			for (int i = 0; i < input.size(); i++)
				Assert::AreEqual(output[i], filter.GetOutput(input[i]), L"Wrong output");
		}

		TEST_METHOD(FIRProcess)
		{
			std::vector<Real> ir = { 1.0, 0.5, 0.0, 0.2, 0.3, 0.0, 0.7, 0.1 };
			std::vector<Real> in = { 1.0, 0.0, 0.2, 0.5, 0.0, 0.3, 0.4, 0.2 };

			std::vector<Real> out = { 1.0, 0.5, 0.2, 0.8, 0.55, 0.34, 1.41, 0.65 };
			Buffer impulseResponse = Buffer(ir);
			FIRFilter filter = FIRFilter(impulseResponse);

			for (int i = 0; i < out.size(); i++)
				Assert::AreEqual(out[i], filter.GetOutput(in[i]), EPS, L"Wrong output");
		}

		TEST_METHOD(Denormals)
		{
			Real current = MIN_VALUE;
			Real target = 0.0;
			Real lerpFactor = 0.5;

			FlushDenormals();
			for (int i = 0; i < 10; i++)
				current = Lerp(current, target, lerpFactor);
			NoFlushDenormals();
			Assert::AreEqual(0.0, current, L"DenormalsFlushed");

			current = MIN_VALUE;
			for (int i = 0; i < 10; i++)
				current = Lerp(current, target, lerpFactor);

			Assert::AreNotEqual(0.0, current, L"NoDenormalsFlushed");
		}

		TEST_METHOD(InterpolateReal)
		{
			Real current = 1.0;
			Real target = 0.0;
			Real lerpFactor = 0.2;
			current = Lerp(current, target, lerpFactor);

			Assert::AreEqual(0.8, current, EPS, L"Wrong output");
			current = Lerp(current, target, lerpFactor);
			Assert::AreEqual(0.64, current, EPS, L"Wrong output");
		}

		TEST_METHOD(InterpolateBuffer)
		{
			std::vector<Real> start = { 1.0, 4.0, 3.0, 2.0 };
			Buffer current = Buffer(start);

			std::vector<Real> end = { 0.0, 2.0, 4.0, 2.0 };
			Buffer target = Buffer(end);
			Real lerpFactor = 0.2;
			Lerp(current, target, lerpFactor);

			{
			std::vector<Real> output = { 0.8, 3.6, 3.2, 2.0 };
			for (int i = 0; i < 4; i++)
				Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
			{
			std::vector<Real> output = { 0.64, 3.28, 3.36, 2.0 };
			Lerp(current, target, lerpFactor);
			for (int i = 0; i < 4; i++)
				Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
		}

		/*TEST_METHOD(ParalleliseInterpolateBuffer)
		{
			Buffer current = Buffer(1e3);
			Buffer target = Buffer(1e3);

			for (int i = 0; i < 400; i++)
			{
				current[i] = 1.0;
				target[i] = 0.2;
			}

			Real lerpFactor = 0.2;

			for (int i = 0; i < 1e4; i++)
				Lerp(current, target, lerpFactor);
		}*/

		TEST_METHOD(InterpolateCoefficients)
		{
			std::vector<Real> start = { 1.0, 4.0, 3.0, 2.0 };
			Coefficients current = Coefficients(start);

			std::vector<Real> end = { 0.0, 2.0, 4.0, 2.0 };
			Coefficients target = Coefficients(end);
			Real lerpFactor = 0.2;
			Lerp(current, target, lerpFactor);

			{
				std::vector<Real> output = { 0.8, 3.6, 3.2, 2.0 };
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
			{
				std::vector<Real> output = { 0.64, 3.28, 3.36, 2.0 };
				Lerp(current, target, lerpFactor);
				for (int i = 0; i < 4; i++)
					Assert::AreEqual(output[i], current[i], EPS, L"Wrong output");
			}
		}

		TEST_METHOD(LowBandFilter)
		{
			int fs = 48000;
			int order = 4;
			std::vector<Real> f = { 250.0, 500.0, 1000.0, 2000.0, 4000.0 };
			std::vector<Real> gain = { 0.06, 0.15, 0.4, 0.4, 0.6 };
			Coefficients fc = Coefficients(f);
			Coefficients g = Coefficients(gain);
			ParametricEQ eq = ParametricEQ(g, order, fc, fs);


			std::vector<Real> in = { 1.0, 0.2, 0.3, 0.5, -0.4, 0.1, 0.0, -0.2, 0.7 };
			std::vector<Real> out = std::vector<Real>(in.size());
			for (int i = 0; i < in.size(); i++)
				out[i] = eq.GetOutput(in[i]);
		}
	};

	TEST_CLASS(GraphicEQTests)
	{
	public:

		TEST_METHOD(ProcessGraphicEQ)
		{
			std::string filePath = _SOLUTIONDIR;
			auto inputData = Parse2Dcsv(filePath + "UnitTestData\\graphicEQInput.csv");
			auto outputData = Parse2Dcsv(filePath + "UnitTestData\\graphicEQOutput.csv");

			std::vector<Real> f0(inputData[0]);
			std::vector<Real> f1(inputData[1]);
			std::vector<Real> f2(inputData[2]);
			std::vector<Real> f3(inputData[3]);
			std::vector<Real> f4(inputData[4]);

			Coefficients fc = Coefficients({ 250.0, 500.0, 1000.0, 2000.0, 4000.0 });
			Real Q = 0.98;
			int fs = 48e3;
			int numFrames = 256;
			Real lerpFactor = 0.0;
			Buffer out = Buffer(numFrames);
			Buffer in = Buffer(numFrames);
			in[0] = 1.0;

			int numTests = f0.size();
			std::vector<Coefficients> gains = std::vector<Coefficients>(numTests, Coefficients(5));
			for (int i = 0; i < numTests; i++)
			{
				Coefficients gain = Coefficients({ f0[i], f1[i], f2[i], f3[i], f4[i] });
				GraphicEQ eq = GraphicEQ(gain, fc, Q, fs);
				eq.ProcessAudio(in, out, numFrames, lerpFactor);

				// AppendBufferToCSV(filePath + "UnitTestData\\graphicEQOutput.csv", out);

				for (int j = 0; j < numFrames; j++)
				{
					std::string error = "Test: " + Unity::IntToStr(i) + ", Incorrect Sample : " + Unity::IntToStr(j);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();
					Assert::AreEqual(outputData[i][j], out[j], 10e-16, werrorchar);
				}
			}			
		}
	};
#pragma optimize("", on)
}