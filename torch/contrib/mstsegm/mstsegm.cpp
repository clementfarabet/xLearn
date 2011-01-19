/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

// To load this lib in LUA:
// require 'libmstsegm'

#include <luaT.h>
#include <TH.h>

#include <cstdio>
#include <cstdlib>
#include <image.h>
#include <misc.h>
#include "segment-image.h"

// conversion functions
static image<rgb> *tensor_to_image(THTensor *tensor) {
  // create output
  image<rgb> *img = new image<rgb>(tensor->size[0], tensor->size[1], false);

  // copy data
  int i1,i0;
  for (i1=0; i1<tensor->size[1]; i1++) {  
    for (i0=0; i0<tensor->size[0]; i0++) {
      img->access[i1][i0].r = THTensor_get3d(tensor, i0, i1, 0) * 255;
      img->access[i1][i0].g = THTensor_get3d(tensor, i0, i1, 1) * 255;
      img->access[i1][i0].b = THTensor_get3d(tensor, i0, i1, 2) * 255;
    }
  }

  return img;
}

static THTensor *image_to_tensor(image<rgb> *img) {
  // create output
  THTensor *tensor = THTensor_newWithSize3d(img->width(), img->height(), 3);

  // copy data
  int i1,i0;
  for (i1=0; i1<tensor->size[1]; i1++) {  
    for (i0=0; i0<tensor->size[0]; i0++) {
      THTensor_set3d(tensor, i0, i1, 0, (double)img->access[i1][i0].r / 255);
      THTensor_set3d(tensor, i0, i1, 1, (double)img->access[i1][i0].g / 255);
      THTensor_set3d(tensor, i0, i1, 2, (double)img->access[i1][i0].b / 255);
    }
  }

  return tensor;
}

// Segmentation function
int segm_lua(lua_State *L) {
  // defaults
  float sigma = 0.8;
  float k = 500;
  int min_size = 20;

  // get args
  THTensor *img = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 2)) sigma = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) k = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) min_size = lua_tonumber(L, 4);

  // load input
  image<rgb> *input = tensor_to_image(img);

  // processes
  int num_ccs;
  image<rgb> *seg = segment_image(input, sigma, k, min_size, &num_ccs);

  // return result
  THTensor *result = image_to_tensor(seg);
  luaT_pushudata(L, result, luaT_checktypename2id(L, "torch.Tensor"));
  lua_pushnumber(L, num_ccs);

  delete input;
  delete seg;
  return 2;
}

// Register functions in LUA
static const struct luaL_reg mstsegm [] = {
  {"infer", segm_lua},
  {NULL, NULL}  /* sentinel */
};

extern "C" {
  int luaopen_libmstsegm (lua_State *L) {
    luaL_openlib(L, "libmstsegm", mstsegm, 0);
    return 1; 
  }
}
