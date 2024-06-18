//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// myNN_internal_types.h
//
// Code generation for function 'myBestNN'
//

#ifndef MYNN_INTERNAL_TYPES_H
#define MYNN_INTERNAL_TYPES_H

// Include files
#include "myNN_types.h"
#include "rtwtypes.h"

// Type Definitions
struct struct_T {
  float scaleCast;
};

namespace coder {
namespace internal {
namespace ctarget {
struct dlnetwork {
  boolean_T matlabCodegenIsDeleted;
  boolean_T IsNetworkInitialized;
};

} // namespace ctarget
} // namespace internal
} // namespace coder

#endif
// End of code generation (myNN_internal_types.h)
