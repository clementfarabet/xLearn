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
/* operateurs de seuillage */
/* Michel Couprie - juillet 1996 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <llabelextrema.h>


/* ==================================== */
int32_t lminima(
        struct xvimage *image,
        char* str_connexity
  )
/* ==================================== */
{
  int32_t nblabels, connex, i;
//  struct xvimage * image;
  struct xvimage * result;
  uint8_t * I;
  int32_t * IL;
  int32_t N;
  int32_t * R;
  
  N = rowsize(image) * colsize(image) * depth(image);

  if (str_connexity[0] == 'b') {
    connex = atoi(str_connexity+1);
  } else 
    connex = atoi(str_connexity);

  result = allocimage(NULL, rowsize(image), colsize(image), depth(image), VFF_TYP_4_BYTE);
  if (result == NULL)
  {   
    fprintf(stderr, "allocimage failed\n");
    exit(1);
  }
  R = SLONGDATA(result);

  if ((connex == 0) && strcmp(str_connexity, "b0") && strcmp(str_connexity, "b1"))
  {
    if (datatype(image) == VFF_TYP_1_BYTE)
    {   
      uint8_t absmin;
      I = UCHARDATA(image);
      absmin = I[0];
      for (i = 1; i < N; i++) if (I[i] < absmin) absmin = I[i];
      for (i = 0; i < N; i++) if (I[i] == absmin) I[i] = NDG_MAX; else I[i] = NDG_MIN;
    }
    else if (datatype(image) == VFF_TYP_4_BYTE) 
    {   
      int32_t absmin;
      IL = SLONGDATA(image);
      absmin = IL[0];
      for (i = 1; i < N; i++) if (IL[i] < absmin) absmin = IL[i];
      for (i = 0; i < N; i++) if (IL[i] == absmin) IL[i] = (int32_t)NDG_MAX; 
                                             else IL[i] = (int32_t)NDG_MIN;
    }
    else
    {
      fprintf(stderr, "wrong image type\n");
      exit(1);
    }
  }
  else
  {
    if (! llabelextrema(image, connex, LABMIN, result, &nblabels))
    {
      fprintf(stderr, "llabelextrema failed\n");
      exit(1);
    }

#ifdef VERBOSE
    printf("NOMBRE DE MINIMA : %d\n", nblabels-1);
#endif

    if (datatype(image) == VFF_TYP_1_BYTE)
    {   
      I = UCHARDATA(image);
      for (i = 0; i < N; i++)
        if (R[i]) I[i] = NDG_MAX; else I[i] = NDG_MIN;
    }
    else if (datatype(image) == VFF_TYP_4_BYTE) 
    {   
      IL = SLONGDATA(image);
      for (i = 0; i < N; i++)
        if (R[i]) IL[i] = (int32_t)NDG_MAX; else IL[i] = (int32_t)NDG_MIN;
    }
  }
  freeimage(result);

  return 1;
} /* lminima */
