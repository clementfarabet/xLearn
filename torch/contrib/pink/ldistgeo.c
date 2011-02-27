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
/****************************************************************
*
* Routine Name: ldistXXX - library call for dist
*
* Purpose:     Calcul de la n-distance (n = 4,8,6,18,26)
*              a un ensemble X geodesiquement a un ensemble Y
*
* Input:       X: Image binaire
* Input:       Y: Image binaire
* Output:      Image en niveaux de gris (entiers longs)
*
* Author: Michel Couprie
*
****************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mccodimage.h>
#include <mclifo.h>
#include <ldistgeo.h>

/* ==================================== */
int32_t ldistgeo(struct xvimage *imgx,   /* donnee: image binaire */       
             struct xvimage *imgy,   /* donnee: image binaire (masque) */
             int32_t connex,
             struct xvimage *res     /* resultat: distances (doit être allouée) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldistgeo"
{ 
  if ((connex == 4) || (connex == 8))
    return ldistgeo2d(imgx, imgy, connex, res);
  else
    return ldistgeo3d(imgx, imgy, connex, res);
} // ldistgeo

/* ==================================== */
int32_t ldistgeo2d(struct xvimage *imgx,   /* donnee: image binaire */       
               struct xvimage *imgy,   /* donnee: image binaire (masque) */
               int32_t connex,
               struct xvimage *res     /* resultat: distances (doit être allouée) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldistgeo"
{ 
  int32_t rs = imgx->row_size;
  int32_t cs = imgx->col_size;
  int32_t N = rs * cs;           /* taille de l'image */
  uint8_t *X;          /* pointeur sur l'image x */
  uint8_t *Y;          /* pointeur sur l'image y */
  uint32_t *D;         /* pointeur sur les distances */
  int32_t i, j, k, d, incr_vois;
  Lifo * LIFO1;
  Lifo * LIFO2;
  Lifo * LIFOtmp;

  if ((rowsize(imgy) != rs) || (colsize(imgy) != cs) || (depth(imgy) != 1) ||
      (rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != 1))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (datatype(res) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: result image type must be int32_t\n", F_NAME);
    return 0;
  }

  X = UCHARDATA(imgx);
  Y = UCHARDATA(imgy);
  D = ULONGDATA(res);

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  for (i = 0; i < N; i++) 
  {
    if (X[i] || !Y[i]) 
      D[i] = 0; 
    else
    { /* X[i] == 0 et Y[i] != 0 */
      D[i] = -1; 
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
        if ((j != -1) && X[j] && Y[j]) { D[i] = 1; LifoPush(LIFO1, i); break; }
      }    
    }
  } /* for (i = 0; i < N; i++) */

  while (! LifoVide(LIFO1))
  {
    while (! LifoVide(LIFO1))
    {
      i = LifoPop(LIFO1);
      d = D[i];
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
        if ((j != -1) && (D[j] == -1)) { D[j] = d + 1; LifoPush(LIFO2, j); }
      }
    } /* while (! LifoVide(LIFO1)) */
    LIFOtmp = LIFO2;
    LIFO2 = LIFO1;
    LIFO1 = LIFOtmp;
  } /* while (! LifoVide(LIFO1)) */

  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} // ldistgeo2d()


/* ==================================== */
int32_t ldistgeo3d(struct xvimage *imgx,   /* donnee: image binaire */       
               struct xvimage *imgy,   /* donnee: image binaire (masque) */
               int32_t connex,
               struct xvimage *res     /* resultat: distances (doit être allouée) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldistgeo3d"
{ 
  int32_t rs = imgx->row_size;
  int32_t cs = imgx->col_size;
  int32_t ds = depth(imgx);
  int32_t ps = rs * cs;          /* taille d'un plan */
  int32_t N = ps * ds;           /* taille de l'image */
  uint8_t *X;                    /* pointeur sur l'image x */
  uint8_t *Y;                    /* pointeur sur l'image y */
  uint32_t *D;                   /* pointeur sur les distances */
  int32_t i, j, k, d;
  Lifo * LIFO1;
  Lifo * LIFO2;
  Lifo * LIFOtmp;

  if ((rowsize(imgy) != rs) || (colsize(imgy) != cs) || (depth(imgy) != ds) ||
      (rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (datatype(res) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: result image type must be int32_t\n", F_NAME);
    return 0;
  }

  X = UCHARDATA(imgx);
  Y = UCHARDATA(imgy);
  D = ULONGDATA(res);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  if (connex == 6)
  {
    for (i = 0; i < N; i++) 
    {
      if (X[i] || !Y[i]) 
        D[i] = 0; 
      else
      { /* X[i] == 0 et Y[i] != 0 */
        D[i] = -1; 
        for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
        {
          j = voisin6(i, k, rs, ps, N);
          if ((j != -1) && X[j] && Y[j]) { D[i] = 1; LifoPush(LIFO1, i); break; }
        }    
      }
    } /* for (i = 0; i < N; i++) */

    while (! LifoVide(LIFO1))
    {
      while (! LifoVide(LIFO1))
      {
        i = LifoPop(LIFO1);
        d = D[i];
        for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
        {
          j = voisin6(i, k, rs, ps, N);
          if ((j != -1) && (D[j] == -1)) { D[j] = d + 1; LifoPush(LIFO2, j); }
        }
      } /* while (! LifoVide(LIFO1)) */
      LIFOtmp = LIFO2;
      LIFO2 = LIFO1;
      LIFO1 = LIFOtmp;
    } /* while (! LifoVide(LIFO1)) */
  }
  else
  if (connex == 18)
  {
    for (i = 0; i < N; i++) 
    {
      if (X[i] || !Y[i]) 
        D[i] = 0; 
      else
      { /* X[i] == 0 et Y[i] != 0 */
        D[i] = -1; 
        for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
          j = voisin18(i, k, rs, ps, N);
          if ((j != -1) && X[j] && Y[j]) { D[i] = 1; LifoPush(LIFO1, i); break; }
        }    
      }
    } /* for (i = 0; i < N; i++) */

    while (! LifoVide(LIFO1))
    {
      while (! LifoVide(LIFO1))
      {
        i = LifoPop(LIFO1);
        d = D[i];
        for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
          j = voisin18(i, k, rs, ps, N);
          if ((j != -1) && (D[j] == -1)) { D[j] = d + 1; LifoPush(LIFO2, j); }
        }
      } /* while (! LifoVide(LIFO1)) */
      LIFOtmp = LIFO2;
      LIFO2 = LIFO1;
      LIFO1 = LIFOtmp;
    } /* while (! LifoVide(LIFO1)) */
  }
  else
  if (connex == 26)
  {
    for (i = 0; i < N; i++) 
    {
      if (X[i] || !Y[i]) 
        D[i] = 0; 
      else
      { /* X[i] == 0 et Y[i] != 0 */
        D[i] = -1; 
        for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
          j = voisin26(i, k, rs, ps, N);
          if ((j != -1) && X[j] && Y[j]) { D[i] = 1; LifoPush(LIFO1, i); break; }
        }    
      }
    } /* for (i = 0; i < N; i++) */

    while (! LifoVide(LIFO1))
    {
      while (! LifoVide(LIFO1))
      {
        i = LifoPop(LIFO1);
        d = D[i];
        for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
          j = voisin26(i, k, rs, ps, N);
          if ((j != -1) && (D[j] == -1)) { D[j] = d + 1; LifoPush(LIFO2, j); }
        }
      } /* while (! LifoVide(LIFO1)) */
      LIFOtmp = LIFO2;
      LIFO2 = LIFO1;
      LIFO1 = LIFOtmp;
    } /* while (! LifoVide(LIFO1)) */
  }
  else
  {
    fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
    return 0;
  }

  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} // ldistgeo3d()
