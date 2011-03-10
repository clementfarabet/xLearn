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
#define VERBOSE

/*
  Andre Vital Saude contributions to lcrop.c
*/


#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
/*
#include <lcrop.h>
*/
#include <avscrop.h>

/* =============================================================== */
struct xvimage *lautocropEnframe3d(struct xvimage *in, int32_t *xi, int32_t *yi, int32_t *zi, int32_t nboxes) 
/* =============================================================== */
/*
  Needed for economy of memory.

  nboxes is the number of boxes enframing

  byte3d
*/
#undef F_NAME
#define F_NAME "lautocropEnframe3d"
{
  int32_t rs, cs, n, N, d;
  int32_t x, y, z, w, h, p;
  int32_t xmin, xmax, ymin, ymax, zmin, zmax;
  struct xvimage * temp1;
  uint8_t *I;
  uint8_t *T1;

  if (datatype(in) !=  VFF_TYP_1_BYTE) {
    fprintf(stderr, "%s: crop not implemented for that datatype\n", F_NAME);
    return NULL;
  }

  rs = rowsize(in);
  cs = colsize(in);
  N = rs * cs;
  d = depth(in);
  I = UCHARDATA(in);
  xmin = rs; xmax = 0; 
  ymin = cs; ymax = 0; 
  zmin = d; zmax = 0; 
  for (z = 0; z < d; z++)
    for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
        if (I[z * N + y * rs + x])
        {
          if (z < zmin) zmin = z; if (z > zmax) zmax = z;
          if (y < ymin) ymin = y; if (y > ymax) ymax = y;
          if (x < xmin) xmin = x; if (x > xmax) xmax = x;
        }

  //H-domain border touching
  if ((xmin <= 1) || (ymin <= 1) || (zmin <= 1) || (xmax >= rs-2) || (ymax >= cs-2) || (zmax >= d-2)) {
    fprintf(stderr, "%s: object touching border\n", F_NAME);
    return NULL;
  }

  w = xmax - xmin + 1;
  h = ymax - ymin + 1;
  p = zmax - zmin + 1;

  //is image is all black?
  if ((w <= 0) && (h <= 0) && (p <= 0)) return NULL;

#ifdef VERBOSE
  printf("Croping at:\nxmin=%d, ymin=%d, zmin=%d\nw=%d,h=%d,p=%d\n", xmin, ymin, zmin, w, h, p);
  fflush(stdout);
#endif

  temp1 = allocimage(NULL, w+nboxes+nboxes, h+nboxes+nboxes, p+nboxes+nboxes, VFF_TYP_1_BYTE);
  if (temp1 == NULL)
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return NULL;
  }
 
  //enframe
  xmin-=nboxes; ymin-=nboxes; zmin-=nboxes;
  *xi = xmin; *yi = ymin; *zi = zmin;

  T1 = UCHARDATA(temp1);
  n = (w+nboxes+nboxes) * (h+nboxes+nboxes);
  for (z = 2; z < p+nboxes; z++)
    for (y = 2; y < h+nboxes; y++)
      for (x = 2; x < w+nboxes; x++)
        T1[z*n + y*(w+nboxes+nboxes) + x] = I[((zmin+z)*N) + ((ymin+y)*rs) + xmin+x];

  return temp1;
}


