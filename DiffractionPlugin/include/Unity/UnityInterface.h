

#ifndef Unity_UnityInterface_h
#define Unity_UnityInterface_h

#include "Unity/IUnityProfiler.h"

#define PROFILE_AUDIO_THREAD
//#define PROFILE_BACKGROUND_THREAD

IUnityProfiler* GetUnityProfiler();
bool* GetDevBuild();

#ifdef PROFILE_AUDIO_THREAD
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

void BeginFIR();
void EndFIR();

void BeginLerp();
void EndLerp();

void BeginFDNChannel();
void EndFDNChannel();

void BeginFDNMatrix();
void EndFDNMatrix();
#endif

#ifdef PROFILE_BACKGROUND_THREAD
void BeginBackgroundLoop();
void EndBackgroundLoop();

void BeginIEM();
void EndIEM();
#endif
#endif