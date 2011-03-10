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
/* operateur de dilatation numerique par un element structurant de taille inferieure a 3x3 */
/* UNIQUEMENT POUR DES ELEMENTS STRUCTURANTS REFLEXIFS */
/* Michel Couprie - juillet 1996 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcutil.h>

/* ==================================== */
int32_t ldilatfast_ldilatfast(struct xvimage *f, uint8_t *mask) 
/* mask : masque du 8-voisinage representant l'element structurant */
/* ==================================== */
{
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                     /* image de travail */
  uint8_t sup;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "ldilatfast_ldilatfast: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"ldilatfast_ldilatfast() : malloc failed for H\n");
      return(0);
  }
  for (x = 0; x < N; x++) H[x] = F[x];

  for (x = 0; x < N; x++)
  {
    sup = H[x];                         /* l'ES est reflexif */
    for (k = 0; k < 8; k += 1)
    {
      if (mask[k])                      /* element structurant */
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (H[y] > sup)) sup = H[y];
      }
    } /* for k */
    F[x] = sup;
  }

  free(H);
  return 1;
}
