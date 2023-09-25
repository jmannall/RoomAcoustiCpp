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
#include "firfilter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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

			Assert::AreEqual(0.0614293031f, in[0]);
			Assert::AreEqual(0.115311489f, in[2]);
			Assert::AreEqual(0.101144485f, in[4]);
			Assert::AreEqual(0.0887180194f, in[6]);
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

			Assert::AreEqual(0.999938965f, in[0]);
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

			Assert::AreEqual(0.0614293031f, in[0]);
			Assert::AreEqual(0.115311489f, in[2]);

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

			Assert::AreEqual(0.446514338f, out[0]);
			Assert::AreEqual(-0.699016035f, out[1]);
			Assert::AreEqual(-0.0866665095f, out[2]);
			Assert::AreEqual(0.188288942f, out[3]);
			Assert::AreEqual(0.252293527f, out[4]);
			Assert::AreEqual(0.203361660f, out[5]);
			Assert::AreEqual(0.109714866f, out[6]);
			Assert::AreEqual(0.0135267237f, out[7]);
			Assert::AreEqual(-0.0631750003f, out[8]);
			Assert::AreEqual(-0.112342872f, out[9]);
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
			Assert::AreEqual(4.0f, test.zW);
			Assert::AreEqual(270.0f, test.GetThetaW());
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

			Assert::AreEqual(1.0f, data.r);
			Assert::AreEqual(0.5f, data.z);
			Assert::AreEqual(1.0f, zA);
			Assert::AreEqual(sqrtf(1.0f + 0.5f * 0.5f), data.d);
			Assert::AreEqual(90.0f, Rad2Deg(data.t));
			Assert::IsTrue(check.valid);
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

namespace MCLTests
{
	TEST_CLASS(MCLTests)
	{
	public:

		TEST_METHOD(MCLTest)
		{
			std::vector<double> ir;
			int n = 512;
			ir.reserve(n);
			std::fill_n(std::back_inserter(ir), n, 0.0f);
			ir[0] = 1.0;
			ir[2] = 0.5;
			mcl::FirFilter filter = mcl::FirFilter(ir);

			Buffer out = Buffer(n);
			out[0] = filter.Filter(1.0f);
			out[1] = filter.Filter(1.0f);
			out[2] = filter.Filter(1.0f);
		}
	};
}