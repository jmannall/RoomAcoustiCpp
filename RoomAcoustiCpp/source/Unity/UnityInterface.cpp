
#ifdef USE_UNITY_PROFILER

// Unity headers
#include "Unity/UnityInterface.h"
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityProfiler.h"

#include <unordered_map>

static IUnityProfiler* unityProfiler = nullptr;
static bool* isDevelopmentBuild = nullptr;

IUnityProfiler* GetUnityProfiler() { return unityProfiler; }
bool* GetDevBuild() { return isDevelopmentBuild; }

// Threads
static UnityProfilerThreadId backgroundThreadID = 999;
static std::unordered_map<int, UnityProfilerThreadId> threadIDs;
int nextThreadID = 0;

// Background thread markers
static const UnityProfilerMarkerDesc* backgroundLoopMarker = nullptr;
static const UnityProfilerMarkerDesc* imageEdgeModelMarker = nullptr;
static const UnityProfilerMarkerDesc* directMarker = nullptr;
static const UnityProfilerMarkerDesc* firstOrderRefMarker = nullptr;
static const UnityProfilerMarkerDesc* secondOrderRefMarker = nullptr;
static const UnityProfilerMarkerDesc* thirdOrderRefMarker = nullptr;
static const UnityProfilerMarkerDesc* higherOrderRefMarker = nullptr;
static const UnityProfilerMarkerDesc* firstOrderDiffMarker = nullptr;
static const UnityProfilerMarkerDesc* secondOrderRefDiffMarker = nullptr;
static const UnityProfilerMarkerDesc* thirdOrderRefDiffMarker = nullptr;
static const UnityProfilerMarkerDesc* higherOrderRefDiffMarker = nullptr;
static const UnityProfilerMarkerDesc* reverbRayTracingMarker = nullptr;
static const UnityProfilerMarkerDesc* updateAudioDataMarker = nullptr;

// Audio thread markers
static const UnityProfilerMarkerDesc* audioThreadMarker = nullptr;
static const UnityProfilerMarkerDesc* submitAudioMarker = nullptr;
static const UnityProfilerMarkerDesc* earlyReflectionMarker = nullptr;
static const UnityProfilerMarkerDesc* lateReverbMarker = nullptr;
static const UnityProfilerMarkerDesc* sourceMarker = nullptr;
static const UnityProfilerMarkerDesc* imageSourceMarker = nullptr;
static const UnityProfilerMarkerDesc* reverbSourceMarker = nullptr;
static const UnityProfilerMarkerDesc* fdnMarker = nullptr;
static const UnityProfilerMarkerDesc* reflectionMarker = nullptr;
static const UnityProfilerMarkerDesc* diffractionMarker = nullptr;
static const UnityProfilerMarkerDesc* airAbsorptionMarker = nullptr;
static const UnityProfilerMarkerDesc* threedtiMarker = nullptr;

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces * unityInterfaces)
{
	unityProfiler = unityInterfaces->Get<IUnityProfiler>();
	if (unityProfiler == nullptr)
		return;
	isDevelopmentBuild = new bool(unityProfiler->IsAvailable() != 0);

	unityProfiler->CreateMarker(&audioThreadMarker, "AudioThread", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&submitAudioMarker, "SubmitAudio", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&earlyReflectionMarker, "EarlyRefelctions", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&lateReverbMarker, "LateReverb", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&sourceMarker, "Source", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&imageSourceMarker, "ImageSource", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&reverbSourceMarker, "ReverbSource", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&fdnMarker, "FDN", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&reflectionMarker, "Reflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&diffractionMarker, "Diffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&airAbsorptionMarker, "AirAbsorption", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&threedtiMarker, "3DTI", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);

	unityProfiler->CreateMarker(&backgroundLoopMarker, "BackgroundLoop", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&imageEdgeModelMarker, "ImageEdgeModel", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&directMarker, "Direct", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&firstOrderRefMarker, "FirstOrderReflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&secondOrderRefMarker, "SecondOrderReflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&thirdOrderRefMarker, "ThirdOrderReflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&higherOrderRefMarker, "HigherOrderReflection", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&firstOrderDiffMarker, "FirstOrderDiffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&secondOrderRefDiffMarker, "SecondOrderReflectionDiffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&thirdOrderRefDiffMarker, "ThirdOrderReflectionDiffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&higherOrderRefDiffMarker, "HigherOrderReflectionDiffraction", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&reverbRayTracingMarker, "ReverbRayTracing", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
	unityProfiler->CreateMarker(&updateAudioDataMarker, "UpdateAudioData", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	unityProfiler = nullptr;
}

//////////////////// Register threads ////////////////////

////////////////////////////////////////

int RegisterThread()
{
	if (GetDevBuild())
	{
		auto [it, success] = threadIDs.emplace(nextThreadID, nextThreadID);
		if (!success)
			return -1;
		nextThreadID++;
		GetUnityProfiler()->RegisterThread(&it->second, "Acoustics", "Thread");
		return it->first;
	}
	return -1;
}

////////////////////////////////////////

void UnregisterThread(int id)
{
	if (GetDevBuild())
	{
		auto it = threadIDs.find(id);
		if (it != threadIDs.end())
			GetUnityProfiler()->UnregisterThread(it->second);
		threadIDs.erase(it);
	}
}

////////////////////////////////////////

void RegisterBackgroundThread()
{
	if (GetDevBuild())
		GetUnityProfiler()->RegisterThread(&backgroundThreadID, "Acoustics", "Background Thread");
}

////////////////////////////////////////

void UnregisterBackgroundThread()
{
	if (GetDevBuild())
		GetUnityProfiler()->UnregisterThread(backgroundThreadID);
}

////////////////////////////////////////

int RegisterAudioThread()
{
	if (GetDevBuild())
	{
		auto [it, success] = threadIDs.emplace(nextThreadID, nextThreadID);
		if (!success)
			return -1;
		nextThreadID++;
		GetUnityProfiler()->RegisterThread(&it->second, "Acoustics", "Audio Thread");
		return it->first;
	}
	return -1;
}

////////////////////////////////////////

void UnregisterAudioThread(int id)
{
	if (GetDevBuild())
	{
		auto it = threadIDs.find(id);
		if (it != threadIDs.end())
			GetUnityProfiler()->UnregisterThread(it->second);
		threadIDs.erase(it);
	}
}

//////////////////// Background thread ////////////////////

////////////////////////////////////////

void BeginBackgroundLoop()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(backgroundLoopMarker);
}

////////////////////////////////////////

void EndBackgroundLoop()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(backgroundLoopMarker);
}

////////////////////////////////////////

void BeginImageEdgeModel()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(imageEdgeModelMarker);
}

////////////////////////////////////////

void EndImageEdgeModel()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(imageEdgeModelMarker);
}

////////////////////////////////////////

void BeginDirect()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(directMarker);
}

////////////////////////////////////////

void EndDirect()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(directMarker);
}

////////////////////////////////////////

void BeginFirstOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(firstOrderRefMarker);
}

////////////////////////////////////////

void EndFirstOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(firstOrderRefMarker);
}

////////////////////////////////////////

void BeginSecondOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(secondOrderRefMarker);
}

////////////////////////////////////////

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

////////////////////////////////////////

void EndThirdOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(thirdOrderRefMarker);
}

void BeginHigherOrderRef()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(higherOrderRefMarker);
}

////////////////////////////////////////

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

////////////////////////////////////////

void EndFirstOrderDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(firstOrderDiffMarker);
}

////////////////////////////////////////

void BeginSecondOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(secondOrderRefDiffMarker);
}

////////////////////////////////////////

void EndSecondOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(secondOrderRefDiffMarker);
}

////////////////////////////////////////

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

////////////////////////////////////////

void BeginHigherOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(higherOrderRefDiffMarker);
}

////////////////////////////////////////

void EndHigherOrderRefDiff()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(higherOrderRefDiffMarker);
}

////////////////////////////////////////

void BeginReverbRayTracing()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(reverbRayTracingMarker);
}

////////////////////////////////////////

void EndReverbRayTracing()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(reverbRayTracingMarker);
}

////////////////////////////////////////

void BeginUpdateAudioData()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(updateAudioDataMarker);
}

////////////////////////////////////////

void EndUpdateAudioData()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(updateAudioDataMarker);
}

//////////////////// Audio thread ////////////////////

////////////////////////////////////////

void BeginAudioThread()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(audioThreadMarker);
}

////////////////////////////////////////

void EndAudioThread()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(audioThreadMarker);
}

////////////////////////////////////////

void BeginSubmitAudio()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(submitAudioMarker);
}

////////////////////////////////////////

void EndSubmitAudio()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(submitAudioMarker);
}

////////////////////////////////////////

void BeginEarlyReflections()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(earlyReflectionMarker);
}

////////////////////////////////////////

void EndEarlyReflections()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(earlyReflectionMarker);
}

////////////////////////////////////////

void BeginLateReverb()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(lateReverbMarker);
}

////////////////////////////////////////

void EndLateReverb()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(lateReverbMarker);
}

////////////////////////////////////////

void BeginSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(sourceMarker);
}

////////////////////////////////////////

void EndSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(sourceMarker);
}

////////////////////////////////////////

void BeginImageSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(imageSourceMarker);
}

////////////////////////////////////////

void EndImageSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(imageSourceMarker);
}

////////////////////////////////////////

void BeginReverbSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(reverbSourceMarker);
}

////////////////////////////////////////

void EndReverbSource()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(reverbSourceMarker);
}

////////////////////////////////////////

void BeginFDN()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(fdnMarker);
}

////////////////////////////////////////

void EndFDN()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(fdnMarker);
}

////////////////////////////////////////

void BeginReflection()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(reflectionMarker);
}

////////////////////////////////////////

void EndReflection()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(reflectionMarker);
}

////////////////////////////////////////

void BeginDiffraction()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(diffractionMarker);
}

////////////////////////////////////////

void EndDiffraction()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(diffractionMarker);
}

////////////////////////////////////////

void BeginAirAbsorption()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(airAbsorptionMarker);
}

////////////////////////////////////////

void EndAirAbsorption()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(airAbsorptionMarker);
}

////////////////////////////////////////

void Begin3DTI()
{
	if (GetDevBuild())
		GetUnityProfiler()->BeginSample(threedtiMarker);
}

////////////////////////////////////////

void End3DTI()
{
	if (GetDevBuild())
		GetUnityProfiler()->EndSample(threedtiMarker);
}

#endif