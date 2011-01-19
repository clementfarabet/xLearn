
// To load this lib in LUA:
// require 'libopticalFlow'

#include <luaT.h>
#include <TH.h>

#include "project.h"
#include "Image.h"
#include "OpticalFlow.h"
#include <iostream>

using namespace std;

// conversion functions
static DImage *tensor_to_image(THTensor *tensor) {
  // create output
  int w = tensor->size[0];
  int h = tensor->size[1]; 
  int c = tensor->size[2];
  DImage *img = new DImage(w,h,c);

  // copy data
  int i1,i0,i2;
  double *dest = img->data();
  int offset = 0;
  for (i1=0; i1<tensor->size[1]; i1++) {  
    for (i0=0; i0<tensor->size[0]; i0++) {
      for (i2=0; i2<tensor->size[2]; i2++) {
        dest[offset++] = THTensor_get3d(tensor, i0, i1, i2);
      }
    }
  }

  return img;
}

static THTensor *image_to_tensor(DImage *img) {
  // create output
  THTensor *tensor = THTensor_newWithSize3d(img->width(), img->height(), img->nchannels());

  // copy data
  int i1,i0,i2;
  double *src = img->data();
  int offset = 0;
  for (i1=0; i1<tensor->size[1]; i1++) {  
    for (i0=0; i0<tensor->size[0]; i0++) {
      for (i2=0; i2<tensor->size[2]; i2++) {
        THTensor_set3d(tensor, i0, i1, i2, src[offset++]);
      }
    }
  }

  return tensor;
}

int optflow_lua(lua_State *L) {
  // defaults
  double alpha=0.01;
  double ratio=0.75;
  int minWidth=30;
  int nOuterFPIterations=15;
  int nInnerFPIterations=1;
  int nCGIterations=40;

  // get args
  THTensor *ten1 = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *ten2 = (THTensor *)luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 3)) alpha = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) ratio = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) minWidth = lua_tonumber(L, 5);
  if (lua_isnumber(L, 6)) nOuterFPIterations = lua_tonumber(L, 6);
  if (lua_isnumber(L, 7)) nInnerFPIterations = lua_tonumber(L, 7);
  if (lua_isnumber(L, 8)) nCGIterations = lua_tonumber(L, 8);

  // copy tensors to images
  DImage *img1 = tensor_to_image(ten1);
  DImage *img2 = tensor_to_image(ten2);

  // declare outputs, and process
  DImage vx,vy,warpI2;
  OpticalFlow::Coarse2FineFlow(vx,vy,warpI2,   // outputs
                               *img1,*img2,      // inputs
                               alpha,ratio,minWidth,  // params...
                               nOuterFPIterations,nInnerFPIterations,nCGIterations);

  // return result
  THTensor *ten_vx = image_to_tensor(&vx);
  THTensor *ten_vy = image_to_tensor(&vy);
  THTensor *ten_warp = image_to_tensor(&warpI2);
  luaT_pushudata(L, ten_vx, luaT_checktypename2id(L, "torch.Tensor"));
  luaT_pushudata(L, ten_vy, luaT_checktypename2id(L, "torch.Tensor"));
  luaT_pushudata(L, ten_warp, luaT_checktypename2id(L, "torch.Tensor"));

  // cleanup
  delete(img1);
  delete(img2);

  return 3;
}

// Register functions in LUA
static const struct luaL_reg opticalFlow [] = {
  {"infer", optflow_lua},
  {NULL, NULL}  /* sentinel */
};

extern "C" {
  int luaopen_libopticalFlow (lua_State *L) {
    luaL_openlib(L, "libopticalFlow", opticalFlow, 0);
    return 1; 
  }
}
