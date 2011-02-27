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
/* modifie la valeur d'un point dans une image */
/* Michel Couprie - avril 2000 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mccodimage.h>
#include <lpoint.h>

/* ==================================== */
int32_t lpoint(struct xvimage * image1, int32_t x, int32_t y, int32_t z, float v)
/* ==================================== */
#undef F_NAME
#define F_NAME "lpoint"
{
  int32_t rs, cs, d, n;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  n = rs * cs;
  
  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    uint8_t *pt1 = UCHARDATA(image1);
    if ((x >= 0) && (x < rs) && (y >= 0) && (y < cs) && (z >= 0) && (z < d) &&
        (v >= NDG_MIN) && (v <= NDG_MAX))
      pt1[z * n + y * rs + x] = (uint8_t)v;
    else
    {
      fprintf(stderr, "%s : out of range\n", F_NAME);
      return 0;
    }
  }
  else
  if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    int32_t *pt1 = SLONGDATA(image1);
    if ((x >= 0) && (x < rs) && (y >= 0) && (y < cs) && (z >= 0) && (z < d) &&
        (v >= 0))
      pt1[z * n + y * rs + x] = (int32_t)v;
    else 
    {
      fprintf(stderr, "%s : out of range\n", F_NAME);
      return 0;
    }
  }
  else
  if (datatype(image1) == VFF_TYP_FLOAT)
  {
    float *pt1 = FLOATDATA(image1);
    if ((x >= 0) && (x < rs) && (y >= 0) && (y < cs) && (z >= 0) && (z < d))
      pt1[z * n + y * rs + x] = v;
    else 
    {
      fprintf(stderr, "%s : out of range\n", F_NAME);
      return 0;
    }
  }
  else
  {
    fprintf(stderr, "%s: bad data type\n", F_NAME);
    return 0;
  }

  return 1;
} // lpoint()

#define DEBUG
/* ==================================== */
void lshowpoint(struct xvimage * image1, int32_t x, int32_t y, int32_t z)
/* ==================================== */
#undef F_NAME
#define F_NAME "lshowpoint"
{
  int32_t rs, cs, ds, ps;

  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  ps = rs * cs;
  
  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    uint8_t *pt1 = UCHARDATA(image1);
#ifdef DEBUG
    printf("rs=%d cs=%d ds=%d type=byte x=%d y=%d z=%d\n", rs, cs, ds, x, y, z);
#endif
    printf("%d\n", pt1[z * ps + y * rs + x]);
  }
  else
  if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    int32_t *pt1 = SLONGDATA(image1);
#ifdef DEBUG
  printf("rs=%d cs=%d ds=%d type=long x=%d y=%d z=%d\n", rs, cs, ds, x, y, z);
#endif
    printf("%ld\n", (long int)pt1[z * ps + y * rs + x]);
  }
  else
  if (datatype(image1) == VFF_TYP_FLOAT)
  {
    float *pt1 = FLOATDATA(image1);
#ifdef DEBUG
  printf("rs=%d cs=%d ds=%d type=float x=%d y=%d z=%d\n", rs, cs, ds, x, y, z);
#endif
    printf("%g\n", pt1[z * ps + y * rs + x]);
  }
  else
  {
    fprintf(stderr, "%s: bad data type\n", F_NAME);
    return;
  }

} // lshowpoint()
