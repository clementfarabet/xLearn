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
   lbarycentre : 
     calcule les isobarycentres des composantes connexes d'une image binaire.

   lbarycentrelab : 
     calcule les isobarycentres des regions etiquetees.

   Michel Couprie - avril 1997 
*/

//#define DEBUG

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <llabelextrema.h>

/* ==================================== */
int32_t lbarycentrelab(struct xvimage * imagelab)
/* ==================================== */
{
  int32_t i, j;
  int32_t *F;
  int32_t rs, cs, N;
  int32_t nblabels;
  double *bxx;             /* pour les tables de barycentres par composantes */
  double *byy;
  int32_t *surf;
  int32_t lab;

  if (depth(imagelab) != 1) 
  {
    fprintf(stderr, "lbarycentre: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  if (datatype(imagelab) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "lbarycentrelab: l'image doit etre de type int32_t\n");
    return 0;
  }

  rs = imagelab->row_size;
  cs = imagelab->col_size;
  N = rs * cs;
  F = SLONGDATA(imagelab);

  nblabels = 0;
  for (j = 0; j < N; j++) if (F[j] > nblabels ) nblabels = F[j];

  bxx = (double *)calloc(1,nblabels * sizeof(double));
  byy = (double *)calloc(1,nblabels * sizeof(double));
  surf = (int32_t *)calloc(1,nblabels * sizeof(int32_t));
  if ((bxx == NULL) || (byy == NULL) || (surf == NULL))
  {
    fprintf(stderr, "lbarycentre: malloc failed\n");
    return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul des isobarycentres par region (sauf fond) */
  /* ---------------------------------------------------------- */
  
  for (i = 0; i < nblabels; i++)
  {
    bxx[i] = 0.0;
    byy[i] = 0.0;
    surf[i] = 0;
  }

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
    {
      if (F[j * rs + i] != 0)
      {
        lab = F[j * rs + i] - 1; /* les valeurs des labels sont entre 1 et nblabels */
        surf[lab] += 1;
        bxx[lab] += (double)i;
        byy[lab] += (double)j;
      }
    }

#ifdef DEBUG
printf("%d\n", nblabels);
#endif

  for (i = 0; i < nblabels; i++)
  {
    bxx[i] = bxx[i] / surf[i];
    byy[i] = byy[i] / surf[i];

#ifdef DEBUG
printf("%g %g\n", bxx[i], byy[i]);
#endif

  }

  /* ---------------------------------------------------------- */
  /* marque l'emplacement approximatif des barycentres dans l'image */
  /* ---------------------------------------------------------- */

  for (j = 0; j < N; j++) F[j] = 0;

  for (i = 0; i < nblabels; i++)
    F[(int32_t)(arrondi(byy[i])) * rs + arrondi(bxx[i])] = NDG_MAX;
  
  free(bxx);
  free(byy);
  free(surf);
  return 1;
} /* lbarycentrelab() */

/* ==================================== */
int32_t lbarycentre(struct xvimage * image1, int32_t connex)
/* ==================================== */
{
  int32_t i, j;
  uint8_t *F;
  int32_t rs, cs, N;
  struct xvimage *label;
  int32_t *LABEL;     /* pour les labels des composantes connexes */
  int32_t nblabels;
  double *bxx;             /* pour les tables de barycentres par composantes */
  double *byy;
  int32_t *surf;
  int32_t lab;

  if (depth(image1) != 1) 
  {
    fprintf(stderr, "lbarycentre: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  if ((connex != 4) && (connex != 8))
  {
    fprintf(stderr, "lbarycentre: mauvaise connexite (%d)\n", connex);
    return 0;
  }

  rs = image1->row_size;
  cs = image1->col_size;
  N = rs * cs;
  F = UCHARDATA(image1);

  label = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  LABEL = SLONGDATA(label);

  if (! llabelextrema(image1, connex, LABMAX, label, &nblabels))
  {
    fprintf(stderr, "lbarycentre: llabelextrema failed\n");
    return 0;
  }

  bxx = (double *)calloc(1,nblabels * sizeof(double));
  byy = (double *)calloc(1,nblabels * sizeof(double));
  surf = (int32_t *)calloc(1,nblabels * sizeof(int32_t));
  if ((bxx == NULL) || (byy == NULL) || (surf == NULL))
  {
    fprintf(stderr, "lbarycentre: malloc failed\n");
    return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul des isobarycentres par composante */
  /* ---------------------------------------------------------- */
  
  for (i = 0; i < nblabels; i++)
  {
    bxx[i] = 0.0;
    byy[i] = 0.0;
    surf[i] = 0;
  }

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
    {
      lab = LABEL[j * rs + i] - 1; /* les valeurs des labels sont entre 1 et nblabels */
      if (F[j * rs + i] != 0)
      {
        surf[lab] += 1;
        bxx[lab] += (double)i;
        byy[lab] += (double)j;
      }
      else 
        surf[lab] = -1;          /* marque la composante "fond" */
    }

#ifdef DEBUG
printf("%d\n", nblabels-1);
#endif

  for (i = 0; i < nblabels-1; i++)
    if (surf[i] != -1)
    {
      bxx[i] = bxx[i] / surf[i];
      byy[i] = byy[i] / surf[i];

#ifdef DEBUG
printf("%g %g\n", bxx[i], byy[i]);
#endif

    }

  /* ---------------------------------------------------------- */
  /* marque l'emplacement approximatif des barycentres dans l'image */
  /* ---------------------------------------------------------- */

  for (j = 0; j < N; j++) F[j] = 0;

  for (i = 0; i < nblabels-1; i++)
    if (surf[i] != -1)
    {
      F[(int32_t)(arrondi(byy[i])) * rs + arrondi(bxx[i])] = NDG_MAX;
    }
  
  freeimage(label);
  /*  
  free(bxx);
  free(byy);
  free(surf);
  */
  return 1;
} /* lbarycentre() */

/* ==================================== */
int32_t lbarycentreold(struct xvimage * image1, double * bx, double * by)
/* ==================================== */
{
  int32_t i, j;
  uint8_t *F;
  int32_t rs, cs, N;
  struct xvimage *label;
  int32_t *LABEL;     /* pour les labels des composantes connexes */
  int32_t nblabels;
  double *bxx;             /* pour les tables de barycentres par composantes */
  double *byy;
  int32_t *surf;
  int32_t lab;

  if (depth(image1) != 1) 
  {
    fprintf(stderr, "lbarycentre: cette version ne traite pas les images volumiques\n");
    exit(0);
  }
  rs = image1->row_size;
  cs = image1->col_size;
  N = rs * cs;
  F = UCHARDATA(image1);

  label = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  LABEL = SLONGDATA(label);

  if (! llabelextrema(image1, 4, LABMAX, label, &nblabels))
  {
    fprintf(stderr, "lbarycentre: llabelextrema failed\n");
    exit(0);
  }

  bxx = (double *)calloc(1,nblabels * sizeof(double));
  byy = (double *)calloc(1,nblabels * sizeof(double));
  surf = (int32_t *)calloc(1,nblabels * sizeof(int32_t));
  if ((bxx == NULL) || (byy == NULL) || (surf == NULL))
  {
    fprintf(stderr, "lbarycentre: malloc failed\n");
    exit(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul des isobarycentres par composante */
  /* ---------------------------------------------------------- */
  
  for (i = 0; i < nblabels; i++)
  {
    bxx[i] = 0.0;
    byy[i] = 0.0;
    surf[i] = 0;
  }

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
    {
      lab = LABEL[j * rs + i] - 1; /* les valeurs des labels sont entre 1 et nblabels */
      if (F[j * rs + i] != 0)
      {
        surf[lab] += 1;
        bxx[lab] += (double)i;
        byy[lab] += (double)j;
      }
      else 
        surf[lab] = -1;          /* marque la composante "fond" */
    }

printf("%d\n", nblabels-1);
  for (i = 0; i < nblabels-1; i++)
    if (surf[i] != -1)
    {
      bxx[i] = bxx[i] / surf[i];
      byy[i] = byy[i] / surf[i];
/*printf("bxx[%d] = %g ; byy[%d] = %g ; surf[%d] = %d\n", i, bxx[i], i, byy[i], i, surf[i]); */
printf("%g %g\n", bxx[i], byy[i]);
    }

  /* ---------------------------------------------------------- */
  /* calcul de l'isobarycentre global */
  /* ---------------------------------------------------------- */

  *bx = 0.0;
  *by = 0.0;
  for (i = 0; i < nblabels-1; i++)
    if (surf[i] != -1)
    {
      *bx += bxx[i];
      *by += byy[i];
    }

  *bx = *bx / (double)(nblabels - 1);
  *by = *by / (double)(nblabels - 1);

  for (j = 0; j < N; j++) F[j] = 0;

  F[(int32_t)(*by) * rs + (int32_t)(*bx)] = NDG_MAX;
  
  freeimage(label);
  free(bxx);
  free(byy);
  free(surf);
  free(LABEL);
  return 1;
} /* lbarycentreold() */
