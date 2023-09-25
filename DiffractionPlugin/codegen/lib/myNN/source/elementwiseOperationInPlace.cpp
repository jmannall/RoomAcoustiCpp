//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// elementwiseOperationInPlace.cpp
//
// Code generation for function 'elementwiseOperationInPlace'
//

// Include files
#include "elementwiseOperationInPlace.h"
#include "anonymous_function.h"
#include "myNN_internal_types.h"
#include "omp.h"
#include <cmath>

// Function Definitions
namespace coder {
namespace internal {
namespace layer {
void b_elementwiseOperationInPlace(const anonymous_function elementwiseFunction,
                                   float X[20])
{
  float f;
  float maxval;
#pragma omp parallel for num_threads(omp_get_max_threads()) private(f, maxval)

  for (int iElem = 0; iElem < 20; iElem++) {
    f = X[iElem];
    if (std::isnan(f) || (f > 0.0F)) {
      maxval = f;
    } else {
      maxval = 0.0F;
    }
    f = maxval + elementwiseFunction.workspace.scaleCast * std::fmin(0.0F, f);
    X[iElem] = f;
  }
}

void elementwiseOperationInPlace(const anonymous_function elementwiseFunction,
                                 float X[36])
{
  float f;
  float maxval;
#pragma omp parallel for num_threads(omp_get_max_threads()) private(f, maxval)

  for (int iElem = 0; iElem < 36; iElem++) {
    f = X[iElem];
    if (std::isnan(f) || (f > 0.0F)) {
      maxval = f;
    } else {
      maxval = 0.0F;
    }
    f = maxval + elementwiseFunction.workspace.scaleCast * std::fmin(0.0F, f);
    X[iElem] = f;
  }
}

} // namespace layer
} // namespace internal
} // namespace coder

// End of code generation (elementwiseOperationInPlace.cpp)
