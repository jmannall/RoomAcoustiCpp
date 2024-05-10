
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
static const UnityProfilerMarkerDesc* airAbsorptionMarker = NULL;
static const UnityProfilerMarkerDesc* diffractionMarker = NULL;
static const UnityProfilerMarkerDesc* threedtiMarker = NULL;
static const UnityProfilerMarkerDesc* firMarker = NULL;
static const UnityProfilerMarkerDesc* lerpMarker = NULL;
static const UnityProfilerMarkerDesc* fdnChannelMarker = NULL;
static const UnityProfilerMarkerDesc* fdnMatrixMarker = NULL;

static const UnityProfilerMarkerDesc* backgroundLoopMarker = NULL;
static const UnityProfilerMarkerDesc* iemMarker = NULL;
static const UnityProfilerMarkerDesc* directMarker = NULL;
static const UnityProfilerMarkerDesc* firstOrderRefMarker = NULL;
static const UnityProfilerMarkerDesc* secondOrderRefMarker = NULL;
static const UnityProfilerMarkerDesc* thirdOrderRefMarker = NULL;
static const UnityProfilerMarkerDesc* fourthOrderRefMarker = NULL;
static const UnityProfilerMarkerDesc* higherOrderRefMarker = NULL;
static const UnityProfilerMarkerDesc* firstOrderDiffMarker = NULL;
static const UnityProfilerMarkerDesc* secondOrderRefDiffMarker = NULL;
static const UnityProfilerMarkerDesc* thirdOrderRefDiffMarker = NULL;
static const UnityProfilerMarkerDesc* fourthOrderRefDiffMarker = NULL;
static const UnityProfilerMarkerDesc* higherOrderRefDiffMarker = NULL;
static const UnityProfilerMarkerDesc* lateReverbMarker = NULL;
static const UnityProfilerMarkerDesc* copyDataMarker = NULL;

static UnityProfilerThreadId backgroundThreadID = 999;
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
	unityProfiler->CreateMarker(&airAbsorptionMarker, "AirAbsorption", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&diffractionMarker, "Diffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&threedtiMarker, "3DTI", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&firMarker, "FIR Filter", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&lerpMarker, "Lerp", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&fdnChannelMarker, "FDN Channel", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&fdnMatrixMarker, "FDN Matrix", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);

	unityProfiler->CreateMarker(&backgroundLoopMarker, "BackgroundLoop", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&iemMarker, "ImageEdgeModel", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&directMarker, "Direct", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&firstOrderRefMarker, "FirstOrderReflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&secondOrderRefMarker, "SecondOrderReflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&thirdOrderRefMarker, "ThirdOrderReflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&fourthOrderRefMarker, "FourthOrderReflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&higherOrderRefMarker, "HigherOrderReflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&firstOrderDiffMarker, "FirstOrderDiffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&secondOrderRefDiffMarker, "SecondOrderReflectionDiffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&thirdOrderRefDiffMarker, "ThirdOrderReflectionDiffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&fourthOrderRefDiffMarker, "FourthOrderReflectionDiffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&higherOrderRefDiffMarker, "HigherOrderReflectionDiffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&lateReverbMarker, "LateReverb", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&copyDataMarker, "CopyData", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);

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

void BeginAirAbsorption()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(airAbsorptionMarker);
}

void EndAirAbsorption()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(airAbsorptionMarker);
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

void BeginBackgroundLoop()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(backgroundLoopMarker);
}

#endif

#ifdef PROFILE_BACKGROUND_THREAD

void RegisterBackgroundThread()
{
	if (GetDevBuild())
		GetUnityProfiler()->RegisterThread(&backgroundThreadID, "Acoustics", "Background Thread");
}

void UnregisterBackgroundThread()
{
	if (GetDevBuild())
		GetUnityProfiler()->UnregisterThread(backgroundThreadID);
}

void EndBackgroundLoop()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(backgroundLoopMarker);
}

void BeginIEM()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(iemMarker);
}

void EndIEM()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(iemMarker);
}

void BeginDirect()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(directMarker);
}

void EndDirect()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(directMarker);
}

void BeginFirstOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(firstOrderRefMarker);
}

void EndFirstOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(firstOrderRefMarker);
}

void BeginSecondOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(secondOrderRefMarker);
}

void EndSecondOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(secondOrderRefMarker);
}

void BeginThirdOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(thirdOrderRefMarker);
}

void EndThirdOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(thirdOrderRefMarker);
}

void BeginFourthOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(fourthOrderRefMarker);
}

void EndFourthOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(fourthOrderRefMarker);
}

void BeginHigherOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(higherOrderRefMarker);
}

void EndHigherOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(higherOrderRefMarker);
}

void BeginFirstOrderDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(firstOrderDiffMarker);
}

void EndFirstOrderDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(firstOrderDiffMarker);
}

void BeginSecondOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(secondOrderRefDiffMarker);
}

void EndSecondOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(secondOrderRefDiffMarker);
}

void BeginThirdOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(thirdOrderRefDiffMarker);
}

void EndThirdOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(thirdOrderRefDiffMarker);
}

void BeginFourthOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(fourthOrderRefDiffMarker);
}

void EndFourthOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(fourthOrderRefDiffMarker);
}

void BeginHigherOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(higherOrderRefDiffMarker);
}

void EndHigherOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(higherOrderRefDiffMarker);
}

void BeginLateReverb()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(lateReverbMarker);
}

void EndLateReverb()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(lateReverbMarker);
}

void BeginCopyData()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(copyDataMarker);
}

void EndCopyData()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(copyDataMarker);
}

#endif