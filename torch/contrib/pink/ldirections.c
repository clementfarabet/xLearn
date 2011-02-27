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
* Routine Name: ldirections
*
* Purpose:     Calcul de la direction approximativement tangente ou normale (entre 0 et 179) 
*              en chaque point objet (la direction pour les points non objet est 255)
*              
* Input:       Image binaire
* Output:      Image en niveaux de directions (en degres, de 0 a 179)
* Written By:  Michel Couprie Juin 98
*
****************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#ifdef HP
#define _INCLUDE_XOPEN_SOURCE
#endif
#include <math.h>
#include <mcutil.h>
#include <mccodimage.h>
#include <mclifo.h>
#include <ldirections.h>

/* =============================================================== */
double calcangle(int32_t pc, int32_t p, int32_t rs)
/* =============================================================== */
{
  double ts, tc, d;
  int32_t x, y, xc, yc;

  xc = pc % rs; yc = pc / rs;
  x = p % rs; y = p / rs; 
  d = sqrt((xc - x) * (xc - x) + (yc - y) * (yc - y));
  ts = asin((y - yc) / d);
  tc = acos((x - xc) / d);
  if ((ts >= 0) && (tc <= M_PI_2))   return ts;
  if ((ts >= 0) && (tc > M_PI_2))    return tc;
  if ((ts < 0) && (tc > M_PI_2))     return - ts + M_PI;
  /* ((ts < 0) && (tc <= M_PI_2)) */ return ts + M_PI + M_PI;
} /* calcangle() */

/* ==================================== */
int32_t ldirections(struct xvimage *img,   /* donnee: image binaire */       
          int32_t connex,
          int32_t nbpoints,          /* nb. de points pour le voisinage */
          int32_t normale            /* booleen */
)
/* ==================================== */
{ 
  int32_t rs = img->row_size;
  int32_t cs = img->col_size;
  int32_t N = rs * cs;           /* taille de l'image */
  uint8_t *F;          /* pointeur sur l'image */
  uint8_t *D;          /* pointeur sur les directions */
  int32_t i, j, k, p, q, r, n;
  int32_t incr_vois;
  Lifo * LIFO;

  F = UCHARDATA(img);
  D = (uint8_t *)calloc(1,N*sizeof(char));
  if (D == NULL)
  {   fprintf(stderr,"ldirections() : malloc failed\n");
      return(0);
  }
  for (p = 0; p < N; p++) D[p] = F[p];

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "ldirections: CreeLifoVide failed\n");
      return 0;
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "ldirections: mauvaise connexite: %d\n", connex);
      return 0;
  } /* switch (connex) */

  for (p = 0; p < N; p++)
  {
    j = i = -1;
    if (D[p] != NDG_MAX)
      F[p] = NDG_MAX;
    else
    {                                     /* c'est un point objet */
      for (k = 0; k < 8; k += incr_vois)  /* cherche deux voisins i, j */
      {
        q = voisin(p, k, rs, N);
        if ((q != -1) && D[q]) 
        {
          if (i == -1) i = q; else j = q;
        }
      } /* for k... */
      if (j != -1)                        /* on a trouve deux voisins */
      {
        D[p] = NDG_MAX - 1;               /* marque les points initiaux */  
        D[i] = NDG_MAX - 1;
        D[j] = NDG_MAX - 1;
        LifoPush(LIFO, i);
        n = 0;
        while (!LifoVide(LIFO) && (n < nbpoints))     /* parcourt la branche de i */
        {
          n++;
          q = LifoPop(LIFO);
          for (k = 0; k < 8; k += incr_vois)
          {
            r = voisin(q, k, rs, N);
            if ((r != -1) && (D[r] == NDG_MAX)) 
	    {
              LifoPush(LIFO, r);
              D[r] = NDG_MAX - 1;
            }
          } /* for k */
          if (LifoVide(LIFO) || (n == nbpoints)) i = r;
        } /* while (!LifoVide(LIFO)) */
        LifoPush(LIFO, j);
        n = 0;
        while (!LifoVide(LIFO) && (n < nbpoints))     /* parcourt la branche de j */
        {
          n++;
          q = LifoPop(LIFO);
          for (k = 0; k < 8; k += incr_vois)
          {
            r = voisin(q, k, rs, N);
            if ((r != -1) && (D[r] == NDG_MAX)) 
	    {
              LifoPush(LIFO, r);
              D[r] = NDG_MAX - 1;
            }
          } /* for k */
          if (LifoVide(LIFO) || (n == nbpoints)) j = r;
        } /* while (!LifoVide(LIFO)) */

        LifoPush(LIFO, p);
        while (!LifoVide(LIFO))          /* re-parcourt pour demarquer */
        {                                /* (necessaire) */
          q = LifoPop(LIFO);
          D[q] = NDG_MAX;
          for (k = 0; k < 8; k += incr_vois)
          {
            r = voisin(q, k, rs, N);
            if ((r != -1) && (D[r] == NDG_MAX - 1)) LifoPush(LIFO, r);
          } /* for k */
        } /* while (!LifoVide(LIFO)) */

        if (normale)
          F[p] = (uint8_t)((int32_t)(90.0 + calcangle(i, j, rs) * 180 / M_PI) % 180);
        else
          F[p] = (uint8_t)((int32_t)(calcangle(i, j, rs) * 180 / M_PI) % 180);

      } /* if (j != -1) */
    } /* if (D[p] == NDG_MAX) */
  } /* for (p = 0; p < N; p++) */

  free(D);
  return 1;
}



