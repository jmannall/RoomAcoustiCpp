
//#include "pch.h"
#include "UnityGAPlugin.h"
#include "IUnityInterface.h"
#include <utility>

// End of code generation (main.cpp)
#define GA_CC UNITY_INTERFACE_API
#define GA_EXPORT UNITY_INTERFACE_EXPORT

static float* buffer = nullptr;

extern "C"
{
#pragma region UnityPluginInterface

	/*GA_EXPORT void GA_CC UnityPluginLoad(IUnityInterfaces* unityInterfaces)
	{
		(void)unityInterfaces;
	}

	GA_EXPORT void GA_CC UnityPluginUnload()
	{

	}*/

#pragma endregion

#pragma region Geometry Functions
	GA_EXPORT void GA_CC GAInitGeometry(int samplingRate, int dspSmoothingFactor)
	{
		DSPConfig config;
		config.samplingRate = samplingRate;
		config.dspSmoothingFactor = dspSmoothingFactor;
		GA::InitGeometry(&config);
	}

	GA_EXPORT void GA_CC GAExitGeometry()
	{
		GA::ExitGeometry();
	}

	GA_EXPORT void GA_CC GASetListenerTransform(float posX, float posY, float posZ)
	{
		GA::SetListenerPosition(vec3(posX, posY, posZ));
	}

	GA_EXPORT void GA_CC GASetModel(int mID)
	{
		Model model;
		switch (mID) {
		case 0:
			model = Model::attenuate;
			break;
		case 1:
			model = Model::off;
			break;
		case 2:
			model = Model::lowPass;
			break;
		case 3:
			model = Model::udfa;
			break;
		case 4:
			model = Model::udfai;
			break;
		case 5:
			model = Model::nnBest;
			break;
		case 6:
			model = Model::nnSmall;
			break;
		case 7:
			model = Model::utd;
			break;
		case 8:
			model = Model::btm;
			break;
		}
		GA::SetModel(model);
	}
#pragma endregion

#pragma region Source Functions
	GA_EXPORT int GA_CC GAInitSource(float posX, float posY, float posZ)
	{
		return (int)GA::InitSource(vec3(posX, posY, posZ));
	}

	GA_EXPORT void GA_CC GARemoveSource(int id)
	{
		GA::RemoveSource((size_t)id);
	}

	GA_EXPORT void GA_CC GAUpdateSourceData(int id, float posX, float posY, float posZ)
	{
		GA::UpdateSourceData((size_t)id, vec3(posX, posY, posZ));
	}
#pragma endregion

#pragma region Wedge Functions
	GA_EXPORT int GA_CC GAInitWedge(float bX, float bY, float bZ, float tX, float tY, float tZ,
		float n1X, float n1Y, float n1Z, float n2X, float n2Y, float n2Z)
	{
		vec3 normals[2] = { vec3(n1X, n1Y, n1Z), vec3(n2X, n2Y, n2Z) };
		Wedge wedge = Wedge(vec3(bX, bY, bZ), vec3(tX, tY, tZ), normals);
		return (int)GA::InitWedge(wedge);
	}

	GA_EXPORT void GA_CC GARemoveWedge(int id)
	{
		GA::RemoveWedge((size_t)id);
	}

	GA_EXPORT void GA_CC GAUpdateWedgeData(int id, float bX, float bY, float bZ, float tX, float tY, float tZ,
		float n1X, float n1Y, float n1Z, float n2X, float n2Y, float n2Z)
	{
		vec3 normals[2] = { vec3(n1X, n1Y, n1Z), vec3(n2X, n2Y, n2Z) };
		Wedge wedge = Wedge(vec3(bX, bY, bZ), vec3(tX, tY, tZ), normals);
		GA::UpdateWedgeData((size_t)id, wedge);
	}

	GA_EXPORT float GA_CC GAGetZ(int sID, int wID)
	{
		return GA::GetZ((size_t)sID, (size_t)wID);
	}

	GA_EXPORT float GA_CC GAGetSd(int sID, int wID)
	{
		return GA::GetSd((size_t)sID, (size_t)wID);
	}

	GA_EXPORT float GA_CC GAGetRd(int sID, int wID)
	{
		return GA::GetRd((size_t)sID, (size_t)wID);
	}
#pragma endregion

#pragma region Audio Functions
	GA_EXPORT void GA_CC GASendAudio(int sID, int wID, const float* data, int numFrames)
	{
		GA::SendAudio((size_t)sID, (size_t)wID, data, (unsigned)numFrames);
	}

	GA_EXPORT bool GA_CC GAProcessOutput()
	{
		GA::GetOutput(&buffer);
		if (!buffer) return false;
		else if (std::isnan(*buffer)) return false;

		return true;
	}

	GA_EXPORT void GA_CC GAGetOutputBuffer(float** buf)
	{
		*buf = buffer;
	}
#pragma endregion

#pragma region Spatialiser
	GA_EXPORT void GA_CC InitSpatialiser()
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

		string resourcePath = "D:\Joshua Mannall\GitHub\3dti_AudioToolkit\resources";
		string hrtfPath = "\HRTF\3DTI\3DTI_HRTF_IRC1008_128s_48000Hz.3dti-hrtf";
		string sofaPath = "\HRTF\SOFA\3DTI_HRTF_IRC1008_128s_48000Hz.sofa";
		bool specifiedDelays;
		bool result = HRTF::CreateFromSofa(hrtfPath + sofaPath, listener, specifiedDelays);
		//bool result = HRTF::CreateFrom3dti(resourcePath + hrtfPath, listener);
		if (result) { cout << "HRTF has been loaded successfully"; }

		string ildPath = "\ILD\NearFieldCompensation_ILD_48000";
		result = ILD::CreateFrom3dti_ILDNearFieldEffectTable(resourcePath + ildPath, listener);

		// If high performance mode
		// string ildPath = "\ILD\HRTF_ILD_48000";
		// result = ILD::CreateFrom3dti_ILDSpatializationTable(resourcePath + ildPath, listener);
		if (result) { cout << "ILD Near Field Effect simulation file has been loaded successfully"; }
	}
#pragma endregion
}