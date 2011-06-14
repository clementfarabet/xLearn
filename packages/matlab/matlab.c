/*
  + This is a wrapper for matlab std I/O functions

  + Supported Types (LOAD):
        mxCELL_CLASS
        mxSTRUCT_CLASS
        mxLOGICAL_CLASS
        mxCHAR_CLASS      Y
        mxDOUBLE_CLASS    Y
        mxSINGLE_CLASS    Y
        mxINT8_CLASS      Y
        mxUINT8_CLASS     Y
        mxINT16_CLASS     Y
        mxUINT16_CLASS    Y (casts to INT16)
        mxINT32_CLASS     Y
        mxUINT32_CLASS    Y (casts to INT32)
        mxINT64_CLASS
        mxUINT64_CLASS
        mxFUNCTION_CLASS

  + Supported Types (SAVE):
        mxCELL_CLASS
        mxSTRUCT_CLASS
        mxLOGICAL_CLASS
        mxCHAR_CLASS      
        mxDOUBLE_CLASS    Y
        mxSINGLE_CLASS    
        mxINT8_CLASS      
        mxUINT8_CLASS     
        mxINT16_CLASS     
        mxUINT16_CLASS    
        mxINT32_CLASS     
        mxUINT32_CLASS    
        mxINT64_CLASS
        mxUINT64_CLASS
        mxFUNCTION_CLASS

  -
*/

// To load this lib in LUA:
// require 'libmatlab'

#include <luaT.h>
#include <TH.h>

#include "mat.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Loader
static int load_l(lua_State *L) {
  // get args
  const char *path = lua_tostring(L,1);

  // open file
  MATFile *file = matOpen(path, "r");
  if (file == NULL) THError("Error opening file %s", file);

  // create table to hold loaded variables
  lua_newtable(L);  // vars = {}
  int vars = lua_gettop(L);
  int varidx = 1;

  // extract each var
  while (true) {
    // get var+name
    const char *name;
    mxArray *pa = matGetNextVariable(file, &name);
    if (pa == NULL) break;

    // get dimensions
    mwSize ndims = mxGetNumberOfDimensions(pa);
    const mwSize *dims = mxGetDimensions(pa);

    // depending on type, create equivalent Lua/torch data structure
    if (mxGetClassID(pa) == mxDOUBLE_CLASS) {
      THTensor *tensor = THTensor_newWithSize(ndims,(long *)dims,NULL);
      memcpy((void *)(THTensor_dataPtr(tensor)),
             (void *)(mxGetPr(pa)), THTensor_nElement(tensor) * sizeof(double));
      lua_pushstring(L, name);
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.Tensor"));
      lua_rawset(L, vars);

    } else if (mxGetClassID(pa) == mxSINGLE_CLASS) {
      THFloatTensor *tensor = THFloatTensor_newWithSize(ndims,(long *)dims,NULL);
      memcpy((void *)(THFloatTensor_dataPtr(tensor)),
             (void *)(mxGetPr(pa)), THFloatTensor_nElement(tensor) * sizeof(float));
      lua_pushstring(L, name);
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.FloatTensor"));
      lua_rawset(L, vars);

    } else if (mxGetClassID(pa) == mxINT32_CLASS) {
      THIntTensor *tensor = THIntTensor_newWithSize(ndims,(long *)dims,NULL);
      memcpy((void *)(THIntTensor_dataPtr(tensor)),
             (void *)(mxGetPr(pa)), THIntTensor_nElement(tensor) * sizeof(int));
      lua_pushstring(L, name);
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.IntTensor"));
      lua_rawset(L, vars);

    } else if (mxGetClassID(pa) == mxUINT32_CLASS) {
      THIntTensor *tensor = THIntTensor_newWithSize(ndims,(long *)dims,NULL);
      memcpy((void *)(THIntTensor_dataPtr(tensor)),
             (void *)(mxGetPr(pa)), THIntTensor_nElement(tensor) * sizeof(int));
      lua_pushstring(L, name);
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.IntTensor"));
      lua_rawset(L, vars);

    } else if ((mxGetClassID(pa) == mxINT16_CLASS)) {
      THShortTensor *tensor = THShortTensor_newWithSize(ndims,(long *)dims,NULL);
      memcpy((void *)(THShortTensor_dataPtr(tensor)),
             (void *)(mxGetPr(pa)), THShortTensor_nElement(tensor) * sizeof(short));
      lua_pushstring(L, name);
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.ShortTensor"));
      lua_rawset(L, vars);

    } else if ((mxGetClassID(pa) == mxUINT16_CLASS)) {
      THShortTensor *tensor = THShortTensor_newWithSize(ndims,(long *)dims,NULL);
      memcpy((void *)(THShortTensor_dataPtr(tensor)),
             (void *)(mxGetPr(pa)), THShortTensor_nElement(tensor) * sizeof(short));
      lua_pushstring(L, name);
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.ShortTensor"));
      lua_rawset(L, vars);

    } else if ((mxGetClassID(pa) == mxINT8_CLASS) || (mxGetClassID(pa) == mxCHAR_CLASS)) {
      THCharTensor *tensor = THCharTensor_newWithSize(ndims,(long *)dims,NULL);
      memcpy((void *)(THCharTensor_dataPtr(tensor)),
             (void *)(mxGetPr(pa)), THCharTensor_nElement(tensor) * sizeof(char));
      lua_pushstring(L, name);
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.CharTensor"));
      lua_rawset(L, vars);

    } else if ((mxGetClassID(pa) == mxUINT8_CLASS)) {
      THByteTensor *tensor = THByteTensor_newWithSize(ndims,(long *)dims,NULL);
      memcpy((void *)(THByteTensor_dataPtr(tensor)),
             (void *)(mxGetPr(pa)), THByteTensor_nElement(tensor) * sizeof(char));
      lua_pushstring(L, name);
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.ByteTensor"));
      lua_rawset(L, vars);

    } else if ((mxGetClassID(pa) == mxLOGICAL_CLASS)) {
      THByteTensor *tensor = THByteTensor_newWithSize(ndims,(long *)dims,NULL);
      memcpy((void *)(THByteTensor_dataPtr(tensor)),
             (void *)(mxGetPr(pa)), THByteTensor_nElement(tensor) * sizeof(char));
      lua_pushstring(L, name);
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.ByteTensor"));
      lua_rawset(L, vars);

    } else {
      lua_pushstring(L, name);
      if ((mxGetClassID(pa) == mxCELL_CLASS)) {
        lua_pushstring(L, "unsupported type: mxCELL_CLASS");
      } else if ((mxGetClassID(pa) == mxSTRUCT_CLASS)) {
        lua_pushstring(L, "unsupported type: mxSTRUCT_CLASS");
      } else if ((mxGetClassID(pa) == mxINT64_CLASS)) {
        lua_pushstring(L, "unsupported type: mxINT64_CLASS");
      } else if ((mxGetClassID(pa) == mxUINT64_CLASS)) {
        lua_pushstring(L, "unsupported type: mxUINT64_CLASS");
      } else if ((mxGetClassID(pa) == mxFUNCTION_CLASS)) {
        lua_pushstring(L, "unsupported type: mxFUNCTION_CLASS");
      } else {
        lua_pushstring(L, "unknown type");
      }
      lua_rawset(L, vars);
    }
    mxDestroyArray(pa);
  }

  // cleanup
  matClose(file);

  // return table 'vars'
  return 1;
}

// Save single tensor
static int save_tensor_l(lua_State *L) {
  // open file for output
  const char *path = lua_tostring(L,1);
  MATFile *file = matOpen(path, "w");

  // load tensor
  THTensor *tensor = (THTensor *)luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));

  // create matlab array
  mxArray *pm = mxCreateNumericArray(tensor->nDimension, (const mwSize *)tensor->size, 
                                     mxDOUBLE_CLASS, mxREAL);
  // copy tensor
  memcpy((void *)(mxGetPr(pm)), 
         (void *)(THTensor_dataPtr(tensor)),
         THTensor_nElement(tensor) * sizeof(double));

  // save it, in a dummy var named 'x'
  const char *name = "x";
  matPutVariable(file, name, pm);

  // done
  matClose(file);
  return 0;
}

// Save table of tensors
static int save_table_l(lua_State *L) {
  // open file for output
  const char *path = lua_tostring(L,1);
  MATFile *file = matOpen(path, "w");

  // table is in the stack at index 2 (2nd var)
  lua_pushnil(L);  // first key
  while (lua_next(L, 2) != 0) {
    // uses 'key' (at index -2) and 'value' (at index -1)
    const char *name = lua_tostring(L,-2);
    THTensor *tensor = (THTensor *)luaT_checkudata(L, -1, luaT_checktypename2id(L, "torch.Tensor"));

    // create matlab array
    mxArray *pm = mxCreateNumericArray(tensor->nDimension, (const mwSize *)tensor->size, 
                                       mxDOUBLE_CLASS, mxREAL);
    // copy tensor into array
    memcpy((void *)(mxGetPr(pm)), 
           (void *)(THTensor_dataPtr(tensor)),
           THTensor_nElement(tensor) * sizeof(double));

    // store it
    matPutVariable(file, name, pm);

    // removes 'value'; keeps 'key' for next iteration
    lua_pop(L, 1);
  }

  // cleanup
  lua_pop(L, 1);
  matClose(file);
  return 0;
}

// Register functions in LUA
static const struct luaL_reg matlab [] = {
  {"load", load_l},
  {"saveTensor", save_tensor_l},
  {"saveTable", save_table_l},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libmatlab (lua_State *L) {
  luaL_openlib(L, "libmatlab", matlab, 0);
  return 1;
}
