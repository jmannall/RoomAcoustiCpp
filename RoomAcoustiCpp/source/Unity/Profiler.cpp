/*
*
*  \Unity profiler interface
*
*/

// Unity headers
//#include "Unity/Profiler.h"
//#include "Unity/UnityInterface.h"
//
//#ifdef PROFILE_AUDIO_THREAD
//static const UnityProfilerMarkerDesc* sourceMarker = NULL;
//static const UnityProfilerMarkerDesc* virtualSourceMarker = NULL;
//static const UnityProfilerMarkerDesc* fdnMarker = NULL;
//static const UnityProfilerMarkerDesc* reverbMarker = NULL;
//static const UnityProfilerMarkerDesc* reverbSourceMarker = NULL;
//static const UnityProfilerMarkerDesc* reflectionMarker = NULL;
//static const UnityProfilerMarkerDesc* diffractionMarker = NULL;
//static const UnityProfilerMarkerDesc* threedtiMarker = NULL;
//static const UnityProfilerMarkerDesc* firMarker = NULL;
//static const UnityProfilerMarkerDesc* lerpMarker = NULL;
//static const UnityProfilerMarkerDesc* fdnChannelMarker = NULL;
//static const UnityProfilerMarkerDesc* fdnMatrixMarker = NULL;
//
//const UnityProfilerMarkerDesc** GetSourceMarker() { return &sourceMarker; }
//const UnityProfilerMarkerDesc** GetVirtualSourceMarker() { return &virtualSourceMarker; }
//const UnityProfilerMarkerDesc** GetFDNMarker() { return &fdnMarker; }
//const UnityProfilerMarkerDesc** GetReverbMarker() { return &reverbMarker; }
//const UnityProfilerMarkerDesc** GetReverbSourceMarker() { return &reverbSourceMarker; }
//const UnityProfilerMarkerDesc** GetReflectionMarker() { return &reflectionMarker; }
//const UnityProfilerMarkerDesc** GetDiffractionMarker() { return &diffractionMarker; }
//const UnityProfilerMarkerDesc** Get3DTIMarker() { return &threedtiMarker; }
//const UnityProfilerMarkerDesc** GetFIRMarker() { return &firMarker; }
//const UnityProfilerMarkerDesc** GetLerpMarker() { return &lerpMarker; }
//const UnityProfilerMarkerDesc** GetFDNChannelMarker() { return &fdnChannelMarker; }
//const UnityProfilerMarkerDesc** GetFDNMatrixMarker() { return &fdnMatrixMarker; }
//
//void BeginSource()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(sourceMarker);
//}
//
//void EndSource()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(sourceMarker);
//}
//
//void BeginVirtualSource()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(virtualSourceMarker);
//}
//
//void EndVirtualSource()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(virtualSourceMarker);
//}
//
//void BeginFDN()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(fdnMarker);
//}
//
//void EndFDN()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(fdnMarker);
//}
//
//void BeginReverb()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(reverbMarker);
//}
//
//void EndReverb()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(reverbMarker);
//}
//
//void BeginReverbSource()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(reverbSourceMarker);
//}
//
//void EndReverbSource()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(reverbSourceMarker);
//}
//
//void BeginReflection()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(reflectionMarker);
//}
//
//void EndReflection()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(reflectionMarker);
//}
//
//void BeginDiffraction()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(diffractionMarker);
//}
//
//void EndDiffraction()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(diffractionMarker);
//}
//
//void Begin3DTI()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(threedtiMarker);
//}
//
//void End3DTI()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(threedtiMarker);
//}
//
//void BeginFIR()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(firMarker);
//}
//
//void EndFIR()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(firMarker);
//}
//
//void BeginLerp()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(lerpMarker);
//}
//
//void EndLerp()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(lerpMarker);
//}
//
//void BeginFDNChannel()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(fdnChannelMarker);
//}
//
//void EndFDNChannel()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(fdnChannelMarker);
//}
//
//void BeginFDNMatrix()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->BeginSample(fdnMatrixMarker);
//}
//
//void EndFDNMatrix()
//{
//	if (GetDevBuild())
//		GetUnityProfiler()->EndSample(fdnMatrixMarker);
//}
//
//#endif