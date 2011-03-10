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
   Decale l'image des offsets ox, oy, oz (peuvent etre < 0) - 
   mode 0: les points ejectes hors du cadre sont perdus (defaut)
   mode 1: il y a rotation torique pour les points ejectes hors du cadre 

   Michel Couprie - juillet 1996

   Modif janvier 2000 : mode 0
   Modif novembre 2002 : 3D
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcutil.h>

/* ==================================== */
int32_t loffset(
  struct xvimage * image,
  int32_t ox,
  int32_t oy,
  int32_t oz,
  int32_t mode)
/* ==================================== */
{
  struct xvimage * temp;
  int32_t x, y, z, xx, yy, zz;
  uint8_t *pti;
  uint8_t *ptt;
  int32_t rs, cs, ds, ps, N;

  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  ps = rs * cs;
  N = ps * ds;

  temp = copyimage(image);
  if (temp == NULL)
  {
    fprintf(stderr, "loffset: copyimage failed\n");
    return 0;
  }

  pti = UCHARDATA(image);
  ptt = UCHARDATA(temp);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  printf("%d %d %d %d %d %d %d\n", rs, cs, ds, ox, oy, oz, mode);

  switch (mode)
  {
    case 0:
      memset(pti, 0, N);
      for (zz = 0; zz < ds; zz++)
        for (yy = 0; yy < cs; yy++)
          for (xx = 0; xx < rs; xx++)
          {
            z = zz - oz;
            y = yy - oy;
            x = xx - ox;
            if ((z >= 0) && (y >= 0) && (x >= 0) && (z < ds) && (y < cs) && (x < rs))
              pti[zz*ps + yy*rs + xx] = ptt[z*ps + y*rs + x];
          }
      break;
    case 1:
      memset(pti, 0, N);
      for (zz = 0; zz < ds; zz++)
        for (yy = 0; yy < cs; yy++)
          for (xx = 0; xx < rs; xx++)
          {
            z = zz - oz;
            if (z < 0) z += ds; else if (z >= ds) z -= ds;
            y = yy - oy;
            if (y < 0) y += cs; else if (y >= cs) y -= cs;
            x = xx - ox;
            if (x < 0) x += rs; else if (x >= rs) x -= rs;
            pti[zz*ps + yy*rs + xx] = ptt[z*ps + y*rs + x];
          }
      break;
    default:
      fprintf(stderr, "loffset: bad mode %d\n", mode);
      return 0;
  } /* switch (mode) */
  freeimage(temp);
  return 1;
}
