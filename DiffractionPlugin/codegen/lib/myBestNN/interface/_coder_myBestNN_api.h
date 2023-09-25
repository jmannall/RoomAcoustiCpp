//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// _coder_myBestNN_api.h
//
// Code generation for function 'myBestNN'
//

#ifndef _CODER_MYBESTNN_API_H
#define _CODER_MYBESTNN_API_H

// Include files
#include "emlrt.h"
#include "tmwtypes.h"
#include <algorithm>
#include <cstring>

// Variable Declarations
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

// Function Declarations
void myBestNN(real_T in[8], real32_T z[2], real32_T p[2], real32_T *k);

void myBestNN_api(const mxArray *prhs, int32_T nlhs, const mxArray *plhs[3]);

void myBestNN_atexit();

void myBestNN_initialize();

void myBestNN_terminate();

void myBestNN_xil_shutdown();

void myBestNN_xil_terminate();

#endif
// End of code generation (_coder_myBestNN_api.h)
