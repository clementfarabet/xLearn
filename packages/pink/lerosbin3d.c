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
   Operateur d'erosion binaire 3D par un element structurant 
   de taille quelconque

   ATTENTION: l'element structurant doit etre CONNEXE et REFLEXIF

   Michel Couprie - decembre 1999 
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <lerosbin3d.h>

/* ==================================== */
int32_t lerosbin3d_lerosbin3d(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc, int32_t zc)
/* m : masque representant l'element structurant */
/* xc, yc, zc : coordonnees de l'origine de l'element structurant */
/* ==================================== */
{
  int32_t x, y, z, v, w;              /* index muet */
  register int32_t i, j, k, l, n, o;  /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t ds = depth(f);           /* nb plans */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t N = ps * ds;             /* taille image */
  int32_t rsm = rowsize(m);        /* taille ligne masque */
  int32_t csm = colsize(m);        /* taille colonne masque */
  int32_t dsm = depth(m);          /* nb plans masque */
  int32_t psm = rsm * csm;         /* taille plan masque */
  int32_t Nm = psm * dsm;          /* taille masque */
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                    /* image de travail */
  uint8_t sup;
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  int32_t *tab_es_z;               /* liste des coord. z des points de l'e.s. */
  int32_t c;
  int32_t frontiere;

  if (!M[zc * psm + yc * rsm + xc]) /* l'element structurant N'est PAS reflexif */
  {
    fprintf(stderr, "lerosbin3d_lerosbin3d: l'element structurant doit etre reflexif\n");
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"lerosbin3d_lerosbin3d() : malloc failed for H\n");
     return(0);
  }

  memcpy(H, F, N);

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_z = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL) || (tab_es_z == NULL))
  {  
     fprintf(stderr,"lerosbin3d_lerosbin3d() : malloc failed for tab_es\n");
     return(0);
  }

  l = 0;
  for (k = 0; k < dsm; k += 1)
    for (j = 0; j < csm; j += 1)
      for (i = 0; i < rsm; i += 1)
        if (M[k * psm + j * rsm + i])
        {
          tab_es_x[l] = i;
          tab_es_y[l] = j;
          tab_es_z[l] = k;
          l += 1;
        }

  for (z = 0; z < ds; z++)
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    /* verifie si (x,y,z) est un point de la frontiere du fond */
    w = z * ps + y * rs + x;
    if (!H[w])
    {
      frontiere = 0;
      for (l = 0; l <= 10; l += 2) /* parcourt les 6 voisins */
      {
        v = voisin6(w, l, rs, ps, N);
        if ((v != -1) && (H[v])) { frontiere = 1; break; }
      } /* for l... */

      if (!frontiere) 
        F[w] = NDG_MIN;
      else
      {
        for (c = 0; c < nptb ; c += 1)
        {
          o = z + tab_es_z[c] - zc;
          n = y + tab_es_y[c] - yc;
          l = x + tab_es_x[c] - xc; 
          if ((o >= 0) && (o < ds) && 
              (n >= 0) && (n < cs) && 
              (l >= 0) && (l < rs))
	    F[o * ps + n * rs + l] = NDG_MIN;
        }
      }
    }
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  free(tab_es_z);
  return 1;
}







