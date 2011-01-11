/*Copyright ESIEE (2009) 

Author :
Camille Couprie (c.couprie@esiee.fr)

Contributors : 
Hugues Talbot (h.talbot@esiee.fr)
Leo Grady (leo.grady@siemens.com)
Laurent Najman (l.najman@esiee.fr)

This software contains some image processing algorithms whose purpose is to be
used primarily for research.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
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
*/

// C. Farabet: changed the original main() for a Lua wrapper funcion

// To load this lib in LUA:
// require 'libpowerwatersegm'

#include <luaT.h>
#include <TH.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <cccodimage.h>
#include <lMSF.h>
#include <MSF_RW.h>
#include <powerwatersegm.h>
#include <image_toolbox.h>
#include <unistd.h>


/* =============================================================== */
void tensor_to_image(THTensor *tensor, struct xvimage **r, struct xvimage **g, struct xvimage **b)
/* =============================================================== */
{
  // idx
  int i1,i0;

  if (tensor->size[2] == 1) {
    // 1 channel
    *r = allocimage(NULL, tensor->size[1], tensor->size[0], 1, VFF_TYP_1_BYTE);    

    // img p
    uint8_t *rp = UCHARDATA(*r);

    // copy data
    for (i1=0; i1<tensor->size[1]; i1++) {
      for (i0=0; i0<tensor->size[0]; i0++) {
        *rp = (uint8_t)(THTensor_get3d(tensor, i0, i1, 0) * 255);
        rp++;
      }
    }
  } else { 
    // should be 3 channels
    *r = allocimage(NULL, tensor->size[1], tensor->size[0], 1, VFF_TYP_1_BYTE);
    *g = allocimage(NULL, tensor->size[1], tensor->size[0], 1, VFF_TYP_1_BYTE);
    *b = allocimage(NULL, tensor->size[1], tensor->size[0], 1, VFF_TYP_1_BYTE);

    // img p
    uint8_t *rp = UCHARDATA(*r);
    uint8_t *gp = UCHARDATA(*r);
    uint8_t *bp = UCHARDATA(*r);

    // copy data
    for (i1=0; i1<tensor->size[1]; i1++) {
      for (i0=0; i0<tensor->size[0]; i0++) {
        *rp = (uint8_t)(THTensor_get3d(tensor, i0, i1, 0) * 255);
        *gp = (uint8_t)(THTensor_get3d(tensor, i0, i1, 0) * 255);
        *bp = (uint8_t)(THTensor_get3d(tensor, i0, i1, 0) * 255);
        rp++; gp++; bp++;
      }
    }
  }
}

/* =============================================================== */
void image_to_tensor(struct xvimage *img, THTensor **tensor)
/* =============================================================== */
{
  // create output
  *tensor = THTensor_newWithSize3d(colsize(img), rowsize(img), 1);

  // get pointer to img data
  uint8_t *ip = UCHARDATA(img);

  // copy data
  int i1,i0;
  for (i1=0; i1<(*tensor)->size[1]; i1++) {
    for (i0=0; i0<(*tensor)->size[0]; i0++) {
      THTensor_set3d(*tensor, i0, i1, 0, ((double)(*ip)) / 255.0);
      ip++;
    }
  }
}

/* =============================================================== */
int power_lua(lua_State *L)
/* =============================================================== */
{
  // default args
  int algo = 1; //  Kruskal(1) PW q=2(2) Prim(3)
  bool geod = false;

  // get args
  THTensor *ten_input = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *ten_seeds = (THTensor *)luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 3)) algo = lua_tonumber(L, 3);
  if (lua_isboolean(L, 4)) geod = lua_toboolean(L, 4);

  bool quicksort = false;
  int32_t nblabels, i,j;
  struct xvimage * image_r = NULL;
  struct xvimage * image_v = NULL;
  struct xvimage * image_b = NULL;
  struct xvimage * output = NULL;

  //unsigned char * s;
  int rs, cs, ds, N, M;  

  bool color=false;
  ds=1; //default
  if (ten_input->size[2] == 1) {
    tensor_to_image(ten_input, &image_r, NULL, NULL);
  } else {
    color = true;
    tensor_to_image(ten_input, &image_r, &image_v, &image_b);
  } 

  rs = rowsize(image_r);
  cs = colsize(image_r);

  N = rs * cs * ds;
  M = ds*rs*(cs-1)+ds*(rs-1)*cs+(ds-1)*cs*rs;  /*number of edges*/
  int ** edges;   
  
  int * index_seeds = (int*)malloc(N*sizeof(int));
  uint8_t * index_labels = (uint8_t*)malloc(N*sizeof(uint8_t));

  //multilabel seed image
  nblabels = 0;
  int x,y;
  for (j=0; j<ten_seeds->size[0]; j++) {
    x = (int)THTensor_get2d(ten_seeds, j, 0);
    y = (int)THTensor_get2d(ten_seeds, j, 1);
    index_labels[j] = (int)THTensor_get2d(ten_seeds, j, 2);
    index_seeds[j] = x + y*cs;
    if (index_labels[j] > nblabels) nblabels = index_labels[j];
  }
  int size_seeds = ten_seeds->size[0];

  edges =  (int**)malloc(2*sizeof(int*));
  for(i=0;i<2;i++) edges[i] = (int*)malloc(M*sizeof(int));

  compute_edges(edges,rs, cs, ds);

   if (algo == 1) // Kruskal
    {
      printf("# exec Kruskal with %0d seeds\n", size_seeds);
      uint32_t * weights = (uint32_t *)malloc(sizeof(uint32_t)*M);
      int max_weight = 255;
      if (color == true) 
	max_weight = color_standard_weights( image_r, image_v, image_b, weights, edges, index_seeds, size_seeds, geod, quicksort);
      else 
	grey_weights(image_r, weights, edges,index_seeds, size_seeds, geod, quicksort);

      output = MSF_Kruskal(edges,weights, max_weight, index_seeds, index_labels, size_seeds, rs, cs, ds, nblabels);
      free(weights);
    }
 
  else if (algo == 3) // Prim RB tree
    {  
      printf("# exec Prim RB tree with %0d seeds\n", size_seeds);
      uint32_t * weights = (uint32_t *)malloc(sizeof(uint32_t)*M);
      if (color == true) 
        color_standard_weights( image_r, image_v, image_b, weights, edges, index_seeds, size_seeds, geod, quicksort);
      else 
	grey_weights(image_r, weights, edges,index_seeds, size_seeds, geod, quicksort);
      output = MSF_Prim(edges,weights, index_seeds, index_labels, size_seeds,rs, cs, ds, nblabels);
      free(weights);
    }

 else if (algo == 2) // Kruskal & RW on plateaus multiseeds linear time
    {
      printf("# exec Kruskal (& RW on plateaus) with %0d seeds\n", size_seeds);
      struct xvimage * img_proba;     
      uint32_t * weights = (uint32_t *)malloc(sizeof(uint32_t)*M);
      uint32_t * normal_weights ;
      uint32_t max_weight = 255;
      if (color == true) 
	normal_weights = color_standard_weights_PW( image_r, image_v, image_b, weights, edges, index_seeds, size_seeds, &max_weight, quicksort);
      else normal_weights = grey_weights_PW(image_r, edges,index_seeds, size_seeds, weights, quicksort);
      if (geod ==true)
	output = PowerWatershed_q2(edges, weights, weights, max_weight,index_seeds, index_labels, size_seeds,rs, cs, ds, nblabels, quicksort, img_proba);
      else
	output = PowerWatershed_q2(edges, weights, normal_weights,max_weight,index_seeds, index_labels, size_seeds,rs, cs, ds, nblabels, quicksort, img_proba);
      free(weights);
      free(normal_weights);
    }

  assert(output != NULL);
 
  // return result
  THTensor *ten_mask = NULL;
  image_to_tensor(output, &ten_mask);
  luaT_pushudata(L, ten_mask, luaT_checktypename2id(L, "torch.Tensor"));

  // cleanup
  freeimage(output);
  free(index_seeds);
  free(index_labels);
  if (color) {
    freeimage(image_v);
    freeimage(image_b);
  }
  freeimage(image_r);

  return 1;
} 

// Register functions in LUA
static const struct luaL_reg power [] = {
  {"infer", power_lua},
  {NULL, NULL}  /* sentinel */
};

extern "C" {
  int luaopen_libpowerwatersegm (lua_State *L) {
    luaL_openlib(L, "libpowerwatersegm", power, 0);
    return 1; 
  }
}

