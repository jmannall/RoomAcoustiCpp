//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// myBestNN.cpp
//
// Code generation for function 'myBestNN'
//

// Include files
#include "myBestNN.h"
#include "myNN_data.h"
#include "myNN_initialize.h"
#include "myNN_internal_types.h"
#include "predict.h"
#include <cmath>

// Variable Definitions
static coder::internal::ctarget::dlnetwork mynet;

static boolean_T mynet_not_empty;

// Function Definitions
void myBestNN(const float in[8], float z[2], float p[2], float *k)
{
  float obj_Data[5];
  float f;
  float f1;
  if (!isInitialized_myNN) {
    myNN_initialize();
  }
  if (!mynet_not_empty) {
    mynet.IsNetworkInitialized = false;
    mynet.matlabCodegenIsDeleted = false;
    mynet_not_empty = true;
  }
  coder::internal::ctarget::dlnetwork_predict(&mynet, in, obj_Data);
  f = std::abs(obj_Data[0]);
  f1 = std::abs(obj_Data[2]);
  z[0] = obj_Data[0] * std::tanh(f) / (f + 1.0E-8F);
  p[0] = obj_Data[2] * std::tanh(f1) / (f1 + 1.0E-8F);
  f = std::abs(obj_Data[1]);
  f1 = std::abs(obj_Data[3]);
  z[1] = obj_Data[1] * std::tanh(f) / (f + 1.0E-8F);
  p[1] = obj_Data[3] * std::tanh(f1) / (f1 + 1.0E-8F);
  //  Sigmoid activation function
  *k = 1.0F / (std::exp(-obj_Data[4]) + 1.0F);
}

void myBestNN_free()
{
  if (!mynet.matlabCodegenIsDeleted) {
    mynet.matlabCodegenIsDeleted = true;
  }
}

void myBestNN_init()
{
  mynet_not_empty = false;
  mynet.matlabCodegenIsDeleted = true;
}

// End of code generation (myBestNN.cpp)
