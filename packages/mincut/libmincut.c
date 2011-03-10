/*
        This software library implements the maxflow algorithm
        described in

                "An Experimental Comparison of Min-Cut/Max-Flow Algorithms for Energy Minimization in Vision."
                Yuri Boykov and Vladimir Kolmogorov.
                In IEEE Transactions on Pattern Analysis and Machine Intelligence (PAMI), 
                September 2004

        This algorithm was developed by Yuri Boykov and Vladimir Kolmogorov
        at Siemens Corporate Research. To make it available for public use,
        it was later reimplemented by Vladimir Kolmogorov based on open publications.

        If you use this software for research purposes, you should cite
        the aforementioned paper in any resulting publication.

        ----------------------------------------------------------------------

        REUSING TREES:

        Starting with version 3.0, there is a also an option of reusing search
        trees from one maxflow computation to the next, as described in

                "Efficiently Solving Dynamic Markov Random Fields Using Graph Cuts."
                Pushmeet Kohli and Philip H.S. Torr
                International Conference on Computer Vision (ICCV), 2005

        If you use this option, you should cite
        the aforementioned paper in any resulting publication.
*/

#ifdef __cplusplus
 extern "C" {
#include <luaT.h>
#include <TH.h>
 }
#else
#include <luaT.h>
#include <TH.h>
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#define max(a,b) a < b ? b : a
#define min(a,b) a > b ? b : a
#define MIN_THSIZE 400
// segmentation stuff
extern void segment(lua_State *L,
                    THTensor *output,
                    THTensor *depth,  
                    THTensor *input,
                    double thres);

static int l_segment(lua_State *L) {
  double thres = 250;
  // get args
  THTensor *img = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *depth = (THTensor *)luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *res = (THTensor *)luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 4)) thres = lua_tonumber(L, 4);
  if (thres>1) thres = thres/2048;
  segment(L, res, depth, img, thres);
  return 0;
}

// Register functions
static const struct luaL_reg mincut [] = {
  {"segment", l_segment},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libmincut(lua_State * L) {
  luaL_openlib(L, "libmincut", mincut, 0);
  return 1;
}


/*   int i,v,cursor=0,t1=0,m=0,t2=0; */
/*   int flag = 0; */
/*   // search first local maxima */
/*   for (i=0;i<2048;i++) { */
/*     v =  (int)THTensor_get1d(histo, i); */
/*     if (v > MIN_THSIZE && v > t1){ */
/*       t1 = v; */
/*       cursor = i; */
/*       flag = 1; */
/*     } */
/*     if (flag && t1>v) */
/*       break;       */
/*   } */
/*   t1 = cursor; */
/*   flag = 0; */
/*   // move to the next minima */
/*   while(THTensor_get1d(histo, cursor) > THTensor_get1d(histo, cursor+1)) */
/*     cursor++; */
/*   // search second local maxima */
/*   for (i=cursor;i<2048;i++) { */
/*     v =  (int)THTensor_get1d(histo, i); */
/*     if (v > MIN_THSIZE && v > t2){ */
/*       t2 = v; */
/*       cursor=i; */
/*       flag = 1; */
/*     } */
/*     if (flag && t2>v) */
/*       break;       */
/*   } */
/*   t2 = cursor; */
/*   //find min between t1 and t2 */
/*   cursor = t1; */
/*   m = t1; */
/*     for(i=t1;i<=t2;i++){ */
/*     v =  (int)THTensor_get1d(histo, i); */
/*     if (v<m){ */
/*       m = v; */
/*       cursor = i; */
/*     } */
/*   } */
