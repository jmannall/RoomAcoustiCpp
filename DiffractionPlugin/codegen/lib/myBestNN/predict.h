//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// predict.h
//
// Code generation for function 'predict'
//

#ifndef PREDICT_H
#define PREDICT_H

// Include files
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Type Declarations
namespace coder {
namespace internal {
namespace ctarget {
struct dlnetwork;

}
} // namespace internal
} // namespace coder

// Function Declarations
namespace coder {
namespace internal {
namespace ctarget {
void dlnetwork_predict(dlnetwork *obj, const float varargin_1_Data[8],
                       float varargout_1_Data[5]);

}
} // namespace internal
} // namespace coder

#endif
// End of code generation (predict.h)
