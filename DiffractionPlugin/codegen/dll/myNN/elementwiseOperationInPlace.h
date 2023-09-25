//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// elementwiseOperationInPlace.h
//
// Code generation for function 'elementwiseOperationInPlace'
//

#ifndef ELEMENTWISEOPERATIONINPLACE_H
#define ELEMENTWISEOPERATIONINPLACE_H

// Include files
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Type Declarations
namespace coder {
class anonymous_function;

}

// Function Declarations
namespace coder {
namespace internal {
namespace layer {
void b_elementwiseOperationInPlace(const anonymous_function elementwiseFunction,
                                   float X[20]);

void elementwiseOperationInPlace(const anonymous_function elementwiseFunction,
                                 float X[36]);

} // namespace layer
} // namespace internal
} // namespace coder

#endif
// End of code generation (elementwiseOperationInPlace.h)
