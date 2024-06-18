//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// myNN_spec.h
//
// Code generation for function 'myBestNN'
//

#ifndef MYNN_SPEC_H
#define MYNN_SPEC_H

// Include files
#ifdef MYBESTNN_XIL_BUILD
#if defined(_MSC_VER) || defined(__LCC__)
#define MYBESTNN_DLL_EXPORT __declspec(dllimport)
#else
#define MYBESTNN_DLL_EXPORT __attribute__((visibility("default")))
#endif
#elif defined(BUILDING_MYBESTNN)
#if defined(_MSC_VER) || defined(__LCC__)
#define MYBESTNN_DLL_EXPORT __declspec(dllexport)
#else
#define MYBESTNN_DLL_EXPORT __attribute__((visibility("default")))
#endif
#else
#define MYBESTNN_DLL_EXPORT
#endif

#endif
// End of code generation (myNN_spec.h)
