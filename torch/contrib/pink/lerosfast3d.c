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
/* operateur d'erosion numerique 3d par un element structurant de taille inferieure a 3x3x3 */
/* Michel Couprie - juillet 1997 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <lerosfast3d.h>

/* ==================================== */
int32_t lerosfast3d_lerosfast3d(struct xvimage *f, uint8_t *mask)
/* mask : masque du 26-voisinage representant l'element structurant */
/* ==================================== */
{
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t d = depth(f);            /* nb plans */
  int32_t n = rs * cs;             /* taille plan */
  int32_t N = n * d;               /* taille image */
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                     /* image de travail */
  uint8_t inf;

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"lerosfast3d_lerosfast3d() : malloc failed for H\n");
      return(0);
  }
  memcpy(H, F, N);

  for (x = 0; x < N; x++)
  {
    inf = H[x];
    for (k = 0; k < 26; k += 1)
    {
      if (mask[k])                      /* element structurant */
      {
        y = voisin26(x, k, rs, n, N);
        if ((y != -1) && (H[y] < inf)) inf = H[y];
      }
    } /* for k */
    F[x] = inf;
  }

  free(H);
  return 1;
}
