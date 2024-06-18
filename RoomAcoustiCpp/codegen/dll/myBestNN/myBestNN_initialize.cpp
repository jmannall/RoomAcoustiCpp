//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// myBestNN_initialize.cpp
//
// Code generation for function 'myBestNN_initialize'
//

// Include files
#include "myBestNN_initialize.h"
#include "myBestNN.h"
#include "myBestNN_data.h"
#include "omp.h"

// Function Definitions
void myBestNN_initialize()
{
  omp_init_nest_lock(&myBestNN_nestLockGlobal);
  myBestNN_init();
  isInitialized_myBestNN = true;
}

// End of code generation (myBestNN_initialize.cpp)
