#ifndef _CL_CONTEXT_
#define _CL_CONTEXT_

#include "cllua-helper.h"
#include <string.h>

// linked list for all kernels within the context
typedef struct list_ker {
  cl_kernel * ker;
   struct list_ker * next;
} kernel_item;

// basic context structure
typedef struct {
  cl_context *pctx;
  cl_command_queue *pqueue;
  cl_platform_id platformID;
  char plat_desc[100];
  cl_device_id deviceID;
  char dev_desc[100];
  int enable_profiling;
  kernel_item *head; // linked list of kernels
  cl_uint dev_address_bytes;
} context;

// lua access routines
static context *tocontext(lua_State *L, int index)
{
  context *c = (context *)lua_touserdata(L, index);
  if (c == NULL) luaL_typerror(L, index, "not a libopencl.Context");
  return c;
}

static context *checkcontext(lua_State *L, int pos) {
  context *c = (context *)luaL_checkudata(L, pos, "libopencl.Context");
  if (c == NULL) THError("<libopencl.Context> requires a valid libopencl.Context");
  return c;
}

static context *pushcontext(lua_State *L, context *c) {
  context *nc = (context *)lua_newuserdata(L, sizeof(context));
  luaL_getmetatable(L, "libopencl.Context");
  lua_setmetatable(L, -2);
  memcpy(nc, c, sizeof(context));
  free(c);
  return nc;
}

/* 
 * libopencl.Context Class
 */
// c constructor
static context *newcontext(const char *plat_name, 
                           const char*dev_name, 
                           cl_uint idx,
                           int enable_profiling) 
{
  context *c = (context *)malloc(sizeof(context));
  c->head = NULL;
  //c->currker = &(c->head);
  c->pctx  = (cl_context *) malloc(sizeof(cl_context));
  c->pqueue  = (cl_command_queue *) malloc(sizeof(cl_command_queue));
  c->enable_profiling = enable_profiling;

  //////////
  // create context
  //////////
  cl_uint plat_count;
  TH_CALL_CL_GUARDED(clGetPlatformIDs, (0, NULL, &plat_count));
  cl_platform_id *platforms = 
    (cl_platform_id *) malloc(plat_count*sizeof(cl_platform_id));
  TH_CHECK_SYS_ERROR(!platforms, "allocating platform array");

  TH_CALL_CL_GUARDED(clGetPlatformIDs, (plat_count, platforms, NULL));
  cl_uint i;
  for (i = 0; i < plat_count; ++i)
  {
    TH_CALL_CL_GUARDED(clGetPlatformInfo, (platforms[i], CL_PLATFORM_VENDOR, 
          sizeof(c->plat_desc), c->plat_desc, NULL));

    if (!plat_name || strstr(c->plat_desc, plat_name))
    {
      cl_uint dev_count;
      TH_CALL_CL_GUARDED(clGetDeviceIDs, (platforms[i], CL_DEVICE_TYPE_ALL,
            0, NULL, &dev_count));

      cl_device_id *devices = 
        (cl_device_id *) malloc(dev_count*sizeof(cl_device_id));
      TH_CHECK_SYS_ERROR(!devices, "allocating device array");

      TH_CALL_CL_GUARDED(clGetDeviceIDs, (platforms[i], CL_DEVICE_TYPE_ALL,
            dev_count, devices, NULL));

      cl_uint j;
      for (j = 0; j < dev_count; ++j)
      {
        TH_CALL_CL_GUARDED(clGetDeviceInfo, (devices[j], CL_DEVICE_NAME,
              sizeof(c->dev_desc), c->dev_desc, NULL));
        if (!dev_name || strstr(c->dev_desc, dev_name))
        {
          if (idx == 0)
          {
            c->platformID = platforms[i];
            c->deviceID = devices[j];

            free(devices);
            free(platforms);

            cl_context_properties cps[3] = { 
              CL_CONTEXT_PLATFORM, (cl_context_properties) c->platformID, 0 };

            cl_int status;
            *(c->pctx) = clCreateContext(cps, 1, &(c->deviceID), NULL, NULL, &status);
            TH_CHECK_CL_ERROR(status, "clCreateContext");


            cl_command_queue_properties qprops = 0;
            if (enable_profiling)
              qprops |= CL_QUEUE_PROFILING_ENABLE;
            //create queue
            *(c->pqueue) = clCreateCommandQueue(*(c->pctx), c->deviceID, qprops, &status);
            TH_CHECK_CL_ERROR(status, "clCreateCommandQueue");
            // get addressing size
            CL_GET_INTEGRAL_INFO(Device, c->deviceID, CL_DEVICE_ADDRESS_BITS, 
                                 c->dev_address_bytes);
            // bit to byte
            c->dev_address_bytes /= 4;

            printf("#Init context on Platform %d: vendor '%s'", i, c->plat_desc);
            TH_CALL_CL_GUARDED(clGetDeviceInfo, (c->deviceID, CL_DEVICE_NAME,
                                              sizeof(c->dev_desc), c->dev_desc, NULL));
            printf("  with Device %d Named '%s' %dbits ARCH\n", 
                   j, c->dev_desc, c->dev_address_bytes*4);
              return c;
          }
          else
            --idx;
        }
      }

      free(devices);
    }
  }

  free(platforms);
  THError("libopencl.Context: specified device not found.");
  return c;
}
// lua constructor
static int l_newcontext(lua_State *L) {
  // args
  const char *plat_name = NULL;
  if (lua_isstring(L, 1)) plat_name = lua_tostring(L, 1);
  const char *dev_name = NULL;
  if (lua_isstring(L, 2)) dev_name = lua_tostring(L, 2);
  cl_uint idx = 0;
  if (lua_isnumber(L, 3)) idx = lua_tonumber(L, 3);
  int enable_profiling = 0;
  if (lua_isboolean(L, 4)) enable_profiling = lua_toboolean(L, 4);
  // create context
  context *c = newcontext(plat_name,dev_name,idx,enable_profiling);
  pushcontext(L,c);
  return 1;
}

// c destructor
static void delcontext(context *c) {
  TH_CALL_CL_GUARDED(clReleaseContext, (*c->pctx));
  TH_CALL_CL_GUARDED(clReleaseCommandQueue, (*c->pqueue));
  //free the kernel linked list
  kernel_item *curr = c->head;
  kernel_item *tmp;
  while (curr != NULL) {
    tmp = curr->next;
    free(curr->ker);
    free(curr);
    curr = tmp;
  }
  free(c);
}
// lua destructor
static int l_delcontext(lua_State *L) {
  // args
  context *c = checkcontext(L, 1);
  // cleanup
  TH_CALL_CL_GUARDED(clReleaseContext, (*c->pctx));
  TH_CALL_CL_GUARDED(clReleaseCommandQueue, (*c->pqueue));
  //free the kernel linked list
  kernel_item *curr = c->head;
  kernel_item *tmp;
  while (curr != NULL) {
    tmp = curr->next;
    free(curr->ker);
    free(curr);
    curr = tmp;
  }
  free(c->pctx);
  free(c->pqueue);
  return 0;
}

static int l_contexttostring(lua_State *L) {
  // args
  context *c = checkcontext(L, 1);
  // print
  char *cstr = (char *)malloc(4096);
  char *str = (char *)cstr;
  str += sprintf(str, "libopencl.context: {\n");
  str += sprintf(str, " + enable_profiling: %s\n", c->enable_profiling?"true":"false");
  str += sprintf(str, " + platform: %s\n", c->plat_desc);
  str += sprintf(str, " + device: %s\n", c->dev_desc);
  str += sprintf(str,"}");
  // print
  lua_pushstring(L, cstr);
  free(cstr);
  return 1;
}

static int l_build_ker_from_source(lua_State *L) {
  // args
  context *ctx = checkcontext(L, 1);
  const char *ker_string = luaL_checkstring(L, 2);
  const char *knl_name = luaL_checkstring(L, 3);
  const char *options = NULL;
  if (lua_isstring(L, 4)) options = lua_tostring(L, 4);

  // malloc new kernel
  kernel_item * knl =  (kernel_item *) malloc(sizeof(kernel_item));
  knl->ker =  (cl_kernel *) malloc(sizeof(cl_kernel));
  // enqueue into the list
  knl->next = ctx->head;
  ctx->head = knl;

  // compile
  size_t sizes[] = { strlen(ker_string) };
  cl_int status;
  cl_program program = clCreateProgramWithSource(*ctx->pctx, 1, &ker_string, sizes, &status);
  TH_CHECK_CL_ERROR(status, "clCreateProgramWithSource");
  status = clBuildProgram(program, 0, NULL, options, NULL, NULL);
  if (status != CL_SUCCESS)
  {
    // build failed, get build log.
    cl_device_id dev;
    TH_CALL_CL_GUARDED(clGetProgramInfo, (program, CL_PROGRAM_DEVICES,
          sizeof(dev), &dev, NULL));

    size_t log_size;
    TH_CALL_CL_GUARDED(clGetProgramBuildInfo, (program, dev, CL_PROGRAM_BUILD_LOG,
          0, NULL, &log_size));

    char *log = malloc(log_size);
    TH_CHECK_SYS_ERROR(!log, "kernel_from_string: allocate log");

    char devname[100];
    TH_CALL_CL_GUARDED(clGetDeviceInfo, (dev, CL_DEVICE_NAME,
          sizeof(devname), devname, NULL));

    TH_CALL_CL_GUARDED(clGetProgramBuildInfo, (program, dev, CL_PROGRAM_BUILD_LOG,
          log_size, log, NULL));
    THError("*** build of '%s' on '%s' failed:\n%s\n*** (end of error)\n",
            knl_name, devname, log);
  }
  else
    TH_CHECK_CL_ERROR(status, "clBuildProgram");

  // build the kernel
  *ctx->head->ker = clCreateKernel(program, knl_name, &status);
  TH_CHECK_CL_ERROR(status, "clCreateKernel");
  TH_CALL_CL_GUARDED(clReleaseProgram, (program));

  // send back pointer to the kernel
  lua_pushlightuserdata(L, ctx->head->ker);

  return 1;
}


#define LUA_GET_DEV_INT_INFO(WHAT, FIRST_ARG, SECOND_ARG, TYPE)        \
  static int l_get##WHAT##_(lua_State *L) { \
    context *c = checkcontext(L, 1); \
    TYPE param_value; \
    CL_GET_INTEGRAL_INFO(Device, FIRST_ARG, SECOND_ARG, param_value); \
    lua_pushnumber(L, param_value); \
    return 1; \
  } \

#define LUA_GET_DEV_TEN_INFO(WHAT, FIRST_ARG, SECOND_ARG)        \
  static int l_get##WHAT##_(lua_State *L) { \
    context *c = checkcontext(L, 1); \
    CL_GET_TENSOR_INFO(Device, FIRST_ARG, SECOND_ARG, res); \
    luaT_pushudata(L, res, luaT_checktypename2id(L, "torch.Tensor")); \
    return 1; \
  } \

/*   *knl->ker = clCreateKernel(program, knl_name, &status); */
/*   CHECK_CL_ERROR(status, "clCreateKernel"); */
/*   CALL_CL_GUARDED(clReleaseProgram, (program)); */
LUA_GET_DEV_INT_INFO(MaxWorkGroupSize, c->deviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t)
LUA_GET_DEV_INT_INFO(MaxComputeUnits, c->deviceID, CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint)

LUA_GET_DEV_TEN_INFO(MaxWorkItemSize, c->deviceID, CL_DEVICE_MAX_WORK_ITEM_SIZES)

static const struct luaL_reg opencl_Context [] = {
  {"getMaxComputeUnits", l_getMaxComputeUnits_},
  {"getMaxWorkGroupSize", l_getMaxWorkGroupSize_},
  {"getMaxWorkItemSize", l_getMaxWorkItemSize_},
  {"buildKernelFromSource", l_build_ker_from_source},
  {"__tostring", l_contexttostring},
  {"__gc", l_delcontext},
  {NULL, NULL}  /* sentinel */
};

void loadclass_Context (lua_State *L) {
  // create Tree class
  luaL_newmetatable(L, "libopencl.Context");
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2); // pushes the metatable
  lua_pushvalue(L, -3); // metatable.__index = metatable
  luaL_openlib(L, NULL, opencl_Context, 0); // register functions for class
}

#endif
