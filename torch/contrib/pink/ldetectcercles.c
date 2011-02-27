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
/* Michel Couprie - mars 1999 */
/* methode : simplification de la transformee de Hough
   pour des cercles de rayon donne
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <lbresen.h>
#include <ldetectcercles.h>

/* ==================================== */
int32_t ldetectcercles(struct xvimage *f, int32_t rayon)
/* rayon : rayon des cercles a detecter */
/* ==================================== */
{
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t i, j;                    /* index muet */
  register int32_t k, l;                    /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  int32_t rsm = 2 * rayon + 1;     /* taille ligne masque */
  int32_t csm = 2 * rayon + 1;     /* taille colonne masque */
  int32_t Nm = rsm * csm;
  struct xvimage *m;
  uint8_t *M;
  uint8_t *F = UCHARDATA(f);
  uint32_t *H;            /* image de travail (accumulateur) */
  uint32_t tmp;
  int32_t nptb;                    /* nombre de points du cercle */
  int32_t *tab_es_x;               /* liste des coord. x des points du cercle */
  int32_t *tab_es_y;               /* liste des coord. y des points du cercle */
  int32_t c;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "ldetectcercles: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  H = (uint32_t *)calloc(N, sizeof(int32_t));
  if (H == NULL)
  {  
     fprintf(stderr,"ldetectcercles() : calloc failed for H\n");
     return(0);
  }

  /* genere un cercle dans une imagette m */
  m = allocimage(NULL, rsm, csm, 1, VFF_TYP_1_BYTE);
  if (m == NULL)
  {  
     fprintf(stderr,"ldetectcercles() : allocimage failed\n");
     return(0);
  }
  M = UCHARDATA(m);
  memset(M, 0, Nm);
  lellipse(M, rsm, csm, (int32_t)rayon, 0, 0, (int32_t)rayon, (int32_t)rayon, (int32_t)rayon);

  /* compte le nombre de points du cercle */
  nptb = 0;             
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  /* dresse la liste des points du cercle */
  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {  
     fprintf(stderr,"ldetectcercles() : malloc failed for tab_es\n");
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

  /* pour tous les points p de l'image, renforce les points de l'accumulateur qui sont a
     une distance 'rayon' du point p */
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    tmp = (uint32_t)F[y * rs + x];
    for (c = 0; c < nptb ; c += 1)
    {
      l = y + tab_es_y[c] - rayon;
      k = x + tab_es_x[c] - rayon; 
      if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs))
	H[l * rs + k] += tmp;
    }
  }

  /* normalisation : le max theorique est obtenu pour un cercle blanc complet */
  for (x = 0; x < N; x++) F[x] = (uint8_t)(H[x] / nptb);

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  freeimage(m);

  return 1;
}
