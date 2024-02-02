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

#endif