//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// myNN_initialize.cpp
//
// Code generation for function 'myNN_initialize'
//

// Include files
#include "myNN_initialize.h"
#include "myBestNN.h"
#include "myNN_data.h"
#include "mySmallNN.h"
#include "omp.h"

// Function Definitions
void myNN_initialize()
{
  omp_init_nest_lock(&myBestNN_nestLockGlobal);
  myBestNN_init();
  mySmallNN_init();
  isInitialized_myNN = true;
}

// End of code generation (myNN_initialize.cpp)
