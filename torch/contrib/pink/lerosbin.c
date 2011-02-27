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
/* operateur d'erosion binaire par un element structurant de taille quelconque */
/* ATTENTION: l'element structurant doit etre CONNEXE et REFLEXIF */
/* Michel Couprie - avril 1998 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <lerosbin.h>

/* ==================================== */
int32_t lerosbin_lerosbin(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc)
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees de l'origine de l'element structurant */
/* ==================================== */
{
  int32_t x, y, z, w;              /* index muet de pixel */
  register int32_t i, j;           /* index muet */
  register int32_t k, l;           /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  int32_t rsm = rowsize(m);        /* taille ligne masque */
  int32_t csm = colsize(m);        /* taille colonne masque */
  int32_t Nm = rsm * csm;
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                    /* image de travail */
  uint8_t sup;
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  int32_t c;
  char *name1, *name2, *name;
  int32_t frontiere;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "lerosbin_lerosbin: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  if (!M[yc * rsm + xc]) /* l'element structurant N'est PAS reflexif */
  {
    fprintf(stderr, "lerosbin_lerosbin: l'element structurant doit etre reflexif\n");
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"lerosbin_lerosbin() : malloc failed for H\n");
     return(0);
  }

  memcpy(H, F, N);

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {  
     fprintf(stderr,"lerosbin_lerosbin() : malloc failed for tab_es\n");
     return(0);
  }

  k = 0;
  for (j = 0; j < csm; j += 1)
    for (i = 0; i < rsm; i += 1)
      if (M[j * rsm + i])
      {
         tab_es_x[k] = i;
         tab_es_y[k] = j;
         k += 1;
      }

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    /* verifie si (x,y) est un point de la frontiere du fond */
    w = y * rs + x;
    if (!H[w])
    {
      frontiere = 0;
      for (k = 0; k < 8; k += 2) /* on cherche une frontiere 8-connexe */
      {
        z = voisin(w, k, rs, N);
        if ((z != -1) && (H[z])) { frontiere = 1; break; }
      } /* for k... */

      if (!frontiere) 
        F[w] = NDG_MIN;
      else
      {
        for (c = 0; c < nptb ; c += 1)
        {
          l = y + tab_es_y[c] - yc;
          k = x + tab_es_x[c] - xc; 
          if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs))
	    F[l * rs + k] = NDG_MIN;
        }
      }
    }
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  return 1;
}







