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
/* compte le nombre de voisins de chaque point dans une image binaire */
/* remarque : on peut operer directement dans l'image de depart ! */
/* Michel Couprie - juin 1998 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <lnbvois.h>

/* ==================================== */
int32_t lnbvois(
  struct xvimage * image,
  int32_t connex)
/* ==================================== */
{
  int32_t nvois, k;
  int32_t p, q;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */

  switch (connex)
  {
    case 4: 
      for (p = 0; p < N; p++)
        if (F[p])
        {
          nvois = 0;                /* compte le nombre de voisins non nuls */
          for (k = 0; k < 8; k += 2)
          {
            q = voisin(p, k, rs, N);
            if ((q != -1) && (F[q])) nvois++;
          }
          F[p] = nvois;
        }
      break;
    case 8: 
      for (p = 0; p < N; p++)
        if (F[p])
        {
          nvois = 0;                /* compte le nombre de voisins non nuls */
          for (k = 0; k < 8; k += 1)
          {
            q = voisin(p, k, rs, N);
            if ((q != -1) && (F[q])) nvois++;
          }
          F[p] = nvois;
        }
      break;
    case 6: 
      for (p = 0; p < N; p++)
        if (F[p])
        {
          nvois = 0;                /* compte le nombre de voisins non nuls */
          for (k = 0; k <= 10; k += 2)
          {
            q = voisin6(p, k, rs, ps, N);
            if ((q != -1) && (F[q])) nvois++;
          }
          F[p] = nvois;
        }
      break;
    case 18: 
      for (p = 0; p < N; p++)
        if (F[p])
        {
          nvois = 0;                /* compte le nombre de voisins non nuls */
          for (k = 0; k < 18; k += 1)
          {
            q = voisin18(p, k, rs, ps, N);
            if ((q != -1) && (F[q])) nvois++;
          }
          F[p] = nvois;
        }
      break;
    case 26: 
      for (p = 0; p < N; p++)
        if (F[p])
        {
          nvois = 0;                /* compte le nombre de voisins non nuls */
          for (k = 0; k < 26; k += 1)
          {
            q = voisin26(p, k, rs, ps, N);
            if ((q != -1) && (F[q])) nvois++;
          }
          F[p] = nvois;
        }
      break;
    default: 
      fprintf(stderr, "lnbvois: mauvaise connexite: %d\n", connex);
      return 0;
  } /* switch (connex) */
  return 1;
}
