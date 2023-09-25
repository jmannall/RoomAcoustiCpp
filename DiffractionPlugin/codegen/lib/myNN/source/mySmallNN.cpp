//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// mySmallNN.cpp
//
// Code generation for function 'mySmallNN'
//

// Include files
#include "mySmallNN.h"
#include "myNN_data.h"
#include "myNN_initialize.h"
#include "myNN_internal_types.h"
#include "predict.h"
#include <cmath>

// Variable Definitions
static coder::internal::ctarget::dlnetwork b_mynet;

static boolean_T b_mynet_not_empty;

// Function Definitions
void mySmallNN(const float in[8], float z[2], float p[2], float *k)
{
  float obj_Data[5];
  float f;
  float f1;
  if (!isInitialized_myNN) {
    myNN_initialize();
  }
  if (!b_mynet_not_empty) {
    b_mynet.IsNetworkInitialized = false;
    b_mynet.matlabCodegenIsDeleted = false;
    b_mynet_not_empty = true;
  }
  coder::internal::ctarget::b_dlnetwork_predict(&b_mynet, in, obj_Data);
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

void mySmallNN_free()
{
  if (!b_mynet.matlabCodegenIsDeleted) {
    b_mynet.matlabCodegenIsDeleted = true;
  }
}

void mySmallNN_init()
{
  b_mynet_not_empty = false;
  b_mynet.matlabCodegenIsDeleted = true;
}

// End of code generation (mySmallNN.cpp)
