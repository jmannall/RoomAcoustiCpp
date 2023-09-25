#pragma once

#ifndef FIRSTDLL_NATIVE_LIB_H
#define FIRSTDLL_NATIVE_LIB_H

// Enforce OS requirement
#if !defined(_WIN32)
#error "FirstDLL only supports Windows!"
#endif

#define DLLExport __declspec(dllexport)

extern "C"
{
	DLLExport int add(int num1, int num2);
	DLLExport int multiply(int num1, int num2);
	DLLExport int substract(int num1, int num2);
	DLLExport int divide(int num1, int num2);
}
#endif