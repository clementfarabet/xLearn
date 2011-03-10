/*
Copyright ESIEE (2009) 

m.couprie@esiee.fr

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
*/
/* 
  operations arithmetiques : 
    ladd
    laddconst
    larea
    laverage
    linverse
    lequal
    lexp
    ldiff
    ldivide
    linf
    llog
    lmax
    lmax1
    lmin
    lmin1
    lmult
    lneg
    lnormalize
    lnull
    lpow
    lscale
    lsub
    lsup
    lvolume
    lxor
*/
/* Michel Couprie - juillet 1996 */
/* Camille Couprie - octobre 2002 (xor) */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <mcutil.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <larith.h>

/* ==================================== */
int32_t ladd(
  struct xvimage * image1,
  struct xvimage * image2)
/* somme de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "ladd"
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2; 
  float *FPT1, *FPT2; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = (uint8_t)mcmin(NDG_MAX,((int32_t)*pt1+(int32_t)*pt2));
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    for (PT1 = SLONGDATA(image1), PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = *PT1 + *PT2;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    for (FPT1 = FLOATDATA(image1), FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = *FPT1 + *FPT2;
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* ladd() */

/* ==================================== */
int32_t laddconst(struct xvimage * image1, int32_t constante)
/* ajoute une constante a une image  - seuil si depassement */
/* ==================================== */
#undef F_NAME
#define F_NAME "laddconst"
{
  int32_t i;
  uint8_t *pt1;
  int32_t *lpt1;
  float *FPT1; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  
  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    pt1 = UCHARDATA(image1);
    for (i = 0; i < N; i++)
    {
      pt1[i] = (uint8_t)mcmin(NDG_MAX, mcmax(NDG_MIN, (int32_t)(pt1[i]) + constante));
    }
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    lpt1 = SLONGDATA(image1);
    for (i = 0; i < N; i++)
    {
      lpt1[i] = (int32_t)mcmin(INT32_MAX,mcmax(INT32_MIN,(int32_t)(lpt1[i])+constante));
    }
  }
  else if (datatype(image1) == VFF_TYP_FLOAT)
  {
    FPT1 = FLOATDATA(image1);
    for (i = 0; i < N; i++)
    {
      FPT1[i] = FPT1[i] + (float)constante;
    }
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* laddconst() */

/* ==================================== */
int32_t larea(
  struct xvimage * image, 
  int32_t *area)
/* retourne le nombre de pixels non nuls */
/* ==================================== */
#undef F_NAME
#define F_NAME "larea"
{
  int32_t i, a = 0;
  uint8_t *pt;
  int32_t *PT; 
  float *FPT; 
  int32_t rs, cs, d, N;

  rs = rowsize(image);
  cs = colsize(image);
  d = depth(image);
  N = rs * cs * d;

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++) if (*pt) a++;
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    for (PT = SLONGDATA(image), i = 0; i < N; i++, PT++) if (*PT) a++;
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++) if (*FPT != 0.0) a++;
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  *area = a;
  return 1;
} /* larea() */

/* ==================================== */
int32_t laverage(
  struct xvimage * image1,
  struct xvimage * image2,
  double alpha)
/* moyenne ponderee de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "laverage"
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2; 
  float *FPT1, *FPT2; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = (uint8_t)((alpha * *pt1) + ((1.0 - alpha) * *pt2));
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    for (PT1 = SLONGDATA(image1), PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = (int32_t)((alpha * *PT1) + ((1.0 - alpha) * *PT2));
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    for (FPT1 = FLOATDATA(image1), FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = (float)((alpha * *FPT1) + ((1.0 - alpha) * *FPT2));
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* laverage() */

/* ==================================== */
int32_t ldiff(
  struct xvimage * image1,
  struct xvimage * image2)
/* difference en valeur absolue de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldiff"
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2; 
  float *FPT1, *FPT2; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = (uint8_t)mcabs((int32_t)*pt1-(int32_t)*pt2);
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    for (PT1 = SLONGDATA(image1), PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = mcabs(*PT1-*PT2);
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    for (FPT1 = FLOATDATA(image1), FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = (float)fabs((float)(*FPT1)-(float)(*FPT2));
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* ldiff() */

/* ==================================== */
int32_t ldivide(
  struct xvimage * image1,
  struct xvimage * image2)
/* quotient (pixel par pixel) de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldivide"
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2; 
  float *FPT1, *FPT2; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      if (*pt2 != 0) *pt1 = *pt1 / *pt2; else *pt1 = 0;
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    for (PT1 = SLONGDATA(image1), PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
      if (*PT2 != 0) *PT1 = *PT1 / *PT2; else *PT1 = 0;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    for (FPT1 = FLOATDATA(image1), FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
      if (*FPT2 != 0.0) *FPT1 = *FPT1 / *FPT2; else *FPT1 = 0.0;
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* ldivide() */

/* ==================================== */
int32_t lequal(
  struct xvimage * image1,
  struct xvimage * image2)
/* test d'egalite de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lequal"
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2; 
  float *FPT1, *FPT2; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 != *pt2) return 0;
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    for (PT1 = SLONGDATA(image1), PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
      if (*PT1 != *PT2) return 0;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    for (FPT1 = FLOATDATA(image1), FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
      if (*FPT1 != *FPT2) return 0;
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lequal() */

/* ==================================== */
int32_t linf(
  struct xvimage * image1,
  struct xvimage * image2)
/* prédicat inf pixelwise */
/* ==================================== */
#undef F_NAME
#define F_NAME "linf"
{
  int32_t i;
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    uint8_t *pt1, *pt2;
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 <= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    int32_t *pt1, *pt2; 
    for (pt1 = SLONGDATA(image1), pt2 = SLONGDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 <= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    float *pt1, *pt2; 
    for (pt1 = FLOATDATA(image1), pt2 = FLOATDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 <= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* linf() */

/* ==================================== */
int32_t linverse(
  struct xvimage * image)
/* inverse d' une image */
/* ==================================== */
#undef F_NAME
#define F_NAME "linverse"
{
  int32_t i, N;

  N = rowsize(image) * colsize(image) * depth(image);

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    uint8_t *pt;
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
      *pt = NDG_MAX - *pt;
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    int32_t *pt, vmax;
    vmax = 0;
    for (pt = SLONGDATA(image), i = 0; i < N; i++, pt++)
      if (*pt > vmax) vmax = *pt;
    for (pt = SLONGDATA(image), i = 0; i < N; i++, pt++)
      *pt = vmax - *pt;
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    float *pt, vmax;
    vmax = 0;
    for (pt = FLOATDATA(image), i = 0; i < N; i++, pt++)
      if (*pt > vmax) vmax = *pt;
    for (pt = FLOATDATA(image), i = 0; i < N; i++, pt++)
      *pt = vmax - *pt;
  }
  else
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return 0;
  }

  return 1;
} /* linverse() */

/* ==================================== */
int32_t lmax(
  struct xvimage * image1,
  struct xvimage * image2)
/* max de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmax"
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2; 
  float *FPT1, *FPT2; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = mcmax(*pt1, *pt2);
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    for (PT1 = SLONGDATA(image1), PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = mcmax(*PT1, *PT2);
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    for (FPT1 = FLOATDATA(image1), FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = mcmax(*FPT1, *FPT2);
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lmax() */

/* ==================================== */
double lmax1(struct xvimage * image1)
/* maximum value in an image */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmax1"
{
  int32_t i, rs, cs, d, N;
  double maxval;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;

  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(image1);
    maxval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] > maxval) maxval = (double)F[i];
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    int32_t *F = SLONGDATA(image1);
    maxval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] > maxval) maxval = (double)F[i];
  }
  else if (datatype(image1) == VFF_TYP_FLOAT)
  {
    float *F = FLOATDATA(image1);
    maxval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] > maxval) maxval = (double)F[i];
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    exit(0);
  }

  return maxval;
} /* lmax1() */

/* ==================================== */
double lmin1(struct xvimage * image1)
/* minimum value in an image */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmin1"
{
  int32_t i, rs, cs, d, N;
  double minval;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;

  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) minval = (double)F[i];
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    int32_t *F = SLONGDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) minval = (double)F[i];
  }
  else if (datatype(image1) == VFF_TYP_FLOAT)
  {
    float *F = FLOATDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) minval = (double)F[i];
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    exit(0);
  }

  return minval;
} /* lmin1() */

/* ==================================== */
int32_t lmin(
  struct xvimage * image1,
  struct xvimage * image2)
/* min de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmin"
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2; 
  float *FPT1, *FPT2; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = mcmin(*pt1, *pt2);
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    for (PT1 = SLONGDATA(image1), PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = mcmin(*PT1, *PT2);
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    for (FPT1 = FLOATDATA(image1), FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = mcmin(*FPT1, *FPT2);
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lmin() */

/* ==================================== */
int32_t lmult(
  struct xvimage * image1,
  struct xvimage * image2)
/* produit de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmult"
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2; 
  float *FPT1, *FPT2; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = (uint8_t)mcmin(NDG_MAX, (int32_t)*pt1 * (int32_t)*pt2);
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    for (PT1 = SLONGDATA(image1), PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = *PT1 * *PT2;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    for (FPT1 = FLOATDATA(image1), FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = *FPT1 * *FPT2;
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lmult() */

/* ==================================== */
int32_t lneg(
  struct xvimage * image)
/* negation d' une image de booleens */
/* ==================================== */
#undef F_NAME
#define F_NAME "lneg"
{
  int32_t i;
  uint8_t *pt;
  int32_t N;

  N = rowsize(image) * colsize(image) * depth(image);

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
      if (*pt) *pt = 0; else *pt = NDG_MAX;
  }
  else
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return 0;
  }

  return 1;
} /* lneg() */

/* ==================================== */
int32_t lnormalize(struct xvimage * image, float nmin, float nmax)
/* ==================================== */
#undef F_NAME
#define F_NAME "lnormalize"
#define EPSILON 1e-6
{
  int32_t x, N;

  if (nmin > nmax)
  {
    fprintf(stderr, "%s: bad output range\n", F_NAME);
    return 0;
  }

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    uint8_t *Im;
    uint8_t ndgmin, ndgmax;
    uint8_t Nmin = arrondi(nmin);
    uint8_t Nmax = arrondi(nmax);
    N = rowsize(image) * colsize(image) * depth(image);
    Im = UCHARDATA(image);
    ndgmin = ndgmax = Im[0];
    for (x = 0; x < N; x++)
    {
      if (Im[x] < ndgmin) ndgmin = Im[x];
      else if (Im[x] > ndgmax) ndgmax = Im[x];
    }
    ndgmax = ndgmax - ndgmin;
    if (ndgmax == 0) ndgmax = 1;
    for (x = 0; x < N; x++)
      Im[x] = Nmin + ((Im[x] - ndgmin) * (Nmax-Nmin)) / ndgmax;
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    int32_t *Im;
    int32_t ndgmin, ndgmax;
    int32_t Nmin = arrondi(nmin);
    int32_t Nmax = arrondi(nmax);
    N = rowsize(image) * colsize(image) * depth(image);
    Im = SLONGDATA(image);
    ndgmin = ndgmax = Im[0];
    for (x = 0; x < N; x++)
    {
      if (Im[x] < ndgmin) ndgmin = Im[x];
      else if (Im[x] > ndgmax) ndgmax = Im[x];
    }
    ndgmax = ndgmax - ndgmin;
    if (ndgmax == 0) ndgmax = 1;
    for (x = 0; x < N; x++)
      Im[x] = Nmin + ((Im[x] - ndgmin) * (Nmax-Nmin)) / ndgmax;
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    float *Im;
    float ndgmin, ndgmax;
    N = rowsize(image) * colsize(image) * depth(image);
    Im = FLOATDATA(image);
    ndgmin = ndgmax = Im[0];
    for (x = 0; x < N; x++)
    {
      if (Im[x] < ndgmin) ndgmin = Im[x];
      else if (Im[x] > ndgmax) ndgmax = Im[x];
    }
    ndgmax = ndgmax - ndgmin;
    if (ndgmax < EPSILON) ndgmax = 1.0;
    for (x = 0; x < N; x++)
      Im[x] = nmin + ((Im[x] - ndgmin) * (nmax-nmin)) / ndgmax;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} // lnormalize()

/* ==================================== */
int32_t lnull(struct xvimage * image1)
/* test de nullite d'une image */
/* ==================================== */
#undef F_NAME
#define F_NAME "lnull"
{
  int32_t i;
  uint8_t *pt1;
  int32_t *PT1; 
  float *FPT1; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;

  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    for (pt1 = UCHARDATA(image1), i = 0; i < N; i++, pt1++)
      if (*pt1) return 0;
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    for (PT1 = SLONGDATA(image1), i = 0; i < N; i++, PT1++)
      if (*PT1) return 0;
  }
  else if (datatype(image1) == VFF_TYP_FLOAT)
  {
    for (FPT1 = FLOATDATA(image1), i = 0; i < N; i++, FPT1++)
      if (*FPT1 != 0.0) return 0;
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lnull() */

/* ==================================== */
int32_t lscale(
  struct xvimage * image,
  double scale)
/* produit d' une image par un scalaire - seuil a NDG_MAX pour les uint8_t */
/* ==================================== */
#undef F_NAME
#define F_NAME "lscale"
{
  int32_t i;
  uint8_t *pt;
  int32_t *PT;
  float *FPT;
  int32_t N;

  N = rowsize(image) * colsize(image) * depth(image);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
    {
      *pt = (uint8_t)mcmin(NDG_MAX, (int32_t)(*pt * scale));
    }
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    for (PT = SLONGDATA(image), i = 0; i < N; i++, PT++)
    {
      *PT = (int32_t)(*PT * scale);
    }
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++)
    {
      *FPT = (float)(*FPT * scale);
    }
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* lscale() */

/* ==================================== */
int32_t lpow(
  struct xvimage * image,
  double p)
/* elevation à la puissance p - seuil a NDG_MAX pour les uint8_t */
/* ==================================== */
#undef F_NAME
#define F_NAME "lpow"
{
  int32_t i;
  uint8_t *pt;
  int32_t *PT;
  float *FPT;
  int32_t N;

  N = rowsize(image) * colsize(image) * depth(image);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
    {
      *pt = (uint8_t)mcmin(NDG_MAX,(pow((double)(*pt),p)));
    }
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    for (PT = SLONGDATA(image), i = 0; i < N; i++, PT++)
    {
      *PT = (int32_t)pow((double)(*PT),p);
    }
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++)
    {
      *FPT = (float)pow((double)(*FPT),p);
    }
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* lpow() */

/* ==================================== */
int32_t lexp(struct xvimage * image)
/* exponentiation */
/* ==================================== */
#undef F_NAME
#define F_NAME "lexp"
{
  int32_t i;
  float *FPT;
  int32_t N;

  N = rowsize(image) * colsize(image) * depth(image);
  if (datatype(image) == VFF_TYP_FLOAT)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++)
    {
      *FPT = (float)exp((double)(*FPT));
    }
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* lexp() */

/* ==================================== */
int32_t llog(struct xvimage * image)
/* logarithme */
/* ==================================== */
#undef F_NAME
#define F_NAME "llog"
{
  int32_t i;
  float *FPT;
  int32_t N;

  N = rowsize(image) * colsize(image) * depth(image);
  if (datatype(image) == VFF_TYP_FLOAT)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++)
    {
      *FPT = (float)log((double)(*FPT));
    }
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* llog() */

/* ==================================== */
int32_t lsub(
  struct xvimage * image1,
  struct xvimage * image2)
/* difference de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lsub"
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2; 
  float *FPT1, *FPT2; 
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = (uint8_t)mcmax(NDG_MIN, (int32_t)*pt1 - (int32_t)*pt2);
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    for (PT1 = SLONGDATA(image1), PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = (int32_t)mcmax(NDG_MIN, (int32_t)*PT1 - (int32_t)*PT2);
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    for (FPT1 = FLOATDATA(image1), FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = *FPT1 - *FPT2;
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lsub() */

/* ==================================== */
int32_t lsup(
  struct xvimage * image1,
  struct xvimage * image2)
/* prédicat inf pixelwise */
/* ==================================== */
#undef F_NAME
#define F_NAME "lsup"
{
  int32_t i;
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    uint8_t *pt1, *pt2;
    for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 >= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    int32_t *pt1, *pt2; 
    for (pt1 = SLONGDATA(image1), pt2 = SLONGDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 >= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    float *pt1, *pt2; 
    for (pt1 = FLOATDATA(image1), pt2 = FLOATDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 >= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lsup() */

/* ==================================== */
int32_t lvolume(
  struct xvimage * image, 
  double *vol)
/* retourne la somme des valeurs de pixels */
/* ==================================== */
#undef F_NAME
#define F_NAME "lvolume"
{
  int32_t i;
  uint8_t *pt;
  int32_t *PT; 
  float *FPT; 
  double fvolume = 0.0; 
  int32_t rs, cs, d, N;

  rs = rowsize(image);
  cs = colsize(image);
  d = depth(image);
  N = rs * cs * d;

  if (datatype(image) == VFF_TYP_1_BYTE)
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++) fvolume += (double)*pt;
  else 
  if (datatype(image) == VFF_TYP_4_BYTE)
    for (PT = SLONGDATA(image), i = 0; i < N; i++, PT++) fvolume += (double)*PT;
  else 
  if (datatype(image) == VFF_TYP_FLOAT)
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++) fvolume += (double)*FPT;
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  *vol = fvolume;
  return 1;
} /* lvolume() */

/* ==================================== */
int32_t lxor(
  struct xvimage * image1,
  struct xvimage * image2)
/* xor de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lxor"
{
  int32_t i;
  uint8_t *F1, *F2;
  int32_t rs, cs, ds, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  N = rs * cs * ds;
  F1 = UCHARDATA(image1);
  F2 = UCHARDATA(image2);
  
  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (i = 0; i < N; i++)
      {
        if (((F1[i] == 0) && (F2[i] == 0)) || ((F1[i] != 0) && (F2[i] != 0)))
	  F1[i]=0;
        else 
     	  F1[i]=255;
      }
  }
  else 
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lxor() */
