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
#include "DSP/GraphicEQ.h"

#include "Spatialiser/Interface.h"

#include <iostream>
#include <chrono>
#include "omp.h"

#include "UtilityFunctions.h"


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

		Vec3 base = Vec3(0.0, 0.0, 0.0);
		Vec3 top = Vec3(0.0, zW, 0.0);
		Vec3 normal1 = Vec3(sin(tW), 0.0, -cos(tW));
		Vec3 normal2 = Vec3(0.0, 0.0, 1.0);

		Edge e = Edge(base, top, normal1, normal2, 0, 1, 0, 1);

		Vec3 s = Vec3(rS * cos(tS), zS, rS * sin(tS));
		Vec3 r = Vec3(rR * cos(tR), zR, rR * sin(tR));

		return Path(s, r, e);
	}

	Binaural::CCore CreateCore(int fs, int numFrames, int hrtfResamplingStep)
	{
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

	/*void CreateShoebox()
	{
		Absorption absorbtion = Absorption(0.7f, 0.7f, 0.7f, 0.7f, 0.7f);
		vec3 pos = vec3(7.0f, 3.0f, 5.0f);

		CreateShoebox(pos, absorbtion);
	}*/

	void CreateShoebox(const Vec3& pos, Absorption absorbtion)
	{
		int numVert = 4;

		Vertices vert1 = { Vec3(0.0, pos.y, 0.0),
						Vec3(pos.x, pos.y, 0.0),
						Vec3(pos.x, pos.y, pos.z) };
		Vertices vert2 = { Vec3(0.0, pos.y, 0.0),
						Vec3(pos.x, pos.y, pos.z),
						Vec3(0.0, pos.y, pos.z) };

		InitWall(vert1, absorbtion);
		InitWall(vert2, absorbtion);

		Vertices vert3 = { Vec3(pos.x, 0.0, 0.0),
						Vec3(0.0, 0.0, 0.0),
						Vec3(0.0, 0.0, pos.z) };
		Vertices vert4 = { Vec3(pos.x, 0.0, 0.0),
						Vec3(0.0, 0.0, pos.z),
						Vec3(pos.x, 0.0, pos.z) };

		InitWall(vert3, absorbtion);
		InitWall(vert4, absorbtion);

		Vertices vert5 = { Vec3(pos.x, 0.0, pos.z),
						Vec3(pos.x, pos.y, pos.z),
						Vec3(pos.x, pos.y, 0.0) };
		Vertices vert6 = { Vec3(pos.x, 0.0, pos.z),
						Vec3(pos.x, pos.y, 0.0),
						Vec3(pos.x, 0.0, 0.0) };

		InitWall(vert5, absorbtion);
		InitWall(vert6, absorbtion);

		Vertices vert7 = { Vec3(0.0, 0.0, 0.0),
						Vec3(0.0, pos.y, 0.0),
						Vec3(0.0, pos.y, pos.z) };
		Vertices vert8 = { Vec3(0.0, 0.0, 0.0),
						Vec3(0.0, pos.y, pos.z),
						Vec3(0.0, 0.0, pos.z) };

		InitWall(vert7, absorbtion);
		InitWall(vert8, absorbtion);

		Vertices vert9 = { Vec3(0.0, 0.0, 0.0),
						Vec3(pos.x, 0.0, 0.0),
						Vec3(pos.x, pos.y, 0.0) };
		Vertices vert10 = { Vec3(0.0, 0.0, 0.0),
						Vec3(pos.x, pos.y, 0.0),
						Vec3(0.0, pos.y, 0.0) };

		InitWall(vert9, absorbtion);
		InitWall(vert10, absorbtion);

		Vertices vert11 = { Vec3(0.0, pos.y, pos.z),
						Vec3(pos.x, pos.y, pos.z),
						Vec3(pos.x, 0.0, pos.z) };
		Vertices vert12 = { Vec3(0.0, pos.y, pos.z),
						Vec3(pos.x, 0.0, pos.z),
						Vec3(0.0, 0.0, pos.z) };

		InitWall(vert11, absorbtion);
		InitWall(vert12, absorbtion);
	}

	Vec4 AzimuthElevationToQuaternion(Real azimuth, Real elevation)
	{
		Real azimuthRad = Deg2Rad(azimuth);
		Real elevationRad = Deg2Rad(elevation);

		Real cosAzimuth = cos(azimuthRad * 0.5);
		Real sinAzimuth = sin(azimuthRad * 0.5);
		Real cosElevation = cos(elevationRad * 0.5);
		Real sinElevation = sin(elevationRad * 0.5);

		Real x = cosAzimuth * sinElevation;
		Real y = sinAzimuth * cosElevation;
		Real z = sinAzimuth * sinElevation;
		Real w = cosAzimuth * cosElevation;

		return Vec4(w, x, y, z);
	}

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

	//////////////////// Shoebox Tests ////////////////////

	TEST_CLASS(AR_Scenes)
	{
	public:

		TEST_METHOD(LTC)
		{

			int fs = 48000;
			int numFrames = 4096;
			int numFDNChannels = 12;
			Real lerpFactor = 0;
			Real q = 0.98;
			Coefficients fBands = Coefficients({ 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0 });

			Config config = Config(fs, numFrames, numFDNChannels, lerpFactor, q, fBands);

			bool init = Init(config);

			Assert::IsTrue(init, L"Failed to initialise RAC");

			int hrtfResamplingStep = 1;
			const std::vector<std::string> filePaths = { "C:/Documents/GitHub/jmannall/RoomAcoustiCpp/Resources/Kemar_HRTF_ITD_48000Hz.3dti-hrtf",
														"C:/Documents/GitHub/jmannall/RoomAcoustiCpp/Resources/NearFieldCompensation_ILD_48000.3dti-ild",
														"C:/Documents/GitHub/jmannall/RoomAcoustiCpp/Resources/HRTF_ILD_48000.3dti-ild" };

			bool success = LoadSpatialisationFiles(hrtfResamplingStep, filePaths);

			Assert::IsTrue(success, L"Failed to load spatialisation files");

			IEMConfig iemConfig = IEMConfig(3, DirectSound::doCheck, true, DiffractionSound::none, DiffractionSound::none, true, 0.0);
			UpdateIEMConfig(iemConfig);
			UpdateSpatialisationMode(SpatMode::quality);
			UpdateReverbTimeModel(ReverbTime::Eyring);
			UpdateFDNModel(FDNMatrix::randomOrthogonal);
			UpdateDiffractionModel(DiffractionModel::attenuate);

			// Absorption absorption = Absorption({ 0.222, 0.258, 0.405, 0.378, 0.284, 0.270, 0.277 }); // Sabine
			Absorption absorption = Absorption({ 0.200, 0.227, 0.333, 0.315, 0.247, 0.237, 0.242 }); // Eyring
			Vec3 roomSize = Vec3(7.0, 2.5, 6.0);
			CreateShoebox(roomSize, absorption);

			Real volume = 105.0;
			Vec dimensions = Vec({ 7.0, 2.5, 6.0 });
			UpdateRoom(volume, dimensions);

			std::vector<Vec3> sourcePositions = { Vec3(5.47, 1.62, 4.5), Vec3(3.72, 1.62, 3.25) };
			std::vector<Vec4> sourceOrientations = { AzimuthElevationToQuaternion(270.0, 0.0), AzimuthElevationToQuaternion(0.0, 0.0) };

			BufferF in = BufferF(numFrames);
			BufferF out = BufferF(2 * numFrames);
			BufferF left = BufferF(fs);
			BufferF right = BufferF(fs);

			float* outPtr = &out[0];

			Real listenerStepPostition = 0.25;
			Real listenerStepRotation = 2.5;
			int numBuffers = fs / numFrames;
			std::vector<std::string> files = { "C:/Documents/GitHub/jmannall/RoomAcoustiCpp/SimulationData/LTC_3rdOrderISM_FDN_Front.csv",
				"C:/Documents/GitHub/jmannall/RoomAcoustiCpp/SimulationData/LTC_3rdOrderISM_FDN_Side.csv",
				"C:/Documents/GitHub/jmannall/RoomAcoustiCpp/SimulationData/LTC_FDN_Front.csv",
				"C:/Documents/GitHub/jmannall/RoomAcoustiCpp/SimulationData/LTC_FDN_Side.csv" };

			std::vector<IEMConfig> iemConfigs = { IEMConfig(3, DirectSound::doCheck, true, DiffractionSound::none, DiffractionSound::none, true, 0.0),
				IEMConfig(0, DirectSound::doCheck, false, DiffractionSound::none, DiffractionSound::none, true, 0.0) };

			for (int t = 1; t < 2; t++)
			{
				UpdateIEMConfig(iemConfigs[t]);
				for (int k = 0; k < 2; k++)
				{
					std::string file = files[k + 2 * t];
					Vec3 sourcePosition = sourcePositions[k];
					Vec4 sourceOrientation = sourceOrientations[k];
					for (int i = 0; i < 9; i++)
					{
						Vec3 listenerPosition = Vec3(4.22 - listenerStepPostition * i, 1.62, 4.5);

						for (int j = 0; j < 360.0 / listenerStepRotation; j++)
						{
							Vec4 listenerOrientation = AzimuthElevationToQuaternion(90.0 - j * listenerStepRotation, 0.0);
							UpdateListener(listenerPosition, listenerOrientation);
							size_t sourceID = InitSource();
							UpdateSourceDirectivity(sourceID, SourceDirectivity::cardioid);
							UpdateSource(sourceID, sourcePosition, sourceOrientation);
							Sleep(20);
							// UpdateSource(sourceID, sourcePosition, sourceOrientation);
							SubmitAudio(sourceID, &in[0]);

							in[0] = 1.0;
							SubmitAudio(sourceID, &in[0]);
							GetOutput(&outPtr);
							for (int k = 0; k < numFrames; k++)
							{
								left[k] = outPtr[2 * k];
								right[k] = outPtr[2 * k + 1];
							}

							in[0] = 0.0;
							for (int n = 1; n < numBuffers; n++)
							{
								SubmitAudio(sourceID, &in[0]);
								GetOutput(&outPtr);
								for (int k = 0; k < numFrames; k++)
								{
									left[k + n * numFrames] = outPtr[2 * k];
									right[k + n * numFrames] = outPtr[2 * k + 1];
								}
							}

							ResetFDN();
							RemoveSource(sourceID);

							WriteDataEntry(file, &left[0], left.Length(), i * listenerStepPostition, j * listenerStepRotation);
							WriteDataEntry(file, &right[0], right.Length(), i * listenerStepPostition, j * listenerStepRotation);
						}
					}
				}
			}

			Exit();
		}

		TEST_METHOD(Shoebox)
		{
			int fs = 48000;
			int numFrames = 4096;
			int numFDNChannels = 12;
			Real lerpFactor = 2;
			Real q = 0.98;
			Coefficients fBands = Coefficients({ 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0 });

			Config config = Config(fs, numFrames, numFDNChannels, lerpFactor, q, fBands);

			bool init = Init(config);

			Assert::IsTrue(init, L"Failed to initialise RAC");

			int hrtfResamplingStep = 30;
			const std::vector<std::string> filePaths = { "C:/Documents/GitHub/jmannall/RoomAcoustiCpp/Resources/Kemar_HRTF_ITD_48000Hz.3dti-hrtf",
														"C:/Documents/GitHub/jmannall/RoomAcoustiCpp/Resources/NearFieldCompensation_ILD_48000.3dti-ild",
														"C:/Documents/GitHub/jmannall/RoomAcoustiCpp/Resources/HRTF_ILD_48000.3dti-ild" };

			bool success = LoadSpatialisationFiles(hrtfResamplingStep, filePaths);

			Assert::IsTrue(success, L"Failed to load spatialisation files");

			IEMConfig iemConfig = IEMConfig(3, DirectSound::doCheck, true, DiffractionSound::none, DiffractionSound::none, true, 0.0);
			UpdateIEMConfig(iemConfig);
			UpdateSpatialisationMode(SpatMode::quality);
			UpdateReverbTimeModel(ReverbTime::Sabine);
			UpdateFDNModel(FDNMatrix::randomOrthogonal);
			UpdateDiffractionModel(DiffractionModel::attenuate);

			Absorption absorption = Absorption({ 0.222, 0.258, 0.405, 0.378, 0.284, 0.270, 0.277 });
			Vec3 roomSize = Vec3(7.0 / 8.0, 2.5 / 8.0, 6.0 / 8.0);
			CreateShoebox(roomSize, absorption);

			Real volume = 105.0;
			Vec dimensions = Vec({ 7.0, 2.5, 6.0 });
			UpdateRoom(volume, dimensions);

			Vec3 sourcePosition = Vec3(5.47, 1.62, 4.5);
			Vec3 sourcePosition2 = Vec3(2.21, 1.52, 1.3);
			Vec4 sourceOrientation = Vec4({ 0.0, 0.0, 0.0, 1.0 });

			Vec3 listenerPosition = Vec3(4.22, 1.62, 4.5);
			Vec4 listenerOrientation = AzimuthElevationToQuaternion(90.0, 0.0);

			UpdateListener(listenerPosition, listenerOrientation);
			size_t sourceID = InitSource();
			UpdateSource(sourceID, sourcePosition, sourceOrientation);
			UpdateSourceDirectivity(sourceID, SourceDirectivity::speaker);

			Sleep(1000);
			BufferF in = BufferF(numFrames);
			BufferF out = BufferF(2 * numFrames);
			BufferF left = BufferF(fs);
			BufferF right = BufferF(fs);

			float* outPtr = &out[0];

			in[0] = 1.0;

			for (int i = 0; i < 1; i++)
			{
				SubmitAudio(sourceID, &in[0]);
				GetOutput(&outPtr);
			}

			Sleep(20);
			UpdateSource(sourceID, sourcePosition2, sourceOrientation);
			Sleep(20);
			UpdateSource(sourceID, sourcePosition2, sourceOrientation);

			for (int i = 0; i < 10; i++)
			{
				SubmitAudio(sourceID, &in[0]);
				GetOutput(&outPtr);
			}

			RemoveSource(sourceID);
			Exit();
		}
	};

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
			Vec3 sPosition = Vec3(1.0, 1.6, 1.0);
			Vec3 lPosition = Vec3(0.0, 1.6, 0.0);

			Vec3 normal = Vec3(0.0, 1.0, 0.0);
			std::vector<Real> in = { 0.3, 0.15, 0.2, 0.18, 0.1 };
			Vertices vData = { Vec3(-10.0, 0.0, -10.0), Vec3(0.0, 0.0, 10.0), Vec3(10.0, 0.0, -10.0) };
			Absorption absorption = Absorption(in);
			Wall wall = Wall(vData, absorption);
			Plane plane = Plane(0, wall);

			core.CreateListener();
			CTransform lTransform;
			lTransform.SetPosition(CVector3(lPosition.x, lPosition.y, lPosition.z));
			core.GetListener()->SetListenerTransform(lTransform);

			Source source = Source(&core, config);

			VirtualSourceDataMap vSources;
			VirtualSourceData vSource = VirtualSourceData(5);
			vSource.SetPreviousPlane(Vec4(plane.GetD(), plane.GetNormal()));

			vSource.Valid();
			vSource.AddPlaneID(0);

			Vec3 position;
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

			source.UpdateData({ 1.0, true }, vSources);
			source.Update(sPosition, Vec4(0.0, 0.0, 0.0, 1.0), (sPosition - lPosition).Length());

			return source;
		}

		TEST_METHOD(Tree)
		{
			Config config;
			Binaural::CCore core = CreateCore(config.fs, config.numFrames, 5);

			Source source = CreateSource(core, config);

			size_t numFrames = 2048;
			size_t numFDNChannels = 12;
			Buffer mInputBuffer = Buffer(numFrames);
			Buffer mOutputBuffer = Buffer(2 * numFrames); // Stereo output buffer
			Matrix mReverbInput = Matrix(numFrames, numFDNChannels);

			mInputBuffer[0] = 1.0;

			source.ProcessAudio(mInputBuffer, mReverbInput, mOutputBuffer);
			source.ProcessAudio(mInputBuffer, mReverbInput, mOutputBuffer);
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

			Binaural::CCore core = CreateCore(config.fs, config.numFrames, 5);

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
	};
#pragma optimize("", on)
}