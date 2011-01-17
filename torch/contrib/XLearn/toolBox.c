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
  THTensor *mask = luaT_checkudata(L, 1, 
				   luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *colorMap = luaT_checkudata(L, 2, 
				       luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *rgbmap = luaT_checkudata(L, 3, 
				     luaT_checktypename2id(L, "torch.Tensor"));

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

  //DEBUG
  // printf("colorMap sizes: %d, %d, %d\n", c_size_0, c_size_1, c_size_2);
  //printf("mask sizes: %d, %d, %d\n", m_size_0, m_size_1, m_size_2);
  //printf("rgbmap sizes: %d, %d, %d\n", r_size_0, r_size_1, r_size_2);

  int k, i,j;
  int pointer = 0;
  /* for (i = 0; i < mask->size[0]; i++){ */
/*     for(j = 0; j < mask->size[1]; j ++){ */
/*       int mask_i = data_mask[pointer]; */
/*       printf("i = %d, j = %d, mask[i,j] = %d\n", i, j, mask_i); */
/*       pointer++; */
      
/*     } */
/*   } */
  int size = mask->size[0]*mask->size[1];
  for(k = 0; k < 3; k++ ){
    for(pointer = 0; pointer < size; pointer++){
      int mask_i = data_mask[pointer];
      //DEBUG
      //printf("k = %d, pointer = %d,mask_i = %d, colorMap[mask_i] = %f\n",
      // 	     k, pointer, mask_i, data_colorMap[k*c_size_0 + mask_i - 1]);
      
      data_rgbmap[k*size + pointer] = data_colorMap[k*c_size_0 + mask_i - 1];
    }
  }
  return 1;  
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
