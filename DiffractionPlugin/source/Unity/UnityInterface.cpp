
// Unity headers
#include "Unity/UnityInterface.h"
#include "Unity/Profiler.h"
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityProfiler.h"

static IUnityProfiler* unityProfiler = NULL;
static bool* isDevelopmentBuild = nullptr;

IUnityProfiler* GetUnityProfiler() { return unityProfiler; }
bool* GetDevBuild() { return isDevelopmentBuild; }

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces * unityInterfaces)
{
	unityProfiler = unityInterfaces->Get<IUnityProfiler>();
	if (unityProfiler == NULL)
		return;
	isDevelopmentBuild = new bool(unityProfiler->IsAvailable() != 0);
#ifdef PROFILE_AUDIO_THREAD
	unityProfiler->CreateMarker(GetSourceMarker(), "Source", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetVirtualSourceMarker(), "VirtualSource", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetFDNMarker(), "FDN", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetReverbMarker(), "Reverb", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetReverbSourceMarker(), "ReverbSource", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetReflectionMarker(), "Reflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetDiffractionMarker(), "Diffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(Get3DTIMarker(), "3DTI", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetFIRMarker(), "FIR Filter", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetLerpMarker(), "Lerp", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetFDNChannelMarker(), "FDN Channel", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(GetFDNMatrixMarker(), "FDN Matrix", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
#endif
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	unityProfiler = NULL;
}