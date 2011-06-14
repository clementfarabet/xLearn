/***********************************************************
 * what: toolBox and imageBox compiled code.
 * why:  any function that needs to be compiled should be 
 *       here.
 ***********************************************************/
#include "luaT.h"
#include <TH.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>
//#include <ncurses.h>
/* need to check these headers on OSX */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

// Useful macros
#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

int image_saturate(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 1, 
                                    luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *output = input;

  TH_TENSOR_APPLY2(double, output, double, input, \
                     *output_p = (*input_p < 0) ? 0 : (*input_p > 1) ? 1 : *input_p;)
    
  return 1;
}

int image_threshold(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 1, 
                                    luaT_checktypename2id(L, "torch.Tensor"));
  double thres = 0.5;
  double low = 0.0;
  double mid = 0.5;
  double high = 1.0;
  THTensor *output = input;

  if (lua_isnumber(L,2)) thres = lua_tonumber(L, 2);
  if (lua_isnumber(L,3)) low = lua_tonumber(L, 3);
  if (lua_isnumber(L,4)) mid = lua_tonumber(L, 4);
  if (lua_isnumber(L,5)) high = lua_tonumber(L, 5);

  TH_TENSOR_APPLY2(double, output, double, input, \
                     *output_p = (*input_p < -thres) ? low : (*input_p > thres) ? high : mid;)

  return 1;
}

int image_maskToRGB(lua_State *L) {
  THTensor *mask = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *colorMap = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *rgbmap = luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));

  double * data_mask = mask->storage->data+ mask->storageOffset;
  double * data_colorMap = colorMap->storage->data+ colorMap->storageOffset; 
  double * data_rgbmap = rgbmap->storage->data+ rgbmap->storageOffset;

  int c_size_0 = colorMap->size[0];
  int c_size_1 = colorMap->size[1];
  int c_size_2 = colorMap->size[2];

  int m_size_0 = mask->size[0];
  int m_size_1 = mask->size[1];
  int m_size_2 = mask->size[2];

  int r_size_0 = rgbmap->size[0];
  int r_size_1 = rgbmap->size[1];
  int r_size_2 = rgbmap->size[2];

  int k, i,j;
  int pointer = 0;

  int size = mask->size[0]*mask->size[1];
  for(k = 0; k < 3; k++ ){
    for(pointer = 0; pointer < size; pointer++){
      int mask_i = data_mask[pointer];      
      data_rgbmap[k*size + pointer] = data_colorMap[k*c_size_0 + mask_i - 1];
    }
  }
  return 1;  
}

#define rgb2hash(r,g,b) ( (int)(r*256*256 + g*256 + b) )

int image_mergeVectorsIntoSegm(lua_State *L) {
  // tensor id
  const void * tid = luaT_checktypename2id(L, "torch.Tensor");

  // merge vectors into segm
  THTensor *vectors = luaT_checkudata(L, 1, tid);
  THTensor *segm = luaT_checkudata(L, 2, tid);
  int nbClusters = lua_tonumber(L, 4);
  int minConfidence = 0;
  if (lua_isnumber(L,4)) minConfidence = lua_tonumber(L, 4);

  // check
  if ((vectors->nDimension != 3) || (segm->nDimension != 3))
    THError("<image.mergeSegms> expecting 3D tensors");

  // get dims
  int width = vectors->size[0];
  int height = vectors->size[1];
  int nbClasses = vectors->size[2];

  // final cluster list
  lua_newtable(L);  // f = {}
  int table_clean = lua_gettop(L);

  // temporary geometry list
  lua_newtable(L);  // g = {}
  int table_geometry = lua_gettop(L);

  // temporary histogram list
  lua_newtable(L);  // c = {}
  int table_hists = lua_gettop(L);

  // optional confidence map
  THTensor *confidence = THTensor_newWithSize2d(width, height);
  THTensor *helper = THTensor_newWithSize1d(nbClasses);

  // loop over segm, and accumulate histograms of vectors pixels
  int x,y,k;
  THTensor *histo = NULL;
  THTensor *select1 = THTensor_new();
  THTensor *select2 = THTensor_new();
  for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
      // compute hash codes for vectors and segm
      int segm_hash = rgb2hash(THTensor_get3d(segm,x,y,0),
                               THTensor_get3d(segm,x,y,1),
                               THTensor_get3d(segm,x,y,2));
      // is this hash already registered ?
      lua_pushinteger(L,segm_hash);
      lua_rawget(L,table_hists);   // c[segm_hash]
      if (lua_isnil(L,-1)) {    // c[segm_hash] == nil ?
        lua_pop(L,1);
        // then create a vector to accumulate an histogram of classes,
        // for this cluster
        histo = THTensor_newWithSize1d(nbClasses);
        THTensor_zero(histo);
        lua_pushinteger(L,segm_hash);
        luaT_pushudata(L, histo, tid);
        lua_rawset(L,table_hists); // c[segm_hash] = histo
      } else {
        // retrieve histo
        histo = luaT_toudata(L, -1, tid);
        lua_pop(L,1);
      }

      // slice the class vector
      THTensor_select(select1, vectors, 0, x);
      THTensor_select(select2, select1, 0, y);

      // measure confidence
      THTensor_copy(helper, select2);
      double max = -1000;
      double idx = 0;
      for (k=0; k<nbClasses; k++) {
        double val = THTensor_get1d(helper, k);
        if (val > max) {
          max = val; idx = k;
        }
      }
      THTensor_set1d(helper, idx, -1000);
      double max2 = -1000;
      for (k=0; k<nbClasses; k++) {
        double val = THTensor_get1d(helper, k);
        if (val > max2) {
          max2 = val;
        }
      }
      double local_conf = max-max2;
      if (local_conf < 0) THError("assert error : max < 2nd max");

      // accumulate current vector into histo
      if (local_conf >= minConfidence)
        THTensor_addTensor(histo, 1, select2);

      // store confidence
      THTensor_set2d(confidence, x, y, local_conf);
    }
  }

  // then merge vectors into segm, based on the histogram's winners
  THTensor_zero(vectors);
  for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
      // compute hash codes for vectors and segm
      int segm_hash = rgb2hash(THTensor_get3d(segm,x,y,0),
                               THTensor_get3d(segm,x,y,1),
                               THTensor_get3d(segm,x,y,2));
      // get max
      int argmax = 0, max = -1;
      // get geometry entry
      lua_pushinteger(L,segm_hash);
      lua_rawget(L,table_geometry);
      if (lua_isnil(L,-1)) {    // g[segm_hash] == nil ?
        lua_pop(L,1);
        // retrieve histogram
        lua_pushinteger(L,segm_hash);
        lua_rawget(L,table_hists);   // c[segm_hash]  (= histo)
        histo = luaT_toudata(L, -1, tid);
        lua_pop(L,1);
        // compute max
        int i;
        for (i=0; i<nbClasses; i++) {
          if (max <= THTensor_get1d(histo,i)) { argmax = i; max = THTensor_get1d(histo,i); }
        }
        // then create a table to store geometry of component:
        // x,y,size,class,hash
        lua_pushinteger(L,segm_hash);
        lua_newtable(L);
        int entry = lua_gettop(L);
        lua_pushnumber(L, x+1);
        lua_rawseti(L,entry,1); // entry[1] = x
        lua_pushnumber(L, y+1);
        lua_rawseti(L,entry,2); // entry[2] = y
        lua_pushnumber(L, 1);
        lua_rawseti(L,entry,3); // entry[3] = size (=1)
        lua_pushnumber(L, argmax+1);
        lua_rawseti(L,entry,4); // entry[4] = class (=argmax+1)
        lua_pushnumber(L, segm_hash);
        lua_rawseti(L,entry,5); // entry[5] = hash
        // store entry
        lua_rawset(L,table_geometry); // g[segm_hash] = entry
      } else {
        // retrieve entry
        int entry = lua_gettop(L);
        lua_rawgeti(L, entry, 1);
        long cx = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_pushnumber(L, cx+x+1);
        lua_rawseti(L, entry, 1); // entry[1] = cx + x + 1
        lua_rawgeti(L, entry, 2);
        long cy = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_pushnumber(L, cy+y+1);
        lua_rawseti(L, entry, 2); // entry[2] = cy + y + 1
        lua_rawgeti(L, entry, 3);
        long size = lua_tonumber(L, -1) + 1; lua_pop(L, 1);
        lua_pushnumber(L, size);
        lua_rawseti(L, entry, 3); // entry[3] = size + 1
        lua_rawgeti(L, entry, 4);
        argmax = lua_tonumber(L, -1) - 1; lua_pop(L, 1);
        // and clear entry
        lua_pop(L,1);
      }
      // set argmax (winning class) to 1
      THTensor_set3d(vectors, x, y, argmax, 1);
    }
  }

  // traverse geometry table to produce final component list
  lua_pushnil(L);
  int cur = 1;
  while (lua_next(L, table_geometry) != 0) {
    // uses 'key' (at index -2) and 'value' (at index -1)

    // normalize cx and cy, by component's size
    int entry = lua_gettop(L);
    lua_rawgeti(L, entry, 3);
    long size = lua_tonumber(L, -1) + 1; lua_pop(L, 1);
    lua_rawgeti(L, entry, 1);
    long cx = lua_tonumber(L, -1); lua_pop(L, 1);
    lua_pushnumber(L, cx/size);
    lua_rawseti(L, entry, 1); // entry[1] = cx/size
    lua_rawgeti(L, entry, 2);
    long cy = lua_tonumber(L, -1); lua_pop(L, 1);
    lua_pushnumber(L, cy/size);
    lua_rawseti(L, entry, 2); // entry[2] = cy/size

    // store entry table into clean table
    lua_rawseti(L, table_clean, cur++);
  }

  // pop/remove histograms
  lua_pop(L, 1);

  // cleanup
  THTensor_free(select1);
  THTensor_free(select2);
  THTensor_free(helper);

  // return two tables: indexed and hashed, plus the confidence map
  luaT_pushudata(L, confidence, luaT_checktypename2id(L, "torch.Tensor"));
  return 3;
}

int image_lower(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 1, 
                                    luaT_checktypename2id(L, "torch.Tensor"));
  double thres = 0.5;
  THTensor *output = input;

  if (lua_isnumber(L,2)) thres = lua_tonumber(L, 2);

  TH_TENSOR_APPLY2(double, output, double, input, \
                     *output_p = (*input_p-thres < 0) ? 0 : *input_p-thres;)

  return 1;
}

int toolbox_createTable(lua_State *L) {
  int arr_size = 0;
  int rec_size = 0;
  if (lua_isnumber(L,1)) arr_size = lua_tonumber(L,1);
  if (lua_isnumber(L,2)) rec_size = lua_tonumber(L,2);
  lua_createtable(L, arr_size, rec_size);
  return 1;
}

int toolbox_usleep(lua_State *L) {
  int time = 1;
  if (lua_isnumber(L, 1)) time = lua_tonumber(L, 1);
  usleep(time);
  return 1;
}

int toolbox_fillFloatTensor(lua_State *L) {
  // tensor to fill:
  THFloatTensor *tofill = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.FloatTensor"));
  // source storage:
  THByteStorage *toget = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.ByteStorage"));
  // optional pointer:
  int pointer = 0;
  if (lua_isnumber(L, 3)) pointer = lua_tonumber(L, 3) - 1;
  // char pointers:
  unsigned char *src = (unsigned char *)toget->data;
  unsigned char *dest = (unsigned char *)tofill->storage->data;
  long int size = tofill->storage->size * 4;
  // fill:
  long int i;
  memcpy(dest, src+pointer, size);
  return 0;
}

int toolbox_fillByteTensor(lua_State *L) {
  // tensor to fill:
  THByteTensor *tofill = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.ByteTensor"));
  // source storage:
  THByteStorage *toget = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.ByteStorage"));
  // optional pointer:
  int pointer = 0;
  if (lua_isnumber(L, 3)) pointer = lua_tonumber(L, 3) - 1;
  // char pointers:
  unsigned char *src = (unsigned char *)toget->data;
  unsigned char *dest = (unsigned char *)tofill->storage->data;
  long int size = tofill->storage->size;
  // fill:
  long int i;
  memcpy(dest, src+pointer, size);
  return 0;
}

int toolbox_fillTensor(lua_State *L) {
  // tensor to fill:
  THTensor *tofill = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  // source storage:
  THByteStorage *toget = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.ByteStorage"));
  // optional pointer:
  int pointer = 0;
  if (lua_isnumber(L, 3)) pointer = lua_tonumber(L, 3) - 1;
  // char pointers:
  unsigned char *src = (unsigned char *)toget->data;
  unsigned char *dest = (unsigned char *)tofill->storage->data;
  long int size = tofill->storage->size * 8;
  // fill:
  long int i;
  memcpy(dest, src+pointer, size);
  return 0;
}

int toolbox_getMicroTime(lua_State *L) {
  struct timeval tv;
  struct timezone tz;
  struct tm *tm;
  gettimeofday(&tv, &tz);
  tm=localtime(&tv.tv_sec);
  double precise_time = tm->tm_hour*3600 + tm->tm_min*60 + tm->tm_sec + tv.tv_usec / 1000000.0;
/*   printf(" %d:%02d:%02d %d \n", tm->tm_hour, tm->tm_min,  */
/*          tm->tm_sec, tv.tv_usec); */
  lua_pushnumber(L,precise_time);
  return 1;
}

int toolbox_spatialDist(lua_State *L) {
  // args
  THTensor *proto = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *maps = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *dists = luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *temp = luaT_checkudata(L, 4, luaT_checktypename2id(L, "torch.Tensor"));

  // vars
  int k;
  THTensor *map = THTensor_new();

  // zero temp
  THTensor_zero(dists);

  // compute dist
  for (k = 0; k < proto->size[0]; k++) {
    THTensor_select(map, maps, 2, k);

    THTensor_copy(temp, map);
    THTensor_add(temp, -THTensor_get1d(proto,k));
    THTensor_cmul(temp, temp);

    THTensor_addTensor(dists, 1.0, temp);
  }

  // sqrt
  THTensor_sqrt(dists);
  THTensor_div(dists, proto->size[0]);

  return 0;
}

int toolbox_dist2vectors(lua_State *L){
  /* get the arguments */
  THTensor * tensor = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));//luaT_toudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * src = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));//luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  double * data_prot = tensor->storage->data+ tensor->storageOffset;
  double * data_out = src->storage->data+ src->storageOffset;
  int size = tensor->size[0];//*tensor->size[1];
  
  int i;
  
  double sum = 0;
  double diff = 0;
  /* for (i = 0; i < size; i++){ */
/*     printf("data_prot = %f\n", data_prot[i]); */
/*   } */


  for (i = 0; i < size; i++){
    //printf("data_prot = %f\n", data_prot[i]);
    //printf("data_out = %f\n", data_out[i]);
    diff = fabs(data_prot[i] - data_out[i]);
    //printf("diff = %f\n", diff);
    sum += diff*diff;
  }
  //printf("sum = %f\n", sum);
  sum = sqrt(sum);
  sum = sum/size; 

  //printf("sqrt of sum = %f\n", sum);
  lua_pushnumber(L, sum);
  return 1;
}

int toolbox_print_tensor_formatted(lua_State *L){
  /* get the arguments */
  THTensor * to_print = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  const char *filename = luaL_checkstring(L, 2);


  double * data = to_print->storage->data+ to_print->storageOffset;
  
  int size_data_c = to_print->size[0];
  int size_data_r = to_print->size[1];
  int size_data_m = to_print->size[2];
  if (to_print->nDimension < 3)
    size_data_m = 1;

  int m,i,j;

  FILE* fp = fopen(filename, "w");
  printf("file name, for writting: %s\n\n", filename);
  //DEBUG
  //printf("size_data_c = %d, size_data_r = %d, size_data_m = %d\n", size_data_c, size_data_r, size_data_m);
  
  double* ptr_o = data;
  for (m = 0; m < size_data_m; m++){
    for (i = 0; i < size_data_r; i++){
      for (j = 0; j < size_data_c; j++){
	fprintf(fp,"%.14f ", ptr_o[j+ i*to_print->stride[1] + m*to_print->stride[2]]);
	//printf("%.4f ", ptr_o[j*to_print->stride[0]]);
      }
      //ptr_o = ptr_o + to_print->stride[1];
      fprintf(fp,"\n");
      ///printf("\n");
    }
    fprintf(fp,"\n");
    //printf("\n");
  }
  
  return 0;

}



/*
 * This function perfoms convolution between 3D tensors 
 * in a fixed point manner -  to imitate the convolution 
 * on the hardware.
 *  The function receives as an input: output, input, kernel.
 * The convolution is performed between input and kernel and
 * then stored in output.
 */

int toolbox_convFixedPoint(lua_State *L){
  /* get the arguments */
  THTensor * output = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * input = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * kernel = luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));

  double * data_output = output->storage->data+ output->storageOffset;
  double * data_input = input->storage->data+ input->storageOffset;
  double * data_kernel = kernel->storage->data+ kernel->storageOffset;

  int size_output_c = output->size[0];
  int size_output_r = output->size[1];
  int size_output_m = output->size[2];
  if (output->nDimension < 3)
    size_output_m = 1;

  int size_input_c = input->size[0];
  int size_input_r = input->size[1];
  int size_input_m = input->size[2];
  if (input->nDimension < 3)
    size_input_m = 1;

  int size_kernel_c = kernel->size[0];
  int size_kernel_r = kernel->size[1];
  int size_kernel_m = kernel->size[2];
  if (kernel->nDimension < 3)
    size_kernel_m = 1;

  int i,j,k,l,m;
  
  // DEBUG
  /* printf("input dim = %d, output dim = %d, kernel dim = %d\n", input->nDimension, output->nDimension, kernel->nDimension); */
/*   printf("output->stride[0] = %d, output->stride[1] = %d, output->stride[2] = %d\n", output->stride[0], output->stride[1], output->stride[2]); */
/*   printf("size_output_c = %d, size_output_r = %d, maps = %d\n", size_output_c, size_output_r, size_output_m); */
/*   printf("input->stride[0] = %d, input->stride[1] = %d, input->stride[2] = %d\n", input->stride[0], input->stride[1], input->stride[2]); */
/*   printf("size_input_c = %d, size_input_r = %d, maps = %d\n", size_input_c, size_input_r, size_input_m); */
/*   printf("size_kernel_c = %d, size_kernel_r = %d, maps = %d\n", size_kernel_c, size_kernel_r, size_kernel_m); */
  
  double* ptr_i;
  double* ptr_k;
  double* ptr_o;
  // DEBUG
  /* printf("data_input:\n"); */
/*   ptr_i = data_input; */
/*   for (m = 0; m < size_input_m; m++){ */
/*     for (i = 0; i < size_input_r; i++){ */
/*       for (j = 0; j < size_input_c; j++){ */
/* 	printf("%.4f ", ptr_i[j*input->stride[0]]); */
/*       } */
/*       ptr_i = ptr_i + input->stride[1]; */
/*       printf("\n"); */
/*     } */
/*     printf("\n"); */
/*   } */

/*   printf("data_kernel:\n"); */
/*   ptr_k = data_kernel; */
/*   for (m = 0; m < size_kernel_m; m++){ */
/*     for (i = 0; i < size_kernel_r; i++){ */
/*       for (j = 0; j < size_kernel_c; j++){ */
/* 	printf("%.4f ", ptr_k[j*kernel->stride[0]]); */
/*       } */
/*       ptr_k = ptr_k + kernel->stride[1]; */
/*       printf("\n"); */
/*     } */
/*     printf("------ %d ------\n", m); */
/*   } */
  
/*   printf("data_output:\n"); */
/*   ptr_o = data_output; */
/*   for (m = 0; m < size_output_m; m++){ */
/*     for (i = 0; i < size_output_r; i++){ */
/*       for (j = 0; j < size_output_c; j++){ */
/* 	printf("%.4f ", ptr_o[j*output->stride[0]]); */
/*       } */
/*       ptr_o = ptr_o + output->stride[1]; */
/*       printf("\n"); */
/*     } */
/*     printf("\n"); */
/*   } */
  
  int* input_int = (int*)malloc(sizeof(int)*size_input_c*size_input_r*size_input_m);
  int* output_int = (int*)malloc(sizeof(int)*size_output_c*size_output_r*size_output_m);
  int* kernel_int = (int*)malloc(sizeof(int)*size_kernel_c*size_kernel_r*size_kernel_m);


  // convert to fixed point (mult 256 and store in int)
  ptr_i = data_input;
  for(m = 0; m < size_input_m; m++){
    for (i = 0; i < size_input_r; i++){
      for (j = 0; j < size_input_c; j++){
	*(input_int + i + j*size_input_r + m*input->stride[2])=  round(ptr_i[j*input->stride[0]] * 256);
      }
      ptr_i += input->stride[1];
    }
  }
  
  ptr_k = data_kernel;
  for(m = 0; m < size_kernel_m; m++){
    for (i = 0; i < size_kernel_r; i++){
      for (j = 0; j < size_kernel_c; j++){
	*(kernel_int + i + j*size_kernel_r) = round(ptr_k[j*kernel->stride[0]] * 256);
	//printf("%d ", *(kernel_int + i + j*size_kernel_r));
      }
      ptr_k += kernel->stride[1];
      //printf("\n");
    }
  }
  
  // inialize the output
  for (m = 0; m < size_output_m; m++){
    for (i = 0; i < size_output_r; i++){
      for (j = 0; j < size_output_c; j++){
	*(output_int + i + j*size_output_r + m*output->stride[2]) = 0;
      }
    }
  }
  
  
  // convolution
  // using the formula:
  //output[i][j][k] = sum_l sum_{s=1}^kW sum_{t=1}^kH weight[s][t][l][k] * input[dW*(i-1)+s)][dH*(j-1)+t][l]
  for (m = 0; m < size_output_m; m++){
    for (i = 0; i < size_output_r; i++){
      for (j = 0; j < size_output_c; j++){
	int sum = 0;
	for (k = 0; k < size_kernel_r; k++){
	  for (l = 0; l < size_kernel_c; l++){
	    int in_i = i + k;
	    int in_j = j + l;
	    //DEBUG
	    //printf("i = %d, j = %d, k = %d, l = %d, in_i = %d, in_j = %d\n", i, j, k, l, in_i, in_j);
	    //int res = *(kernel_int + l + k*size_kernel_c) *
	    //  *(input_int + in_i + in_j*size_input_c);
	    
	    int res = *(kernel_int + k + l*size_kernel_r) *
	      *(input_int + in_i + in_j*size_input_r + m*input->stride[2]);
	    
	    sum += res;
	    
	  }
	}
	
	double sum_d = sum / 256.0;
	sum = round(sum_d);
	
	*(output_int + i + j*size_output_r + m*output->stride[2]) += sum;
      }
    }
  }
  
  

  // copy the result to output and scale it back to double
  ptr_o = data_output;
  for (m = 0; m < size_output_m; m++){
    for (i = 0; i < size_output_r; i++){
      for (j = 0; j < size_output_c; j++){
	ptr_o[j*output->stride[0]] = *(output_int + i + j*size_output_r + m*output->stride[2]);
	ptr_o[j*output->stride[0]] /= 256.0;
      }
      ptr_o += output->stride[1];
    }
  }


  //DEBUG
  /* printf("data_output:\n"); */
/*   ptr_o = data_output; */
/*   for (m = 0; m < size_output_m; m++){ */
/*     for (i = 0; i < size_output_r; i++){ */
/*       for (j = 0; j < size_output_c; j++){ */
/* 	printf("%.4f ", ptr_o[j*output->stride[0]]); */
/*       } */
/*       ptr_o = ptr_o + output->stride[1]; */
/*       printf("\n"); */
/*     } */
/*     printf("\n"); */
/*   } */
  


  
  // free the allocated memory
  free(input_int);
  free(output_int);
  free(kernel_int);
  
  return 0;
}


/* 
 * This function perfoms a linear mapping (ax+b)
 * in a fixed point maaner - to imitate map on hardware.
 *  The function reseives as an input: ( output, input, linear_segments).
 * Linear segments reseived here in form of a tensor num_of_segs x 3.
 * seg[0] - min value, seg[1] - a, seg[2] - b.
 * The input and segments is considered to be in fixed point: 
 * (intup:mul(256); input:add(0.5); input:floor()).
 * ax+b in fixed point calculations is performed on each value of input
 * and the result is stored in output.
 */
int toolbox_mapFixedPoint(lua_State *L){
  /* get the arguments */
  THTensor * output = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * input = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * seg = luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));

  double * data_output = output->storage->data+ output->storageOffset;
  double * data_input = input->storage->data+ input->storageOffset;
  double * data_seg = seg->storage->data+ seg->storageOffset;

  int size_output_c = output->size[0];
  int size_output_r = output->size[1];
  int size_output_m = output->size[2];
  if (output->nDimension < 3)
    size_output_m = 1;

  int size_input_c = input->size[0];
  int size_input_r = input->size[1];
  int size_input_m = input->size[2];
  if (input->nDimension < 3)
    size_input_m = 1;

  int size_seg_c = seg->size[0];
  int size_seg_r = seg->size[1];
  int i,j,k,l,m;
  
  // DEBUG
  /* printf("input dim = %d, output dim = %d, seg dim = %d\n", input->nDimension, output->nDimension, seg->nDimension); */
/*   printf("input->stride[0] = %d, input->stride[1] = %d, input->stride[2] = %d\n", input->stride[0], input->stride[1], input->stride[2]); */
/*   printf("size_output_c = %d, size_output_r = %d, maps = %d\n", size_output_c, size_output_r, size_output_m); */
/*   printf("size_input_c = %d, size_input_r = %d, maps = %d\n", size_input_c, size_input_r, size_input_m); */
/*   printf("size_seg_c = %d, size_seg_r = %d\n", size_seg_c, size_seg_r); */
 

  double* ptr_i;
  double* ptr_s;
  double* ptr_o;

  //for linear approx we do:
  //       linear = a*t + b
  // now we want to use fixed point linear approx:
  //
  // a_fixed = round(a*256)
  // t_fixed = round(t*256)
  // at_fixed = floor(a_fixed*t_fixed/256)
  // b_fixed = round(b*256)
  // linear_fixed = at_fixed + b_fixed
  // 
  // and go back:
  //
  // linear = linear_fixed/256
  //
  // note: we use floor for at_fixed because hardware 
  // does floor instead of round (which should be fixed really)


  
  ptr_o = data_output;
  ptr_i = data_input;
  for (m = 0; m < size_output_m; m++){
    for (i = 0; i < size_output_r; i++){
      for (j = 0; j < size_output_c; j++){
	
	int found_idx = 0;
	ptr_s = data_seg;
	for (k = 0; k < size_seg_r; k++){
	  if (ptr_i[j*input->stride[0]] >= ptr_s[0]){
	    found_idx = k;
	    k = size_seg_r;// break
	  }
	  ptr_s += seg->stride[1];
	}
	
	ptr_s = data_seg + found_idx*seg->stride[1];
	
	// DEBUG 
	//printf("input == %f, found_idx = %d, min = %f, a = %f, b = %f\n",
	//	     ptr_i[j*input->stride[0]], found_idx, ptr_s[0], ptr_s[1], ptr_s[2] );
	
	ptr_o[j*output->stride[0]] = floor(ptr_i[j*input->stride[0]] * 
					   ptr_s[1]/256.0) + ptr_s[2];
	ptr_o[j*output->stride[0]] /= 256.0;
      }
      ptr_o += output->stride[1];
      ptr_i += input->stride[1];
    }
  }

  return 0;
}








void get_k_max_heap(double* array, int n, int k, int* answer){
  int i,j,f;

  int heap_full = 0;
  answer[0] = 0;
  int curr_full = 1;
  
  
  for(i = 1; i < n; i++){
    if (heap_full == 0){
      //DEBUG
      //printf("heap before, i = %d, array[i] = %f:\n", i, array[i]);
      //for(f = 0; f< curr_full; f++){
      //	printf("%d, max = %12.4f\n", answer[f], array[answer[f]]);
      //}

      answer[curr_full] = i;
      curr_full++;
      if(curr_full >= k)
	heap_full = 1;
      for(j = curr_full-1; j > 0; j=(j-1)/2){
	int head_idx = (j-1)/2;
    
	if(array[answer[j]] < array[answer[head_idx]]){
	  int temp = answer[head_idx];
	  answer[head_idx] = answer[j];
	  answer[j] = temp;
	}
	
      }
      //DEBUG
      //printf("heap after, i = %d, array[i] = %f:\n", i, array[i]);
      //for(f = 0; f< curr_full; f++){
      //	printf("%d, max = %12.4f\n", answer[f], array[answer[f]]);
      //}
    }
    else{
      //DEBUG
      //printf("heap full\n");
      
      //printf("heap before, i = %d, array[i] = %f:\n", i, array[i]);
      //for(f = 0; f< curr_full; f++){
      //	printf("%d, max = %12.4f\n", answer[f], array[answer[f]]);
      //}

      if (array[i] > array[answer[0]]){
	answer[0] = i;
	
	for(j = 0;  j < k; ){
	  // left son
	  int left_idx = j*2 + 1;
	  // right son
	  int right_idx = j*2 + 2;

	  if(left_idx < k && right_idx < k){
	    //DEBUG
	    //printf("2 sons\n");
	    int max_son = 0;
	    if(array[answer[left_idx]] < array[answer[right_idx]])
	      max_son = left_idx;
	    else max_son = right_idx;
	    
	    if(array[answer[j]] > array[answer[max_son]]){
	      int temp1 = answer[j];
	      answer[j] = answer[max_son];
	      answer[max_son] = temp1;
	      //DEBUG
	      //printf("2 sons swap between: %d and %d\n", j, max_son);
	      j = max_son;
	    }
	    else {
	      //DEBUG
	      //printf("out of 2 sons with j = %d\n", j);
	      goto out;
	    }
	    
	  }
	  else if(left_idx < k){
	    //DEBUG
	    //printf("only left son\n");
	    if(array[answer[j]] > array[answer[left_idx]]){
	      int temp1 = answer[j];
	      answer[j] = answer[left_idx];
	      answer[left_idx] = temp1;
	      //DEBUG
	      //printf("left son swap between: %d and %d\n", j, left_idx);
	      j = left_idx;
	    }
	    else{
	      //DEBUG
	      //printf("out of left son with j = %d\n", j);
	      goto out;
	    }
	  
	  }
	  else if(right_idx < k){
	    printf("never get here\n");
	  }
	  else{
	    //DEBUG
	    //printf("out of no sons with j = %d\n", j);
	    goto out; }
	}
      
      }
    out: 
      ;
      //DEBUG
      //printf("out\n");
	
      //printf("heap after, i = %d, array[i] = %f:\n", i, array[i]);
      //for(f = 0; f< curr_full; f++){
      //	printf("%d, max = %12.4f\n", answer[f], array[answer[f]]);
      //}
    
    }
  }
}

void sort_heap(double* array, int n, int k, int* answer, int* sorted){
  int i,j,f;
  int curr_full = k;
  
  for(i = 0; i < k; i++){
    //DEBUG
    //printf("heap before, i = %d, answer[i] = %d:\n", i, answer[i]);
    //for(f = 0; f< curr_full; f++){
    //  printf("%d, max = %12.4f\n", answer[f], array[answer[f]]);
    //}

    sorted[(k-1) - i] = answer[0];
    answer[0] = answer[curr_full-1];

    curr_full--;
    
    //DEBUG
    //printf("heap before, i = %d, answer[i] = %d:\n", i, answer[i]);
    //for(f = 0; f< curr_full; f++){
    //  printf("%d, max = %12.4f\n", answer[f], array[answer[f]]);
    //}
    
    for(j = 0;  j < curr_full; ){
      // left son
      int left_idx = j*2 + 1;
      // right son
      int right_idx = j*2 + 2;
      
      if(left_idx < curr_full && right_idx < curr_full){
	//DEBUG
	//printf("2 sons\n");
	int max_son = 0;
	if(array[answer[left_idx]] < array[answer[right_idx]])
	  max_son = left_idx;
	else max_son = right_idx;
	
	if(array[answer[j]] > array[answer[max_son]]){
	  int temp1 = answer[j];
	  answer[j] = answer[max_son];
	  answer[max_son] = temp1;
	  //DEBUG
	  //printf("2 sons swap between: %d and %d\n", j, max_son);
	  j = max_son;
	}
	else {
	  //DEBUG
	  //printf("out of 2 sons with j = %d\n", j);
	  goto out;
	}
	  
      }
      else if(left_idx < curr_full){
	//DEBUG
	//printf("only left son\n");
	if(array[answer[j]] > array[answer[left_idx]]){
	  int temp1 = answer[j];
	  answer[j] = answer[left_idx];
	  answer[left_idx] = temp1;
	  //DEBUG
	  //printf("left son swap between: %d and %d\n", j, left_idx);
	  j = left_idx;
	}
	else{
	  //DEBUG
	  //printf("out of left son with j = %d\n", j);
	  goto out;
	}
	  
      }
      else if(right_idx < curr_full){
	printf("never get here\n");
      }
      else{
	//DEBUG
	//printf("out of no sons with j = %d\n", j);
	goto out; }
    }
      
  out:
    ;
    //DEBUG
    //printf("out\n");
    
    //printf("heap after, i = %d, answer[i] = %d:\n", i, answer[i]);
    //for(f = 0; f< curr_full; f++){
    //  printf("%d, max = %12.4f\n", answer[f], array[answer[f]]);
    //}
  }
  
}


/*
 * This function receives a tensor, 'pointSize' - of the point area, and 
 * 'k' - number of local maxima needed to find, and 'answer' - array
 * with answer coordinates
 * 
 * The function saves to array 'answer' coordinates of the left top corner
 * squares of size 'pointSize'x'pointSize' sorted in the decending order.
 * These squares are the local maxima areas in the input tensor.
 */

int toolbox_get_k_local_maxima(lua_State *L){
  /* get the arguments */
  THTensor * tensor = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * answer = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  int pointSize =  lua_tonumber(L, 3);
  int k = lua_tonumber(L, 4);

  double * data_tensor = tensor->storage->data+ tensor->storageOffset;
  double * data_answer = answer->storage->data+ answer->storageOffset;
  
  int tensor_width = tensor->size[0];
  int tensor_height = tensor->size[1];

  int area_width = floor(tensor_width/pointSize);
  int area_height = floor(tensor_height/pointSize);
    
  //allocate array to store max areas of input tensor
  double* area_max = (double*)malloc(sizeof(double)*area_height*area_width);

  int i, j, l, m, area_idx = 0, c_border = 0, h_border;
  double* ptr_tens ;
  
  // DEBUG
  //printf("tens. height = %d, tens. width = %d, tens->stride[0] = %d, tens->stride[1] = %d\n", tensor_height, tensor_width, tensor->stride[0], tensor->stride[1]);
  //printf("\nInput tensor:\n");
  //ptr_tens = data_tensor;
  //for(i = 0; i < tensor_height; i++){
  //  for(j = 0; j < tensor_width; j++){
  //    printf("%f ", ptr_tens[j*tensor->stride[0]]);
  //  }
  //  ptr_tens += tensor->stride[1];
  //  printf("\n");
  //} 




  ptr_tens = data_tensor;
  for(i = 0; i < tensor_height;){
    for(j = 0; j < tensor_width;){


      c_border = min(pointSize, tensor_width - j);
      h_border = min(pointSize, tensor_height - i);

      if (c_border == pointSize && h_border == pointSize){
	double max_value = ptr_tens[j*tensor->stride[0]];
	
	// find max
	for(l = 0; l < c_border; l++){
	  for(m = 0; m < h_border; m++){
	    
	    //DEBUG
	    //printf("i = %d, j = %d, l = %d, m = %d, c_border = %d, h_border = %d, area_idx = %d, tensor_idx = %d, value = %f\n", i,j,l,m,c_border, h_border, area_idx, j*tensor->stride[0]+ l + m*tensor->stride[1], ptr_tens[j*tensor->stride[0]+ l + m*tensor->stride[1]]);
	    
	    if(max_value < ptr_tens[j*tensor->stride[0]+ l + m*tensor->stride[1]])
	      max_value = ptr_tens[j*tensor->stride[0]+ l + m*tensor->stride[1]];
	  }
	}
	//DEBUG
	//printf("max value = %f\n", max_value);
	// save value to array of max areas
	area_max[area_idx] = max_value;
	// update pointers
	area_idx++;
      }
      j += pointSize;//h_border;
    }
    i += pointSize;//c_border;
    ptr_tens += tensor->stride[1]*pointSize;//*c_border;
  }
  
  // DEBUG
  //printf("\nArray of max areas:\n");
  //for(i = 0; i < area_width; i++){
  //  for(j = 0; j < area_height; j++){
  //    printf("%f ", area_max[i*area_width+j]);
  //  }
  //  printf("\n");
  //} 


  // allocate array to store heap
  int* heap_of_k_max = (int*)malloc(sizeof(int)*k);
  
  get_k_max_heap(area_max, area_height*area_width, k, heap_of_k_max);

  // DEBUG
  //printf("\nHeap of k max:\n");
  //for(i = 0; i < k; i++){
  //  printf("max index = %d, max value = %f \n", heap_of_k_max[i], area_max[heap_of_k_max[i]]);
  //} 
  // allocate array to store sorted heap
  int* sorted_heap = (int*)malloc(sizeof(int)*k);

  sort_heap(area_max, area_height*area_width, k, heap_of_k_max, sorted_heap);
  
  // DEBUG
  //printf("\nSorted heap:\n");
  //for(i = 0; i < k; i++){
  //  printf("max index = %d, max value = %f \n", sorted_heap[i], area_max[sorted_heap[i]]);
  //}  
  
  // now get the (x,y) position in the original array
  for(i = 0; i < k; i++){
    // convert from 1D coordinate to 2D coordnates
    double temp_x = sorted_heap[i]%area_width;
    double temp_y = floor(sorted_heap[i]/area_height);
    
    // now convert from max area array coordinates to input tensor coordinates
    double x = temp_x*pointSize;
    double y = temp_y*pointSize;
    // add 1 because in Lua index strats from 1
    data_answer[i*answer->stride[1] + 0] = x + 1; 
    data_answer[i*answer->stride[1] + 1] = y + 1;
  }

  // free the allocated memory
  free(area_max);
  free(heap_of_k_max);
  free(sorted_heap);

  return 0;
}





/*
 * Converts an RGB color value to HSL. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes r, g, and b are contained in the set [0, 1] and
 * returns h, s, and l in the set [0, 1].
 */
int image_rgb2hsl(lua_State *L) {
  THTensor *rgb = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *hsl = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));

  int i1,i0;
  double r,g,b,h,s,l;
  for (i1=0; i1<rgb->size[1]; i1++) {
    for (i0=0; i0<rgb->size[0]; i0++) {
      // get Rgb
      r = THTensor_get3d(rgb, i0, i1, 0);
      g = THTensor_get3d(rgb, i0, i1, 1);
      b = THTensor_get3d(rgb, i0, i1, 2);

      double mx = max(max(r, g), b);
      double mn = min(min(r, g), b);
      h = (mx + mn) / 2;
      s = h;
      l = h;

      if(mx == mn) {
        h = 0; // achromatic
        s = 0;
      } else {
        double d = mx - mn;
        s = l > 0.5 ? d / (2 - mx - mn) : d / (mx + mn);
        if (mx == r) {
          h = (g - b) / d + (g < b ? 6 : 0);
        } else if (mx == g) {
          h = (b - r) / d + 2; 
        } else {
          h = (r - g) / d + 4;
        }
        h /= 6;
      }

      // set hsl
      THTensor_set3d(hsl, i0, i1, 0, h);
      THTensor_set3d(hsl, i0, i1, 1, s);
      THTensor_set3d(hsl, i0, i1, 2, l);
    }
  }
  return 0;
}

// helper
static inline double hue2rgb(double p, double q, double t) {
  if (t < 0.) t += 1;
  if (t > 1.) t -= 1;
  if (t < 1./6) 
    return p + (q - p) * 6. * t;
  else if (t < 1./2) 
    return q;
  else if (t < 2./3) 
    return p + (q - p) * (2./3 - t) * 6.;
  else
    return p;                                       
}

/*
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 1].
 */
int image_hsl2rgb(lua_State *L) {
  THTensor *hsl = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *rgb = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));

  int i1,i0;
  double r,g,b,h,s,l;
  for (i1=0; i1<hsl->size[1]; i1++) {
    for (i0=0; i0<hsl->size[0]; i0++) {
      // get hsl
      h = THTensor_get3d(hsl, i0, i1, 0);
      s = THTensor_get3d(hsl, i0, i1, 1);
      l = THTensor_get3d(hsl, i0, i1, 2);

      if(s == 0) {
        // achromatic
        r = l;
        g = l;
        b = l;
      } else {
        double q = (l < 0.5) ? (l * (1 + s)) : (l + s - l * s);
        double p = 2 * l - q;
        double hr = h + 1./3;
        double hg = h;
        double hb = h - 1./3;
        r = hue2rgb(p, q, hr);
        g = hue2rgb(p, q, hg);
        b = hue2rgb(p, q, hb);
      }

      // set rgb
      THTensor_set3d(rgb, i0, i1, 0, r);
      THTensor_set3d(rgb, i0, i1, 1, g);
      THTensor_set3d(rgb, i0, i1, 2, b);
    }
  }
  return 0;
}

/*
 * Converts an RGB color value to HSV. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes r, g, and b are contained in the set [0, 1] and
 * returns h, s, and v in the set [0, 1].
 */
int image_rgb2hsv(lua_State *L) {
  THTensor *rgb = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *hsv = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));

  int i1,i0;
  double r,g,b,h,s,v;
  for (i1=0; i1<rgb->size[1]; i1++) {
    for (i0=0; i0<rgb->size[0]; i0++) {
      // get Rgb
      r = THTensor_get3d(rgb, i0, i1, 0);
      g = THTensor_get3d(rgb, i0, i1, 1);
      b = THTensor_get3d(rgb, i0, i1, 2);

      double mx = max(max(r, g), b);
      double mn = min(min(r, g), b);
      h = mx;
      v = mx;

      double d = mx - mn;
      s = (mx==0) ? 0 : d/mx;

      if(mx == mn) {
        h = 0; // achromatic
      } else {
        if (mx == r) {
          h = (g - b) / d + (g < b ? 6 : 0);
        } else if (mx == g) {
          h = (b - r) / d + 2; 
        } else {
          h = (r - g) / d + 4;
        }
        h /= 6;
      }

      // set hsv
      THTensor_set3d(hsv, i0, i1, 0, h);
      THTensor_set3d(hsv, i0, i1, 1, s);
      THTensor_set3d(hsv, i0, i1, 2, v);
    }
  }
  return 0;
}

/*
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 1].
 */
int image_hsv2rgb(lua_State *L) {
  THTensor *hsv = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *rgb = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));

  int i1,i0;
  double r,g,b,h,s,v;
  for (i1=0; i1<hsv->size[1]; i1++) {
    for (i0=0; i0<hsv->size[0]; i0++) {
      // get hsv
      h = THTensor_get3d(hsv, i0, i1, 0);
      s = THTensor_get3d(hsv, i0, i1, 1);
      v = THTensor_get3d(hsv, i0, i1, 2);

      int i = floor(h*6.);
      double f = h*6-i;
      double p = v*(1-s);
      double q = v*(1-f*s);
      double t = v*(1-(1-f)*s);

      switch (i % 6) {
      case 0: r = v, g = t, b = p; break;
      case 1: r = q, g = v, b = p; break;
      case 2: r = p, g = v, b = t; break;
      case 3: r = p, g = q, b = v; break;
      case 4: r = t, g = p, b = v; break;
      case 5: r = v, g = p, b = q; break;
      default: r=0; g = 0, b = 0; break;
      }

      // set rgb
      THTensor_set3d(rgb, i0, i1, 0, r);
      THTensor_set3d(rgb, i0, i1, 1, g);
      THTensor_set3d(rgb, i0, i1, 2, b);
    }
  }
  return 0;
}


/* calls POSIX fstat from c lib rather than executing */
int toolbox_fstat_time(lua_State *L) {
  const char * fname = luaL_checkstring(L, 1);
  int file=0;
  if((file=open(fname,O_RDONLY)) < -1)
    return 0;
  struct stat fileStat;
  if(fstat(file,&fileStat) < 0)    
    return 0;
  lua_pushnumber(L, (double) fileStat.st_atime);
  lua_pushnumber(L, (double) fileStat.st_ctime);
  lua_pushnumber(L, (double) fileStat.st_mtime);
  return 3;
}

/*
int toolbox_ncurseStart(lua_State *L) {
  initscr();
  noecho();
  raw();
  keypad(stdscr, true);
  if(has_colors() == FALSE){	
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }
  return 0;
}

int toolbox_ncurseEnd(lua_State *L) {
  endwin();
  return 0;
}

int toolbox_ncursePrint(lua_State *L) {
  int x = 0, y = 0;
  const char * toprint = luaL_checkstring(L, 1);
  if (lua_isnumber(L, 2)) y = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) x = lua_tonumber(L, 3);
  mvprintw(y,x,toprint);
  return 0;
}

int toolbox_ncurseRefresh(lua_State *L) {
  refresh();
  return 0;
}

int toolbox_ncurseGetDims(lua_State *L) {
  int width, height;
  getmaxyx(stdscr,height,width);
  lua_pushnumber(L,height);
  lua_pushnumber(L,width);
  return 2;
}

int toolbox_ncurseGetChar(lua_State *L) {
  char a = getch();
  lua_pushlstring(L,&a,1);
  return 1;
}
*/
