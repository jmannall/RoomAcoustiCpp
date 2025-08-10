
#ifdef USE_UNITY_PROFILER

#ifndef Unity_UnityInterface_h
#define Unity_UnityInterface_h

#include "Unity/IUnityProfiler.h"

IUnityProfiler* GetUnityProfiler();
bool* GetDevBuild();

// Register threads
int RegisterThread();
void UnregisterThread(int id);

void RegisterBackgroundThread();
void UnregisterBackgroundThread();

int RegisterAudioThread();
void UnregisterAudioThread(int id);

// Background thread functions
void BeginBackgroundLoop();
void EndBackgroundLoop();

void BeginImageEdgeModel();
void EndImageEdgeModel();

void BeginDirect();
void EndDirect();

void BeginFirstOrderRef();
void EndFirstOrderRef();

void BeginSecondOrderRef();
void EndSecondOrderRef();

void BeginThirdOrderRef();
void EndThirdOrderRef();

void BeginHigherOrderRef();
void EndHigherOrderRef();

void BeginFirstOrderDiff();
void EndFirstOrderDiff();

void BeginSecondOrderRefDiff();
void EndSecondOrderRefDiff();

void BeginThirdOrderRefDiff();
void EndThirdOrderRefDiff();

void BeginHigherOrderRefDiff();
void EndHigherOrderRefDiff();

void BeginReverbRayTracing();
void EndReverbRayTracing();

void BeginUpdateAudioData();
void EndUpdateAudioData();

// Audio thread functions
void BeginAudioThread();
void EndAudioThread();

void BeginSubmitAudio();
void EndSubmitAudio();

void BeginEarlyReflections();
void EndEarlyReflections();

void BeginLateReverb();
void EndLateReverb();

void BeginSource();
void EndSource();

void BeginImageSource();
void EndImageSource();

void BeginReverbSource();
void EndReverbSource();

void BeginFDN();
void EndFDN();

void BeginReflection();
void EndReflection();

void BeginDiffraction();
void EndDiffraction();

void BeginAirAbsorption();
void EndAirAbsorption();

void Begin3DTI();
void End3DTI();

#endif // Unity_UnityInterface_h
#endif 
