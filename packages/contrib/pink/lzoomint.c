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
/* zoom par sous- ou sur-echantillonage */
/* Michel Couprie - septembre 1996 Christophe Doublier - mai 2002 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcutil.h>
#include <lzoomint.h>

/* ==================================== */
int32_t lzoomint(
  struct xvimage * in,
  struct xvimage ** out,
  int32_t z,
  int32_t fill) /* booleen : remplit l'espace inter-pixels */
/* ==================================== */
{
  int32_t xx, yy, zz;
  int32_t i, j, k;
  uint8_t *ptin;
  uint8_t *ptout;
  int32_t rs, cs, ds, ps, N, rsz, csz, psz;

  rs = in->row_size;
  cs = in->col_size;
  ds = depth(in);
  ps = rs * cs;
  N = ps * ds;
  ptin = UCHARDATA(in);

  if (z == 0)
  {   fprintf(stderr,"lzoomint() : bad zoom factor : 0\n");
      return 0;
  }

  if (ds == 1)
  {
    if (z > 0)
    {
      *out = allocimage(in->name, rs * z, cs * z, ds, VFF_TYP_1_BYTE);
    }
    else
    {
      *out = allocimage(in->name, rs / (-z), cs / (-z), ds, VFF_TYP_1_BYTE);
    }

    if (*out == NULL)
    {   fprintf(stderr,"lzoomint() : allocimage failed\n");
        return 0;
    }
    ptout = (UCHARDATA(*out));

    if (z > 0)
    {
      if (fill)  /* pas efficace - a ameliorer */
        for (yy = 0; yy < cs; yy++)
          for (j = 0; j < z; j++)
            for (xx = 0; xx < rs; xx++)
              for (i = 0; i < z; i++)
              {
                ptout[(yy*z+j)*rs*z + xx*z+i] = ptin[zz*ps + yy*rs + xx];
 	      }
      else
        for (yy = 0; yy < cs; yy++)
          for (xx = 0; xx < rs; xx++)
            ptout[(yy*z)*rs*z + xx*z] = ptin[zz*ps + yy*rs + xx];
    }
    else
      for (yy = 0; yy < cs; yy++)
        for (xx = 0; xx < rs; xx++)
          if (((zz % (-z)) == 0) && ((yy % (-z)) == 0) && ((xx % (-z)) == 0)) 
            ptout[(yy/(-z))*(rs/(-z)) + (xx/(-z))] = ptin[zz*ps + yy*rs + xx];

  }
  else /* ds != 1 */
  {
    if (z > 0)
    {
      *out = allocimage(in->name, rs * z, cs * z, ds * z, VFF_TYP_1_BYTE);
      rsz = rs * z;
      csz = cs * z;
      psz = rsz * csz;
    }
    else
    {
      *out = allocimage(in->name, rs / (-z), cs / (-z), ds / (-z), VFF_TYP_1_BYTE);
      rsz = rs / (-z);
      csz = cs / (-z);
      psz = rsz * csz;
    }

    if (*out == NULL)
    {   fprintf(stderr,"lzoomint() : allocimage failed\n");
        return 0;
    }
    ptout = (UCHARDATA(*out));

    if (z > 0)
    {
      if (fill)  /* pas efficace - a ameliorer */
        for (zz = 0; zz < ds; zz++)
          for (k = 0; k < z; k++)
            for (yy = 0; yy < cs; yy++)
              for (j = 0; j < z; j++)
                for (xx = 0; xx < rs; xx++)
                  for (i = 0; i < z; i++)
	          {
                    ptout[(zz*z+k)*psz + (yy*z+j)*rsz + xx*z+i] = ptin[zz*ps + yy*rs + xx];
	          }
      else
        for (zz = 0; zz < ds; zz++)
          for (yy = 0; yy < cs; yy++)
            for (xx = 0; xx < rs; xx++)
              ptout[(zz*z)*psz + (yy*z)*rsz + xx*z] = ptin[zz*ps + yy*rs + xx];
    }
    else
      for (zz = 0; zz < ds; zz++)
        for (yy = 0; yy < cs; yy++)
          for (xx = 0; xx < rs; xx++)
            if (((zz % (-z)) == 0) && ((yy % (-z)) == 0) && ((xx % (-z)) == 0)) 
              ptout[(zz/(-z))*(psz) + (yy/(-z))*(rsz) + (xx/(-z))] = ptin[zz*ps + yy*rs + xx];
  }
  return 1;
}

/* ==================================== */
struct xvimage* lzoomintxyz(
  struct xvimage * in,
  int32_t zoomx,
  int32_t zoomy,
  int32_t zoomz,
  int32_t fill) /* booleen : remplit l'espace inter-pixels */
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lzoomintxyz"
  int32_t xx, yy, zz;
  int32_t i, j, k;
  uint8_t *ptin;
  uint8_t *ptout;
  int32_t rs, cs, ds, ps, N, rsz, csz, dsz, psz;
  struct xvimage * out;

  printf("%s: zoomx=%d, zoomy=%d, zoomz=%d, fill=%d\n", F_NAME, zoomx, zoomy, zoomz, fill);

  rs = in->row_size;
  cs = in->col_size;
  ds = depth(in);
  ps = rs * cs;
  N = ps * ds;
  ptin = UCHARDATA(in);

  if (zoomx == 0||zoomy ==0||zoomz==0)
  {   fprintf(stderr,"lzoomintxyz() : bad zoom factor : 0\n");
      return 0;
  }

  if (ds == 1)
  {
    if (zoomx > 0 && zoomy>0 && zoomz>0)
    {
       out = allocimage(in->name, rs * zoomx, cs * zoomy, ds , VFF_TYP_1_BYTE);
    }
    else if (zoomx < 0 && zoomy<0 && zoomz<0)
    {
       out = allocimage(in->name, rs / (-zoomx), cs / (-zoomy), ds , VFF_TYP_1_BYTE);
    }
    else 
      {
	fprintf (stderr,"erreur facteur de zoom non tous positif ou non tous négatif");
	return 0;
      }

    if (out == NULL)
    {   fprintf(stderr,"lzoomintxyz() : allocimage failed\n");
        return 0;
    }
    ptout = UCHARDATA(out);

    if (zoomx>0 && zoomy>0 && zoomz>0)
    {
      if (fill)  /* pas efficace - a ameliorer */
        for (yy = 0; yy < cs; yy++)
          for (j = 0; j < zoomy; j++)
            for (xx = 0; xx < rs; xx++)
              for (i = 0; i < zoomx; i++)
              {
                ptout[(yy*zoomy+j)*rs*zoomx + xx*zoomx+i] = ptin[yy*rs + xx];
 	      }
      else
        for (yy = 0; yy < cs; yy++)
          for (xx = 0; xx < rs; xx++)
            ptout[(yy*zoomy)*rs*zoomx + xx*zoomx] = ptin[yy*rs + xx];
    }
    else if (zoomx<0 && zoomy<0 && zoomz<0)
    {
      for (yy = 0; yy < (cs); yy++)
        for (xx = 0; xx < (rs); xx++)
	  if ( yy%zoomy==0 && xx%zoomx==0)
            ptout[(yy/(-zoomy))*(rs/(-zoomx)) + (xx/(-zoomx))] = ptin[ yy*rs + xx];
    }
    else 
    {
      fprintf(stderr,"lzoomintxyz() : facteur zoom non tous positifs ou non tous negatifs\n");
      return 0;
    }

  }
  else /* ds != 1 */
  {
    if (zoomx>0 && zoomy>0 && zoomz>0)
    {
      rsz = rs * zoomx;
      csz = cs * zoomy;
      dsz = ds * zoomz;
      psz = rsz * csz;
      out = allocimage(in->name, rsz, csz, dsz, VFF_TYP_1_BYTE);
    }
    else if (zoomx<0 && zoomy<0 && zoomz<0)
    {
      rsz = rs / (-zoomx);
      csz = cs / (-zoomy);
      dsz = ds / (-zoomz);
      psz = rsz * csz;
      out = allocimage(in->name, rsz, csz, dsz, VFF_TYP_1_BYTE);
    }
    else 
    {
      fprintf(stderr,"lzoomintxyz() : facteur zoom non tous positifs ou non tous negatifs\n");
      return 0;
    }

    if (out == NULL)
    {   fprintf(stderr,"lzoomintxyz() : allocimage failed\n");
        return 0;
    }
    ptout = UCHARDATA(out);

    if (zoomx > 0) //zoomy>0 et zoomz>0 aussi
    {
      if (fill)  /* pas efficace - a ameliorer */
        for (zz = 0; zz < ds; zz++)
          for (k = 0; k < zoomz; k++)
            for (yy = 0; yy < cs; yy++)
              for (j = 0; j < zoomy; j++)
                for (xx = 0; xx < rs; xx++)
                  for (i = 0; i < zoomx; i++)
	          {
                    ptout[(zz*zoomz+k)*psz + (yy*zoomy+j)*rsz + xx*zoomx+i] = ptin[zz*ps + yy*rs + xx];
	          }
      else
        for (zz = 0; zz < ds; zz++)
          for (yy = 0; yy < cs; yy++)
            for (xx = 0; xx < rs; xx++)
              ptout[(zz*zoomz)*psz + (yy*zoomy)*rsz + (xx*zoomx)] = ptin[zz*ps + yy*rs + xx];
    } 
    else // zoomx<0 et zoomy<0 et zoomz<0 
      for (zz = 0; zz < ds; zz++)
        for (yy = 0; yy < cs; yy++)
          for (xx = 0; xx < rs; xx++)
           if ( xx%zoomx==0 && yy%zoomy==0 && zz%zoomz==0 &&
		xx/(-zoomx) < rsz && yy/(-zoomy) < csz && zz/(-zoomz) < dsz
	      )
	   {
              ptout[(zz/(-zoomz))*psz + ((yy/(-zoomy)))*rsz + (xx/(-zoomx))] = ptin[zz*ps + yy*rs + xx];
	   }
  }
  return out;
}
