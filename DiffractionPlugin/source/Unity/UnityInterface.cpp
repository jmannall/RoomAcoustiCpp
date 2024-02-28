
// Unity headers
#include "Unity/UnityInterface.h"
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityProfiler.h"

static IUnityProfiler* unityProfiler = NULL;
static bool* isDevelopmentBuild = nullptr;

IUnityProfiler* GetUnityProfiler() { return unityProfiler; }
bool* GetDevBuild() { return isDevelopmentBuild; }

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
static const UnityProfilerMarkerDesc* lerpMarker = NULL;
static const UnityProfilerMarkerDesc* fdnChannelMarker = NULL;
static const UnityProfilerMarkerDesc* fdnMatrixMarker = NULL;
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