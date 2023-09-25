//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// myNN_terminate.cpp
//
// Code generation for function 'myNN_terminate'
//

// Include files
#include "myNN_terminate.h"
#include "myBestNN.h"
#include "myNN_data.h"
#include "mySmallNN.h"
#include "omp.h"

// Function Definitions
void myNN_terminate()
{
  mySmallNN_free();
  myBestNN_free();
  omp_destroy_nest_lock(&myBestNN_nestLockGlobal);
  isInitialized_myNN = false;
}

// End of code generation (myNN_terminate.cpp)
