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

//#include "BinauralSpatializer/3DTI_BinauralSpatializer.h"
//#include "HRTF/HRTFFactory.h"
//#include "HRTF/HRTFCereal.h"
//#include "ILD/ILDCereal.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(UnitTest)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			float x = 1.0f;
			Assert::AreEqual(x, 1.0f);
		}

		TEST_METHOD(TestMethod2)
		{
			Buffer test;

			size_t num = 100;
			test.ResizeBuffer(num);
			Assert::AreEqual(0.0f, test[0]);
		}

		TEST_METHOD(TestMethod3)
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
