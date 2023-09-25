//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// myBestNN_terminate.cpp
//
// Code generation for function 'myBestNN_terminate'
//

// Include files
#include "myBestNN_terminate.h"
#include "myBestNN.h"
#include "myBestNN_data.h"
#include "omp.h"

// Function Definitions
void myBestNN_terminate()
{
  myBestNN_free();
  omp_destroy_nest_lock(&myBestNN_nestLockGlobal);
  isInitialized_myBestNN = false;
}

// End of code generation (myBestNN_terminate.cpp)
