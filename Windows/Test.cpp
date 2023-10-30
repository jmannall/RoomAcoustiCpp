#include "pch.h"
#include "CppUnitTest.h"
#include "AudioManager.h"
#include "DiffractionGeometry.h"
#include "GeometryManager.h"
//#include "vec3.h"
#include "HelloWorld.h"
#define NOMINMAX
#include <Windows.h>
#include <fstream>
#include <chrono>
//#include "firfilter.h"

#include "BinauralSpatializer/3DTI_BinauralSpatializer.h"
#include "HRTF/HRTFFactory.h"
#include "HRTF/HRTFCereal.h"
#include "ILD/ILDCereal.h"

#include "Spatialiser/Interface.h"
#include "Spatialiser/Types.h"

#include "Spatialiser/Room.h"
#include "Spatialiser/FDN.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//namespace ThreeDTI
//{
//	TEST_CLASS(ThreeDTI)
//	{
//	public:
//
//		TEST_METHOD(Initialise)
//		{
//			// Instatiate //
//			Binaural::CCore myCore;
//
//			// Audio settings
//			int sampleRate = 48000;
//			int bufferSize = 1024;
//			myCore.SetAudioState({ sampleRate, bufferSize });
//
//			// HRT resampling
//			int HRTF_resamplingStep = 45;
//			myCore.SetHRTFResamplingStep(HRTF_resamplingStep);
//
//			// Listener
//			shared_ptr<Binaural::CListener> listener = myCore.CreateListener();
//
//			// Sources //
//			shared_ptr<Binaural::CSingleSourceDSP> mySource;
//			mySource = myCore.CreateSingleSourceDSP();
//
//			// Spatialisation mode
//			mySource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
//
//			string resourcePath = "D:\Joshua Mannall\GitHub\3dti_AudioToolkit\resources";
//			string hrtfPath = "\HRTF\3DTI\3DTI_HRTF_IRC1008_128s_48000Hz.3dti-hrtf";
//			string sofaPath = "\HRTF\SOFA\3DTI_HRTF_IRC1008_128s_48000Hz.sofa";
//			bool specifiedDelays;
//			bool result = HRTF::CreateFromSofa(hrtfPath + sofaPath, listener, specifiedDelays);
//			//bool result = HRTF::CreateFrom3dti(resourcePath + hrtfPath, listener);
//			if (result) { cout << "HRTF has been loaded successfully"; }
//
//			string ildPath = "\ILD\NearFieldCompensation_ILD_48000";
//			result = ILD::CreateFrom3dti_ILDNearFieldEffectTable(resourcePath + ildPath, listener);
//
//			// If high performance mode
//			// string ildPath = "\ILD\HRTF_ILD_48000";
//			// result = ILD::CreateFrom3dti_ILDSpatializationTable(resourcePath + ildPath, listener);
//			if (result) { cout << "ILD Near Field Effect simulation file has been loaded successfully"; }
//
//			Assert::AreEqual(0.0f, 0.0f);
//		}
//	};
//}

namespace InitialiseTests
{
	TEST_CLASS(InitialiseTests)
	{
	public:

		TEST_METHOD(GeometryTest)
		{

			std::ofstream out("out.txt");
			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 2;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(-2, 1, 1));

			size_t sID[5] = { 0 };
			size_t wID[5] = { 0 };
			sID[0] = GA::InitSource(vec3(2, 2, 3));
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge wedge = Wedge(vec3(1, 0, 0), vec3(1, 8, 0), normals);
			wID[0] = GA::InitWedge(wedge);

			sID[1] = GA::InitSource(vec3(1, 3, 4));
			wedge = Wedge(vec3(0, 0, 0), vec3(0, 10, 0), normals);
			wID[1] = GA::InitWedge(wedge);

			sID[2] = GA::InitSource(vec3(3.4f, 1, -2.51f));
			vec3 normals2[] = { vec3(-1, 0, 0), vec3(0, 0, -1) };
			wedge = Wedge(vec3(1, 0, 0), vec3(1, 4, 0), normals2);
			wID[2] = GA::InitWedge(wedge);

			float z = GA::GetZ(sID[2], wID[2]);
			vec3 zA = vec3(1, 0, 0) + z * (vec3(1, 4, 0) - vec3(1, 0, 0));
			static float* buffer = nullptr;

			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);
			in[0] = 1.0f;
			GA::SendAudio(sID[0], wID[0], in, numFrames);
			GA::GetOutput(&buffer);

			in[0] = 0.0f;
			GA::SendAudio(sID[0], wID[0], in, numFrames);
			GA::GetOutput(&buffer);

			GA::SendAudio(sID[0], wID[0], in, numFrames);
			GA::GetOutput(&buffer);

			GA::SendAudio(sID[0], wID[0], in, numFrames);
			GA::GetOutput(&buffer);
			GA::SendAudio(sID[0], wID[0], in, numFrames);
			GA::GetOutput(&buffer);
			GA::SendAudio(sID[0], wID[0], in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			Sleep(5000);
			z = GA::GetZ(sID[0], wID[0]);
			GA::UpdateSourceData(sID[0], vec3(3, 3, 3));

			
			Sleep(5000);
			z = GA::GetZ(sID[0], wID[0]);

			sID[3] = GA::InitSource(vec3(4, 7, 2));
			wedge = Wedge(vec3(0, 0, 0), vec3(0, 9, 0), normals);
			wID[3] = GA::InitWedge(wedge);

			GA::RemoveSource(sID[1]);
			GA::RemoveWedge(wID[1]);

			sID[4] = GA::InitSource(vec3(2, 7, 2));
			wedge = Wedge(vec3(0, 0, 0), vec3(0, 6, 0), normals);
			wID[4] = GA::InitWedge(wedge);

			GA::ExitGeometry();

			GA::RemoveSource(sID[0]);

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}

		TEST_METHOD(ValidateModel)
		{

			std::ofstream out("out.txt");
			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 2;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(-cosf(PI / 9), 2, sinf(PI / 9)));

			size_t sID = GA::InitSource(vec3(2 * sinf(PI / 6), 1.5, -2 * cosf(PI / 6)));
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge wedge = Wedge(vec3(0, 0, 0), vec3(0, 4, 0), normals);
			size_t wID = GA::InitWedge(wedge);

			Model model = Model::lowPass;
			GA::SetModel(model);

			Sleep(500);

			static float* buffer = nullptr;
			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);

			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			in[0] = 1.0f;
			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again

			Assert::AreEqual(0.0614293031f, in[0], 0.0001f, L"Incorrect Sample 1");
			Assert::AreEqual(0.115311489f, in[2], 0.0001f, L"Incorrect Sample 2");
			Assert::AreEqual(0.101144485f, in[4], 0.0001f, L"Incorrect Sample 3");
			Assert::AreEqual(0.0887180194f, in[6], 0.0001f, L"Incorrect Sample 4");
		}

		TEST_METHOD(UDFAModel)
		{

			std::ofstream out("out.txt");
			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 1;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(-cosf(PI / 3), 1, sinf(PI / 3)));

			size_t sID = GA::InitSource(vec3(2 * sinf(PI / 6), 1, -2 * cosf(PI / 6)));
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge wedge = Wedge(vec3(0, 0, 0), vec3(0, 4, 0), normals);
			size_t wID = GA::InitWedge(wedge);

			Model model = Model::udfa;
			GA::SetModel(model);

			Sleep(1000);

			static float* buffer = nullptr;
			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);

			in[0] = 1.0f;
			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}

		TEST_METHOD(UTDModel)
		{

			std::ofstream out("out.txt");
			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 1;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(-cosf(PI / 9), 1, sinf(PI / 9)));

			size_t sID = GA::InitSource(vec3(2 * sinf(PI / 18), 1, -2 * cosf(PI / 18)));
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge wedge = Wedge(vec3(0, 0, 0), vec3(0, 4, 0), normals);
			size_t wID = GA::InitWedge(wedge);

			Model model = Model::utd;
			GA::SetModel(model);

			Sleep(1000);

			static float* buffer = nullptr;
			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);

			for (int i = 0; i < 10; i++)
			{
				GA::SendAudio(sID, wID, in, numFrames);
			}

			in[0] = 1.0f;
			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}

		TEST_METHOD(NNModel)
		{

			std::ofstream out("out.txt");
			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 1;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(-cosf(PI / 9), 1, sinf(PI / 9)));

			size_t sID = GA::InitSource(vec3(2 * sinf(PI / 18), 1, -2 * cosf(PI / 18)));
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge wedge = Wedge(vec3(0, 0, 0), vec3(0, 4, 0), normals);
			size_t wID = GA::InitWedge(wedge);

			Model model = Model::nnBest;
			GA::SetModel(model);

			Sleep(1000);

			static float* buffer = nullptr;
			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);

			for (int i = 0; i < 10; i++)
			{
				GA::SendAudio(sID, wID, in, numFrames);
			}
			
			in[0] = 1.0f;
			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}

		TEST_METHOD(BTMModel)
		{

			std::ofstream out("out.txt");
			std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 1;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(-cosf(PI / 9), 1, sinf(PI / 9)));

			size_t sID = GA::InitSource(vec3(2 * sinf(PI / 18), 1, -2 * cosf(PI / 18)));
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge wedge = Wedge(vec3(0, 0, 0), vec3(0, 4, 0), normals);
			size_t wID = GA::InitWedge(wedge);

			Model model = Model::btm;
			GA::SetModel(model);

			Sleep(1000);

			Source s = Source(0.0871557427476582f, 2.0f, -0.996194698091746f);
			Receiver r = Receiver(-0.984807753012208, 2.0f, 0.173648177666930f);
			vec3 base = vec3(0, 0, 0);
			vec3 top = vec3(0, 4, 0);
			Wedge w = Wedge(base, top, normals);
			DiffractionPath path = DiffractionPath(&s, &r, &w);

			static float* buffer = nullptr;
			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);

			for (int i = 0; i < 10; i++)
			{
				GA::SendAudio(sID, wID, in, numFrames);
			}

			in[0] = 1.0f;
			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			Assert::AreEqual(0.000000000f, in[0]);
			Assert::AreEqual(0.109138496f, in[2]);
			Assert::AreEqual(0.0487937927f, in[4]);
			Assert::AreEqual(0.0362281837f, in[6]);

			GA::UpdateSourceData(sID, vec3(0.0871557427476582f, 2.0f, -0.996194698091746f));
			GA::UpdateWedgeData(wID, w);

			Sleep(1000);

			std::fill_n(in, numFrames, 0.0f);

			for (int i = 0; i < 10; i++)
			{
				GA::SendAudio(sID, wID, in, numFrames);
			}

			in[0] = 1.0f;
			//in[2] = 1.0f;
			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			Assert::AreEqual(0.0974655151f, in[0]);
			Assert::AreEqual(0.0711593553f, in[2]);
			Assert::AreEqual(0.0477256738f, in[4]);
			Assert::AreEqual(0.0378913917f, in[6]);
			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}

		TEST_METHOD(CheckModels)
		{

			std::ofstream out("out.txt");
			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 1;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(0.658, 1, 0.9397));

			size_t sID = GA::InitSource(vec3(2.9696, 1, -0.3473));
			vec3 normals[] = { vec3(-1, 0, 0), vec3(0, 0, -1) };
			Wedge wedge = Wedge(vec3(1, 0, 0), vec3(1, 4, 0), normals);
			size_t wID = GA::InitWedge(wedge);

			Model model = Model::nnBest;
			GA::SetModel(model);

			Sleep(1000);

			static float* buffer = nullptr;
			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);

			for (int i = 0; i < 10; i++)
			{
				GA::SendAudio(sID, wID, in, numFrames);
			}

			in[0] = 1.0f;
			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			model = Model::udfa;
			GA::SetModel(model);
			float in2[numFrames];
			std::fill_n(in2, numFrames, 0.0f);

			in2[0] = 1.0f;
			GA::SendAudio(sID, wID, in2, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in2[i] = *buffer++;
			}

			model = Model::btm;
			GA::SetModel(model);
			float in3[numFrames];
			std::fill_n(in3, numFrames, 0.0f);

			in3[0] = 1.0f;
			GA::SendAudio(sID, wID, in3, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in3[i] = *buffer++;
			}
			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}

		TEST_METHOD(ChangeModel)
		{

			std::ofstream out("out.txt");
			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 1;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(-cosf(PI / 9), 2, sinf(PI / 9)));

			size_t sID = GA::InitSource(vec3(2 * sinf(PI / 6), 1.5, -2 * cosf(PI / 6)));
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge wedge = Wedge(vec3(0, 0, 0), vec3(0, 4, 0), normals);
			size_t wID = GA::InitWedge(wedge);

			Model model = Model::attenuate;
			GA::SetModel(model);

			Sleep(500);

			static float* buffer = nullptr;
			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);

			for (int i = 0; i < 20; i++)
			{
				GA::SendAudio(sID, wID, in, numFrames);
			}

			in[0] = 1.0f;
			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			Assert::AreEqual(0.999938965f, in[0], 0.0001f, L"Attenuate Incorrect");
			Assert::AreEqual(0.0f, in[2]);

			model = Model::lowPass;
			GA::SetModel(model);

			std::fill_n(in, numFrames, 0.0f);

			in[0] = 1.0f;
			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			GA::SendAudio(sID, wID, in, numFrames);
			GA::GetOutput(&buffer);

			for (int i = 0; i < numFrames; i++)
			{
				in[i] = *buffer++;
			}

			Assert::AreEqual(0.0614293031f, in[0], 0.0001f, L"LPF Incorrect");
			Assert::AreEqual(0.115311489f, in[2], 0.0001f, L"LPF Incorrect");

			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}
	};
}

namespace RuntimeTests
{
	TEST_CLASS(RuntimeTests)
	{
	public:

		TEST_METHOD(AudioProcessing)
		{
			std::ofstream out("AudioProcessing.txt");
			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 1;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(-cosf(PI / 9), 1, sinf(PI / 9)));

			size_t sID = GA::InitSource(vec3(2 * sinf(PI / 18), 1, -2 * cosf(PI / 18)));
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge wedge = Wedge(vec3(0, 0, 0), vec3(0, 4, 0), normals);
			size_t wID = GA::InitWedge(wedge);

			Model model = Model::btm;
			GA::SetModel(model);

			static float* buffer = nullptr;
			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);

			int numIterations = 10000;
			for (int i = 0; i < numIterations; i++)
			{
				GA::SendAudio(sID, wID, in, numFrames);
			}

			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}

		TEST_METHOD(ModelUpdates)
		{
			std::ofstream out("ModelUpdates.txt");
			std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			DSPConfig config;
			config.dspSmoothingFactor = 1;
			config.samplingRate = 48000;
			GA::InitGeometry(&config);
			GA::SetListenerPosition(vec3(-cosf(PI / 9), 1, sinf(PI / 9)));

			size_t sID = GA::InitSource(vec3(2 * sinf(PI / 18), 1, -2 * cosf(PI / 18)));
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge wedge = Wedge(vec3(0, 0, 0), vec3(0, 4, 0), normals);
			size_t wID = GA::InitWedge(wedge);

			Model model = Model::off;
			GA::SetModel(model);

			Sleep(10000);

			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}

		TEST_METHOD(ISMProcessing)
		{
			std::ofstream out("ISMProcessing.txt");
			std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			int maxRefOrder = 3;

			Spatialiser::Config config;

			config.sampleRate = 44100;
			const int numFrames = 4096;
			config.bufferSize = numFrames;
			config.hrtfResamplingStep = 30;
			config.maxRefOrder = 3;
			Spatialiser::Init(&config);

			float vert[] = { 0.0f, 0.0f, 0.0f,
							4.0f, 0.0f, 0.0f,
							4.0f, 2.0f, 0.0f,
							0.0f, 2.0f, 0.0f };
			vec3 normal = vec3(0.0f, 0.0f, 1.0f);
			int numVert = 4;

			Spatialiser::Absorption absorbtion = Spatialiser::Absorption(0.1f, 0.1f, 0.1f, 0.1f, 0.1f);
			size_t wID1 = Spatialiser::InitWall(normal, &vert[0], (size_t)numVert, absorbtion, Spatialiser::ReverbWall::negZ);
			bool result = Spatialiser::FilesLoaded();

			Spatialiser::UpdateListener(vec3(1.0f, 1.0f, 1.0f), quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			size_t sID1 = Spatialiser::InitSource();
			Spatialiser::UpdateSource(sID1, vec3(2.5f, 1.0f, 4.0f), quaternion(0.0f, 1.0f, 0.0f, 0.0f));

			Sleep(10000);

			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}

		TEST_METHOD(ISMAudioProcessing)
		{
			std::ofstream out("ISMAudioProcessing.txt");
			std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
			std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

			int maxRefOrder = 3;

			Spatialiser::Config config;

			config.sampleRate = 44100;
			const int numFrames = 2048;
			config.bufferSize = numFrames;
			config.hrtfResamplingStep = 30;
			config.maxRefOrder = 0;
			Spatialiser::Init(&config);
			float vert[] = { 0.0f, 0.0f, 0.0f,
							4.0f, 0.0f, 0.0f,
							4.0f, 2.0f, 0.0f,
							0.0f, 2.0f, 0.0f };
			vec3 normal = vec3(0.0f, 0.0f, 1.0f);
			int numVert = 4;

			Spatialiser::Absorption absorbtion = Spatialiser::Absorption(0.1f, 0.1f, 0.1f, 0.1f, 0.1f);
			size_t wID1 = Spatialiser::InitWall(normal, &vert[0], (size_t)numVert, absorbtion, Spatialiser::ReverbWall::negZ);
			bool result = Spatialiser::FilesLoaded();

			Spatialiser::UpdateListener(vec3(1.0f, 1.0f, 1.0f), quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			size_t sID1 = Spatialiser::InitSource();
			Spatialiser::UpdateSource(sID1, vec3(2.5f, 1.0f, 4.0f), quaternion(0.0f, 1.0f, 0.0f, 0.0f));

			static float* buffer = nullptr;
			float in[numFrames];
			std::fill_n(in, numFrames, 0.0f);

			int numIterations = 1000;
			for (int i = 0; i < numIterations; i++)
			{
				Spatialiser::SubmitAudio(sID1, in, numFrames);
			}

			GA::ExitGeometry();

			std::cout.rdbuf(coutbuf); //reset to standard output again
		}
	};
}

namespace DefaultTests
{
	TEST_CLASS(DefaultTests)
	{
	public:

		TEST_METHOD(HelloWorldTest)
		{
			HelloWorld hello;
			std::string compare = "Hello World!";
			Assert::AreEqual(compare, hello.GetString());
		}

		TEST_METHOD(LinkageTest)
		{
			Assert::AreEqual(1.0f, Test());
		}
	};
}

namespace FDNTests
{
	TEST_CLASS(FDNTests)
	{
	public:

		TEST_METHOD(Initialise)
		{
			Spatialiser::FrequencyDependence T60 = Spatialiser::FrequencyDependence();
			size_t numChannels = 12;
			vec dimensions = vec(numChannels);
			for (int i = 0; i < numChannels; i++)
			{
				dimensions[i] = i / 2;
			}
			int fs = 48000;
			Spatialiser::FDN fdn = Spatialiser::FDN(T60, dimensions, numChannels, fs);

			/*rowvec in = rowvec(12);
			for (int i = 0; i < 12; i++)
			{
				in[i] = 1.0f;
			}
			rowvec out = fdn.GetOutput(in);
			for (int i = 0; i < 12; i++)
			{
				in[i] = 0.0f;
			}
			for (int i = 0; i < 140; i++)
			{
				rowvec out = fdn.GetOutput(in);
			}
			for (int i = 0; i < 200; i++)
			{
				rowvec out = fdn.GetOutput(in);
			}*/
		}
	};
}

namespace DSPTests
{
	TEST_CLASS(BufferTests)
	{
	public:
		
		TEST_METHOD(Initialise)
		{
			Buffer test;

			size_t num = 100;
			test.ResizeBuffer(num);
			Assert::AreEqual(0.0f, test[0]);

		}
	};

	TEST_CLASS(VectorTests)
	{
	public:

		TEST_METHOD(InitialiseTest)
		{
			vec3 test = vec3(1, 2, 3);
			Assert::AreEqual(1.0f, test.x);
			Assert::AreEqual(2.0f, test.y);
			Assert::AreEqual(3.0f, test.z);
		}

		TEST_METHOD(LengthTest)
		{
			vec3 test = vec3(2, 2, 1);
			float lengthCheck = test.Length();
			Assert::AreEqual(3.0f, test.Length());
		}

		TEST_METHOD(UnitVectorTest)
		{
			vec3 test = vec3(0, 2, 0);
			vec3 check = UnitVector(test);

			Assert::AreEqual(0.0f, check.x);
			Assert::AreEqual(1.0f, check.y);
			Assert::AreEqual(0.0f, check.z);
		}

		TEST_METHOD(DotProductTest)
		{
			vec3 test1 = vec3(1, 0, 0);
			vec3 test2 = vec3(1, 1, 0);
			float check = Dot(test1, test2) / test2.Length();
			Assert::AreEqual(sqrtf(0.5), check);
		}

		TEST_METHOD(CrossProductTest)
		{
			vec3 test1 = vec3(1, 0, 0);
			vec3 test2 = vec3(0, 1, 0);
			vec3 check = Cross(test1, test2);
			Assert::AreEqual(0.0f, check.x);
			Assert::AreEqual(0.0f, check.y);
			Assert::AreEqual(1.0f, check.z);
		}
	};

	TEST_CLASS(MatrixTests)
	{
	public:
#
		TEST_METHOD(vecTest)
		{
			float v[] = { 3.0f, 4.5f, 5.0f };
			const float* vPtr = &v[0];

			vec test = vec(vPtr, 3);
			Assert::AreEqual(3.0f, test[0]);
			Assert::AreEqual(4.5f, test[1]);
			Assert::AreEqual(5.0f, test[2]);
		}

		TEST_METHOD(Multiply)
		{
			int r = 2;
			int c = 3;
			float in1[] = { 2, 3, 4, 1, 5, 2 };
			matrix mat1 = matrix(&in1[0], r, c);

			float in2[] = { 1, 1, 4, 7, 2, 3 };
			matrix mat2 = matrix(&in2[0], c, r);

			matrix out = mat1 * mat2;
			Assert::AreEqual(22.0f, out.GetEntry(0, 0));
			Assert::AreEqual(35.0f, out.GetEntry(0, 1));
			Assert::AreEqual(25.0f, out.GetEntry(1, 0));
			Assert::AreEqual(42.0f, out.GetEntry(1, 1));
		}

		TEST_METHOD(Sum)
		{
			int r = 2;
			int c = 3;
			float in1[] = { 2, 3, 4, 1, 5, 2 };
			matrix mat1 = matrix(&in1[0], r, c);

			float in2[] = { 1, 1, 4, 7, 2, 3 };
			matrix mat2 = matrix(&in2[0], r, c);

			matrix out = mat1 + mat2;
			Assert::AreEqual(3.0f, out.GetEntry(0, 0));
			Assert::AreEqual(4.0f, out.GetEntry(0, 1));
			Assert::AreEqual(8.0f, out.GetEntry(0, 2));
			Assert::AreEqual(8.0f, out.GetEntry(1, 0));
			Assert::AreEqual(7.0f, out.GetEntry(1, 1));
			Assert::AreEqual(5.0f, out.GetEntry(1, 2));

			out = mat1 - mat2;
			Assert::AreEqual(1.0f, out.GetEntry(0, 0));
			Assert::AreEqual(2.0f, out.GetEntry(0, 1));
			Assert::AreEqual(0.0f, out.GetEntry(0, 2));
			Assert::AreEqual(-6.0f, out.GetEntry(1, 0));
			Assert::AreEqual(3.0f, out.GetEntry(1, 1));
			Assert::AreEqual(-1.0f, out.GetEntry(1, 2));

			matrix mat3;
			mat3 = mat1;
			mat1 += mat2;
			Assert::AreEqual(3.0f, mat1.GetEntry(0, 0));
			Assert::AreEqual(4.0f, mat1.GetEntry(0, 1));
			Assert::AreEqual(8.0f, mat1.GetEntry(0, 2));
			Assert::AreEqual(8.0f, mat1.GetEntry(1, 0));
			Assert::AreEqual(7.0f, mat1.GetEntry(1, 1));
			Assert::AreEqual(5.0f, mat1.GetEntry(1, 2));

			mat3 -= mat2;
			Assert::AreEqual(1.0f, mat3.GetEntry(0, 0));
			Assert::AreEqual(2.0f, mat3.GetEntry(0, 1));
			Assert::AreEqual(0.0f, mat3.GetEntry(0, 2));
			Assert::AreEqual(-6.0f, mat3.GetEntry(1, 0));
			Assert::AreEqual(3.0f, mat3.GetEntry(1, 1));
			Assert::AreEqual(-1.0f, mat3.GetEntry(1, 2));
		}

		TEST_METHOD(Equals)
		{
			rowvec test1 = rowvec(12);
			matrix test2 = matrix(12, 12);
			matrix test3 = test1 * test2;
			matrix test4 = test3;
			rowvec test5 = rowvec(12);
			test5 = test3;

			Assert::AreEqual(1, test4.Rows());
			Assert::AreEqual(12, test4.Cols());
			Assert::AreEqual(1, test5.Rows());
			Assert::AreEqual(12, test5.Cols());
		}
	};
	TEST_CLASS(FilterTests)
	{
	public:

		TEST_METHOD(LRFilter)
		{
			int fs = 48000;
			LinkwitzRiley lr = LinkwitzRiley(fs);

			float g[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			lr.UpdateParameters(g);

			const int numSamples = 10;
			float in[numSamples];
			float out[numSamples];
			std::fill_n(in, numSamples, 0.0f);

			in[0] = 1.0f;
			for (int i = 0; i < numSamples; i++)
			{
				out[i] = lr.GetOutput(in[i]);
			}

			Assert::AreEqual(0.446514338f, out[0], 0.0001f, L"Incorrect Sample 1");
			Assert::AreEqual(-0.699016035f, out[1], 0.0001f, L"Incorrect Sample 2");
			Assert::AreEqual(-0.0866665095f, out[2], 0.0001f, L"Incorrect Sample 3");
			Assert::AreEqual(0.188288942f, out[3], 0.0001f, L"Incorrect Sample 4");
			Assert::AreEqual(0.252293527f, out[4], 0.0001f, L"Incorrect Sample 5");
			Assert::AreEqual(0.203361660f, out[5], 0.0001f, L"Incorrect Sample 6");
			Assert::AreEqual(0.109714866f, out[6], 0.0001f, L"Incorrect Sample 7");
			Assert::AreEqual(0.0135267237f, out[7], 0.0001f, L"Incorrect Sample 8");
			Assert::AreEqual(-0.0631750003f, out[8], 0.0001f, L"Incorrect Sample 9");
			Assert::AreEqual(-0.112342872f, out[9], 0.0001f, L"Incorrect Sample 10");
		}

		TEST_METHOD(ParaEQ)
		{
			int fs = 48000;
			size_t order = 4;

			float fc[] = { 250, 500, 1000, 2000, 4000 };
			float g[] = { 1- 0.02, 1 - 0.06, 1 - 0.15, 1 - 0.25, 1 - 0.45 };


			ParametricEQ eq = ParametricEQ(order, fc, g, fs);

			const int numSamples = 10;
			float in[numSamples];
			float out[numSamples];
			std::fill_n(in, numSamples, 0.0f);

			in[0] = 1.0f;
			for (int i = 0; i < numSamples; i++)
			{
				out[i] = eq.GetOutput(in[i]);
			}

			Assert::AreEqual(0.578350827f, out[0], 0.0001f, L"Incorrect Sample 1");
			Assert::AreEqual(0.056732276f, out[1], 0.0001f, L"Incorrect Sample 2");
			Assert::AreEqual(0.055465145f, out[2], 0.0001f, L"Incorrect Sample 3");
			Assert::AreEqual(0.051770401f, out[3], 0.0001f, L"Incorrect Sample 4");
			Assert::AreEqual(0.046152923f, out[4], 0.0001f, L"Incorrect Sample 5");
			Assert::AreEqual(0.039299639f, out[5], 0.0001f, L"Incorrect Sample 6");
			Assert::AreEqual(0.031929693f, out[6], 0.0001f, L"Incorrect Sample 7");
			Assert::AreEqual(0.024717277f, out[7], 0.0001f, L"Incorrect Sample 8");
			Assert::AreEqual(0.018197102f, out[8], 0.0001f, L"Incorrect Sample 9");
			Assert::AreEqual(0.012740087f, out[9], 0.0001f, L"Incorrect Sample 10");
		}
	};
}

namespace GeometryTests
{
	TEST_CLASS(DiffractionGeometryTests)
	{
	public:

		TEST_METHOD(InitialiseWedge)
		{
			vec3 base = vec3(1, 0, 0);
			vec3 top = vec3(1, 4, 0);
			vec3 normals[2] = { vec3(-1, 0, 0), vec3(0, 0, -1) };
			Wedge test = Wedge(base, top, normals);

			Assert::AreEqual(4.0f, test.zW, 0.01f, L"Wedge Length Incorrect");
			Assert::AreEqual(270.0f, test.GetThetaW(), 0.01f, L"Wedge Angle Incorrect");
		}

		TEST_METHOD(CheckZCoord)
		{
			vec3 base = vec3(1, 1, 1);
			vec3 top = vec3(1, 3, 1);
			vec3 normals[2] = { vec3(0, 0, 1), vec3(1, 0, 0) };
			Wedge test = Wedge(base, top, normals);

			float z = 1;
			vec3 check = test.GetEdgeCoord(z);
			Assert::AreEqual(1.0f, check.x);
			Assert::AreEqual(2.0f, check.y);
			Assert::AreEqual(1.0f, check.z);

		}

		TEST_METHOD(InitialiseDiffractionPath)
		{
			vec3 base = vec3(0, 2, 0);
			vec3 top = vec3(0, 4, 0);
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge test = Wedge(base, top, normals);

			Source source = Source(1, 2.5, 0);
			Receiver receiver = Receiver(0, 3.5, 1);

			DiffractionPath check = DiffractionPath(&source, &receiver, &test);

			float z = 1;
			vec3 apex = test.GetEdgeCoord(z);

			SRData data = check.sData;
			float zA = check.zA;

			Assert::AreEqual(1.0f, data.r, 0.01f, L"Radius Incorrect");
			Assert::AreEqual(0.5f, data.z, 0.01f, L"Z Value Incorrect");
			Assert::AreEqual(1.0f, zA, 0.01f, L"Apex Incorrect");
			Assert::AreEqual(sqrtf(1.0f + 0.5f * 0.5f), data.d, 0.01f, L"Distance Incorrect");
			Assert::AreEqual(90.0f, Rad2Deg(data.t), 0.01f, L"Theta Incorrect");
			Assert::IsTrue(check.valid, L"Path Invalid");
		}
	};
}

namespace BTMTests
{
	TEST_CLASS(BTMTests)
	{
	public:

		TEST_METHOD(BTMTest)
		{
			Source s = Source(0.0871557427476582f, 2.0f, -0.996194698091746f);
			Receiver r = Receiver(-0.984807753012208, 2.0f, 0.173648177666930f);
			vec3 base = vec3(0, 0, 0);
			vec3 top = vec3(0, 4, 0);
			vec3 normals[] = { vec3(1, 0, 0), vec3(0, 0, 1) };
			Wedge w = Wedge(base, top, normals);
			DiffractionPath path = DiffractionPath(&s, &r, &w);
			BTM btm = BTM(&path, 48000);
		}
	};
}

namespace ThreeDTITests
{
	TEST_CLASS(ThreeDTI)
	{
	public:

		TEST_METHOD(Initialise)
		{
			Binaural::CCore myCore;

			// Audio settings
			int sampleRate = 48000;
			int bufferSize = 1024;
			myCore.SetAudioState({ sampleRate, bufferSize });

			// HRT resampling
			int HRTF_resamplingStep = 45;
			myCore.SetHRTFResamplingStep(HRTF_resamplingStep);

			// Listener
			shared_ptr<Binaural::CListener> listener = myCore.CreateListener();

			// Sources //
			shared_ptr<Binaural::CSingleSourceDSP> mySource;
			mySource = myCore.CreateSingleSourceDSP();

			// Spatialisation mode
			mySource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);

			string resourcePath = "D:\\Joshua Mannall\\GitHub\\3dti_AudioToolkit\\resources";
			string hrtfPath = "\\HRTF\\3DTI\\3DTI_HRTF_IRC1008_128s_48000Hz.3dti-hrtf";
			string sofaPath = "\\HRTF\\SOFA\\3DTI_HRTF_IRC1008_128s_48000Hz.sofa";
			bool specifiedDelays;
			bool result = HRTF::CreateFromSofa(resourcePath + sofaPath, listener, specifiedDelays);
			//bool result = HRTF::CreateFrom3dti(resourcePath + hrtfPath, listener);
			if (result) { cout << "HRTF has been loaded successfully"; }
			Assert::IsTrue(result, L"HRTF load failed");

			string ildPath = "\\ILD\\NearFieldCompensation_ILD_48000.3dti-ild";
			result = ILD::CreateFrom3dti_ILDNearFieldEffectTable(resourcePath + ildPath, listener);

			// If high performance mode
			// string ildPath = "\ILD\HRTF_ILD_48000";
			// result = ILD::CreateFrom3dti_ILDSpatializationTable(resourcePath + ildPath, listener);
			if (result) { cout << "ILD Near Field Effect simulation file has been loaded successfully"; }

			Assert::IsTrue(result, L"ILD load failed");
		}
	};
}

namespace HRTFTests
{
	TEST_CLASS(HRTFTests)
	{
	public:
		TEST_METHOD(Initialise)
		{
			Spatialiser::Config config;

			config.sampleRate = 44100;
			config.bufferSize = 1024;
			config.hrtfResamplingStep = 30;
			Spatialiser::Init(&config);

			bool result = Spatialiser::FilesLoaded();
			
			Spatialiser::UpdateListener(vec3(2, 2, 2), quaternion(1, 0, 1, 0));
			size_t sID1 = Spatialiser::InitSource();
			size_t sID2 = Spatialiser::InitSource();
			size_t sID3 = Spatialiser::InitSource();
			Spatialiser::UpdateSource(sID1, vec3(1, 2, 1), quaternion(1, 0, 1, 0));
			Spatialiser::UpdateSource(sID2, vec3(3, 2, 0), quaternion(1, 0, 0, 1));
			Spatialiser::UpdateSource(sID3, vec3(3, 2, 0), quaternion(1, 1, 0, 1));

			Spatialiser::UpdateSource(sID1, vec3(4, 2, 3), quaternion(1, 1, 1, 1));
			Spatialiser::RemoveSource(sID2);

			size_t sID4 = Spatialiser::InitSource();
			size_t sID5 = Spatialiser::InitSource();

			Spatialiser::Exit();
		}

		TEST_METHOD(Audio)
		{
			Spatialiser::Config config;

			config.sampleRate = 44100;
			config.bufferSize = 2048;
			config.hrtfResamplingStep = 30;
			Spatialiser::Init(&config);

			bool result = Spatialiser::FilesLoaded();

			Spatialiser::UpdateListener(vec3(0, 1, 0), quaternion(1, 1, 0, 0));
			size_t sID1 = Spatialiser::InitSource();
			size_t sID2 = Spatialiser::InitSource();
			size_t sID3 = Spatialiser::InitSource();
			Spatialiser::UpdateSource(sID1, vec3(1, 1, 0), quaternion(1, -1, 0, 0));
			Spatialiser::UpdateSource(sID2, vec3(2, 1, 0), quaternion(1, -1, 0, 0));
			Spatialiser::UpdateSource(sID3, vec3(3, 1, 0), quaternion(1, -1, 0, 0));

			static float* buffer = nullptr;
			const int numFrames = 2048;
			float in[numFrames];
			std::fill_n(in, config.bufferSize, 0.0f);

			for (int i = 0; i < 10; i++)
			{
				Spatialiser::SubmitAudio(sID1, in, (size_t)numFrames);
				Spatialiser::SubmitAudio(sID2, in, (size_t)numFrames);
				Spatialiser::SubmitAudio(sID3, in, (size_t)numFrames);
				Spatialiser::GetOutput(&buffer);
			}

			in[0] = 1.0f;
			Spatialiser::SubmitAudio(sID1, in, (size_t)numFrames);
			Spatialiser::SubmitAudio(sID2, in, (size_t)numFrames);
			Spatialiser::SubmitAudio(sID3, in, (size_t)numFrames);

			Spatialiser::GetOutput(&buffer);

			float out[numFrames];
			std::fill_n(out, config.bufferSize, 0.0f);
			for (int i = 0; i < numFrames; i++)
			{
				out[i] = *buffer++;
			}

			Spatialiser::Exit();
		}
	};
}

namespace ImageSource
{
	TEST_CLASS(ImageSource)
	{
	public:
		TEST_METHOD(Run)
		{
			//int maxRefOrder = 3;

			//Spatialiser::Config config;

			//config.sampleRate = 44100;
			//const int numFrames = 4096;
			//config.bufferSize = numFrames;
			//config.hrtfResamplingStep = 30;
			//config.maxRefOrder = maxRefOrder;
			//config.hrtfMode = Spatialiser::HRTFMode::quality;
			//Spatialiser::Init(&config);
			//int numVert = 4;
			//Spatialiser::Absorption absorbtion = Spatialiser::Absorption(0.02f, 0.04f, 0.06f, 0.08f, 0.1f); // Concrete
			////Spatialiser::Absorption absorbtion = Spatialiser::Absorption(0.04f, 0.07f, 0.06f, 0.06f, 0.07f); // Wood
			////Spatialiser::Absorption absorbtion = Spatialiser::Absorption(0.02f, 0.06f, 0.15f, 0.35f, 0.45f); // Carpet

			//Spatialiser::Absorption carpetOnConcrete = Spatialiser::Absorption(0.06f, 0.15f, 0.4f, 0.6f, 0.6f); // Concrete
			//Spatialiser::Absorption concrete = Spatialiser::Absorption(0.01f, 0.02f, 0.02f, 0.02f, 0.03f); // Concrete
			//Spatialiser::Absorption windowGlass = Spatialiser::Absorption(0.2f, 0.2f, 0.1f, 0.07f, 0.04f); // Concrete
			//Spatialiser::Absorption gypsum = Spatialiser::Absorption(0.1f, 0.05f, 0.04f, 0.07f, 0.1f); // Concrete
			//Spatialiser::Absorption plywood = Spatialiser::Absorption(0.3f, 0.1f, 0.1f, 0.1f, 0.1f); // Concrete
			//Spatialiser::Absorption plasterSprayed = Spatialiser::Absorption(0.7f, 0.6f, 0.7f, 0.7f, 0.5f); // Concrete

			//vec3 pos = vec3(4.97f, 4.12f, 3.0f);
			//vec3 source = vec3(1.4f, 3.02f, 1.36);
			//vec3 receiver = vec3(2.77, 1.3, 1.51);
			////vec3 pos = vec3(3.0f, 2.0f, 5.0f);
			////vec3 source = vec3(1.0f, 1.0f, 1.5f);
			////vec3 receiver = vec3(2.0f, 1.0f, 1.5f);
			//float vert1[] = { 0.0f, 0.0f, pos.z,
			//				0.0f, pos.y, pos.z,
			//				pos.x, pos.y, pos.z,
			//				pos.x, 0.0f, pos.z };
			//vec3 normal1 = vec3(0.0f, 0.0f, -1.0f);
			//size_t wID1 = Spatialiser::InitWall(normal1, &vert1[0], (size_t)numVert, concrete, Spatialiser::ReverbWall::posZ);

			//float vert2[] = { pos.x, 0.0f, 0.0f,
			//				pos.x, pos.y, 0.0f,
			//				0.0f, pos.y, 0.0f,
			//				0.0f, 0.0f, 0.0f };
			//vec3 normal2 = vec3(0.0f, 0.0f, 1.0f);
			//size_t wID2 = Spatialiser::InitWall(normal2, &vert2[0], (size_t)numVert, carpetOnConcrete, Spatialiser::ReverbWall::negZ);

			//float vert3[] = { pos.x, 0.0f, pos.z,
			//				pos.x, pos.y, pos.z,
			//				pos.x, pos.y, 0.0f,
			//				pos.x, 0.0f, 0.0f };
			//vec3 normal3 = vec3(-1.0f, 0.0f, 0.0f);
			//size_t wID3 = Spatialiser::InitWall(normal3, &vert3[0], (size_t)numVert, windowGlass, Spatialiser::ReverbWall::posX);

			//float vert4[] = { 0.0f, 0.0f, 0.0f,
			//				0.0f, pos.y, 0.0f,
			//				0.0f, pos.y, pos.z,
			//				0.0f, 0.0f, pos.z };
			//vec3 normal4 = vec3(1.0f, 0.0f, 0.0f);
			//size_t wID4 = Spatialiser::InitWall(normal4, &vert4[0], (size_t)numVert, plywood, Spatialiser::ReverbWall::negX);

			//float vert5[] = { 0.0f, 0.0f, 0.0f,
			//				0.0f, 0.0f, pos.z,
			//				pos.x, 0.0f, pos.z,
			//				pos.x, 0.0f, 0.0f };
			//vec3 normal5 = vec3(0.0f, 1.0f, 0.0f);
			//size_t wID5 = Spatialiser::InitWall(normal5, &vert5[0], (size_t)numVert, gypsum, Spatialiser::ReverbWall::negY);

			//float vert6[] = { 0.0f, pos.y, pos.z,
			//				0.0f, pos.y, 0.0f,
			//				pos.x, pos.y, 0.0f,
			//				pos.x, pos.y, pos.z };
			//vec3 normal6 = vec3(0.0f, -1.0f, 0.0f);
			//size_t wID6 = Spatialiser::InitWall(normal6, &vert6[0], (size_t)numVert, plasterSprayed, Spatialiser::ReverbWall::posY);

			//float volume = pos.x * pos.y * pos.z;
			//float dim[] = { pos.x, pos.y, pos.z };
			//vec dimensions = vec(&dim[0], 3);
			//Spatialiser::SetFDNParameters(volume, dimensions);
			//bool result = Spatialiser::FilesLoaded();

			///*Spatialiser::UpdateListener(vec3(2.5f, 1.0f, 2.1f), quaternion(1.0f, 0.0f, 1.0f, 0.0f));

			//size_t sID1 = Spatialiser::InitSource();
			//Spatialiser::UpdateSource(sID1, vec3(2.8f, 1.0f, 2.5f), quaternion(1.0f, 0.0f, 1.0f, 0.0f));*/

			//Spatialiser::UpdateListener(receiver, quaternion(0.0f, 0.0f, 0.0f, 1.0f));
			//size_t sID1 = Spatialiser::InitSource();
			////Spatialiser::UpdateSource(sID1, vec3(2.5f, 1.0f, 4.4f), quaternion(0.0f, 1.0f, 0.0f, 0.0f));
			//Spatialiser::UpdateSource(sID1, source, quaternion(0.0f, 1.0f, 0.0f, 0.0f));
			////Spatialiser::UpdateSource(sID1, vec3(2.5f, -1.0f, 4.0f), quaternion(0.0f, 1.0f, 0.0f, 0.0f));

			//static float* buffer = nullptr;
			//float in[numFrames];
			//std::fill_n(in, numFrames, 0.0f);

			//Sleep(500);
			//Spatialiser::UpdateListener(receiver, quaternion(0.0f, 0.0f, 0.0f, 1.0f));
			//Spatialiser::UpdateSource(sID1, source, quaternion(0.0f, 1.0f, 0.0f, 0.0f));

			//for (int i = 0; i < 10; i++)
			//{
			//	Spatialiser::SubmitAudio(sID1, in, (size_t)numFrames);
			//	Spatialiser::GetOutput(&buffer);
			//	Spatialiser::UpdateListener(receiver, quaternion(0.0f, 0.0f, 0.0f, 1.0f));
			//	Spatialiser::UpdateSource(sID1, source, quaternion(0.0f, 1.0f, 0.0f, 0.0f));
			//}

			//in[0] = 1.0f;
			//Spatialiser::SubmitAudio(sID1, in, (size_t)numFrames);

			//Spatialiser::GetOutput(&buffer);

			//float outL[numFrames]; // Work this out
			//float outR[numFrames];
			//std::fill_n(outL, config.bufferSize, 0.0f);
			//std::fill_n(outR, config.bufferSize, 0.0f);
			//for (int i = 0; i < numFrames; i++)
			//{
			//	outL[i] = *buffer++;
			//	outR[i] = *buffer++;
			//}

			//in[0] = 0.0f;
			//for (int i = 0; i < 20; i++)
			//{
			//	Spatialiser::SubmitAudio(sID1, in, (size_t)numFrames);
			//	Spatialiser::GetOutput(&buffer);
			//	Spatialiser::UpdateListener(receiver, quaternion(0.0f, 0.0f, 0.0f, 1.0f));
			//	Spatialiser::UpdateSource(sID1, source, quaternion(0.0f, 1.0f, 0.0f, 0.0f));
			//}

			//in[0] = 1.0f;
			//Spatialiser::SubmitAudio(sID1, in, (size_t)numFrames);

			//Spatialiser::GetOutput(&buffer);

			//std::fill_n(outL, config.bufferSize, 0.0f);
			//std::fill_n(outR, config.bufferSize, 0.0f);
			//for (int i = 0; i < numFrames; i++)
			//{
			//	outL[i] = *buffer++;
			//	outR[i] = *buffer++;
			//}

			//Spatialiser::UpdateListener(vec3(1.0f, 1.0f, 2.0f), quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			//Sleep(1000);

			//Spatialiser::RemoveWall(wID2);

			//Sleep(500);

			//Spatialiser::UpdateSource(sID1, vec3(1.0f, 1.0f, 1.5f), quaternion(0.0f, 1.0f, 0.0f, 0.0f));

			//Sleep(500);

			//Spatialiser::RemoveSource(sID1);

			//Sleep(1000);

			//Spatialiser::Exit();
		}

		TEST_METHOD(InitWall)
		{
			Spatialiser::Config config;

			config.sampleRate = 44100;
			config.bufferSize = 2048;
			config.hrtfResamplingStep = 30;
			Spatialiser::Init(&config);

			float vert[] = { 0.0f, 0.0f, 0.0f,
							4.0f, 0.0f, 0.0f,
							4.0f, 2.0f, 0.0f,
							0.0f, 2.0f, 0.0f };
			vec3 normal = vec3(0.0f, 0.0f, 1.0f);
			int numVert = 4;

			Spatialiser::Absorption absorbtion = Spatialiser::Absorption(1.0f, 0.0f, 0.2f, 1.0f, 0.9f);
			size_t wID1 = Spatialiser::InitWall(normal, &vert[0], (size_t)numVert, absorbtion, Spatialiser::ReverbWall::negZ);

			float vert2[] = { 1.0f, 1.0f, 2.0f,
							4.0f, 1.0f, 2.0f,
							4.0f, 4.0f, 2.0f,
							1.0f, 4.0f, 2.0f };
			vec3 normal2 = vec3(0.0f, 0.0f, 1.0f);
			int numVert2 = 4;

			absorbtion = Spatialiser::Absorption(1.0f, 0.3f, 1.0f, 0.0f, 0.5f);

			Spatialiser::UpdateWall(wID1, normal2, &vert2[0], (size_t)numVert2, absorbtion, Spatialiser::ReverbWall::negZ);
			Spatialiser::RemoveWall(wID1, Spatialiser::ReverbWall::negZ);
		}

		TEST_METHOD(InitEdges)
		{
			Spatialiser::Config config;

			config.sampleRate = 44100;
			config.bufferSize = 2048;
			config.hrtfResamplingStep = 30;
			Spatialiser::Init(&config);

			float vert[] = { 0.0f, 0.0f, 0.0f,
							4.0f, 0.0f, 0.0f,
							4.0f, 2.0f, 0.0f,
							0.0f, 2.0f, 0.0f };
			vec3 normal = vec3(0.0f, 0.0f, 1.0f);
			int numVert = 4;

			Spatialiser::Absorption absorbtion = Spatialiser::Absorption(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
			size_t wID1 = Spatialiser::InitWall(normal, &vert[0], (size_t)numVert, absorbtion, Spatialiser::ReverbWall::negZ);

			float vert2[] = { 0.0f, 0.0f, 0.0f,
							0.0f, 2.0f, 0.0f,
							4.0f, 2.0f, 0.0f,
							4.0f, 0.0f, 0.0f };
			vec3 normal2 = vec3(0.0f, 0.0f, -1.0f);
			int numVert2 = 4;

			size_t wID2 = Spatialiser::InitWall(normal2, &vert2[0], (size_t)numVert2, absorbtion, Spatialiser::ReverbWall::negZ);
			
			float vert3[] = { 0.0f, 0.0f, 1.0f,
							0.0f, 2.0f, 1.0f,
							4.0f, 2.0f, 1.0f,
							4.0f, 0.0f, 1.0f };
			vec3 normal3 = vec3(0.0f, 0.0f, -1.0f);
			int numVert3 = 4;

			size_t wID3 = Spatialiser::InitWall(normal3, &vert3[0], (size_t)numVert3, absorbtion, Spatialiser::ReverbWall::negZ);

			float vert4[] = { 4.0f, 0.0f, 3.0f,
							4.0f, 0.0f, 1.0f,
							4.0f, 2.0f, 1.0f,
							4.0f, 2.0f, 3.0f, };
			vec3 normal4 = vec3(1.0f, 0.0f, 0.0f);
			int numVert4 = 4;

			size_t wID4 = Spatialiser::InitWall(normal4, &vert4[0], (size_t)numVert4, absorbtion, Spatialiser::ReverbWall::negZ);

			Spatialiser::RemoveWall(wID3, Spatialiser::ReverbWall::negZ);
			Spatialiser::RemoveWall(wID4, Spatialiser::ReverbWall::negZ);
			Spatialiser::RemoveWall(wID1, Spatialiser::ReverbWall::negZ);

			//Spatialiser::InitEdges();
		}
	};
}
//namespace MCLTests
//{
//	TEST_CLASS(MCLTests)
//	{
//	public:
//
//		TEST_METHOD(MCLTest)
//		{
//			std::vector<double> ir;
//			int n = 512;
//			ir.reserve(n);
//			std::fill_n(std::back_inserter(ir), n, 0.0f);
//			ir[0] = 1.0;
//			ir[2] = 0.5;
//			mcl::FirFilter filter = mcl::FirFilter(ir);
//
//			Buffer out = Buffer(n);
//			out[0] = filter.Filter(1.0f);
//			out[1] = filter.Filter(1.0f);
//			out[2] = filter.Filter(1.0f);
//		}
//	};
//}