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
/* transformation multi-lineaire de l'echelle de gris d' une image */
/* Michel Couprie - juillet 1997 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#ifdef HP
#define _INCLUDE_XOPEN_SOURCE
#endif
#include <math.h>
#include <mccodimage.h>
#include <mcfah.h>
#include <mcutil.h>
#include <lhistscal.h>

#define CHECKBOUNDS
/*
#define DEBUG
*/
void histscal(
  int32_t * hs,       /* histogramme source */
  int32_t mins,
  int32_t maxs,
  double *hd,     /* histogramme destination (alloue) */
  int32_t mind,
  int32_t maxd)
{
  double f = (double)(maxs - mins + 1) / (double)(maxd - mind + 1);
  int32_t i, j;
  double reste, flo, flo1;
  
  if (f >= 1.0)
  {
    i = mins;
    j = mind;
    reste = 0.0;
    while ((i <= maxs) && (j <= maxd))
    {
#ifdef CHECKBOUNDS
if ((j < mind) || (j > maxd)) fprintf(stderr, "j OUT OF BOUNDS 1 : %d\n", j);
if ((i < mins) || (i > maxs)) fprintf(stderr, "i OUT OF BOUNDS 1 : %d\n", i);
#endif
      hd[j] = reste;
      flo = floor((j-mind+1) * f);
      while (((i-mins+1) <= flo) && (i <= maxs))
      {
#ifdef CHECKBOUNDS
if ((j < mind) || (j > maxd)) fprintf(stderr, "j OUT OF BOUNDS 2 : %d\n", j);
if ((i < mins) || (i > maxs)) fprintf(stderr, "i OUT OF BOUNDS 2 : %d\n", i);
#endif
        hd[j] += (double)hs[i];
        i++;
      } /* while (i <= flo) */
      if ((flo < (j-mind)*f) && (i <= maxs))
      {
        flo1 = (j-mind+1)*f - flo;
#ifdef CHECKBOUNDS
if ((j < mind) || (j > maxd)) fprintf(stderr, "j OUT OF BOUNDS 3 : %d\n", j);
if ((i < mins) || (i > maxs)) fprintf(stderr, "i OUT OF BOUNDS 3 : %d\n", i);
#endif
        hd[j] += (double)hs[i] * flo1;
        reste =  (double)hs[i] * (1.0 - flo1);
        i++;
      } /* if (flo < j*f) */
      else
        reste = 0.0;
      j++;
    } /* while ((i <= maxs) && (j <= maxd)) */
    hd[maxd] += reste;
  } /* if (f >= 1.0) */
  else
  {
    i = mins;
    j = mind;
    reste = 1.0;
    hd[j] = 0.0;
    while ((j <= maxd) && (i <= maxs))
    {
#ifdef CHECKBOUNDS
if ((j < mind) || (j > maxd)) fprintf(stderr, "j OUT OF BOUNDS 4 : %d\n", j);
if ((i < mins) || (i > maxs)) fprintf(stderr, "i OUT OF BOUNDS 4 : %d\n", i);
#endif
      hd[j] += reste * hs[i] * f;
      j++;
      flo = floor((i-mins+1) / f);
      while (((j-mind+1) <= flo) && (j <= maxd))
      {
#ifdef CHECKBOUNDS
if ((j < mind) || (j > maxd)) fprintf(stderr, "j OUT OF BOUNDS 5 : %d\n", j);
if ((i < mins) || (i > maxs)) fprintf(stderr, "i OUT OF BOUNDS 5 : %d\n", i);
#endif
        hd[j] = (double)hs[i] * f;
        j++;
      } /* while (j <= flo) */
      if (j <= maxd)
      {
      	flo1 = (i-mins+1)/f - flo;
#ifdef CHECKBOUNDS
if ((j < mind) || (j > maxd)) fprintf(stderr, "j OUT OF BOUNDS 6 : %d\n", j);
if ((i < mins) || (i > maxs)) fprintf(stderr, "i OUT OF BOUNDS 6 : %d\n", i);
#endif
      	hd[j] = (hs[i] * f) * flo1;
      	reste = 1.0 - flo1;
      } /* if (j <= maxd) */
      i++;
    } /* while (j <= maxd) */
  } /* else if (f >= 1.0) */
} /* histscal() */

/* ==================================== */
int32_t lhistscal(
  struct xvimage * image,
  int32_t a,
  int32_t A,
  int32_t b,
  int32_t B)
/* ==================================== */
{
  int32_t i, k;
  int32_t x;                       /* index muet de pixel */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t d = depth(image);        /* nombre plans */
  int32_t N = rs * cs * d;         /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */
  int32_t    hs[NDG_MAX+1];
  double hd[NDG_MAX+1];
  Fah * FAH;                   /* la file d'attente hierarchique */
  double cumul;

  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "lhistscal() : CreeFahVide failed\n");
      return(0);
  }

  for (i = 0; i <= NDG_MAX; i++) hs[i] = 0;
  for (x = 0; x < N; x++) hs[SOURCE[x]] += 1;

  histscal(hs, NDG_MIN, a-1, hd, NDG_MIN, A-1);

#ifdef DEBUG
cumul = 0;
for (i = NDG_MIN; i <= a-1; i++) cumul += (double)hs[i];
printf("cumul hs = %g\n", cumul);
cumul = 0;
for (i = NDG_MIN; i <= A-1; i++) cumul += (double)hd[i];
printf("cumul hd = %g\n", cumul);
#endif

  hd[A] = (double)hs[a];
  histscal(hs, a+1, b-1, hd, A+1, B-1);

#ifdef DEBUG
cumul = 0;
for (i = a+1; i <= b-1; i++) cumul += (double)hs[i];
printf("cumul hs = %g\n", cumul);
cumul = 0;
for (i = A+1; i <= B-1; i++) cumul += (double)hd[i];
printf("cumul hd = %g\n", cumul);
#endif

  hd[B] = (double)hs[b];
  histscal(hs, b+1, NDG_MAX, hd, B+1, NDG_MAX);

#ifdef DEBUG
cumul = 0;
for (i = b+1; i <= NDG_MAX; i++) cumul += (double)hs[i];
printf("cumul hs = %g\n", cumul);
cumul = 0;
for (i = B+1; i <= NDG_MAX; i++) cumul += (double)hd[i];
printf("cumul hs = %g\n", cumul);
#endif

#ifdef DEBUG
for (i = 0; i <= NDG_MAX; i++) printf("hd[%d] = %g\n", i, hd[i]);
cumul = 0;
for (i = 0; i <= NDG_MAX; i++) cumul += hd[i];
printf("N = %d ; cumul = %g\n", N, cumul);
#endif

  FahPush(FAH, -1, 0);               /* force la creation du niveau 0 dans la Fah. */
  for (x = 0; x < N; x++) FahPush(FAH, x, SOURCE[x]);
  (void)FahPop(FAH);

  k = NDG_MIN;
  i = 0;
  cumul = hd[k];
  while (hd[k] == 0.0) k++;
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    SOURCE[x] = mcmin(k,NDG_MAX);
    i++;
    if (k < NDG_MAX) while ((double)i >= cumul) { k++; cumul += hd[k]; }
  } /* while (! FahVide(FAH)) */

  FahTermine(FAH);
  return 1;
} /* lhistscal() */

/* ==================================== */
int32_t lhistscal3(
  struct xvimage * image,
  int32_t a,
  int32_t A,
  int32_t b,
  int32_t B,
  int32_t c,
  int32_t C)
/* ==================================== */
{
  int32_t i, k;
  int32_t x;                       /* index muet de pixel */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t d = depth(image);        /* nombre plans */
  int32_t N = rs * cs * d;         /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */
  int32_t    hs[NDG_MAX+1];
  double hd[NDG_MAX+1];
  Fah * FAH;                   /* la file d'attente hierarchique */
  double cumul;

  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "lhistscal() : CreeFahVide failed\n");
      return(0);
  }

  for (i = 0; i <= NDG_MAX; i++) hs[i] = 0;
  for (x = 0; x < N; x++) hs[SOURCE[x]] += 1;

  histscal(hs, NDG_MIN, a-1, hd, NDG_MIN, A-1);

  hd[A] = (double)hs[a];
  histscal(hs, a+1, b-1, hd, A+1, B-1);

  hd[B] = (double)hs[b];
  histscal(hs, b+1, c-1, hd, B+1, C-1);

  hd[C] = (double)hs[c];
  histscal(hs, c+1, NDG_MAX, hd, C+1, NDG_MAX);

  FahPush(FAH, -1, 0);               /* force la creation du niveau 0 dans la Fah. */
  for (x = 0; x < N; x++) FahPush(FAH, x, SOURCE[x]);
  (void)FahPop(FAH);

  k = NDG_MIN;
  i = 0;
  cumul = hd[k];
  while (hd[k] == 0.0) k++;
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    SOURCE[x] = mcmin(k,NDG_MAX);
    i++;
    if (k < NDG_MAX) while ((double)i >= cumul) { k++; cumul += hd[k]; }
  } /* while (! FahVide(FAH)) */

  FahTermine(FAH);
  return 1;
} /* lhistscal3() */



