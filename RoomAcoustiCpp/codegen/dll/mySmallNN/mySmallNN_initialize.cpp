//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// mySmallNN_initialize.cpp
//
// Code generation for function 'mySmallNN_initialize'
//

// Include files
#include "mySmallNN_initialize.h"
#include "mySmallNN.h"
#include "mySmallNN_data.h"
#include "omp.h"

// Function Definitions
void mySmallNN_initialize()
{
  omp_init_nest_lock(&mySmallNN_nestLockGlobal);
  mySmallNN_init();
  isInitialized_mySmallNN = true;
}

// End of code generation (mySmallNN_initialize.cpp)
