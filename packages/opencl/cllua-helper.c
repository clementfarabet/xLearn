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
#include "cllua-helper.h"
#include <string.h>

const char *cl_error_to_str(cl_int e)
{
  switch (e)
  {
    case CL_SUCCESS: return "success";
    case CL_DEVICE_NOT_FOUND: return "device not found";
    case CL_DEVICE_NOT_AVAILABLE: return "device not available";
#if !(defined(CL_PLATFORM_NVIDIA) && CL_PLATFORM_NVIDIA == 0x3001)
    case CL_COMPILER_NOT_AVAILABLE: return "device compiler not available";
#endif
    case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "mem object allocation failure";
    case CL_OUT_OF_RESOURCES: return "out of resources";
    case CL_OUT_OF_HOST_MEMORY: return "out of host memory";
    case CL_PROFILING_INFO_NOT_AVAILABLE: return "profiling info not available";
    case CL_MEM_COPY_OVERLAP: return "mem copy overlap";
    case CL_IMAGE_FORMAT_MISMATCH: return "image format mismatch";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "image format not supported";
    case CL_BUILD_PROGRAM_FAILURE: return "build program failure";
    case CL_MAP_FAILURE: return "map failure";

    case CL_INVALID_VALUE: return "invalid value";
    case CL_INVALID_DEVICE_TYPE: return "invalid device type";
    case CL_INVALID_PLATFORM: return "invalid platform";
    case CL_INVALID_DEVICE: return "invalid device";
    case CL_INVALID_CONTEXT: return "invalid context";
    case CL_INVALID_QUEUE_PROPERTIES: return "invalid queue properties";
    case CL_INVALID_COMMAND_QUEUE: return "invalid command queue";
    case CL_INVALID_HOST_PTR: return "invalid host ptr";
    case CL_INVALID_MEM_OBJECT: return "invalid mem object";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "invalid image format descriptor";
    case CL_INVALID_IMAGE_SIZE: return "invalid image size";
    case CL_INVALID_SAMPLER: return "invalid sampler";
    case CL_INVALID_BINARY: return "invalid binary";
    case CL_INVALID_BUILD_OPTIONS: return "invalid build options";
    case CL_INVALID_PROGRAM: return "invalid program";
    case CL_INVALID_PROGRAM_EXECUTABLE: return "invalid program executable";
    case CL_INVALID_KERNEL_NAME: return "invalid kernel name";
    case CL_INVALID_KERNEL_DEFINITION: return "invalid kernel definition";
    case CL_INVALID_KERNEL: return "invalid kernel";
    case CL_INVALID_ARG_INDEX: return "invalid arg index";
    case CL_INVALID_ARG_VALUE: return "invalid arg value";
    case CL_INVALID_ARG_SIZE: return "invalid arg size";
    case CL_INVALID_KERNEL_ARGS: return "invalid kernel args";
    case CL_INVALID_WORK_DIMENSION: return "invalid work dimension";
    case CL_INVALID_WORK_GROUP_SIZE: return "invalid work group size";
    case CL_INVALID_WORK_ITEM_SIZE: return "invalid work item size";
    case CL_INVALID_GLOBAL_OFFSET: return "invalid global offset";
    case CL_INVALID_EVENT_WAIT_LIST: return "invalid event wait list";
    case CL_INVALID_EVENT: return "invalid event";
    case CL_INVALID_OPERATION: return "invalid operation";
    case CL_INVALID_GL_OBJECT: return "invalid gl object";
    case CL_INVALID_BUFFER_SIZE: return "invalid buffer size";
    case CL_INVALID_MIP_LEVEL: return "invalid mip level";

#if defined(cl_khr_gl_sharing) && (cl_khr_gl_sharing >= 1)
    case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR: return "invalid gl sharegroup reference number";
#endif

#ifdef CL_VERSION_1_1
    case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "misaligned sub-buffer offset";
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "exec status error for events in wait list";
    case CL_INVALID_GLOBAL_WORK_SIZE: return "invalid global work size";
#endif

    default: return "invalid/unknown error code";
  }
}

//TODO move this to the routine opencl.printAllPlatform()
void print_platforms_devices()
{
  cl_uint plat_count;

  CALL_CL_GUARDED(clGetPlatformIDs, (0, NULL, &plat_count));

  cl_platform_id *platforms = 
    (cl_platform_id *) malloc(plat_count*sizeof(cl_platform_id));
  CHECK_SYS_ERROR(!platforms, "allocating platform array");

  CALL_CL_GUARDED(clGetPlatformIDs, (plat_count, platforms, NULL));
  cl_uint i;
  for (i = 0; i < plat_count; ++i)
  {
    char buf[100];
    CALL_CL_GUARDED(clGetPlatformInfo, (platforms[i], CL_PLATFORM_VENDOR, 
          sizeof(buf), buf, NULL));
    printf("plat %d: vendor '%s'\n", i, buf);

    cl_uint dev_count;
    CALL_CL_GUARDED(clGetDeviceIDs, (platforms[i], CL_DEVICE_TYPE_ALL,
          0, NULL, &dev_count));

    cl_device_id *devices = 
      (cl_device_id *) malloc(dev_count*sizeof(cl_device_id));
    CHECK_SYS_ERROR(!devices, "allocating device array");

    CALL_CL_GUARDED(clGetDeviceIDs, (platforms[i], CL_DEVICE_TYPE_ALL,
          dev_count, devices, NULL));

    cl_uint j;
    for (j = 0; j < dev_count; ++j)
    {
      char buf[100];
      CALL_CL_GUARDED(clGetDeviceInfo, (devices[j], CL_DEVICE_NAME,
            sizeof(buf), buf, NULL));
      printf("  dev %d '%s'\n", j, buf);
    }

    free(devices);
  }

  free(platforms);
}

//TODO remove this at some point (deprecated)
cl_kernel kernel_from_string(cl_context ctx, 
    char const *knl, char const *knl_name, char const *options)
{
  size_t sizes[] = { strlen(knl) };

  cl_int status;
  cl_program program = clCreateProgramWithSource(ctx, 1, &knl, sizes, &status);
  CHECK_CL_ERROR(status, "clCreateProgramWithSource");

  status = clBuildProgram(program, 0, NULL, options, NULL, NULL);

  if (status != CL_SUCCESS)
  {
    // build failed, get build log.

    cl_device_id dev;
    CALL_CL_GUARDED(clGetProgramInfo, (program, CL_PROGRAM_DEVICES,
          sizeof(dev), &dev, NULL));

    size_t log_size;
    CALL_CL_GUARDED(clGetProgramBuildInfo, (program, dev, CL_PROGRAM_BUILD_LOG,
          0, NULL, &log_size));

    char *log = malloc(log_size);
    CHECK_SYS_ERROR(!log, "kernel_from_string: allocate log");

    char devname[100];
    CALL_CL_GUARDED(clGetDeviceInfo, (dev, CL_DEVICE_NAME,
          sizeof(devname), devname, NULL));

    CALL_CL_GUARDED(clGetProgramBuildInfo, (program, dev, CL_PROGRAM_BUILD_LOG,
          log_size, log, NULL));
    THError("*** build of '%s' on '%s' failed:\n%s\n*** (end of error)\n",
            knl_name, devname, log);
  }
  else
    CHECK_CL_ERROR(status, "clBuildProgram");

  cl_kernel kernel = clCreateKernel(program, knl_name, &status);
  CHECK_CL_ERROR(status, "clCreateKernel");

  CALL_CL_GUARDED(clReleaseProgram, (program));

  return kernel;
}
