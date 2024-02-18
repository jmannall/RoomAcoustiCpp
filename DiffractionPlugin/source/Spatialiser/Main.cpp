/*
*
*  \Defines linakage between the C# unity code and C++ code
* 
*/

// Unity headers
#include "Unity/Debug.h"
#include "Unity/Profiler.h"

// Common headers
#include "Common/AudioManager.h" 

// Spatialiser headers
#include "Spatialiser/Main.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/Interface.h"

#define UI_API UNITY_INTERFACE_API
#define UI_EXPORT UNITY_INTERFACE_EXPORT

using namespace UIE::Spatialiser;
using namespace UIE::Common;
using namespace UIE::Unity;

// Pointer to return buffer
static float* buffer = nullptr;

static IUnityProfiler* unityProfiler = NULL;
static bool* isDevelopmentBuild = nullptr;

#ifdef PROFILE_AUDIO_THREAD
static const UnityProfilerMarkerDesc* sourceMarker = NULL;
static const UnityProfilerMarkerDesc* virtualSourceMarker = NULL;
static const UnityProfilerMarkerDesc* fdnMarker = NULL;
static const UnityProfilerMarkerDesc* reverbMarker = NULL;
static const UnityProfilerMarkerDesc* reverbSourceMarker = NULL;
static const UnityProfilerMarkerDesc* reflectionMarker = NULL;
static const UnityProfilerMarkerDesc* diffractionMarker = NULL;
static const UnityProfilerMarkerDesc* threedtiMarker = NULL;
static const UnityProfilerMarkerDesc* firMarker = NULL;
static const UnityProfilerMarkerDesc* fir1Marker = NULL;
static const UnityProfilerMarkerDesc* lerpMarker = NULL;
static const UnityProfilerMarkerDesc* fdnChannelMarker = NULL;
static const UnityProfilerMarkerDesc* fdnMatrixMarker = NULL;
#endif
IUnityProfiler* GetUnityProfiler() { return unityProfiler; }

bool* GetDevBuild() { return isDevelopmentBuild; }

#ifdef PROFILE_AUDIO_THREAD
void BeginSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(sourceMarker);
}

void EndSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(sourceMarker);
}

void BeginVirtualSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(virtualSourceMarker);
}

void EndVirtualSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(virtualSourceMarker);
}

void BeginFDN()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(fdnMarker);
}

void EndFDN()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(fdnMarker);
}

void BeginReverb()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(reverbMarker);
}

void EndReverb()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(reverbMarker);
}

void BeginReverbSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(reverbSourceMarker);
}

void EndReverbSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(reverbSourceMarker);
}

void BeginReflection()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(reflectionMarker);
}

void EndReflection()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(reflectionMarker);
}

void BeginDiffraction()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(diffractionMarker);
}

void EndDiffraction()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(diffractionMarker);
}

void Begin3DTI()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(threedtiMarker);
}

void End3DTI()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(threedtiMarker);
}

void BeginFIR()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(firMarker);
}

void EndFIR()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(firMarker);
}

void BeginLerp()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(lerpMarker);
}

void EndLerp()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(lerpMarker);
}

void BeginFDNChannel()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(fdnChannelMarker);
}

void EndFDNChannel()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(fdnChannelMarker);
}

void BeginFDNMatrix()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(fdnMatrixMarker);
}

void EndFDNMatrix()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(fdnMatrixMarker);
}
#endif

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces * unityInterfaces)
{
	unityProfiler = unityInterfaces->Get<IUnityProfiler>();
	if (unityProfiler == NULL)
		return;
	isDevelopmentBuild = new bool(unityProfiler->IsAvailable() != 0);
#ifdef PROFILE_AUDIO_THREAD
	unityProfiler->CreateMarker(&sourceMarker, "Source", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&virtualSourceMarker, "VirtualSource", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&fdnMarker, "FDN", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&reverbMarker, "Reverb", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&reverbSourceMarker, "ReverbSource", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&reflectionMarker, "Reflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&diffractionMarker, "Diffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&threedtiMarker, "3DTI", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&firMarker, "FIR Filter", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&lerpMarker, "Lerp", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&fdnChannelMarker, "FDN Channel", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&fdnMatrixMarker, "FDN Matrix", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
#endif
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	unityProfiler = NULL;
}

extern "C"
{
	//////////////////// Unity Plugin Interface ////////////////////


	//////////////////// Spatialiser ////////////////////

	// Load and Destroy

	UI_EXPORT bool UI_API SPATInit(int fs, int numFrames, int numChannels, int numFDNChannels, float lerpFactor, int hrtfResamplingStep, int hrtfMode, const char** paths)
	{
		std::vector<std::string> filePaths = { string(*(paths)), string(*(paths + 1)), string(*(paths + 2)) };
		HRTFMode mode;
		switch (hrtfMode)
		{
			case 0:
			{ mode = HRTFMode::quality; break; }
			case 1:
			{ mode = HRTFMode::performance; break; }
			case 2:
			{ mode = HRTFMode::none; break; }
			default:
			{ mode = HRTFMode::performance; break; }
		}

		Config config = Config(fs, numFrames, numChannels, numFDNChannels, static_cast<Real>(lerpFactor), hrtfResamplingStep, mode);
		return Init(&config, filePaths);
	}

	UI_EXPORT void UI_API SPATExit()
	{
		Exit();
	}

	// Image Source Model

	UI_EXPORT void UI_API SPATUpdateISMConfig(int order, bool dir, bool ref, bool diff, bool refDiff, bool rev, bool spDiff)
	{
		UpdateISMConfig(ISMConfig(order, dir, ref, diff, refDiff, rev, spDiff));
	}

	// Reverb

	UI_EXPORT void UI_API SPATSetFDNParameters(float volume, const float* dim, int numDimensions)
	{
		Buffer in = Buffer(numDimensions);
		vec dimensions = vec(numDimensions);
		for (int i = 0; i < numDimensions; i++)
			dimensions.AddEntry(static_cast<Real>(dim[i]), i);

		SetFDNParameters(static_cast<Real>(volume), dimensions);
	}

	// Listener

	UI_EXPORT void UI_API SPATUpdateListener(float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateListener(vec3(posX, posY, posZ), vec4(oriW, oriX, oriY, oriZ));
	}

	// Source

	UI_EXPORT int UI_API SPATInitSource()
	{
		return InitSource();
	}

	UI_EXPORT void UI_API SPATUpdateSource(int id, float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateSource(static_cast<size_t>(id), vec3(posX, posY, posZ), vec4(oriW, oriX, oriY, oriZ));
	}

	UI_EXPORT void UI_API SPATRemoveSource(int id)
	{
		RemoveSource(static_cast<size_t>(id));
	}

	// Walls

	ReverbWall ReturnReverbWall(int id)
	{
		switch (id)
		{
		case 0:
		{ return ReverbWall::posZ; break; }
		case 1:
		{ return ReverbWall::negZ; break; }
		case 2:
		{ return ReverbWall::posX; break; }
		case 3:
		{ return ReverbWall::negX; break; }
		case 4:
		{ return ReverbWall::posY; break; }
		case 5:
		{ return ReverbWall::negY; break; }
		default:
		{ return ReverbWall::none; break; }
		}
	}

	UI_EXPORT int UI_API SPATInitWall(float nX, float nY, float nZ, const float* vData, int numVertices, float aL, float aML, float aM, float aMH, float aH, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);
		Absorption absorption = Absorption(aL, aML, aM, aMH, aH);

		int numCoords = 3 * numVertices;
		Buffer in = Buffer(numCoords);
		for (int i = 0; i < numCoords; i++)
			in[i] = static_cast<Real>(vData[i]);

		return InitWall(vec3(nX, nY, nZ), &in[0], static_cast<size_t>(numVertices), absorption, reverbWall);
	}

	UI_EXPORT void UI_API SPATUpdateWall(int id, float nX, float nY, float nZ, const float* vData, int numVertices, float aL, float aML, float aM, float aMH, float aH, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);
		Absorption absorption = Absorption(aL, aML, aM, aMH, aH);

		int numCoords = 3 * numVertices;
		Buffer in = Buffer(numCoords);
		for (int i = 0; i < numCoords; i++)
			in[i] = static_cast<Real>(vData[i]);

		UpdateWall(static_cast<size_t>(id), vec3(nX, nY, nZ), &in[0], static_cast<size_t>(numVertices), absorption, reverbWall);
	}

	UI_EXPORT void UI_API SPATFreeWallId(int id)
	{
		FreeWallId(static_cast<size_t>(id));
	}

	UI_EXPORT void UI_API SPATRemoveWall(int id, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);

		RemoveWall(static_cast<size_t>(id), reverbWall);
	}

	// Audio

	UI_EXPORT void UI_API SPATSubmitAudio(int id, const float* data)
	{
		SubmitAudio(static_cast<size_t>(id), data);
	}

	UI_EXPORT bool UI_API SPATProcessOutput()
	{
		GetOutput(&buffer);
		if (!buffer)
		{
#ifdef DEBUG_AUDIO_THREAD
	Debug::Log("Process Output Failed", Colour::Orange);
#endif
			return false;
		}
		else if (std::isnan(*buffer))
		{
#ifdef DEBUG_AUDIO_THREAD
	Debug::Log("Process Output is NaN", Colour::Orange);
#endif
			return false;
		}
#ifdef DEBUG_AUDIO_THREAD
	Debug::Log("Process Output Success", Colour::Orange);
#endif
		return true;
	}

	UI_EXPORT void UI_API SPATGetOutputBuffer(float** buf)
	{
		*buf = buffer;
	}
}