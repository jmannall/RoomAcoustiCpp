//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// _coder_myNN_api.cpp
//
// Code generation for function 'myBestNN'
//

// Include files
#include "_coder_myNN_api.h"
#include "_coder_myNN_mex.h"

// Variable Definitions
emlrtCTX emlrtRootTLSGlobal{nullptr};

emlrtContext emlrtContextGlobal{
    true,                                                 // bFirstTime
    false,                                                // bInitialized
    131627U,                                              // fVersionInfo
    nullptr,                                              // fErrorFunction
    "myNN",                                               // fFunctionName
    nullptr,                                              // fRTCallStack
    false,                                                // bDebugMode
    {2045744189U, 2170104910U, 2743257031U, 4284093946U}, // fSigWrd
    nullptr                                               // fSigMem
};

// Function Declarations
static real32_T (*b_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src,
                                     const emlrtMsgIdentifier *msgId))[8];

static real32_T (*emlrt_marshallIn(const emlrtStack *sp, const mxArray *in,
                                   const char_T *identifier))[8];

static real32_T (*emlrt_marshallIn(const emlrtStack *sp, const mxArray *u,
                                   const emlrtMsgIdentifier *parentId))[8];

static const mxArray *emlrt_marshallOut(const real32_T u[2]);

static const mxArray *emlrt_marshallOut(const real32_T u);

// Function Definitions
static real32_T (*b_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src,
                                     const emlrtMsgIdentifier *msgId))[8]
{
  static const int32_T dims[2]{1, 8};
  real32_T(*ret)[8];
  emlrtCheckBuiltInR2012b((emlrtConstCTX)sp, msgId, src, "single", false, 2U,
                          (const void *)&dims[0]);
  ret = (real32_T(*)[8])emlrtMxGetData(src);
  emlrtDestroyArray(&src);
  return ret;
}

static real32_T (*emlrt_marshallIn(const emlrtStack *sp, const mxArray *in,
                                   const char_T *identifier))[8]
{
  emlrtMsgIdentifier thisId;
  real32_T(*y)[8];
  thisId.fIdentifier = const_cast<const char_T *>(identifier);
  thisId.fParent = nullptr;
  thisId.bParentIsCell = false;
  y = emlrt_marshallIn(sp, emlrtAlias(in), &thisId);
  emlrtDestroyArray(&in);
  return y;
}

static real32_T (*emlrt_marshallIn(const emlrtStack *sp, const mxArray *u,
                                   const emlrtMsgIdentifier *parentId))[8]
{
  real32_T(*y)[8];
  y = b_emlrt_marshallIn(sp, emlrtAlias(u), parentId);
  emlrtDestroyArray(&u);
  return y;
}

static const mxArray *emlrt_marshallOut(const real32_T u[2])
{
  static const int32_T i{0};
  static const int32_T i1{2};
  const mxArray *m;
  const mxArray *y;
  y = nullptr;
  m = emlrtCreateNumericArray(1, (const void *)&i, mxSINGLE_CLASS, mxREAL);
  emlrtMxSetData((mxArray *)m, (void *)&u[0]);
  emlrtSetDimensions((mxArray *)m, &i1, 1);
  emlrtAssign(&y, m);
  return y;
}

static const mxArray *emlrt_marshallOut(const real32_T u)
{
  const mxArray *m;
  const mxArray *y;
  y = nullptr;
  m = emlrtCreateNumericMatrix(1, 1, mxSINGLE_CLASS, mxREAL);
  *static_cast<real32_T *>(emlrtMxGetData(m)) = u;
  emlrtAssign(&y, m);
  return y;
}

void myBestNN_api(const mxArray *prhs, int32_T nlhs, const mxArray *plhs[3])
{
  emlrtStack st{
      nullptr, // site
      nullptr, // tls
      nullptr  // prev
  };
  real32_T(*in)[8];
  real32_T(*p)[2];
  real32_T(*z)[2];
  real32_T k;
  st.tls = emlrtRootTLSGlobal;
  z = (real32_T(*)[2])mxMalloc(sizeof(real32_T[2]));
  p = (real32_T(*)[2])mxMalloc(sizeof(real32_T[2]));
  // Marshall function inputs
  in = emlrt_marshallIn(&st, emlrtAlias(prhs), "in");
  // Invoke the target function
  myBestNN(*in, *z, *p, &k);
  // Marshall function outputs
  plhs[0] = emlrt_marshallOut(*z);
  if (nlhs > 1) {
    plhs[1] = emlrt_marshallOut(*p);
  }
  if (nlhs > 2) {
    plhs[2] = emlrt_marshallOut(k);
  }
}

void myNN_atexit()
{
  emlrtStack st{
      nullptr, // site
      nullptr, // tls
      nullptr  // prev
  };
  mexFunctionCreateRootTLS();
  st.tls = emlrtRootTLSGlobal;
  emlrtEnterRtStackR2012b(&st);
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
  myNN_xil_terminate();
  myNN_xil_shutdown();
  emlrtExitTimeCleanup(&emlrtContextGlobal);
}

void myNN_initialize()
{
  emlrtStack st{
      nullptr, // site
      nullptr, // tls
      nullptr  // prev
  };
  mexFunctionCreateRootTLS();
  st.tls = emlrtRootTLSGlobal;
  emlrtClearAllocCountR2012b(&st, false, 0U, nullptr);
  emlrtEnterRtStackR2012b(&st);
  emlrtFirstTimeR2012b(emlrtRootTLSGlobal);
}

void myNN_terminate()
{
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
}

void mySmallNN_api(const mxArray *prhs, int32_T nlhs, const mxArray *plhs[3])
{
  emlrtStack st{
      nullptr, // site
      nullptr, // tls
      nullptr  // prev
  };
  real32_T(*in)[8];
  real32_T(*p)[2];
  real32_T(*z)[2];
  real32_T k;
  st.tls = emlrtRootTLSGlobal;
  z = (real32_T(*)[2])mxMalloc(sizeof(real32_T[2]));
  p = (real32_T(*)[2])mxMalloc(sizeof(real32_T[2]));
  // Marshall function inputs
  in = emlrt_marshallIn(&st, emlrtAlias(prhs), "in");
  // Invoke the target function
  mySmallNN(*in, *z, *p, &k);
  // Marshall function outputs
  plhs[0] = emlrt_marshallOut(*z);
  if (nlhs > 1) {
    plhs[1] = emlrt_marshallOut(*p);
  }
  if (nlhs > 2) {
    plhs[2] = emlrt_marshallOut(k);
  }
}

// End of code generation (_coder_myNN_api.cpp)
