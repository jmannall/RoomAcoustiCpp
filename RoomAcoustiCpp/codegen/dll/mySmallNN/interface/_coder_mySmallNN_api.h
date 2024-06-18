//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// _coder_mySmallNN_api.h
//
// Code generation for function 'mySmallNN'
//

#ifndef _CODER_MYSMALLNN_API_H
#define _CODER_MYSMALLNN_API_H

// Include files
#include "mySmallNN_spec.h"
#include "emlrt.h"
#include "tmwtypes.h"
#include <algorithm>
#include <cstring>

// Variable Declarations
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

// Function Declarations
void mySmallNN(real_T in[8], real32_T z[2], real32_T p[2], real32_T *k);

void mySmallNN_api(const mxArray *prhs, int32_T nlhs, const mxArray *plhs[3]);

void mySmallNN_atexit();

void mySmallNN_initialize();

void mySmallNN_terminate();

void mySmallNN_xil_shutdown();

void mySmallNN_xil_terminate();

#endif
// End of code generation (_coder_mySmallNN_api.h)
