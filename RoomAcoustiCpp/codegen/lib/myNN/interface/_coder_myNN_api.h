//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// _coder_myNN_api.h
//
// Code generation for function 'myBestNN'
//

#ifndef _CODER_MYNN_API_H
#define _CODER_MYNN_API_H

// Include files
#include "emlrt.h"
#include "tmwtypes.h"
#include <algorithm>
#include <cstring>

// Variable Declarations
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

// Function Declarations
void myBestNN(real32_T in[8], real32_T z[2], real32_T p[2], real32_T *k);

void myBestNN_api(const mxArray *prhs, int32_T nlhs, const mxArray *plhs[3]);

void myNN_atexit();

void myNN_initialize();

void myNN_terminate();

void myNN_xil_shutdown();

void myNN_xil_terminate();

void mySmallNN(real32_T in[8], real32_T z[2], real32_T p[2], real32_T *k);

void mySmallNN_api(const mxArray *prhs, int32_T nlhs, const mxArray *plhs[3]);

#endif
// End of code generation (_coder_myNN_api.h)
