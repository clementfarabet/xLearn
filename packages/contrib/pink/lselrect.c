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
/* selection d'une sous-image de forme rectangulaire - les autres pixels sont mis a NDG_MIN */
/* Michel Couprie - juillet 1996 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mccodimage.h>

/* ==================================== */
int32_t lselrect(
  struct xvimage * image,
  int32_t x,
  int32_t y,
  int32_t w,
  int32_t h)
/* ==================================== */
{
  int32_t xx, yy;
  uint8_t *pt;
  int32_t rs, cs, N;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lhtkern: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  rs = image->row_size;
  cs = image->col_size;
  N = rs * cs;
  pt = UCHARDATA(image);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  for (yy = 0; yy < cs; yy++)
    for (xx = 0; xx < rs; xx++)
      if ((yy < y) || (yy > y + h) || (xx < x) || (xx > x + w))
        pt[yy * rs + xx] = NDG_MIN;
  return 1;
}
