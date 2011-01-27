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

 + This is a wrapper for P.Felzenszwalb's (Clement Farabet)
 + Added segment-features.h to process N-dim features rather than 
   simple RGB images
 -
*/

// To load this lib in LUA:
// require 'libmstsegm'

#ifdef LUAJIT
 extern "C" {
#include <luaT.h>
#include <TH.h>
 }
#else
#include <luaT.h>
#include <TH.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <image.h>
#include <misc.h>
#include "segment-volume.h"
#include "segment-features.h"
#include "segment-image.h"

// conversion functions
static image<rgb> *tensor_to_rgbimage(THTensor *tensor) {
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

static image<float> **tensor_to_images(THTensor *tensor) {
  // create output
  image<float> **img = new image<float> *[tensor->size[2]];
  for (int i=0; i<tensor->size[2]; i++)
    img[i] = new image<float>(tensor->size[0], tensor->size[1], false);

  // copy data
  int i2,i1,i0;
  for (i1=0; i1<tensor->size[1]; i1++)  
    for (i0=0; i0<tensor->size[0]; i0++)
      for (i2=0; i2<tensor->size[2]; i2++)
        img[i2]->access[i1][i0] = THTensor_get3d(tensor, i0, i1, i2) * 255;

  return img;
}

static image<float> ***tensor_to_volume(THTensor *tensor) {
  // create output
  image<float> ***img = new image<float> **[tensor->size[3]];
  for (int i=0; i<tensor->size[3]; i++) {
    img[i] = new image<float> *[tensor->size[2]];
    for (int k=0; k<tensor->size[2]; k++)
      img[i][k] = new image<float>(tensor->size[0], tensor->size[1], false);
  }

  // copy data
  int i3,i2,i1,i0;
  for (i3=0; i3<tensor->size[3]; i3++)
    for (i2=0; i2<tensor->size[2]; i2++)
      for (i1=0; i1<tensor->size[1]; i1++)  
        for (i0=0; i0<tensor->size[0]; i0++)
          img[i3][i2]->access[i1][i0] = THTensor_get4d(tensor, i0, i1, i2, i3) * 255;

  return img;
}

static THTensor *rgbimage_to_tensor(image<rgb> *img) {
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

static THTensor *rgbvolume_to_tensor(image<rgb> **img, int z) {
  // create output
  THTensor *tensor = THTensor_newWithSize4d(img[0]->width(), img[0]->height(), 3, z);

  // copy data
  int i3,i1,i0;
  for (i3=0; i3<tensor->size[3]; i3++)
    for (i1=0; i1<tensor->size[1]; i1++)
      for (i0=0; i0<tensor->size[0]; i0++) {
        THTensor_set4d(tensor, i0, i1, 0, i3, (double)img[i3]->access[i1][i0].r / 255);
        THTensor_set4d(tensor, i0, i1, 1, i3, (double)img[i3]->access[i1][i0].g / 255);
        THTensor_set4d(tensor, i0, i1, 2, i3, (double)img[i3]->access[i1][i0].b / 255);
      }

  return tensor;
}

static THTensor *volume_to_tensor(image<float> ***img, int feats, int z) {
  // create output
  THTensor *tensor = THTensor_newWithSize4d(img[0][0]->width(), img[0][0]->height(), feats, z);

  // copy data
  int i3,i2,i1,i0;
  for (i3=0; i3<tensor->size[3]; i3++)
    for (i2=0; i2<tensor->size[2]; i2++)
      for (i1=0; i1<tensor->size[1]; i1++)
        for (i0=0; i0<tensor->size[0]; i0++)
          THTensor_set4d(tensor, i0, i1, i2, i3, (double)img[i3][i2]->access[i1][i0] / 255);

  return tensor;
}

// Segmentation function
int segm_lua(lua_State *L) {
  // defaults
  float sigma   = 0.8;
  float k       = 500;
  int min_size  = 20;
  int dist_type = 0; // 0 == euclidean, 1 == angle
  int method = 1;

  // get args
  THTensor *img = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 2)) sigma = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) k = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) min_size = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) dist_type = lua_tonumber(L, 5);
  if (lua_isnumber(L, 6)) method = lua_tonumber(L, 6);

  // holds nb of segments
  int num_ccs;
  image<rgb> *seg = NULL;
  image<rgb> **segs = NULL;
  THTensor *result = NULL;
  int ndims = img->nDimension;
  int nfeats = img->size[2];

  // Nb of channels ?
  if (ndims == 4) {

    // nb slices
    int nslices = img->size[3];

    // load input (features)
    image<float> ***input = tensor_to_volume(img);

    // process
    segs = segment_volume(input, nslices, nfeats, sigma, k, min_size, dist_type, &num_ccs, method);
    result = rgbvolume_to_tensor(segs, nslices);

    // clean up
    for (int i=0; i<nslices; i++) {
      for (int k=0; k<nfeats; k++) delete input[i][k];
      delete input[i];
    }
    delete [] input;

  } else if (ndims == 3) {
    if (nfeats == 3) {
    
      // load input
      image<rgb> *input = tensor_to_rgbimage(img);

      // process
      seg = segment_image(input, sigma, k, min_size, &num_ccs);
      result = rgbimage_to_tensor(seg);

      // clean up
      delete input;

    } else { // N channels
      
      // load input (features)
      image<float> **input = tensor_to_images(img);
      
      // process
      seg = segment_features(input, nfeats, sigma, k, min_size, dist_type, &num_ccs);
      result = rgbimage_to_tensor(seg);

      // clean up
      for (int i=0; i<nfeats; i++) delete input[i];
      delete [] input;
    }
  } else {
    // unsupported dims
    luaL_error(L, "<libmstsegm.infer> unsupported nb of dimensions");
  }

  // return result
  luaT_pushudata(L, result, luaT_checktypename2id(L, "torch.Tensor"));
  lua_pushnumber(L, num_ccs);

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
