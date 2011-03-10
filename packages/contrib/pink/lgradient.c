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
/* gradient */
/* Michel Couprie - janvier 2011 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mctopo.h>

/* ==================================== */
int32_t lgradinf(struct xvimage *image)
/* ==================================== */
#undef F_NAME
#define F_NAME "lgradinf"
{
  int32_t i;
  uint8_t *pt;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;
  uint8_t *imagetmp;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  imagetmp = (uint8_t *)calloc(1,N*sizeof(char));
  if (imagetmp == NULL)
    {   fprintf(stderr, "%s: calloc failed\n", F_NAME);
      return(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  pt = UCHARDATA(image);
  for (i = 0; i < N; i++)
    imagetmp[i] = pt[i] - beta8m(pt, i, rs, N);

  for (i = 0; i < N; i++)
    pt[i] = imagetmp[i];

  free(imagetmp);
  return 1;
} // lgradinf()

/* ==================================== */
int32_t lgradinf3d(struct xvimage *image)
/* ==================================== */
#undef F_NAME
#define F_NAME "lgradinf3d"
{
  perror("FARABET: this function is left out because beta26m not defined anywhere\n");
  /*
  int32_t i;
  uint8_t *pt;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t d = depth(image);
  int32_t n = rs * cs;
  int32_t N = d * n;
  uint8_t *imagetmp;

  imagetmp = (uint8_t *)calloc(1,N*sizeof(char));
  if (imagetmp == NULL)
    {   fprintf(stderr, "%s: calloc failed\n", F_NAME);
      return(0);
  }
  */
  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  /*
  pt = UCHARDATA(image);
  for (i = 0; i < N; i++)
    imagetmp[i] = pt[i] - beta26m(pt, i, rs, n, N);

  for (i = 0; i < N; i++)
    pt[i] = imagetmp[i];

  free(imagetmp);
  */
  return 1;
} // lgradinf3d()

/* ==================================== */
int32_t lgradsup(
        struct xvimage *image)
/* ==================================== */
#undef F_NAME
#define F_NAME "lgradsup"
{
  int32_t i;
  uint8_t *pt;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;
  uint8_t *imagetmp;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  imagetmp = (uint8_t *)calloc(1,N*sizeof(char));
  if (imagetmp == NULL)
    {   fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  pt = UCHARDATA(image);
  for (i = 0; i < N; i++)
    imagetmp[i] = beta8p(pt, i, rs, N) -  pt[i];

  for (i = 0; i < N; i++)
    pt[i] = imagetmp[i];

  free(imagetmp);
  return 1;
} // lgradsup()

/* ==================================== */
int32_t lsobel(
        struct xvimage *image)
/* ==================================== */
#undef F_NAME
#define F_NAME "lsobel"
{
  int32_t i;
  uint8_t *pt;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;
  uint8_t *imagetmp;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  imagetmp = (uint8_t *)calloc(1,N*sizeof(char));
  if (imagetmp == NULL)
    {   fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  pt = UCHARDATA(image);
  for (i = 0; i < N; i++)
    imagetmp[i] = beta8p(pt, i, rs, N) -  pt[i];

  for (i = 0; i < N; i++)
    pt[i] = imagetmp[i];

  free(imagetmp);
  return 1;
} // lsobel()
