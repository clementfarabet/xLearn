/*
  Copyright ESIEE (2009,2010) 

  m.couprie@esiee.fr, cfarabet@nyu.edu
  
  This software is an image processing library whose purpose is to be
  used primarily for research and teaching.
  
  This software is governed by the CeCILL  license under French law and
  abiding by the rules of distribution of free software. You can  use, 
  modify and/ or redistribute the software under the terms of the CeCILL
  license as circulated by CEA, CNRS and INRIA at the following URL
  "http://www.cecill.info". 
  
  As a counterpart to the access to the source code and  rights to copy,
  modify and redistribute granted by the license, users are provided only
  with a limited warranty  and the software's author,  the holder of the
  economic rights,  and the successive licensors  have only  limited
  liability. 
  
  In this respect, the user's attention is drawn to the risks associated
  with loading,  using,  modifying and/or developing or reproducing the
  software by the user in light of its specific status of free software,
  that may mean  that it is complicated to manipulate,  and  that  also
  therefore means  that it is reserved for developers  and  experienced
  professionals having in-depth computer knowledge. Users are therefore
  encouraged to load and test the software's suitability as regards their
  requirements in conditions enabling the security of their systems and/or 
  data to be ensured and,  more generally, to use and operate it in the 
  same conditions as regards security. 
  
  The fact that you are presently reading this means that you have had
  knowledge of the CeCILL license and that you accept its terms.

  + This is a wrapper for Pink - a morphology library
  -
*/

// To load this lib in LUA:
// require 'libpink'

extern "C" {
#include <luaT.h>
#include <TH.h>
}

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <mcimage.h>
#include <mccodimage.h>
#include <jcimage.h>
#include <jccodimage.h>
#include <lppm2GA.h>
#include <lwshedtopo.h>
#include <lderiche.h>
#include <lminima.h>
#include <lhierarchie.h>
#include <lga2khalimsky.h>

#include <disjoint-set.h>

// conversion functions
static struct xvimage *tensor_to_xvimage_3d(THTensor *tensor) {
  // create output
  struct xvimage *img = allocimage(NULL, 
                                   tensor->size[0], tensor->size[1], tensor->size[2], 
                                   VFF_TYP_1_BYTE);

  // get pointer
  uint8_t *raw = UCHARDATA(img);

  // copy data
  index_t i2,i1,i0;
  for (i2=0; i2<tensor->size[2]; i2++) {  
    for (i1=0; i1<tensor->size[1]; i1++) {  
      for (i0=0; i0<tensor->size[0]; i0++) {
        raw[(i2*img->col_size + i1)*img->row_size + i0] = THTensor_get3d(tensor, i0, i1, i2) * 255;
      }
    }
  }

  // return copy
  return img;
}

static THTensor *xvimage_to_tensor_3d(struct xvimage *img) {
  // create output
  THTensor *tensor = THTensor_newWithSize3d(img->row_size, img->col_size, img->depth_size);

  // get pointer
  uint8_t *raw = UCHARDATA(img);

  // copy data
  index_t i2,i1,i0;
  for (i2=0; i2<tensor->size[2]; i2++) {  
    for (i1=0; i1<tensor->size[1]; i1++) {  
      for (i0=0; i0<tensor->size[0]; i0++) {
        THTensor_set3d(tensor, i0, i1, i2, 
                       (double)raw[(i2*img->col_size + i1)*img->row_size + i0] / 255);
      }
    }
  }

  // return copy
  return tensor;
}

// conversion functions
static struct xvimage *tensor_to_xvimage_2d(THTensor *tensor) {
  // create output
  struct xvimage *img = allocimage(NULL, 
                                   tensor->size[0], tensor->size[1], 1, 
                                   VFF_TYP_1_BYTE);

  // get pointer
  uint8_t *raw = UCHARDATA(img);

  // copy data
  index_t i1,i0;
  for (i1=0; i1<tensor->size[1]; i1++) {  
    for (i0=0; i0<tensor->size[0]; i0++) {
      raw[i1*img->row_size + i0] = THTensor_get2d(tensor, i0, i1) * 255;
    }
  }

  // return copy
  return img;
}

static THTensor *xvimage_to_tensor_2d(struct xvimage *img) {
  // create output
  THTensor *tensor = THTensor_newWithSize2d(img->row_size, img->col_size);

  // get pointer
  uint8_t *raw = UCHARDATA(img);

  // copy data
  index_t i1,i0;
  for (i1=0; i1<tensor->size[1]; i1++) {  
    for (i0=0; i0<tensor->size[0]; i0++) {
      THTensor_set2d(tensor, i0, i1, 
                     (double)raw[i1*img->row_size + i0] / 255);
    }
  }

  // return copy
  return tensor;
}

// conversion functions
static struct xvimage *tensor_to_xvgraph_2d(THTensor *tensor) {
  // create output
  struct xvimage *img = allocGAimage(NULL, 
                                     tensor->size[0], tensor->size[1]/2, 1, 
                                     VFF_TYP_GABYTE);

  // get pointer
  uint8_t *raw = UCHARDATA(img);

  // copy data
  index_t i1,i0,idx=0;
  for (i1=0; i1<tensor->size[1]; i1++) {  
    for (i0=0; i0<tensor->size[0]; i0++) {
      raw[idx++] = THTensor_get2d(tensor, i0, i1) * 255;
    }
  }

  // return copy
  return img;
}

static THTensor *xvgraph_to_tensor_2d(struct xvimage *img) {
  // create output
  THTensor *tensor = THTensor_newWithSize2d(img->row_size, img->col_size*2);

  // get pointer
  uint8_t *raw = UCHARDATA(img);

  // copy data
  index_t i1,i0,idx=0;
  for (i1=0; i1<tensor->size[1]; i1++) {  
    for (i0=0; i0<tensor->size[0]; i0++) {
      THTensor_set2d(tensor, i0, i1, (double)raw[idx++] / 255);
    }
  }

  // return copy
  return tensor;
}

// helper
static void inverse(struct xvimage * image) {
  int32_t i, N = rowsize(image) * colsize(image) * depth(image);
  uint8_t *pt;
  for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
    *pt = NDG_MAX - *pt;
}

// Watershed function
static int watershed_l(lua_State *L) {
  // defaults
  int connex = 4;
  bool invert = false;

  // get args
  THTensor *image_th = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *marker_th = (THTensor *)luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 3)) connex = lua_tonumber(L, 3);
  if (lua_isboolean(L, 4)) invert = lua_toboolean(L, 4);

  // convert
  struct xvimage * image = NULL; 
  struct xvimage * marker = NULL;
  if ((image_th->nDimension == 2) && (marker_th->nDimension == 2)) {
    image = tensor_to_xvimage_2d(image_th);
    marker = tensor_to_xvimage_2d(marker_th);
  } else THError("<libpink.watershed> unsupported nb of dimensions (2D tensors required)");

  // compute
  lwshedtopobin(image, marker, connex);
  if (invert) inverse(image);

  // convert back
  THTensor *result_th = xvimage_to_tensor_2d(image);

  // cleanup
  freeimage(image);
  freeimage(marker);

  // return
  luaT_pushudata(L, result_th, luaT_checktypename2id(L, "torch.Tensor"));
  return 1;
}

// Gradient function
static int gradient_l(lua_State *L) {
  // defaults
  int alpha = 1;
  int mode = 0;

  // get args
  THTensor *image_th = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 2)) mode = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) alpha = lua_tonumber(L, 3);

  // convert
  struct xvimage * image = NULL; 
  if ((image_th->nDimension == 2)) image = tensor_to_xvimage_2d(image_th);
  else THError("<libpink.gradient> unsupported nb of dimensions (2D tensors required)");

  // compute
  lderiche3d(image, alpha, mode, 0);

  // convert back
  THTensor *result_th = xvimage_to_tensor_2d(image);

  // cleanup
  freeimage(image);

  // return
  luaT_pushudata(L, result_th, luaT_checktypename2id(L, "torch.Tensor"));
  return 1;
}

// Minima function
static int minima_l(lua_State *L) {
  // get args
  THTensor *image_th = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  const char *connex = lua_tostring(L, 2);

  // convert
  struct xvimage * image = NULL; 
  if ((image_th->nDimension == 2)) image = tensor_to_xvimage_2d(image_th);
  else if ((image_th->nDimension == 3)) image = tensor_to_xvimage_3d(image_th);
  else THError("<libpink.gradient> unsupported nb of dimensions (2D or 3D tensors required)");

  // compute
  printf("using = %s\n", connex);
  lminima(image, (char *)connex);

  // convert back
  THTensor *result_th = NULL;
  if ((image_th->nDimension == 2)) result_th = xvimage_to_tensor_2d(image);
  else result_th = xvimage_to_tensor_3d(image);

  // cleanup
  freeimage(image);

  // return
  luaT_pushudata(L, result_th, luaT_checktypename2id(L, "torch.Tensor"));
  return 1;
}

// image to edge graph converter
//
// mode = 0 difference entre pixel extremite, 
// mode = 1 max entre pixel extremite,
// mode = 2 Deriche default for alpha is 1,
//
static int image2graph_l(lua_State *L) {
  // defaults
  int mode = 0;
  double alpha = 1;

  // get args
  THTensor *image_th = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 2)) mode = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) alpha = lua_tonumber(L, 3);

  // convert
  struct xvimage *image = NULL; 
  if ((image_th->nDimension == 2))
    image = tensor_to_xvimage_2d(image_th);
  else 
    THError("<libpink.saliency> unsupported nb of dimensions (2D tensors required)");

  // alloc resulting GA image
  if (datatype(image) != VFF_TYP_1_BYTE) {
    THError("<libpink.saliency> unsupported type (byte only)");
  }
  struct xvimage *graph = NULL;
  graph = allocGAimage(NULL, image->row_size , image->col_size, depth(image), VFF_TYP_GABYTE);

  // compute graph
  lpgm2ga(image, graph, mode, alpha);
  
  // convert back
  THTensor *graph_th = xvgraph_to_tensor_2d(graph);

  // cleanup
  freeimage(image);
  freeimage(graph);

  // return
  luaT_pushudata(L, graph_th, luaT_checktypename2id(L, "torch.Tensor"));
  return 1;
}

// edge graph to image, using Khalimsky method
//
// bar = false : object
// bar = true : complement (background)
//
static int graph2image_l(lua_State *L) {
  // defaults
  bool bar = false;

  // get args
  THTensor *graph_th = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 2)) bar = lua_toboolean(L, 2);

  // convert
  struct xvimage *graph = NULL; 
  if ((graph_th->nDimension == 2))
    graph = tensor_to_xvgraph_2d(graph_th);
  else 
    THError("<libpink.saliency> unsupported nb of dimensions (2D tensors required)");

  // alloc resulting GA image
  struct xvimage *image = NULL;
  image = allocimage(NULL, rowsize(graph)*2 , colsize(graph)*2, 1, VFF_TYP_1_BYTE);

  // compute image
  lga2khalimsky(graph, image, bar);
  
  // convert back
  THTensor *image_th = xvimage_to_tensor_2d(image);

  // cleanup
  freeimage(image);
  freeimage(graph);

  // return
  luaT_pushudata(L, image_th, luaT_checktypename2id(L, "torch.Tensor"));
  return 1;
}

//
// Saliency function
//
// mode_sal = 0 surface, 1 dynamic, 2 volumeSimple, 3 alphaOmega
//
static int saliency_l(lua_State *L) {
  // defaults
  int mode = 0;

  // get args
  THTensor *graph_th = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 2)) mode = lua_tonumber(L, 2);
  THTensor *image_th = NULL;
  if (lua_isuserdata(L, 3))
    image_th = (THTensor *)luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));

  // convert
  struct xvimage *graph = NULL; 
  struct xvimage *image = NULL; 
  if ((graph_th->nDimension == 2)) {
    graph = tensor_to_xvgraph_2d(graph_th);
    if (image_th != NULL) {
      image = tensor_to_xvimage_2d(image_th);
    }
  }
  else 
    THError("<libpink.saliency> unsupported nb of dimensions (2D tensors required)");

  // compute saliency
  saliencyGa(graph, mode, image);

  // convert back
  THTensor *res_graph_th = xvgraph_to_tensor_2d(graph);

  // cleanup
  freeimage(graph);
  if (image != NULL) freeimage(image);

  // return
  luaT_pushudata(L, res_graph_th, luaT_checktypename2id(L, "torch.Tensor"));
  return 1;
}

//
// helper to generate unique colors
//
static int colors_l(lua_State *L)
{
  long n = luaL_checklong(L, 1);
  THByteTensor *t;
  int *t_p;
  long i;

  luaL_argcheck(L, n > 0, 1, "must be strictly positive");
  luaL_argcheck(L, n <= (1<<24), 1, "must not exceed 2^24");

  t = THByteTensor_newWithSize2d(4,n);
  t_p = (int *)THByteTensor_dataPtr(t);

  for(i = 0; i < n; i++) {
    t_p[i] = (int)(i);
  }

  for(i = 1; i < n-1; i++)
  {
    long z = THRandom_random() % (n-i);
    int sav = t_p[i];
    t_p[i] = t_p[z+i];
    t_p[z+i] = sav;
  }

  luaT_pushudata(L, t, luaT_checktypename2id(L, "torch.ByteTensor"));
  return 1;
}

//
// transform edges map into ID map
//
static int edges2ids_l(lua_State *L) {
  // get args
  THTensor *img_th = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));

  // dims
  long int width = img_th->size[0];
  long int height = img_th->size[1];

  // create result
  THTensor *ids_th = THTensor_newWithSize2d(width, height);

  // make a disjoint-set forest
  universe *u = new universe(width*height);

  // process in one pass
  for (int y = 0; y < height-1; y++) {
    for (int x = 0; x < width-1; x++) {
      int a = u->find(y*width + x);
      int b = u->find(y*width + x+1);
      int c = u->find((y+1)*width + x);
      if ((a != b) && (THTensor_get2d(img_th, x, y) == THTensor_get2d(img_th, x+1, y))) u->join(a,b);
      if ((a != c) && (THTensor_get2d(img_th, x, y) == THTensor_get2d(img_th, x, y+1))) u->join(a,c);
    }
  }

  // generate output
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int comp = u->find(y * width + x);
      if (THTensor_get2d(img_th, x, y) > 0)
        THTensor_set2d(ids_th, x, y, 0);
      else
        THTensor_set2d(ids_th, x, y, comp+1);
    }
  }

  // cleanup
  delete u;

  // return result
  luaT_pushudata(L, ids_th, luaT_checktypename2id(L, "torch.Tensor"));
  return 1;
}

// Register functions in LUA
static const struct luaL_reg pink [] = {
  {"watershed", watershed_l},
  {"gradient", gradient_l},
  {"minima", minima_l},
  {"saliency", saliency_l},
  {"graph2image", graph2image_l},
  {"image2graph", image2graph_l},
  {"colors", colors_l},
  {"edges2ids", edges2ids_l},
  {NULL, NULL}  /* sentinel */
};

extern "C" {
  int luaopen_libpink (lua_State *L) {
    luaL_openlib(L, "libpink", pink, 0);
    return 1; 
  }
}
