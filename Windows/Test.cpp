#include "CppUnitTest.h"
#define NOMINMAX
#include <Windows.h>	// Sleep function

#include "Spatialiser/Interface.h"
#include "DSP/Buffer.h"

#include "UtilityFunctions.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RAC
{
	using namespace Common;
	using namespace Spatialiser;
	using namespace DSP;

#pragma optimize("", off)

	//////////////////// Create functions ////////////////////

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
			UpdateSpatialisationMode(SpatialisationMode::quality);
			UpdateReverbTimeFormula(ReverbFormula::Eyring);
			InitFDNMatrix(FDNMatrix::randomOrthogonal);
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

			for (int t = 0; t < 2; t++)
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
			UpdateSpatialisationMode(SpatialisationMode::quality);
			UpdateReverbTimeFormula(ReverbFormula::Sabine);
			InitFDNMatrix(FDNMatrix::randomOrthogonal);
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
			UpdateSourceDirectivity(sourceID, SourceDirectivity::omni);

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
#pragma optimize("", on)
}