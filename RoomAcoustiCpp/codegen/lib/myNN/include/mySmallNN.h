//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// mySmallNN.h
//
// Code generation for function 'mySmallNN'
//

#ifndef MYSMALLNN_H
#define MYSMALLNN_H

// Include files
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Function Declarations
extern void mySmallNN(const float in[8], float z[2], float p[2], float *k);

void mySmallNN_free();

void mySmallNN_init();

#endif
// End of code generation (mySmallNN.h)
