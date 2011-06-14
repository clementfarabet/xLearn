/*
 * Copyright (c) 2010 Andreas Kloeckner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _CLLUA_HELPER_
#define _CLLUA_HELPER_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
  #include <luaT.h>
  #include <TH.h>
}
#else
#include <luaT.h>
#include <TH.h>
#endif

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define TH_CHECK_CL_ERROR(STATUS_CODE, WHAT) \
  if ((STATUS_CODE) != CL_SUCCESS) \
  { \
    THError("*** '%s' in '%s' on line %d failed with error '%s'.\n", \
                    WHAT, __FILE__, __LINE__,                           \
                    cl_error_to_str(STATUS_CODE));                     \
  }

#define TH_CALL_CL_GUARDED(NAME, ARGLIST) \
  { \
    cl_int status_code; \
      status_code = NAME ARGLIST; \
    TH_CHECK_CL_ERROR(status_code, #NAME); \
  }

#define TH_CHECK_SYS_ERROR(COND, MSG) \
  if (COND) \
  { \
    THError(MSG); \
  }

#define CL_GET_INTEGRAL_INFO(WHAT, FIRST_ARG, SECOND_ARG, VAR)        \
    TH_CALL_CL_GUARDED(clGet##WHAT##Info, \
        (FIRST_ARG, SECOND_ARG, sizeof(VAR), &VAR, 0)) \

#define CL_GET_TENSOR_INFO(WHAT, FIRST_ARG, SECOND_ARG, VAR) \
    size_t size; \
    TH_CALL_CL_GUARDED(clGet##WHAT##Info, \
        (FIRST_ARG, SECOND_ARG, 0, 0, &size)); \
    \
    size_t *items =  (size_t *)malloc(size);\
    TH_CALL_CL_GUARDED(clGet##WHAT##Info, \
        (FIRST_ARG, SECOND_ARG, size, items, &size)); \
    int nbelmt = size/sizeof(size_t); \
    THTensor *VAR = THTensor_newWithSize1d(nbelmt); \
    for (int i=0; i<nbelmt; i++) { \
       VAR->storage->data[i] = (double)items[i]; \
    }

const char *cl_error_to_str(cl_int e);

////////////////////////////////////////////////////////////
// old misc
////////////////////////////////////////////////////////////

#define CHECK_CL_ERROR(STATUS_CODE, WHAT) \
  if ((STATUS_CODE) != CL_SUCCESS) \
  { \
    fprintf(stderr, \
        "*** '%s' in '%s' on line %d failed with error '%s'.\n", \
        WHAT, __FILE__, __LINE__, \
        cl_error_to_str(STATUS_CODE)); \
    exit(1); \
  }

#define CALL_CL_GUARDED(NAME, ARGLIST) \
  { \
    cl_int status_code; \
      status_code = NAME ARGLIST; \
    CHECK_CL_ERROR(status_code, #NAME); \
  }

#define CHECK_SYS_ERROR(COND, MSG) \
  if (COND) \
  { \
    perror(MSG); \
    exit(1); \
  }


void print_platforms_devices();

cl_kernel kernel_from_string(cl_context ctx, 
    char const *knl, char const *knl_name, char const *options);

#define SET_1_KERNEL_ARG(knl, arg0) \
  CALL_CL_GUARDED(clSetKernelArg, (knl, 0, sizeof(arg0), &arg0));

#define SET_2_KERNEL_ARGS(knl, arg0, arg1) \
  CALL_CL_GUARDED(clSetKernelArg, (knl, 0, sizeof(arg0), &arg0)); \
  CALL_CL_GUARDED(clSetKernelArg, (knl, 1, sizeof(arg1), &arg1));

#define SET_3_KERNEL_ARGS(knl, arg0, arg1, arg2) \
  CALL_CL_GUARDED(clSetKernelArg, (knl, 0, sizeof(arg0), &arg0)); \
  CALL_CL_GUARDED(clSetKernelArg, (knl, 1, sizeof(arg1), &arg1)); \
  CALL_CL_GUARDED(clSetKernelArg, (knl, 2, sizeof(arg2), &arg2));

#endif
