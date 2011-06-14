#ifndef _CL_TENSOR_
#define _CL_TENSOR_

#include <string.h>
#include "context.h"
#include "luaT.h"
#include "TH.h"

// macros
#define CLT_NSIZES 6

static const void* torch_Tensor_id = NULL;

// basic tensor structure
typedef struct {
  context *context;
  THFloatTensor *pTensorLua;
  cl_mem TensorCL;
  long size [CLT_NSIZES];
  long nElement;
  int nDimension;
} tensor;

// Lua access routines
static tensor *totensor (lua_State *L, int index)
{
  tensor *t = (tensor *)lua_touserdata(L, index);
  if (t == NULL) luaL_typerror(L, index, "not a libopencl.tensor");
  return t;
}

static tensor *checktensor(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  tensor *t = (tensor *)luaL_checkudata(L, index, "libopencl.Tensor");
  if (t == NULL) THError("<libopencl.Tensor> requires a valid libopencl.Tensor");
  return t;
}

static tensor *pushtensor(lua_State *L, tensor *t) {
  tensor *nt = (tensor *)lua_newuserdata(L, sizeof(tensor));
  luaL_getmetatable(L, "libopencl.Tensor");
  lua_setmetatable(L, -2);
  memcpy(nt, t, sizeof(tensor));
  free(t);
  return nt;
}

/* 
 * libopencl.tensor Class
 */
// c constructor
static tensor *newtensor(context *c, long *sizes) {
  tensor *t = (tensor *)malloc(sizeof(tensor));
  t->pTensorLua = THFloatTensor_new();
  t->context = c;
  long sz = 1;
  for (int i=0; i<CLT_NSIZES; i++) {
    t->size[i] = sizes[i];
    if (t->size[i] != -1) sz *= t->size[i];
    else {
      t->nDimension = i;
      break;
    }
  }
  t->nElement = sz;
  cl_int status;
  t->TensorCL = clCreateBuffer(*t->context->pctx, CL_MEM_READ_WRITE,
                               sizeof(float) * sz, 0, &status);
  TH_CHECK_CL_ERROR(status, "clCreateBuffer");
  return t;
}

// lua constructor
static int l_newtensor(lua_State *L) {
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  context *c = checkcontext(L,1);
  long sizes[CLT_NSIZES];
  for (int i=0; i<CLT_NSIZES; i++) 
    if (lua_isnumber(L,2+i)) sizes[i] = lua_tonumber(L,2+i);
    else sizes[i] = -1;
  tensor *t = newtensor(c, sizes);
  pushtensor(L,t);
  return 1;
}

static int l_copytensor(lua_State *L) {
  tensor *t = checktensor(L, 1);
  THFloatTensor *dest = t->pTensorLua;
  THTensor *src = luaT_checkudata(L, 2, torch_Tensor_id);
  THFloatTensor_resizeAs(t->pTensorLua, (THFloatTensor*)src);
  TH_TENSOR_APPLY2(float, dest, double, src,  *dest_p = (float) * src_p;)
  // transfert to device
  TH_CALL_CL_GUARDED(clEnqueueWriteBuffer, (
        *t->context->pqueue, t->TensorCL, /*blocking*/ CL_TRUE, /*offset*/ 0,
        t->nElement * sizeof(float), t->pTensorLua->storage->data,
        0, NULL, NULL));  
  return 0;
}

static int l_getluatensor(lua_State *L) {
  tensor *t = checktensor(L, 1);
  THFloatTensor *dest = t->pTensorLua;
  //resize tensor if empty
  THFloatTensor_resize(t->pTensorLua, t->nDimension, t->size);
  // tranfer back
  TH_CALL_CL_GUARDED(clEnqueueReadBuffer, (
        *t->context->pqueue, t->TensorCL, /*blocking*/ CL_TRUE, /*offset*/ 0,
        t->nElement * sizeof(float), dest->storage->data,
        0, NULL, NULL));
  THFloatTensor *destcopyForLua = THFloatTensor_newWithTensor(dest);
  luaT_pushudata(L, destcopyForLua, luaT_checktypename2id(L, "torch.FloatTensor"));
  return 1;
}

static int l_strtensor(lua_State *L) {
  // args
  tensor *t = checktensor(L, 1);

  // print
  char *cstr = (char *)malloc(4096);
  char *str = (char *)cstr;
  str += sprintf(str, "libopencl.Tensor: {\n");
  str += sprintf(str, " + number of elements: %ld\n", t->nElement);
  str += sprintf(str, " + number of dimensions: %d\n", t->nDimension);
  str += sprintf(str, " + shape: (");
  for (int i=0; i<t->nDimension; i++)
    str += sprintf(str, "%ld,", t->size[i]);
  str--;
  str += sprintf(str,")\n}");
  // print
  lua_pushstring(L, cstr);
  free(cstr);
  return 1;
}


// c destructor
static void deltensor(tensor *t) { 
  TH_CALL_CL_GUARDED(clReleaseMemObject, (t->TensorCL));
  THFloatTensor_free(t->pTensorLua);
  free(t);
}
// lua destructor
static int l_deltensor(lua_State *L) {
  tensor *t = checktensor(L, 1);
  TH_CALL_CL_GUARDED(clReleaseMemObject, (t->TensorCL));
  THFloatTensor_free(t->pTensorLua);
  return 0;
}

static const struct luaL_reg opencl_Tensor [] = {
  {"copy",l_copytensor},
  {"getLuaTensor",l_getluatensor},
  {"__tostring", l_strtensor},
  {"__gc", l_deltensor},
  {NULL, NULL}  /* sentinel */
};

void loadclass_Tensor (lua_State *L) {
  // create Tree class
  luaL_newmetatable(L, "libopencl.Tensor");
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2); // pushes the metatable
  lua_pushvalue(L, -3); // metatable.__index = metatable
  luaL_openlib(L, NULL, opencl_Tensor, 0); // register functions for class
}


#endif
