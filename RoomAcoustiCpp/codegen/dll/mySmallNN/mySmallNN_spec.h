//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// mySmallNN_spec.h
//
// Code generation for function 'mySmallNN'
//

#ifndef MYSMALLNN_SPEC_H
#define MYSMALLNN_SPEC_H

// Include files
#ifdef MYSMALLNN_XIL_BUILD
#if defined(_MSC_VER) || defined(__LCC__)
#define MYSMALLNN_DLL_EXPORT __declspec(dllimport)
#else
#define MYSMALLNN_DLL_EXPORT __attribute__((visibility("default")))
#endif
#elif defined(BUILDING_MYSMALLNN)
#if defined(_MSC_VER) || defined(__LCC__)
#define MYSMALLNN_DLL_EXPORT __declspec(dllexport)
#else
#define MYSMALLNN_DLL_EXPORT __attribute__((visibility("default")))
#endif
#else
#define MYSMALLNN_DLL_EXPORT
#endif

#endif
// End of code generation (mySmallNN_spec.h)
