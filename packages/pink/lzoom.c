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
/* Michel Couprie - decembre 1996 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#ifdef HP
#define _INCLUDE_XOPEN_SOURCE
#endif
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcutil.h>
#include <lzoom.h>

/* ==================================== */
int32_t lzoomoutbyte(
  struct xvimage * in,
  struct xvimage ** out,
  double zoomx,
  double zoomy,
  double zoomz)
/* ==================================== */
#undef F_NAME
#define F_NAME "lzoomoutbyte"
{
  index_t x, y, z, x1, y1, z1, xn, yn, zn, xx, yy, zz;
  uint8_t *ptin;
  uint8_t *ptout;
  index_t rs, cs, ds, ps;
  index_t rs2, cs2, ds2, ps2;
  double kx, ky, kz, tmp, d, dx1, dxn, dy1, dyn, dz1, dzn, sigmad;

  rs = in->row_size;
  cs = in->col_size;
  ds = depth(in);
  ps = rs * cs;
  ptin = UCHARDATA(in);

  if ((zoomx <= 0.0) || (zoomx > 1.0) || 
      (zoomy <= 0.0) || (zoomy > 1.0) ||
      (zoomz <= 0.0) || (zoomz > 1.0))
  {  
    fprintf(stderr,"%s : bad zoom factor\n", F_NAME);
    return 0;
  }

  if (datatype(in) != VFF_TYP_1_BYTE)
  {  
    fprintf(stderr,"%s : bad data type\n", F_NAME);
    return 0;
  }

  rs2 = (index_t)(rs * zoomx); if (rs2 < 1) rs2 = 1;
  cs2 = (index_t)(cs * zoomy); if (cs2 < 1) cs2 = 1;
  ds2 = (index_t)(ds * zoomz); if (ds2 < 1) ds2 = 1;
  ps2 = rs2 * cs2;
  kx = 1.0 / zoomx;
  ky = 1.0 / zoomy;
  kz = 1.0 / zoomz;

  /* ---------------------------------------------------------- */
  /* alloue l'image resultat */
  /* ---------------------------------------------------------- */
  *out = allocimage(NULL, rs2, cs2, ds2, VFF_TYP_1_BYTE);

  if (*out == NULL)
  {   
    fprintf(stderr,"%s : allocimage failed\n", F_NAME);
    return 0;
  }
  ptout = (UCHARDATA(*out));

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if ((cs == 1) && (ds == 1))
  {
    /* pas efficace - a ameliorer */
    for (x = 0; x < rs2; x++)
    {
      tmp = 0.0;
      sigmad = 0.0;
      x1 = (index_t)(x * kx); dx1 = 1.0 - ((x * kx) - x1);
      xn = (index_t)((x+1) * kx); dxn = ((x+1) * kx) - xn; if (xn == rs) xn = rs-1;
      for (xx = x1; xx <= xn; xx++)
      {
	d = 1.0;
	if (xx == x1) d *= dx1; else if (xx == xn) d *= dxn;
	tmp += d * ptin[xx];
	sigmad += d;
      }
      ptout[x] = (uint8_t)(tmp / sigmad);
    }
  } // if ((cs == 1) && (ds == 1))
  else if (ds == 1)
  {
    /* pas efficace - a ameliorer */
    for (y = 0; y < cs2; y++)
    for (x = 0; x < rs2; x++)
    {
      tmp = 0.0;
      sigmad = 0.0;
      x1 = (index_t)(x * kx); dx1 = 1.0 - ((x * kx) - x1);
      xn = (index_t)((x+1) * kx); dxn = ((x+1) * kx) - xn; if (xn == rs) xn = rs-1;
      y1 = (index_t)(y * ky); dy1 = 1.0 - ((y * ky) - y1);
      yn = (index_t)((y+1) * ky); dyn = ((y+1) * ky) - yn; if (yn == cs) yn = cs-1;
      for (yy = y1; yy <= yn; yy++)
        for (xx = x1; xx <= xn; xx++)
	{
          d = 1.0;
          if (xx == x1) d *= dx1; else if (xx == xn) d *= dxn;
          if (yy == y1) d *= dy1; else if (yy == yn) d *= dyn;
          tmp += d * ptin[yy * rs + xx];
          sigmad += d;
	}
      ptout[y * rs2 + x] = (uint8_t)(tmp / sigmad);
    }
  } /* if (ds == 1) */
  else
  {
    /* pas efficace - a ameliorer */
    for (z = 0; z < ds2; z++)
    for (y = 0; y < cs2; y++)
    for (x = 0; x < rs2; x++)
    {
      tmp = 0.0;
      sigmad = 0.0;
      x1 = (index_t)(x * kx); dx1 = 1.0 - ((x * kx) - x1);
      xn = (index_t)((x+1) * kx); dxn = ((x+1) * kx) - xn; if (xn == rs) xn = rs-1;
      y1 = (index_t)(y * ky); dy1 = 1.0 - ((y * ky) - y1);
      yn = (index_t)((y+1) * ky); dyn = ((y+1) * ky) - yn; if (yn == cs) yn = cs-1;
      z1 = (index_t)(z * kz); dz1 = 1.0 - ((z * kz) - z1);
      zn = (index_t)((z+1) * kz); dzn = ((z+1) * kz) - zn; if (zn == ds) zn = ds-1;
      for (zz = z1; zz <= zn; zz++)
      for (yy = y1; yy <= yn; yy++)
      for (xx = x1; xx <= xn; xx++)
      {
        d = 1.0;
        if (xx == x1) d *= dx1; else if (xx == xn) d *= dxn;
        if (yy == y1) d *= dy1; else if (yy == yn) d *= dyn;
        if (zz == z1) d *= dz1; else if (zz == zn) d *= dzn;
        tmp += d * ptin[zz*ps + yy*rs + xx];
        sigmad += d;
      }
      ptout[z*ps2 + y*rs2 + x] = (uint8_t)(tmp / sigmad);
    }
  } /* if (ds != 1) */

  return 1;
} /* lzoomoutbyte() */

/* ==================================== */
int32_t lzoomoutlong(
  struct xvimage * in,
  struct xvimage ** out,
  double zoomx,
  double zoomy,
  double zoomz)
/* ==================================== */
#undef F_NAME
#define F_NAME "lzoomoutlong"
{
  index_t x, y, z, x1, y1, z1, xn, yn, zn, xx, yy, zz;
  int32_t *ptin;
  int32_t *ptout;
  index_t rs, cs, ds, ps;
  index_t rs2, cs2, ds2, ps2;
  double kx, ky, kz, tmp, d, dx1, dxn, dy1, dyn, dz1, dzn, sigmad;

  rs = in->row_size;
  cs = in->col_size;
  ds = depth(in);
  ps = rs * cs;
  ptin = SLONGDATA(in);

  if ((zoomx <= 0.0) || (zoomx > 1.0) || 
      (zoomy <= 0.0) || (zoomy > 1.0) ||
      (zoomz <= 0.0) || (zoomz > 1.0))
  {  
    fprintf(stderr,"%s : bad zoom factor\n", F_NAME);
    return 0;
  }

  if (datatype(in) != VFF_TYP_4_BYTE)
  {  
    fprintf(stderr,"%s : bad data type\n", F_NAME);
    return 0;
  }

  rs2 = (index_t)(rs * zoomx); if (rs2 < 1) rs2 = 1;
  cs2 = (index_t)(cs * zoomy); if (cs2 < 1) cs2 = 1;
  ds2 = (index_t)(ds * zoomz); if (ds2 < 1) ds2 = 1;
  ps2 = rs2 * cs2;
  kx = 1.0 / zoomx;
  ky = 1.0 / zoomy;
  kz = 1.0 / zoomz;

  /* ---------------------------------------------------------- */
  /* alloue l'image resultat */
  /* ---------------------------------------------------------- */
  *out = allocimage(NULL, rs2, cs2, ds2, VFF_TYP_4_BYTE);

  if (*out == NULL)
  {   
    fprintf(stderr,"%s : allocimage failed\n", F_NAME);
    return 0;
  }
  ptout = SLONGDATA(*out);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (ds == 1)
  {
    /* pas efficace - a ameliorer */
    for (y = 0; y < cs2; y++)
    for (x = 0; x < rs2; x++)
    {
      tmp = 0.0;
      sigmad = 0.0;
      x1 = (index_t)(x * kx); dx1 = 1.0 - ((x * kx) - x1);
      xn = (index_t)((x+1) * kx); dxn = ((x+1) * kx) - xn; if (xn == rs) xn = rs-1;
      y1 = (index_t)(y * ky); dy1 = 1.0 - ((y * ky) - y1);
      yn = (index_t)((y+1) * ky); dyn = ((y+1) * ky) - yn; if (yn == cs) yn = cs-1;
      for (yy = y1; yy <= yn; yy++)
        for (xx = x1; xx <= xn; xx++)
	{
          d = 1.0;
          if (xx == x1) d *= dx1; else if (xx == xn) d *= dxn;
          if (yy == y1) d *= dy1; else if (yy == yn) d *= dyn;
          tmp += d * ptin[yy * rs + xx];
          sigmad += d;
	}
      ptout[y * rs2 + x] = (uint8_t)(tmp / sigmad);
    }
  } /* if (ds == 1) */
  else
  {
    /* pas efficace - a ameliorer */
    for (z = 0; z < ds2; z++)
    for (y = 0; y < cs2; y++)
    for (x = 0; x < rs2; x++)
    {
      tmp = 0.0;
      sigmad = 0.0;
      x1 = (index_t)(x * kx); dx1 = 1.0 - ((x * kx) - x1);
      xn = (index_t)((x+1) * kx); dxn = ((x+1) * kx) - xn; if (xn == rs) xn = rs-1;
      y1 = (index_t)(y * ky); dy1 = 1.0 - ((y * ky) - y1);
      yn = (index_t)((y+1) * ky); dyn = ((y+1) * ky) - yn; if (yn == cs) yn = cs-1;
      z1 = (index_t)(z * kz); dz1 = 1.0 - ((z * kz) - z1);
      zn = (index_t)((z+1) * kz); dzn = ((z+1) * kz) - zn; if (zn == ds) zn = ds-1;
      for (zz = z1; zz <= zn; zz++)
      for (yy = y1; yy <= yn; yy++)
      for (xx = x1; xx <= xn; xx++)
      {
        d = 1.0;
        if (xx == x1) d *= dx1; else if (xx == xn) d *= dxn;
        if (yy == y1) d *= dy1; else if (yy == yn) d *= dyn;
        if (zz == z1) d *= dz1; else if (zz == zn) d *= dzn;
        tmp += d * ptin[zz*ps + yy*rs + xx];
        sigmad += d;
      }
      ptout[z*ps2 + y*rs2 + x] = (uint8_t)(tmp / sigmad);
    }
  } /* if (ds != 1) */

  return 1;
} /* lzoomoutlong() */

/* ==================================== */
int32_t lzoomoutfloat(
  struct xvimage * in,
  struct xvimage ** out,
  double zoomx,
  double zoomy,
  double zoomz)
/* ==================================== */
#undef F_NAME
#define F_NAME "lzoomoutfloat"
{
  index_t x, y, z, x1, y1, z1, xn, yn, zn, xx, yy, zz;
  float *ptin;
  float *ptout;
  index_t rs, cs, ds, ps;
  index_t rs2, cs2, ds2, ps2;
  double kx, ky, kz, tmp, d, dx1, dxn, dy1, dyn, dz1, dzn, sigmad;

  rs = in->row_size;
  cs = in->col_size;
  ds = depth(in);
  ps = rs * cs;
  ptin = FLOATDATA(in);

  if ((zoomx <= 0.0) || (zoomx > 1.0) || 
      (zoomy <= 0.0) || (zoomy > 1.0) ||
      (zoomz <= 0.0) || (zoomz > 1.0))
  {  
    fprintf(stderr,"%s : bad zoom factor\n", F_NAME);
    return 0;
  }

  if (datatype(in) != VFF_TYP_FLOAT)
  {  
    fprintf(stderr,"%s : bad data type\n", F_NAME);
    return 0;
  }

  rs2 = (index_t)(rs * zoomx); if (rs2 < 1) rs2 = 1;
  cs2 = (index_t)(cs * zoomy); if (cs2 < 1) cs2 = 1;
  ds2 = (index_t)(ds * zoomz); if (ds2 < 1) ds2 = 1;
  ps2 = rs2 * cs2;
  kx = 1.0 / zoomx;
  ky = 1.0 / zoomy;
  kz = 1.0 / zoomz;

  /* ---------------------------------------------------------- */
  /* alloue l'image resultat */
  /* ---------------------------------------------------------- */
  *out = allocimage(NULL, rs2, cs2, ds2, VFF_TYP_FLOAT);

  if (*out == NULL)
  {   
    fprintf(stderr,"%s : allocimage failed\n", F_NAME);
    return 0;
  }
  ptout = FLOATDATA(*out);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (ds == 1)
  {
    /* pas efficace - a ameliorer */
    for (y = 0; y < cs2; y++)
    for (x = 0; x < rs2; x++)
    {
      tmp = 0.0;
      sigmad = 0.0;
      x1 = (index_t)(x * kx); dx1 = 1.0 - ((x * kx) - x1);
      xn = (index_t)((x+1) * kx); dxn = ((x+1) * kx) - xn; if (xn == rs) xn = rs-1;
      y1 = (index_t)(y * ky); dy1 = 1.0 - ((y * ky) - y1);
      yn = (index_t)((y+1) * ky); dyn = ((y+1) * ky) - yn; if (yn == cs) yn = cs-1;
      for (yy = y1; yy <= yn; yy++)
        for (xx = x1; xx <= xn; xx++)
	{
          d = 1.0;
          if (xx == x1) d *= dx1; else if (xx == xn) d *= dxn;
          if (yy == y1) d *= dy1; else if (yy == yn) d *= dyn;
          tmp += d * ptin[yy * rs + xx];
          sigmad += d;
	}
      ptout[y * rs2 + x] = (float)(tmp / sigmad);
    }
  } /* if (ds == 1) */
  else
  {
    /* pas efficace - a ameliorer */
    for (z = 0; z < ds2; z++)
    for (y = 0; y < cs2; y++)
    for (x = 0; x < rs2; x++)
    {
      tmp = 0.0;
      sigmad = 0.0;
      x1 = (index_t)(x * kx); dx1 = 1.0 - ((x * kx) - x1);
      xn = (index_t)((x+1) * kx); dxn = ((x+1) * kx) - xn; if (xn == rs) xn = rs-1;
      y1 = (index_t)(y * ky); dy1 = 1.0 - ((y * ky) - y1);
      yn = (index_t)((y+1) * ky); dyn = ((y+1) * ky) - yn; if (yn == cs) yn = cs-1;
      z1 = (index_t)(z * kz); dz1 = 1.0 - ((z * kz) - z1);
      zn = (index_t)((z+1) * kz); dzn = ((z+1) * kz) - zn; if (zn == ds) zn = ds-1;
      for (zz = z1; zz <= zn; zz++)
      for (yy = y1; yy <= yn; yy++)
      for (xx = x1; xx <= xn; xx++)
      {
        d = 1.0;
        if (xx == x1) d *= dx1; else if (xx == xn) d *= dxn;
        if (yy == y1) d *= dy1; else if (yy == yn) d *= dyn;
        if (zz == z1) d *= dz1; else if (zz == zn) d *= dzn;
        tmp += d * ptin[zz*ps + yy*rs + xx];
        sigmad += d;
      }
      ptout[z*ps2 + y*rs2 + x] = (float)(tmp / sigmad);
    }
  } /* if (ds != 1) */

  return 1;
} /* lzoomoutfloat() */

/* ==================================== */
int32_t lzoominbyte(
  struct xvimage * in,
  struct xvimage ** out,
  double zoomx,
  double zoomy,
  double zoomz)
/* ==================================== */
#undef F_NAME
#define F_NAME "lzoominbyte"
{
  index_t x2, y2, z2, xs, ys, zs, xi, yi, zi;
  double x, y, z;
  uint8_t *ptin;
  uint8_t *ptout;
  index_t rs, cs, ds, ps;
  index_t rs2, cs2, ds2, ps2, N2;

  rs = in->row_size;
  cs = in->col_size;
  ds = depth(in);
  ps = rs * cs;
  ptin = UCHARDATA(in);

  if ((zoomx < 1.0) || (zoomy < 1.0) || (zoomz < 1.0))
  {   
    fprintf(stderr,"%s : bad zoom factor - must be >= 1\n", F_NAME);
    return 0;
  }

  if (datatype(in) != VFF_TYP_1_BYTE)
  {  
    fprintf(stderr,"%s : bad data type\n", F_NAME);
    return 0;
  }

  rs2 = (index_t)(rs * zoomx);
  cs2 = (index_t)(cs * zoomy);
  if (ds == 1) ds2 = 1; else ds2 = (index_t)(ds * zoomz);
  ps2 = rs2 * cs2;
  N2 = ps2 * ds2;

  /* ---------------------------------------------------------- */
  /* alloue l'image resultat */
  /* ---------------------------------------------------------- */
  *out = allocimage(NULL, rs2, cs2, ds2, VFF_TYP_1_BYTE);

  if (*out == NULL)
  {   fprintf(stderr,"%s : allocimage failed\n", F_NAME);
      return 0;
  }
  ptout = UCHARDATA(*out);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  memset(ptout, 0, N2);
  if ((cs == 1) && (ds == 1))
  {
    double f;
    for (x2 = 0; x2 < rs2; x2++)
    {
      x = x2 / zoomx;
      xi = (index_t)floor(x); xs = xi + 1;
      if ((xi >= 0) && (xs < rs))
      {
	f = (x - xi) * ptin[xs] + (xs - x) * ptin[xi];
	ptout[x2] = arrondi(f);
      }
    } // for x2
  }
  else if (ds == 1)
  {
    double f, f1, f2;
    for (y2 = 0; y2 < cs2; y2++)
    {
      for (x2 = 0; x2 < rs2; x2++)
      {
        x = x2 / zoomx;
        y = y2 / zoomy;
        xi = (index_t)floor(x); xs = xi + 1;
        yi = (index_t)floor(y); ys = yi + 1;
        if ((xi >= 0) && (yi >= 0) && (xs < rs) && (ys < cs))
        {
          f1 = (x - xi) * ptin[yi*rs + xs] + (xs - x) * ptin[yi*rs + xi];
          f2 = (x - xi) * ptin[ys*rs + xs] + (xs - x) * ptin[ys*rs + xi];
          f = (y - yi) * f2 + (ys - y) * f1;
          ptout[y2*rs2 + x2] = arrondi(f);
        }
      } // for x2
    } // for y2
  } /* if (ds == 1) */
  else
  {
    double f, fzi, fzs, fziyi, fziys, fzsyi, fzsys;
    for (z2 = 0; z2 < ds2; z2++)
    {
      for (y2 = 0; y2 < cs2; y2++)
      {
        for (x2 = 0; x2 < rs2; x2++)
        {
          x = x2 / zoomx;
          y = y2 / zoomy;
          z = z2 / zoomz;
          xi = (index_t)floor(x); xs = xi + 1;
          yi = (index_t)floor(y); ys = yi + 1;
          zi = (index_t)floor(z); zs = zi + 1;
          if ((xi >= 0) && (yi >= 0) && (zi >= 0) && (xs < rs) && (ys < cs) && (zs < ds))
          {
            fziyi = (x - xi) * ptin[zi*ps + yi*rs + xs] + (xs - x) * ptin[zi*ps + yi*rs + xi];
            fziys = (x - xi) * ptin[zi*ps + ys*rs + xs] + (xs - x) * ptin[zi*ps + ys*rs + xi];
            fzsyi = (x - xi) * ptin[zs*ps + yi*rs + xs] + (xs - x) * ptin[zs*ps + yi*rs + xi];
            fzsys = (x - xi) * ptin[zs*ps + ys*rs + xs] + (xs - x) * ptin[zs*ps + ys*rs + xi];
            fzi = (y - yi) * fziys + (ys - y) * fziyi;
            fzs = (y - yi) * fzsys + (ys - y) * fzsyi;
            f = (z - zi) * fzs + (zs - z) * fzi;
            ptout[z2*ps2 + y2*rs2 + x2] = arrondi(f);
          }
        } // for x2
      } // for y2
    } // for z2
  } /* if (ds != 1) */

  return 1;
} /* lzoominbyte() */

/* ==================================== */
int32_t lzoominlong(
  struct xvimage * in,
  struct xvimage ** out,
  double zoomx,
  double zoomy,
  double zoomz)
/* ==================================== */
#undef F_NAME
#define F_NAME "lzoominlong"
{
  index_t x2, y2, z2, xs, ys, zs, xi, yi, zi;
  double x, y, z;
  int32_t *ptin;
  int32_t *ptout;
  index_t rs, cs, ds, ps;
  index_t rs2, cs2, ds2, ps2, N2;

  rs = in->row_size;
  cs = in->col_size;
  ds = depth(in);
  ps = rs * cs;
  ptin = SLONGDATA(in);

  if ((zoomx < 1.0) || (zoomy < 1.0) || (zoomz < 1.0))
  {   
    fprintf(stderr,"%s : bad zoom factor - must be >= 1\n", F_NAME);
    return 0;
  }

  if (datatype(in) != VFF_TYP_4_BYTE)
  {  
    fprintf(stderr,"%s : bad data type\n", F_NAME);
    return 0;
  }

  rs2 = (index_t)(rs * zoomx);
  cs2 = (index_t)(cs * zoomy);
  if (ds == 1) ds2 = 1; else ds2 = (index_t)(ds * zoomz);
  ps2 = rs2 * cs2;
  N2 = ps2 * ds2;

  /* ---------------------------------------------------------- */
  /* alloue l'image resultat */
  /* ---------------------------------------------------------- */
  *out = allocimage(NULL, rs2, cs2, ds2, VFF_TYP_4_BYTE);

  if (*out == NULL)
  {   fprintf(stderr,"%s : allocimage failed\n", F_NAME);
      return 0;
  }
  ptout = SLONGDATA(*out);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  memset(ptout, 0, N2);
  if (ds == 1)
  {
    double f, f1, f2;
    for (y2 = 0; y2 < cs2; y2++)
    {
      for (x2 = 0; x2 < rs2; x2++)
      {
        x = x2 / zoomx;
        y = y2 / zoomy;
        xi = (index_t)floor(x); xs = xi + 1;
        yi = (index_t)floor(y); ys = yi + 1;
        if ((xi >= 0) && (yi >= 0) && (xs < rs2) && (ys < cs2))
        {
          f1 = (x - xi) * ptin[yi*rs + xs] + (xs - x) * ptin[yi*rs + xi];
          f2 = (x - xi) * ptin[ys*rs + xs] + (xs - x) * ptin[ys*rs + xi];
          f = (y - yi) * f2 + (ys - y) * f1;
          ptout[y2*rs2 + x2] = arrondi(f);
        }
      } // for x2
    } // for y2

  } /* if (ds == 1) */
  else
  {
    double f, fzi, fzs, fziyi, fziys, fzsyi, fzsys;
    for (z2 = 0; z2 < ds2; z2++)
    {
      for (y2 = 0; y2 < cs2; y2++)
      {
        for (x2 = 0; x2 < rs2; x2++)
        {
          x = x2 / zoomx;
          y = y2 / zoomy;
          z = z2 / zoomz;
          xi = (index_t)floor(x); xs = xi + 1;
          yi = (index_t)floor(y); ys = yi + 1;
          zi = (index_t)floor(z); zs = zi + 1;
          if ((xi >= 0) && (yi >= 0) && (zi >= 0) && (xs < rs2) && (ys < cs2) && (zs < ds2))
          {
            fziyi = (x - xi) * ptin[zi*ps + yi*rs + xs] + (xs - x) * ptin[zi*ps + yi*rs + xi];
            fziys = (x - xi) * ptin[zi*ps + ys*rs + xs] + (xs - x) * ptin[zi*ps + ys*rs + xi];
            fzsyi = (x - xi) * ptin[zs*ps + yi*rs + xs] + (xs - x) * ptin[zs*ps + yi*rs + xi];
            fzsys = (x - xi) * ptin[zs*ps + ys*rs + xs] + (xs - x) * ptin[zs*ps + ys*rs + xi];
            fzi = (y - yi) * fziys + (ys - y) * fziyi;
            fzs = (y - yi) * fzsys + (ys - y) * fzsyi;
            f = (z - zi) * fzs + (zs - z) * fzi;
            ptout[z2*ps2 + y2*rs2 + x2] = arrondi(f);
          }
        } // for x2
      } // for y2
    } // for z2
  } /* if (ds != 1) */

  return 1;
} /* lzoominlong() */

/* ==================================== */
int32_t lzoominfloat(
  struct xvimage * in,
  struct xvimage ** out,
  double zoomx,
  double zoomy,
  double zoomz)
/* ==================================== */
#undef F_NAME
#define F_NAME "lzoominfloat"
{
  index_t x2, y2, z2, xs, ys, zs, xi, yi, zi;
  double x, y, z;
  float *ptin;
  float *ptout;
  index_t rs, cs, ds, ps;
  index_t rs2, cs2, ds2, ps2, N2;

  rs = in->row_size;
  cs = in->col_size;
  ds = depth(in);
  ps = rs * cs;
  ptin = FLOATDATA(in);

  if ((zoomx < 1.0) || (zoomy < 1.0) || (zoomz < 1.0))
  {   
    fprintf(stderr,"%s : bad zoom factor - must be >= 1\n", F_NAME);
    return 0;
  }

  if (datatype(in) != VFF_TYP_FLOAT)
  {  
    fprintf(stderr,"%s : bad data type\n", F_NAME);
    return 0;
  }

  rs2 = (index_t)(rs * zoomx);
  cs2 = (index_t)(cs * zoomy);
  if (ds == 1) ds2 = 1; else ds2 = (index_t)(ds * zoomz);
  ps2 = rs2 * cs2;
  N2 = ps2 * ds2;

  /* ---------------------------------------------------------- */
  /* alloue l'image resultat */
  /* ---------------------------------------------------------- */
  *out = allocimage(NULL, rs2, cs2, ds2, VFF_TYP_FLOAT);

  if (*out == NULL)
  {   fprintf(stderr,"%s : allocimage failed\n", F_NAME);
      return 0;
  }
  ptout = FLOATDATA(*out);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  memset(ptout, 0, N2);
  if (ds == 1)
  {
    double f, f1, f2;
    for (y2 = 0; y2 < cs2; y2++)
    {
      for (x2 = 0; x2 < rs2; x2++)
      {
        x = x2 / zoomx;
        y = y2 / zoomy;
        xi = (index_t)floor(x); xs = xi + 1;
        yi = (index_t)floor(y); ys = yi + 1;
        if ((xi >= 0) && (yi >= 0) && (xs < rs2) && (ys < cs2))
        {
          f1 = (x - xi) * ptin[yi*rs + xs] + (xs - x) * ptin[yi*rs + xi];
          f2 = (x - xi) * ptin[ys*rs + xs] + (xs - x) * ptin[ys*rs + xi];
          f = (y - yi) * f2 + (ys - y) * f1;
          ptout[y2*rs2 + x2] = (float)f;
        }
      } // for x2
    } // for y2

  } /* if (ds == 1) */
  else
  {
    double f, fzi, fzs, fziyi, fziys, fzsyi, fzsys;
    for (z2 = 0; z2 < ds2; z2++)
    {
      for (y2 = 0; y2 < cs2; y2++)
      {
        for (x2 = 0; x2 < rs2; x2++)
        {
          x = x2 / zoomx;
          y = y2 / zoomy;
          z = z2 / zoomz;
          xi = (index_t)floor(x); xs = xi + 1;
          yi = (index_t)floor(y); ys = yi + 1;
          zi = (index_t)floor(z); zs = zi + 1;
          if ((xi >= 0) && (yi >= 0) && (zi >= 0) && (xs < rs2) && (ys < cs2) && (zs < ds2))
          {
            fziyi = (x - xi) * ptin[zi*ps + yi*rs + xs] + (xs - x) * ptin[zi*ps + yi*rs + xi];
            fziys = (x - xi) * ptin[zi*ps + ys*rs + xs] + (xs - x) * ptin[zi*ps + ys*rs + xi];
            fzsyi = (x - xi) * ptin[zs*ps + yi*rs + xs] + (xs - x) * ptin[zs*ps + yi*rs + xi];
            fzsys = (x - xi) * ptin[zs*ps + ys*rs + xs] + (xs - x) * ptin[zs*ps + ys*rs + xi];
            fzi = (y - yi) * fziys + (ys - y) * fziyi;
            fzs = (y - yi) * fzsys + (ys - y) * fzsyi;
            f = (z - zi) * fzs + (zs - z) * fzi;
            ptout[z2*ps2 + y2*rs2 + x2] = (float)f;
          }
        } // for x2
      } // for y2
    } // for z2
  } /* if (ds != 1) */

  return 1;
} /* lzoominfloat() */

/* ==================================== */
int32_t lzoom(
  struct xvimage * in,
  struct xvimage ** out,
  double zoomx,
  double zoomy,
  double zoomz)
/* ==================================== */
#undef F_NAME
#define F_NAME "lzoom"
{
  if ((zoomx <= 0.0) || (zoomy <= 0.0) || (zoomz <= 0.0))
  {  
    fprintf(stderr,"%s: bad zoom factor: must be > 0\n", F_NAME);
    return 0;
  }
  else if ((zoomx >= 1.0) && (zoomy >= 1.0) && (zoomz >= 1.0)) 
  {
    if (datatype(in) == VFF_TYP_1_BYTE)
      return lzoominbyte(in, out, zoomx, zoomy, zoomz);
    else if (datatype(in) == VFF_TYP_4_BYTE)
      return lzoominlong(in, out, zoomx, zoomy, zoomz);
    else if (datatype(in) == VFF_TYP_FLOAT)
      return lzoominfloat(in, out, zoomx, zoomy, zoomz);
    else
    {  
      fprintf(stderr,"%s : bad data type\n", F_NAME);
      return 0;
    }
  }
  else if ((zoomx <= 1.0) && (zoomy <= 1.0) && (zoomz <= 1.0)) 
  {
    if (datatype(in) == VFF_TYP_1_BYTE)
      return lzoomoutbyte(in, out, zoomx, zoomy, zoomz);
    else if (datatype(in) == VFF_TYP_4_BYTE)
      return lzoomoutlong(in, out, zoomx, zoomy, zoomz);
    else if (datatype(in) == VFF_TYP_FLOAT)
      return lzoomoutfloat(in, out, zoomx, zoomy, zoomz);
    else
    {  
      fprintf(stderr,"%s : bad data type\n", F_NAME);
      return 0;
    }
  }
  else
  {  
    fprintf(stderr,"%s : bad zoom factor : they must be all >= 1 or all <= 1\n", F_NAME);
    return 0;
  }
} /* lzoom() */

/* ==================================== */
int32_t lzoom2(
  struct xvimage * in,
  struct xvimage ** out,
  int32_t newdim, 
  char dim)
/* ==================================== */
#undef F_NAME
#define F_NAME "lzoom2"
{
  double zoom;

  if (dim == 'x') zoom = (double)newdim / (double)rowsize(in);
  else if (dim == 'y') zoom = (double)newdim / (double)colsize(in);
  else if (dim == 'z') zoom = (double)newdim / (double)depth(in);

  return lzoom(in, out, zoom, zoom, zoom);
} // lzoom2()
