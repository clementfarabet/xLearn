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

/* lconvol: operateur de convolution par un masque de taille quelconque */
/* algorithme naif */
/* Michel Couprie - septembre 1997 */

/* ldirectionalfilter: supremum  de filtres linéaires utilisant un jeu de masques */
/* Michel Couprie - avril 2003 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <lfft.h>
#include <lcrop.h>

#define EPSILON 1E-50
#define BIGNUMBER 1E30
#define VERBOSE
//#define DEBUG

#define TRAITEBORDS

/* ==================================== */
int32_t lconvol(struct xvimage *f, struct xvimage *m, int32_t mode)
/* mode = 0: algo naif - image nulle en dehors du support */
/* mode = 1: algo naif - extension des valeurs du bord en dehors du support */
/* mode = 2: convolution par FFT - image nulle en dehors du support */
/* mode = 3: convolution par FFT - extension des valeurs du bord en dehors du support */
/* ==================================== */
#undef F_NAME
#define F_NAME "lconvol"
{
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t i, j;           /* index muet */
  register int32_t k, l;           /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  int32_t rsm = rowsize(m);        /* taille ligne masque */
  int32_t csm = colsize(m);        /* taille colonne masque */
  int32_t Nm = rsm * csm;
  int32_t xc = rsm / 2;            /* coord. du centre du masque */
  int32_t yc = csm / 2;            /* coord. du centre du masque */
  float *M = FLOATDATA(m);
  float *F = FLOATDATA(f);
  float *H;            /* image de travail */
  int32_t nptb, c;
  int32_t *tab_m_x;       /* liste des coord. x des points non nuls du masque. */
  int32_t *tab_m_y;       /* liste des coord. y des points non nuls du masque. */
  float *tab_m_val;   /* liste des valeurs des points non nuls du masque. */
  float sum;
  int32_t rs2, cs2;
  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if ((datatype(f) != VFF_TYP_FLOAT) || (datatype(m) != VFF_TYP_FLOAT))
  {
    fprintf(stderr, "%s: image and mask type must be float\n", F_NAME);
    return(0);
  }

  if ((mode == 0) || (mode == 1))
  {
    H = (float *)calloc(1,N*sizeof(float));
    if (H == NULL)
    {  
      fprintf(stderr,"%s : malloc failed for H\n", F_NAME);
      return(0);
    }

    for (x = 0; x < N; x++) H[x] = F[x];

    nptb = 0;
    for (i = 0; i < Nm; i += 1)
      if (mcabs(M[i]) > EPSILON)
        nptb += 1;

#ifdef DEBUG
    printf("%s: nb. kernel points: %d\n", F_NAME, nptb);
#endif

    tab_m_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
    tab_m_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
    tab_m_val = (float *)calloc(1,nptb * sizeof(float));
    if ((tab_m_x == NULL) || (tab_m_y == NULL) || (tab_m_val == NULL))
    {  
      fprintf(stderr,"%s : malloc failed for tab_m\n", F_NAME);
      return(0);
    }
    k = 0;
    for (j = 0; j < csm; j += 1)
      for (i = 0; i < rsm; i += 1)
        if (mcabs(M[j * rsm + i]) > EPSILON)
        {
          tab_m_x[k] = i;
          tab_m_y[k] = j;
          tab_m_val[k] = M[j * rsm + i];
          k += 1;
        }
    if (mode == 0)
    {
      for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
      {
        sum = 0.0;
        for (c = 0; c < nptb ; c += 1)
        {
          l = y + tab_m_y[c] - yc;
          k = x + tab_m_x[c] - xc; 
          if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs))
            sum += H[l * rs + k] * tab_m_val[c];
        }
        F[y * rs + x] = sum;
      }
    } 
    else if (mode == 1)
    {
      for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
      {
        sum = 0.0;
        for (c = 0; c < nptb ; c += 1)
        {
          l = y + tab_m_y[c] - yc;
          k = x + tab_m_x[c] - xc; 
          if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs))
            sum += H[l * rs + k] * tab_m_val[c];
          else if ((l < 0) && (k >= 0) && (k < rs))
            sum += H[0 * rs + k] * tab_m_val[c];
          else if ((l >= cs) && (k >= 0) && (k < rs))
            sum += H[(cs-1) * rs + k] * tab_m_val[c];
          else if ((k < 0) && (l >= 0) && (l < cs))
            sum += H[l * rs + 0] * tab_m_val[c];
          else if ((k >= rs) && (l >= 0) && (l < cs))
            sum += H[l * rs + rs-1] * tab_m_val[c];
          else if ((l < 0) && (k < 0))
            sum += H[0 * rs + 0] * tab_m_val[c];
          else if ((l >= cs) && (k < 0))
            sum += H[(cs-1) * rs + 0] * tab_m_val[c];
          else if ((l < 0) && (k >= rs))
            sum += H[0 * rs + rs-1] * tab_m_val[c];
          else if ((l >= cs) && (k >= rs))
            sum += H[(cs-1) * rs + rs-1] * tab_m_val[c];
        }
        F[y * rs + x] = sum;
      }
    } 
    free(H);
    free(tab_m_x);
    free(tab_m_y);
    free(tab_m_val);
  } // if ((mode == 0) || (mode == 1))
  else if (mode == 2)
  {
    struct xvimage *if1;  // FFT de imagepad (partie reelle)
    struct xvimage *if2;  // FFT de imagepad (partie imaginaire)
    struct xvimage *mf1;  // FFT de maskpad (partie reelle)
    struct xvimage *mf2;  // FFT de maskpad (partie imaginaire)
    float *IF1, *IF2, *MF1, *MF2;

    rs2 = mcmax(rs+rs,cs+cs);
    cs2 = 1;
    while (cs2 < rs2) cs2 = cs2 << 1;
    rs2 = cs2;

#ifdef DEBUG
    printf("%s: rs2 = %d\n", F_NAME, rs2);
#endif

    if1 = allocimage(NULL, rs2, cs2, 1, VFF_TYP_FLOAT);
    if2 = allocimage(NULL, rs2, cs2, 1, VFF_TYP_FLOAT);
    mf1 = allocimage(NULL, rs2, cs2, 1, VFF_TYP_FLOAT);
    mf2 = allocimage(NULL, rs2, cs2, 1, VFF_TYP_FLOAT);
    if ((if1 == NULL) || (if2 == NULL) || (mf1 == NULL) || (mf2 == NULL))
    {
      fprintf(stderr, "%s: allocimage failed\n", F_NAME);
      return 0;
    }
    IF1 = FLOATDATA(if1);
    IF2 = FLOATDATA(if2);
    MF1 = FLOATDATA(mf1);
    MF2 = FLOATDATA(mf2);
    for (x = 0; x < rs2*cs2; x++) MF1[x] = MF2[x] = 0.0; 
    for (y = 0; y < csm; y++) 
      for (x = 0; x < rsm; x++) 
        MF1[((y-yc+cs2)%cs2)*rs2 + (x-xc+rs2)%rs2] = M[y*rsm + x];    
#ifdef DEBUG
    writeimage(mf1, "_maskpad");
#endif
    if (! lfft2(mf1, mf2, 0))
    {
      fprintf(stderr, "%s: function lfft failed\n", F_NAME);
      return 0;
    }
#ifdef DEBUG
    printf("%s: fft mask done\n", F_NAME);
    writeimage(mf1, "_mf1");
    writeimage(mf2, "_mf2");
    printf("%s: xc = %d yc = %d\n", F_NAME, xc, yc);
#endif

    for (x = 0; x < rs2*cs2; x++) IF1[x] = IF2[x] = 0.0; 
    for (y = 0; y < cs; y++) 
      for (x = 0; x < rs; x++) 
        IF1[y*rs2 + x] = F[y*rs + x];    
#ifdef DEBUG
    writeimage(if1, "_imagepad");
#endif
    if (! lfft2(if1, if2, 0))
    {
      fprintf(stderr, "%s: function lfft failed\n", F_NAME);
      return 0;
    }
#ifdef DEBUG
    printf("%s: fft image done\n", F_NAME);
    writeimage(if1, "_if1");
    writeimage(if2, "_if2");
#endif
    // multiplication point a point
    for (y = 0; y < cs2; y++) 
      for (x = 0; x < rs2; x++) 
      {
        IF1[y*rs2 + x] = IF1[y*rs2 + x] * MF1[y*rs2 + x];
        IF2[y*rs2 + x] = IF2[y*rs2 + x] * MF2[y*rs2 + x];
      }    
#ifdef DEBUG
    printf("%s: pointwise product done\n", F_NAME);
    writeimage(if1, "_pf1");
    writeimage(if2, "_pf2");
#endif

    // FFT inverse
    if (! lfft2(if1, if2, 1))
    {
      fprintf(stderr, "%s: function lfft failed\n", F_NAME);
      return 0;
    }
#ifdef DEBUG
    printf("%s: inverse fft done\n", F_NAME);
    writeimage(if1, "_c1");
    writeimage(if2, "_c2");
#endif

    for (y = 0; y < cs; y++) 
      for (x = 0; x < rs; x++) 
      {
        F[y*rs + x] = IF1[y*rs2 + x];
      }

    freeimage(if1);
    freeimage(if2);
    freeimage(mf1);
    freeimage(mf2);
  } 
  else if (mode == 3)
  {
    int32_t n = (mcmax(rsm,csm) + 1) / 2;
    struct xvimage *tmp = lexpandframe(f, n);
    if (tmp == NULL)
    {
      fprintf(stderr, "%s: function lexpandframe failed\n", F_NAME);
      return 0;
    }
#ifdef DEBUG
    printf("%s: expandframe done\n", F_NAME);
    writeimage(tmp, "_tmp");
#endif


    if (! lconvol(tmp, m, 2))
    {
      fprintf(stderr, "%s: function lfft failed\n", F_NAME);
      return 0;
    }
#ifdef DEBUG
    printf("%s: lconvol mode 2 done\n", F_NAME);
    writeimage(tmp, "_tmpconvol");
#endif

    freeimage(f);
    f = lcrop(tmp, n, n, rs, cs);
    if (f == NULL)
    {
      fprintf(stderr, "%s: function lcrop failed\n", F_NAME);
      return 0;
    }
    freeimage(tmp);
  }
  else
  {
    fprintf(stderr,"%s : bad value for mode : %d\n", F_NAME, mode);
    return(0);
  } 

  return 1;
} // lconvol()

/* ==================================== */
int32_t ldirectionalfilter(
  struct xvimage *image, 
  double width, double length, int32_t ndir)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldirectionalfilter"
{
  double sigma, lambda, theta;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t N = rs * cs;
  struct xvimage *kernel;
  int32_t rsk, csk;
  float *K;
  int32_t n, i, j, x, y, xr, yr, x0, y0;
  struct xvimage *result;
  float *R;
  struct xvimage *temp;
  float *T;
  float tmp, k1, k2, t1, t2;
#ifdef DEBUG
  float sum, sum1, sum2;
  char buf1[256];
  char buf2[256];
#endif

  result = allocimage(NULL, rs, cs, 1, VFF_TYP_FLOAT);
  temp = allocimage(NULL, rs, cs, 1, VFF_TYP_FLOAT);
  if ((result == NULL) || (temp == NULL))
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  R = FLOATDATA(result);
  T = FLOATDATA(temp);
  for (i = 0; i < N; i++) R[i] = 0.0;

  csk = rsk = (int32_t)(length * 5);
  x0 = rsk / 2;
  y0 = csk / 2;
  sigma = 1.0 / width;
  lambda = 1.0 / length;
  kernel = allocimage(NULL, rsk, csk, 1, VFF_TYP_FLOAT);
  if (kernel == NULL)
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  K = FLOATDATA(kernel);

  for (n = 0; n < ndir; n++)
  {
#ifdef VERBOSE
    printf("n = %d\n", n);
#endif
    theta = (n * M_PI) / ndir; 
    // calcul de la constante de normalisation k1
    t1 = t2 = 0.0;
    for (j = 0; j < csk; j++)
      for (i = 0; i < rsk; i++)
      {
        x = i - x0;
        y = j - y0;
        xr = cos(theta) * x + sin(theta) * y;
        yr = -sin(theta) * x + cos(theta) * y;
        tmp = exp(-lambda*mcsqr(yr)) * exp(-sigma*mcsqr(xr));
        t1 += tmp;
        t2 += mcsqr(xr) * tmp;
      } 
    k1 = t1 / (sigma * t2);
    // calcul de la constante de normalisation k2
    t1 = 0.0;
    for (j = 0; j < csk; j++)
      for (i = 0; i < rsk; i++)
      {
        x = i - x0;
        y = j - y0;
        xr = cos(theta) * x + sin(theta) * y;
        yr = -sin(theta) * x + cos(theta) * y;
        tmp = exp(-lambda*mcsqr(yr)) * exp(-sigma*mcsqr(xr));
        t2 = 1.0 - k1 * sigma * mcsqr(xr); 
        if (t2 > 0) t1 += t2 * tmp; 
      } 
    k2 = 1.0 / t1;
    // calcul du kernel
#ifdef DEBUG
    sum = sum1 = sum2 = 0.0;
#endif
    for (j = 0; j < csk; j++)
    {
      for (i = 0; i < rsk; i++)
      {
        x = i - x0;
        y = j - y0;
        xr = cos(theta) * x + sin(theta) * y;
        yr = -sin(theta) * x + cos(theta) * y;
        K[j * rsk + i] = (float)(k2 * exp(-lambda*mcsqr(yr)) *
                         (1.0 - k1*sigma*mcsqr(xr)) * exp(-sigma*mcsqr(xr))); 
#ifdef DEBUG
        sum += K[j * rsk + i];
        if (K[j * rsk + i] > 0) sum1 += K[j * rsk + i];
        if (K[j * rsk + i] < 0) sum2 += K[j * rsk + i];
#endif
      } // for i
    } // for j

#ifdef DEBUG
    printf("sum = %g sum1 = %g sum2 = %g\n", sum, sum1, sum2);
    strcpy(buf1, "_kernel_");
    sprintf(buf2, "%d", n);
    strcat(buf1, buf2);
    writeimage(kernel, buf1);
#endif

    copy2image(temp, image);
    lconvol(temp, kernel, 3);

#ifdef DEBUG
    strcpy(buf1, "_convol_");
    strcat(buf1, buf2);
    writeimage(temp, buf1);
#endif

    // result = mcmax(result, temp)
    for (i = 0; i < N; i++) 
      if (T[i] > R[i]) R[i] = T[i];
  } // for n

  copy2image(image, result);
  freeimage(temp);
  freeimage(result);
  freeimage(kernel);
  return 1;
} // ldirectionalfilter()
