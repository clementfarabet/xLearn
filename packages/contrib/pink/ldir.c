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
/* selectionne les points est, sud, ouest ou nord */
/* Michel Couprie - decembre 1998 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mccodimage.h>
#include <mcindic.h>
#include <ldir.h>

/* ==================================== */
int32_t ldir(struct xvimage * image1, int32_t dir)
/* ==================================== */
{
  int32_t i, v;
  uint8_t *pt1;
  int32_t rs, cs, N;

  if (depth(image1) != 1) 
  {
    fprintf(stderr, "ldir: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  rs = rowsize(image1);
  cs = colsize(image1);
  N = rs * cs;
  
  IndicsInit(N);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  pt1 = UCHARDATA(image1);
  for (i = 0; i < N; i++)
    if (pt1[i])
    {
      v = voisin(i, dir, rs, N);
      if ((v != -1) && (pt1[v] < pt1[i]))
         Set(i,0);
    }

  for (i = 0; i < N; i++) if (IsSet(i,0)) pt1[i] = NDG_MAX; else pt1[i] = NDG_MIN;

  IndicsTermine();
  return 1;
}





