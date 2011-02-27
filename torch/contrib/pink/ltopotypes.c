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
/* calcul des types topologiques des points d' une image */
/* Michel Couprie - juillet 1996 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mctopo.h>
#include <mctopo3d.h>

/* ==================================== */
int32_t ltopotypes(
  struct xvimage * image,
  struct xvimage * types, 
  int32_t connex,
  int32_t bin)
/* ==================================== */
#undef F_NAME
#define F_NAME "ltopotypes"
{
  int32_t i;
  uint8_t *pti = UCHARDATA(image);
  uint8_t *ptt = UCHARDATA(types);
  int32_t rs = image->row_size;
  int32_t N = rs * image->col_size;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (bin)
  {
    if (connex == 4)
    {
      for (i = 0; i < N; i++)
      {
        ptt[i] = (uint8_t)typetopobin(pti, i, rs, N);
      }
    }
    else
    {
      for (i = 0; i < N; i++)
      {
        ptt[i] = (uint8_t)typetopobin8(pti, i, rs, N);
      }
    }
  }
  else
  {
    if (connex == 4)
    {
      for (i = 0; i < N; i++)
      {
        ptt[i] = (uint8_t)typetopo(pti, i, rs, N);
      }
    }
    else
    {
      for (i = 0; i < N; i++)
      {
        ptt[i] = (uint8_t)typetopo8(pti, i, rs, N);
      }
    }
  }
  return 1;
} /* ltopotypes() */

/* ==================================== */
int32_t lt4pp(struct xvimage * image)
/* ==================================== */
#undef F_NAME
#define F_NAME "lt4pp"
{
  int32_t i, j;
  struct xvimage * temp;
  uint8_t *pti;
  uint8_t *ptt;
  int32_t *pti_l;
  int32_t *ptt_l;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  temp = copyimage(image);
  if (temp == NULL) 
  {
    fprintf(stderr, "%s: copyimage failed\n", F_NAME);
    return(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    pti = UCHARDATA(image);
    ptt = UCHARDATA(temp);
    memset(pti, 0, N);
    for (j = 1; j < cs - 1; j++) 
      for (i = 1; i < rs - 1; i++) 
        pti[j * rs + i] = (uint8_t)t4pp(ptt, j * rs + i, rs, N);
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    pti_l = SLONGDATA(image);
    ptt_l = SLONGDATA(temp);
    memset(pti_l, 0, 4*N);
    for (j = 1; j < cs - 1; j++) 
      for (i = 1; i < rs - 1; i++) 
        pti_l[j * rs + i] = (int32_t)t4pp_l(ptt_l, j * rs + i, rs, N);
  }
  else
  {
    fprintf(stderr, "%s: bad data type: %d\n", F_NAME, datatype(image));
    return(0);
  }

  freeimage(temp);
  return 1;
} /* lt4pp() */

/* ==================================== */
int32_t lt8pp(struct xvimage * image)
/* ==================================== */
#undef F_NAME
#define F_NAME "lt8pp"
{
  int32_t i, j;
  struct xvimage * temp;
  uint8_t *pti;
  uint8_t *ptt;
  int32_t *pti_l;
  int32_t *ptt_l;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  temp = copyimage(image);
  if (temp == NULL) 
  {
    fprintf(stderr, "%s: copyimage failed\n", F_NAME);
    return(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    pti = UCHARDATA(image);
    ptt = UCHARDATA(temp);
    memset(pti, 0, N);
    for (j = 1; j < cs - 1; j++) 
      for (i = 1; i < rs - 1; i++) 
        pti[j * rs + i] = (uint8_t)t8pp(ptt, j * rs + i, rs, N);
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    pti_l = SLONGDATA(image);
    ptt_l = SLONGDATA(temp);
    memset(pti_l, 0, 4*N);
    for (j = 1; j < cs - 1; j++) 
      for (i = 1; i < rs - 1; i++) 
        pti_l[j * rs + i] = (int32_t)t8pp_l(ptt_l, j * rs + i, rs, N);
  }
  else
  {
    fprintf(stderr, "%s: bad data type: %d\n", F_NAME, datatype(image));
    return(0);
  }

  freeimage(temp);
  return 1;
} /* lt8pp() */

/* ==================================== */
int32_t lt6pp(struct xvimage * image)
/* ==================================== */
#undef F_NAME
#define F_NAME "lt6pp"
{
  int32_t i, j, k;
  struct xvimage * temp;
  uint8_t *pti;
  uint8_t *ptt;
  int32_t *pti_l;
  int32_t *ptt_l;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t ds = depth(image);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;

  temp = copyimage(image);
  if (temp == NULL) 
  {
    fprintf(stderr, "%s: copyimage failed\n", F_NAME);
    return(0);
  }

  mctopo3d_init_topo3d();

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    pti = UCHARDATA(image);
    ptt = UCHARDATA(temp);
    memset(pti, 0, N);
    for (k = 1; k < ds - 1; k++) 
      for (j = 1; j < cs - 1; j++) 
        for (i = 1; i < rs - 1; i++) 
          pti[k*ps + j*rs + i] = (uint8_t)mctopo3d_t6pp(ptt, k*ps + j*rs + i, rs, ps, N);
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    pti_l = SLONGDATA(image);
    ptt_l = SLONGDATA(temp);
    memset(pti_l, 0, 4*N);
    for (k = 1; k < ds - 1; k++) 
      for (j = 1; j < cs - 1; j++) 
        for (i = 1; i < rs - 1; i++) 
          pti_l[k*ps + j*rs + i] = (int32_t)mctopo3d_t6pp_l(ptt_l, k*ps + j*rs + i, rs, ps, N);
  }
  else
  {
    fprintf(stderr, "%s: bad data type: %d\n", F_NAME, datatype(image));
    return(0);
  }

  mctopo3d_termine_topo3d();
  freeimage(temp);
  return 1;
} /* lt6pp() */

/* ==================================== */
int32_t ltopotypes_t26pp(struct xvimage * image)
/* ==================================== */
#undef F_NAME
#define F_NAME "ltopotypes_t26pp"
{
  int32_t i, j, k;
  struct xvimage * temp;
  uint8_t *pti;
  uint8_t *ptt;
  int32_t *pti_l;
  int32_t *ptt_l;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t ds = depth(image);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;

  temp = copyimage(image);
  if (temp == NULL) 
  {
    fprintf(stderr, "%s: copyimage failed\n", F_NAME);
    return(0);
  }

  mctopo3d_init_topo3d();

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    pti = UCHARDATA(image);
    ptt = UCHARDATA(temp);
    memset(pti, 0, N);
    for (k = 1; k < ds - 1; k++) 
      for (j = 1; j < cs - 1; j++) 
        for (i = 1; i < rs - 1; i++) 
          pti[k*ps + j*rs + i] = (uint8_t)mctopo3d_t26pp(ptt, k*ps + j*rs + i, rs, ps, N);
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    pti_l = SLONGDATA(image);
    ptt_l = SLONGDATA(temp);
    memset(pti_l, 0, 4*N);
    for (k = 1; k < ds - 1; k++) 
      for (j = 1; j < cs - 1; j++) 
        for (i = 1; i < rs - 1; i++) 
          pti_l[k*ps + j*rs + i] = (int32_t)mctopo3d_t26pp_l(ptt_l, k*ps + j*rs + i, rs, ps, N);
  }
  else
  {
    fprintf(stderr, "%s: bad data type: %d\n", F_NAME, datatype(image));
    return(0);
  }

  mctopo3d_termine_topo3d();
  freeimage(temp);
  return 1;
} /* ltopotypes_t26pp() */

/* ==================================== */
int32_t lalpha(struct xvimage * image, int32_t connex, char sign)
/* ==================================== */
#undef F_NAME
#define F_NAME "lalpha"
{
  int32_t k;
  struct xvimage * temp;
  uint8_t *pti;
  uint8_t *ptt;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t ds = depth(image);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;

  temp = copyimage(image);
  if (temp == NULL) 
  {
    fprintf(stderr, "%s: copyimage failed\n", F_NAME);
    return(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    pti = UCHARDATA(image);
    ptt = UCHARDATA(temp);
    memset(pti, 0, N);

    if (sign == 'm')
    {
      if (connex == 26)
	for (k = 0; k < N; k++) 
	  pti[k] = (uint8_t)mctopo3d_alpha26m(ptt, k, rs, ps, N);
      else
      if (connex == 6)
	for (k = 0; k < N; k++) 
	  pti[k] = (uint8_t)mctopo3d_alpha6m(ptt, k, rs, ps, N);
      else
      if (connex == 8)
	for (k = 0; k < N; k++) 
	  pti[k] = (uint8_t)alpha8m(ptt, k, rs, N);
      else
      if (connex == 4)
	for (k = 0; k < N; k++) 
	  pti[k] = (uint8_t)alpha4m(ptt, k, rs, N);
      else
      {
	fprintf(stderr, "%s: bad connexity: %d\n", F_NAME, connex);
	return(0);
      }
    }
    else
    if (sign == 'p')
    {
      if (connex == 26)
	for (k = 0; k < N; k++) 
	  pti[k] = (uint8_t)mctopo3d_alpha26p(ptt, k, rs, ps, N);
      else
      if (connex == 6)
	for (k = 0; k < N; k++) 
	  pti[k] = (uint8_t)mctopo3d_alpha6p(ptt, k, rs, ps, N);
      else
      if (connex == 8)
	for (k = 0; k < N; k++) 
	  pti[k] = (uint8_t)alpha8p(ptt, k, rs, N);
      else
      if (connex == 4)
	for (k = 0; k < N; k++) 
	  pti[k] = (uint8_t)alpha4p(ptt, k, rs, N);
      else
      {
	fprintf(stderr, "%s: bad connexity: %d\n", F_NAME, connex);
	return(0);
      }
    }
    else
    {
      fprintf(stderr, "%s: bad sign: %c\n", F_NAME, sign);
      return(0);
    }
  }
  else
  {
    fprintf(stderr, "%s: bad data type: %d\n", F_NAME, datatype(image));
    return(0);
  }

  freeimage(temp);
  return 1;
} /* lalpha() */
