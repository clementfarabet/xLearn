#include "cllua-helper.h"
#include "context.h"
#include "tensor.h"

//TODO: remove this at some point since nobody uses it
static int l_kernel_from_string(lua_State *L) {
  context *ctx =checkcontext(L, 1);
  const char *ker_string = luaL_checkstring(L, 2);
  const char *fname = luaL_checkstring(L, 3);
  cl_kernel * knl =  (cl_kernel *) malloc(sizeof(knl));
  printf("kernel from string is deprecated...\n");
  *knl = kernel_from_string(*ctx->pctx, ker_string, fname, NULL);
  lua_pushlightuserdata(L, knl);
  return 1;
}

static int l_print_platforms_devices(lua_State *L) {
  print_platforms_devices();
  return 0;
}


/*
  - take a context
  - run elwise kerne on the array
  @params:
  - context
  - kernel_source
  - kernel_name
  - global dims
  - local dims
  - TencsorCl
  - fillvalue
 */
static int l_runelwiseWithVal(lua_State *L) {
  // get params
  context *ctx = checkcontext(L, 1);
  const char *ker_string = luaL_checkstring(L, 2);
  const char *knl_name = luaL_checkstring(L, 3);
  const void* torch_LongTensor_id = luaT_checktypename2id(L, "torch.LongTensor");
  THLongTensor *gdim = luaT_checkudata(L, 4, torch_LongTensor_id);
  THLongTensor *ldim = luaT_checkudata(L, 5, torch_LongTensor_id);
  tensor *t = (tensor *)luaL_checkudata(L, 6, "libopencl.Tensor");
  float val = (float)luaL_checknumber(L, 7);

  // create kernel
  cl_int status;
  size_t sizes[] = { strlen(ker_string) };
  cl_program program = clCreateProgramWithSource(*ctx->pctx, 1, &ker_string, sizes, &status);
  TH_CHECK_CL_ERROR(status, "clCreateProgramWithSource");
  status = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (status != CL_SUCCESS)
    {printf("didnot compile\n"); }
  cl_kernel kernel = clCreateKernel(program, knl_name, &status);
  TH_CHECK_CL_ERROR(status, "clCreateKernel");
  TH_CALL_CL_GUARDED(clReleaseProgram, (program));

  // run code on GPU
  unsigned int n = t->nElement;
  cl_mem ptr = t->TensorCL;
  cl_uint work_dim = (cl_uint)gdim->size[0];  
  if (work_dim  != 3)
    luaL_error(L, "<libopencl.runelwise> local and global work dim must be 3 dimensions");
  SET_3_KERNEL_ARGS(kernel, ptr, val, n);
  size_t ngdim[] = { gdim->storage->data[0],
                     gdim->storage->data[1],
                     gdim->storage->data[2] };
  size_t nldim[] = { ldim->storage->data[0],
                     ldim->storage->data[1],
                     ldim->storage->data[2] };
  TH_CALL_CL_GUARDED(clEnqueueNDRangeKernel,
      (*ctx->pqueue, kernel,
        /*dimensions*/ work_dim, NULL, ngdim, nldim,
       0, NULL, NULL));
  TH_CALL_CL_GUARDED(clReleaseKernel, (kernel));
  return 0;
}

static int l_test(lua_State *L) {
  context *ctx = checkcontext(L, 1);
  cl_kernel * pknl = NULL;
  if (lua_islightuserdata(L, 2))
    pknl = (cl_kernel *) lua_topointer (L, 2);
  else
    luaL_error(L, "<libopencl.test> you need to provide the kernel");
  float p = luaL_checknumber(L, 3);

  const size_t sz = 10;
  float a[sz], b[sz], c[sz];
  int i;
  for (i = 0; i < sz; ++i)
  {
    a[i] = i+p;
    b[i] = 2*i+p;
  }


  // --------------------------------------------------------------------------
  // allocate GPU memory
  // --------------------------------------------------------------------------
  cl_int status;
  cl_mem buf_a = clCreateBuffer(*ctx->pctx, CL_MEM_READ_WRITE,
      sizeof(float) * sz, 0, &status);
  TH_CHECK_CL_ERROR(status, "clCreateBuffer");

  cl_mem buf_b = clCreateBuffer(*ctx->pctx, CL_MEM_READ_WRITE,
      sizeof(float) * sz, 0, &status);
  TH_CHECK_CL_ERROR(status, "clCreateBuffer");

  cl_mem buf_c = clCreateBuffer(*ctx->pctx, CL_MEM_READ_WRITE,
      sizeof(float) * sz, 0, &status);
  TH_CHECK_CL_ERROR(status, "clCreateBuffer");


  // --------------------------------------------------------------------------
  // transfer to GPU
  // --------------------------------------------------------------------------
  TH_CALL_CL_GUARDED(clEnqueueWriteBuffer, (
        *ctx->pqueue, buf_a, /*blocking*/ CL_TRUE, /*offset*/ 0,
        sz * sizeof(float), a,
        0, NULL, NULL));

  TH_CALL_CL_GUARDED(clEnqueueWriteBuffer, (
        *ctx->pqueue, buf_b, /*blocking*/ CL_TRUE, /*offset*/ 0,
        sz * sizeof(float), b,
        0, NULL, NULL));

  // --------------------------------------------------------------------------
  // run code on GPU
  // --------------------------------------------------------------------------
  SET_3_KERNEL_ARGS(*pknl, buf_a, buf_b, buf_c);
  size_t gdim[] = { sz };
  size_t ldim[] = { 1 };
  TH_CALL_CL_GUARDED(clEnqueueNDRangeKernel,
      (*ctx->pqueue, *pknl,
       /*dimensions*/ 1, NULL, gdim, ldim,
       0, NULL, NULL));

  // --------------------------------------------------------------------------
  // transfer back & check
  // --------------------------------------------------------------------------
  TH_CALL_CL_GUARDED(clEnqueueReadBuffer, (
        *ctx->pqueue, buf_c, /*blocking*/ CL_TRUE, /*offset*/ 0,
        sz * sizeof(float), c,
        0, NULL, NULL));

  
  printf("a: ");
  for (i = 0; i < sz; ++i)
    printf(" %f, ",a[i]);
  printf("\nb:  ");
  for (i = 0; i < sz; ++i)
    printf(" %f, ",b[i]);
  printf("\n a + b :  ");
  for (i = 0; i < sz; ++i) {
    printf(" %f, ",c[i]);
    if (c[i] != a[i]+b[i])
      printf("BAD %d!\n", i);
  }
  printf("\nALL GOOD ?\n");
  // --------------------------------------------------------------------------
  // clean up
  // --------------------------------------------------------------------------
  TH_CALL_CL_GUARDED(clReleaseMemObject, (buf_a));
  TH_CALL_CL_GUARDED(clReleaseMemObject, (buf_b));
  TH_CALL_CL_GUARDED(clReleaseMemObject, (buf_c));

  return 0;
}

static int l_kernel_setarg(lua_State *L) {
  cl_kernel * pknl = NULL;
  if (lua_islightuserdata(L, 1))
    pknl = (cl_kernel *) lua_topointer (L, 1);
  else
    luaL_error(L, "<libopencl.setarg> you need to provide the kernel");
  cl_uint arg_idx = (cl_uint)luaL_checknumber(L, 2);

  const char * arg_type = luaL_checkstring(L, 3);;

  if (!strcmp(arg_type, "float*"))
    { // we got a opencl.Tensor
      tensor *t = (tensor *)luaL_checkudata(L, 4, "libopencl.Tensor");
      TH_CALL_CL_GUARDED(clSetKernelArg, \
                         (*pknl, arg_idx, sizeof(float *), &(t->TensorCL)));
    } 
  else if (!strcmp(arg_type, "float"))
    { // got a float
      float n = (float)luaL_checknumber(L, 4);
      TH_CALL_CL_GUARDED(clSetKernelArg, \
                         (*pknl, arg_idx, sizeof(float), &n));

    } 
  else if (!strcmp(arg_type, "unsigned int"))
    { // got a u int
      unsigned int n = (unsigned int)luaL_checknumber(L, 4);
      TH_CALL_CL_GUARDED(clSetKernelArg, \
                         (*pknl, arg_idx, sizeof(unsigned int), &n));
    }
  else
    THError("<libopencl.setarg> arg '%s' type unknow (or not implemented)",arg_type);

  return 0;
}

static int l_kernel_enqueue(lua_State *L) {
  context *ctx = checkcontext(L, 1);
  cl_kernel * pknl = NULL;
  if (lua_islightuserdata(L, 2))
    pknl = (cl_kernel *) lua_topointer (L, 2);
  else
    luaL_error(L, "<libopencl.enqueue> you need to provide the kernel");

  const void* torch_LongTensor_id = luaT_checktypename2id(L, "torch.LongTensor");
  THLongTensor *gdim = luaT_checkudata(L, 3, torch_LongTensor_id);
  THLongTensor *ldim = luaT_checkudata(L, 4, torch_LongTensor_id);

  if (gdim->nDimension  != 1 || ldim->nDimension != 1)
    luaL_error(L, "<libopencl.enqueue> global work or local_work should be 1D longTensors");
  if (gdim->size[0]  != ldim->size[0])
    luaL_error(L, "<libopencl.enqueue> nb of local_work and global_work dimension array must have same size");
  if (ldim->nDimension  > CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS)
    luaL_error(L, "<libopencl.enqueue> nb of local_work dimension is too high for the kernel call max=%d", 
               CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
  if (gdim->nDimension > CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS)
    THError("<libopencl.enqueue> nb of global_work dimension is too high for the kernel call max=%d", 
            CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);

  cl_uint work_dim = (cl_uint)gdim->size[0];
  size_t *sgdim = (size_t*) malloc(work_dim*sizeof(size_t));
  for(int i=0; i < work_dim; ++i)
    sgdim[i] = (size_t) gdim->storage->data[i];
  size_t *sldim = (size_t*) malloc(work_dim*sizeof(size_t));
  for(int i=0; i < work_dim; ++i)
    sldim[i] = (size_t) ldim->storage->data[i];
  // enqueue the run
  cl_event ceEvent;
  TH_CALL_CL_GUARDED(clEnqueueNDRangeKernel,
      (*ctx->pqueue, *pknl,
        /*dimensions*/ work_dim, NULL, sgdim, sldim,
       0, NULL, &ceEvent));
  TH_CALL_CL_GUARDED(clEnqueueWaitForEvents,
      (*ctx->pqueue, 1, &ceEvent));
  double dSeconds = 0;
  if (ctx->enable_profiling)
    {
      TH_CALL_CL_GUARDED(clWaitForEvents,
                         (1, &ceEvent));
      cl_ulong start, end;
      TH_CALL_CL_GUARDED(clGetEventProfilingInfo,
                         (ceEvent, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL));
      TH_CALL_CL_GUARDED(clGetEventProfilingInfo,
                         (ceEvent, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL));
      dSeconds = 1.0e-9 * (double)(end - start);
    }
  free(sgdim);
  free(sldim);
  // return running time if profiling, 0 otherwise
  lua_pushnumber(L, dSeconds);
  return 1;
}

//============================================================
// Register functions in LUA
//
static const struct luaL_reg opencl [] = {
  /* Classes */
  {"Context", l_newcontext},
  {"Tensor", l_newtensor},
  /* Routines */
  {"enqueue", l_kernel_enqueue},
  {"setarg", l_kernel_setarg},
  {"printDevices", l_print_platforms_devices},
  {"runElwiseWithVal", l_runelwiseWithVal},
  {"test", l_test},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libopencl (lua_State *L) {
  // create Context class
  loadclass_Context(L);
  // create TensorCL class
  loadclass_Tensor(L);
  // register global library
  luaL_openlib(L, "libopencl", opencl, 0);
  return 1; 
}
