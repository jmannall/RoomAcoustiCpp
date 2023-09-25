//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// mySmallNN_internal_types.h
//
// Code generation for function 'mySmallNN'
//

#ifndef MYSMALLNN_INTERNAL_TYPES_H
#define MYSMALLNN_INTERNAL_TYPES_H

// Include files
#include "mySmallNN_types.h"
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
// End of code generation (mySmallNN_internal_types.h)
