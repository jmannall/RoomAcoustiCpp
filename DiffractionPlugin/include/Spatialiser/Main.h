/*
* 
* \Defines linakage between the C# unity code and C++ code
* 
*/

#ifndef Spatialiser_Main_h
#define Spatialiser_Main_h

// Unity headers
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityProfiler.h"

// Spatialiser headers
#include "Spatialiser/Types.h"

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

#endif