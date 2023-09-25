//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// main.cpp
//
// Code generation for function 'main'
//

/*************************************************************************/
/* This automatically generated example C++ main file shows how to call  */
/* entry-point functions that MATLAB Coder generated. You must customize */
/* this file for your application. Do not modify this file directly.     */
/* Instead, make a copy of this file, modify it, and integrate it into   */
/* your development environment.                                         */
/*                                                                       */
/* This file initializes entry-point function arguments to a default     */
/* size and value before calling the entry-point functions. It does      */
/* not store or use any values returned from the entry-point functions.  */
/* If necessary, it does pre-allocate memory for returned values.        */
/* You can use this file as a starting point for a main function that    */
/* you can deploy in your application.                                   */
/*                                                                       */
/* After you copy the file, and before you deploy it, you must make the  */
/* following changes:                                                    */
/* * For variable-size function arguments, change the example sizes to   */
/* the sizes that your application requires.                             */
/* * Change the example values of function arguments to the values that  */
/* your application requires.                                            */
/* * If the entry-point functions return values, store these values or   */
/* otherwise use them as required by your application.                   */
/*                                                                       */
/*************************************************************************/

// Include files
#include "main.h"
#include "myBestNN.h"
#include "myNN_terminate.h"
#include "mySmallNN.h"

// Function Declarations
static void argInit_1x8_real32_T(float result[8]);

static float argInit_real32_T();

// Function Definitions
static void argInit_1x8_real32_T(float result[8])
{
  // Loop over the array to initialize each element.
  for (int idx1{0}; idx1 < 8; idx1++) {
    // Set the value of the array element.
    // Change this value to the value that the application requires.
    result[idx1] = argInit_real32_T();
  }
}

static float argInit_real32_T()
{
  return 0.0F;
}

int main(int, char **)
{
  // The initialize function is being called automatically from your entry-point
  // function. So, a call to initialize is not included here. Invoke the
  // entry-point functions.
  // You can call entry-point functions multiple times.
  main_myBestNN();
  main_mySmallNN();
  // Terminate the application.
  // You do not need to do this more than one time.
  myNN_terminate();
  return 0;
}

void main_myBestNN()
{
  float fv[8];
  float p[2];
  float z[2];
  float k;
  // Initialize function 'myBestNN' input arguments.
  // Initialize function input argument 'in'.
  // Call the entry-point 'myBestNN'.
  argInit_1x8_real32_T(fv);
  myBestNN(fv, z, p, &k);
}

void main_mySmallNN()
{
  float fv[8];
  float p[2];
  float z[2];
  float k;
  // Initialize function 'mySmallNN' input arguments.
  // Initialize function input argument 'in'.
  // Call the entry-point 'mySmallNN'.
  argInit_1x8_real32_T(fv);
  mySmallNN(fv, z, p, &k);
}

// End of code generation (main.cpp)
