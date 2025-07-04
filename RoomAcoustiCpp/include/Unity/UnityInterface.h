

#ifndef Unity_UnityInterface_h
#define Unity_UnityInterface_h

#include "Unity/IUnityProfiler.h"

#define PROFILE_AUDIO_THREAD
#ifdef PROFILE_AUDIO_THREAD
// #define PROFILE_DETAILED
#endif
#define PROFILE_BACKGROUND_THREAD

IUnityProfiler* GetUnityProfiler();
bool* GetDevBuild();

int RegisterThread();
void UnregisterThread(int id);


#ifdef PROFILE_AUDIO_THREAD
int RegisterAudioThread();
void UnregisterAudioThread(int id);

void BeginSource();
void EndSource();

void BeginVirtualSource();
void EndVirtualSource();

void BeginFDN();
void EndFDN();

void BeginReverb();
void EndReverb();

void BeginReverbSource();
void EndReverbSource();

void BeginReflection();
void EndReflection();

void BeginAirAbsorption();
void EndAirAbsorption();

void BeginDiffraction();
void EndDiffraction();

void Begin3DTI();
void End3DTI();
#ifdef PROFILE_DETAILED
void BeginFIR();
void EndFIR();

void BeginLerp();
void EndLerp();

void BeginFDNChannel();
void EndFDNChannel();

void BeginFDNMatrix();
void EndFDNMatrix();
#endif // PROFILE_DETAILED
#endif // PROFILE_AUDIO_THREAD

#ifdef PROFILE_BACKGROUND_THREAD

void RegisterBackgroundThread();
void UnregisterBackgroundThread();

void BeginBackgroundLoop();
void EndBackgroundLoop();

void BeginIEM();
void EndIEM();

void BeginDirect();
void EndDirect();

void BeginFirstOrderRef();
void EndFirstOrderRef();

void BeginSecondOrderRef();
void EndSecondOrderRef();

void BeginThirdOrderRef();
void EndThirdOrderRef();

void BeginFourthOrderRef();
void EndFourthOrderRef();

void BeginHigherOrderRef();
void EndHigherOrderRef();

void BeginFirstOrderDiff();
void EndFirstOrderDiff();

void BeginSecondOrderRefDiff();
void EndSecondOrderRefDiff();

void BeginThirdOrderRefDiff();
void EndThirdOrderRefDiff();

void BeginFourthOrderRefDiff();
void EndFourthOrderRefDiff();

void BeginHigherOrderRefDiff();
void EndHigherOrderRefDiff();

void BeginLateReverb();
void EndLateReverb();

void BeginUpdateAudioData();
void EndUpdateAudioData();

#endif

#endif