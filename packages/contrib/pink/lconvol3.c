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
/* convolution par un masque 3x3 */
/* Michel Couprie - juillet 1996 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>

/* ==================================== */
int32_t lconvol3(struct xvimage * image, struct xvimage * mask, int32_t normalise)
/* ==================================== */
{
  int32_t i, x, y;
  uint8_t *img;
  char *msk;                        /* LE MASQUE EST UNE IMAGE SIGNEE */
  int32_t rs, cs, N, sum;
  uint8_t *trv;                     /* image de travail */

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lconvol3: cette version ne traite pas les images volumiques\n");
    exit(0);
  }
  rs = image->row_size;
  cs = image->col_size;
  N = rs * cs;
  trv = (uint8_t *)calloc(1,N*sizeof(char));
  if (trv == NULL)
  {   fprintf(stderr,"lconvol3() : malloc failed for trv\n");
      return(0);
  }

  /* ---------------------------------------------------------- */
  /* recupere et verifie les operandes */
  /* ---------------------------------------------------------- */

  if ((mask->row_size != 3) || (mask->col_size != 3))
  {
    fprintf(stderr, "convol3: bad mask size\n");
    return 0;
  }
    
  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  msk = (char *)(UCHARDATA(mask));
  img = UCHARDATA(image);
  for (i = 0; i < N; i++) trv[i] = img[i];

  sum = 0;
  for (i = 0; i < 9; i++) sum += msk[i];

  for (y = 1; y < cs-1; y++)
    for (x = 1; x < rs-1; x++)
    {
      int32_t tmp = *(trv + y*rs + x) * msk[4];
      tmp += *(trv + (y-1)*rs + x-1) * msk[0];
      tmp += *(trv + (y-1)*rs + x)   * msk[1];
      tmp += *(trv + (y-1)*rs + x+1) * msk[2];
      tmp += *(trv + y*rs + x-1)     * msk[3];
      tmp += *(trv + y*rs + x+1)     * msk[5];
      tmp += *(trv + (y+1)*rs + x-1) * msk[6];
      tmp += *(trv + (y+1)*rs + x)   * msk[7];
      tmp += *(trv + (y+1)*rs + x+1) * msk[8];
      if (normalise) tmp = tmp / sum;
      if (tmp < 0) tmp = 0; else if (tmp > 255) tmp = 255;
      *(img + y*rs + x) = (uint8_t)tmp;
    }

  free(trv);
  return 1;
}





