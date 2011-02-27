/*
  + This is a wrapper for vlfeat - a vision library
  -
*/

// To load this lib in LUA:
// require 'libvlfeat'

#include <luaT.h>
#include <TH.h>

#include "generic.h"
#include "mathop.h"
#include "sift.h"
#include "dsift.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/** ------------------------------------------------------------------
 ** Transpose desriptor
 **
 ** @param dst destination buffer.
 ** @param src source buffer.
 **
 ** The function writes to @a dst the transpose of the SIFT descriptor
 ** @a src. The tranpsose is defined as the descriptor that one
 ** obtains from computing the normal descriptor on the transposed
 ** image.
 **/

VL_INLINE void
transpose_descriptor (vl_sift_pix* dst, vl_sift_pix* src)
{
  int const BO = 8 ;  /* number of orientation bins */
  int const BP = 4 ;  /* number of spatial bins     */
  int i, j, t ;

  for (j = 0 ; j < BP ; ++j) {
    int jp = BP - 1 - j ;
    for (i = 0 ; i < BP ; ++i) {
      int o  = BO * i + BP*BO * j  ;
      int op = BO * i + BP*BO * jp ;
      dst [op] = src[o] ;
      for (t = 1 ; t < BO ; ++t)
        dst [BO - t + op] = src [t + o] ;
    }
  }
}

/** ------------------------------------------------------------------
 ** Ordering of tuples by increasing scale
 **
 ** @param a tuple.
 ** @param b tuble.
 **
 ** @return @c a[2] < b[2]
 **/

static int
korder (void const* a, void const* b) {
  double x = ((double*) a) [2] - ((double*) b) [2] ;
  if (x < 0) return -1 ;
  if (x > 0) return +1 ;
  return 0 ;
}

/** ------------------------------------------------------------------
 ** Check for sorted keypoints
 **
 ** @param keys keypoint list to check
 ** @param nkeys size of the list.
 **
 ** @return 1 if the keypoints are storted.
 **/

vl_bool
check_sorted (double const * keys, vl_size nkeys)
{
  vl_uindex k ;
  for (k = 0 ; k + 1 < nkeys ; ++ k) {
    if (korder(keys, keys + 4) > 0) {
      return VL_FALSE ;
    }
    keys += 4 ;
  }
  return VL_TRUE ;
}

/** ------------------------------------------------------------------
 ** converter function
 **/

static vl_sift_pix *tensor2d_to_vlsiftpix(THTensor *tensor) {
  vl_sift_pix *data = (vl_sift_pix *)malloc(tensor->size[0]*tensor->size[1]*sizeof(vl_sift_pix));
  long int idx = 0;
  for (int y=0; y<tensor->size[1]; y++)
    for (int x=0; x<tensor->size[0]; x++)
      data[idx++] = (vl_sift_pix)THTensor_get2d(tensor, x, y) * 255;
  return data;
}

/** ------------------------------------------------------------------
 ** Lua wrapper: sift
 **/

static int sift_l(lua_State *L) {

  /* -----------------------------------------------------------------
   * check arguments
   */

  // defaults:
  int                verbose = 0 ;

  vl_sift_pix       *data ;
  int                M, N ;

  int                O     = - 1 ;
  int                S     =   3 ;
  int                o_min =   0 ;

  double             edge_thresh = -1 ;
  double             peak_thresh = -1 ;
  double             norm_thresh = -1 ;
  double             magnif      = -1 ;
  double             window_size = -1 ;

  double            *ikeys = 0 ;
  int                nikeys = -1 ;
  vl_bool            force_orientations = 0 ;
  vl_bool            computeDescriptors = 0 ;

  // from the stack
  THTensor *img_th = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));

  // check
  if (img_th->nDimension != 2) {
    THError("image must be a WxH tensor");
  }

  // convert
  data = (vl_sift_pix*) tensor2d_to_vlsiftpix(img_th);
  M    = img_th->size[0];
  N    = img_th->size[1];

  // get options
  if (lua_isboolean(L, 2)) verbose = lua_toboolean(L, 2) ? 1 : 0;       // verbose
  if (lua_isnumber(L, 3)) O = lua_tonumber(L, 3);                       // octaves
  if (lua_isnumber(L, 4)) S = lua_tonumber(L, 4);                       // levels
  if (lua_isnumber(L, 5)) o_min = lua_tonumber(L, 5);                   // firstOctave
  if (lua_isnumber(L, 6)) edge_thresh = lua_tonumber(L, 6);             // edgeThresh
  if (lua_isnumber(L, 7)) peak_thresh = lua_tonumber(L, 7);             // peakThresh
  if (lua_isnumber(L, 8)) norm_thresh = lua_tonumber(L, 8);             // normThresh
  if (lua_isnumber(L, 9)) magnif = lua_tonumber(L, 9);                  // magnif
  if (lua_isnumber(L, 10)) window_size = lua_tonumber(L, 10);           // windowSize
  if (lua_isboolean(L, 11)) force_orientations = lua_toboolean(L, 11) ? 1 : 0;  // orientations
  if (lua_isboolean(L, 12)) computeDescriptors = lua_toboolean(L, 12) ? 1 : 0;  // computeDescriptors
  if (lua_isuserdata(L, 13)) {                                                 // frames
    THTensor *ikeys_th = (THTensor *)luaT_checkudata(L, 13, luaT_checktypename2id(L, "torch.Tensor"));
    if (ikeys_th->nDimension != 2 || ikeys_th->size[0] != 4) {
      THError("frames must be a 4xN tensor");
    }
    nikeys = ikeys_th->size[1];
    ikeys = ikeys_th->storage->data;
    if (! check_sorted (ikeys, nikeys)) {
      qsort (ikeys, nikeys, 4 * sizeof(double), korder) ;
    }
  }

  /* -----------------------------------------------------------------
   * do job
   */
  VlSiftFilt        *filt ;
  vl_bool            first ;
  double            *frames = NULL;
  double            *descr  = NULL;
  int                nframes = 0, reserved = 0, i,j,q ;

  /* create a filter to process the image */
  filt = vl_sift_new (M, N, O, S, o_min) ;

  if (peak_thresh >= 0) vl_sift_set_peak_thresh (filt, peak_thresh) ;
  if (edge_thresh >= 0) vl_sift_set_edge_thresh (filt, edge_thresh) ;
  if (norm_thresh >= 0) vl_sift_set_norm_thresh (filt, norm_thresh) ;
  if (magnif      >= 0) vl_sift_set_magnif      (filt, magnif) ;
  if (window_size >= 0) vl_sift_set_window_size (filt, window_size) ;

  if (verbose) {
    printf("vl_sift: filter settings:\n") ;
    printf("vl_sift:   octaves      (O)      = %d\n",
           vl_sift_get_noctaves      (filt)) ;
    printf("vl_sift:   levels       (S)      = %d\n",
           vl_sift_get_nlevels       (filt)) ;
    printf("vl_sift:   first octave (o_min)  = %d\n",
           vl_sift_get_octave_first  (filt)) ;
    printf("vl_sift:   edge thresh           = %g\n",
           vl_sift_get_edge_thresh   (filt)) ;
    printf("vl_sift:   peak thresh           = %g\n",
           vl_sift_get_peak_thresh   (filt)) ;
    printf("vl_sift:   norm thresh           = %g\n",
           vl_sift_get_norm_thresh   (filt)) ;
    printf("vl_sift:   window size           = %g\n",
           vl_sift_get_window_size   (filt)) ;

    printf((nikeys >= 0) ?
           "vl_sift: will source frames? yes (%d read)\n" :
           "vl_sift: will source frames? no\n", nikeys) ;
    printf("vl_sift: will force orientations? %s\n",
           force_orientations ? "yes" : "no") ;
  }

  /* -----------------------------------------------------------------
   * process each octave
   */
  i     = 0 ;
  first = 1 ;
  while (1) {
    int                   err ;
    VlSiftKeypoint const *keys  = 0 ;
    int                   nkeys = 0 ;

    if (verbose) {
      printf ("vl_sift: processing octave %d\n",
              vl_sift_get_octave_index (filt)) ;
    }

    /* Calculate the GSS for the next octave .................... */
    if (first) {
      err   = vl_sift_process_first_octave (filt, data) ;
      first = 0 ;
    } else {
      err   = vl_sift_process_next_octave  (filt) ;
    }

    if (err) break ;

    if (verbose > 1) {
      printf("vl_sift: GSS octave %d computed\n",
             vl_sift_get_octave_index (filt));
    }

    /* Run detector ............................................. */
    if (nikeys < 0) {
      vl_sift_detect (filt) ;

      keys  = vl_sift_get_keypoints  (filt) ;
      nkeys = vl_sift_get_nkeypoints (filt) ;
      i     = 0 ;

      if (verbose > 1) {
        printf ("vl_sift: detected %d (unoriented) keypoints\n", nkeys) ;
      }
    } else {
      nkeys = nikeys ;
    }

    /* For each keypoint ........................................ */
    for (; i < nkeys ; ++i) {
      double                angles [4] ;
      int                   nangles ;
      VlSiftKeypoint        ik ;
      VlSiftKeypoint const *k ;

      /* Obtain keypoint orientations ........................... */
      if (nikeys >= 0) {
        vl_sift_keypoint_init (filt, &ik,
                               ikeys [4 * i + 1] - 1,
                               ikeys [4 * i + 0] - 1,
                               ikeys [4 * i + 2]) ;

        if (ik.o != vl_sift_get_octave_index (filt)) {
          break ;
        }

        k = &ik ;

        /* optionally compute orientations too */
        if (force_orientations) {
          nangles = vl_sift_calc_keypoint_orientations
            (filt, angles, k) ;
        } else {
          angles [0] = VL_PI / 2 - ikeys [4 * i + 3] ;
          nangles    = 1 ;
        }
      } else {
        k = keys + i ;
        nangles = vl_sift_calc_keypoint_orientations
          (filt, angles, k) ;
      }

      /* For each orientation ................................... */
      for (q = 0 ; q < nangles ; ++q) {
        vl_sift_pix  buf [128] ;
        vl_sift_pix rbuf [128] ;

        /* compute descriptor (if necessary) */
        if (computeDescriptors) {
          vl_sift_calc_keypoint_descriptor (filt, buf, k, angles [q]) ;
          transpose_descriptor (rbuf, buf) ;
        }

        /* make enough room for all these keypoints and more */
        if (reserved < nframes + 1) {
          reserved += 2 * nkeys ;
          frames = (double *)realloc(frames, 4 * sizeof(double) * reserved);
          if (computeDescriptors) {
            descr  = (double *)realloc(descr,  128 * sizeof(double) * reserved);
          }
        }

        /* Save back with MATLAB conventions. Notice tha the input
         * image was the transpose of the actual image. */
        frames [4 * nframes + 0] = k -> x + 1 ;
        frames [4 * nframes + 1] = k -> y + 1 ;
        frames [4 * nframes + 2] = k -> sigma ;
        frames [4 * nframes + 3] = VL_PI / 2 - angles [q] ;

        if (computeDescriptors) {
          for (j = 0 ; j < 128 ; ++j) {
            double x = 512.0F * rbuf [j] ;
            descr[128 * nframes + j] = x ;
          }
        }

        ++ nframes ;
      } /* next orientation */
    } /* next keypoint */
  } /* next octave */

  if (verbose) {
    printf ("vl_sift: found %d keypoints\n", nframes) ;
  }

  /* -----------------------------------------------------------------
   * save results
   */
  if (nframes > 0)
  {
    /* create an array, and set content to frames buffer */
    THTensor *frames_th = THTensor_newWithSize2d(4, nframes);
    int idx = 0;
    for (int y=0; y<frames_th->size[1]; y++) {
      for (int x=0; x<frames_th->size[0]; x++) {
        THTensor_set2d(frames_th, x, y, (double)frames[idx++]);
      }
    }

    /* return frames_th */
    luaT_pushudata(L, frames_th, luaT_checktypename2id(L, "torch.Tensor"));

    if (computeDescriptors) {
      /* create an array, and set content to frames buffer */
      THTensor *descr_th = THTensor_newWithSize2d(128, nframes);
      int idx = 0;
      for (int y=0; y<descr_th->size[1]; y++) {
        for (int x=0; x<descr_th->size[0]; x++) {
          THTensor_set2d(descr_th, x, y, (double)descr[idx++]);
        }
      }

      /* return descr_th */
      luaT_pushudata(L, descr_th, luaT_checktypename2id(L, "torch.Tensor"));
    }
  }
  else
  {
    lua_pushboolean(L, 0);
    if (computeDescriptors) lua_pushboolean(L, 0);
  }

  /* cleanup */
  vl_sift_delete (filt);
  free(data);
  free(frames);
  free(descr);

  /* done */
  return 1 + computeDescriptors;
}

/** ------------------------------------------------------------------
 ** Lua wrapper: siftmatch
 **/

// base structure
typedef struct {
  int k1;
  int k2;
  double score;
} Pair;

// matching function
static Pair* compare_double (Pair* pairs_iterator,
                             const double * L1_pt, const double * L2_pt,
                             int K1, int K2, int ND, float thresh) {
  int k1, k2;
  const double maxval = INFINITY;
  for(k1 = 0 ; k1 < K1 ; ++k1, L1_pt += ND ) {

    double best = maxval ;
    double second_best = maxval ;
    int bestk = -1 ;

    /* For each point P2[k2] in the second image... */
    for(k2 =  0 ; k2 < K2 ; ++k2, L2_pt += ND) {

      int bin ;
      double acc = 0 ;
      for(bin = 0 ; bin < ND ; ++bin) {
        double delta =
          ((double) L1_pt[bin]) -
          ((double) L2_pt[bin]) ;
        acc += delta*delta ;
      }

      /* Filter the best and second best matching point. */
      if(acc < best) {
        second_best = best ;
        best = acc ;
        bestk = k2 ;
      } else if(acc < second_best) {
        second_best = acc ;
      }
    }

    L2_pt -= ND*K2 ;

    /* Lowe's method: accept the match only if unique. */
    if(thresh * (float) best <= (float) second_best &&
       bestk != -1) {
      pairs_iterator->k1 = k1 ;
      pairs_iterator->k2 = bestk ;
      pairs_iterator->score = best ;
      pairs_iterator++ ;
    }
  }

  return pairs_iterator ;
}

static int siftmatch_l(lua_State *L) {

  // defaults
  double threshold = 1.5;

  // from the stack
  THTensor *descs1_th = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *descs2_th = (THTensor *)luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  if (lua_isnumber(L, 3)) threshold = lua_tonumber(L, 3);

  // get dimensions
  long int K1 = descs1_th->size[1];
  long int K2 = descs2_th->size[1];
  long int ND = descs1_th->size[0];

  // raw pointers to data
  double *L1_pt = descs1_th->storage->data;
  double *L2_pt = descs2_th->storage->data;

  // compute matches
  Pair* pairs_begin = (Pair*) malloc(sizeof(Pair) * (K1+K2));
  Pair* pairs_iterator = pairs_begin;
  pairs_iterator = compare_double(pairs_iterator, L1_pt, L2_pt,
                                  K1, K2, ND, threshold);
  Pair* pairs_end = pairs_iterator;

  // create outputs (results)
  THTensor *matches_th = THTensor_newWithSize2d(2, pairs_end-pairs_begin);
  double *M_pt = matches_th->storage->data;

  THTensor *dists_th = THTensor_newWithSize1d(pairs_end-pairs_begin);
  double *D_pt = dists_th->storage->data;

  for (pairs_iterator=pairs_begin; pairs_iterator<pairs_end; ++pairs_iterator) {
    *M_pt++ = pairs_iterator->k1 + 1;
    *M_pt++ = pairs_iterator->k2 + 1;
    *D_pt++ = pairs_iterator->score;
  }

  // return results
  luaT_pushudata(L, matches_th, luaT_checktypename2id(L, "torch.Tensor"));
  luaT_pushudata(L, dists_th, luaT_checktypename2id(L, "torch.Tensor"));

  // cleanup
  free(pairs_begin) ;

  // done
  return 2;
}

/** ------------------------------------------------------------------
 ** @brief Lua wrapper: dsift
 **/

static int dsift_l(lua_State *L) {
  return 0;
}

// Register functions in LUA
static const struct luaL_reg vlfeat [] = {
  {"sift", sift_l},
  {"siftmatch", siftmatch_l},
  {"dsift", dsift_l},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libvlfeat (lua_State *L) {
  luaL_openlib(L, "libvlfeat", vlfeat, 0);
  return 1;
}
