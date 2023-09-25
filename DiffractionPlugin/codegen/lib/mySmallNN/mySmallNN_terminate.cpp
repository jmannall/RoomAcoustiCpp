//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// mySmallNN_terminate.cpp
//
// Code generation for function 'mySmallNN_terminate'
//

// Include files
#include "mySmallNN_terminate.h"
#include "mySmallNN.h"
#include "mySmallNN_data.h"
#include "omp.h"

// Function Definitions
void mySmallNN_terminate()
{
  mySmallNN_free();
  omp_destroy_nest_lock(&mySmallNN_nestLockGlobal);
  isInitialized_mySmallNN = false;
}

// End of code generation (mySmallNN_terminate.cpp)
