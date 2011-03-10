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
#include <stdlib.h>
#include <mcutil.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <lseuil.h>

/* ==================================== */
int32_t lseuil(
        struct xvimage *f,
        double seuil)
/* ==================================== */
/* tous les pixels < seuil sont mis a 0, les autres a 255 */
{
  index_t x;                       /* index muet de pixel */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t ds = depth(f);           /* nb. plans */
  index_t N = rs * cs * ds;        /* taille image */

  if (datatype(f) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(f);
    for (x = 0; x < N; x++) if (F[x] < seuil) F[x] = NDG_MIN; else F[x] = NDG_MAX;  
  }
  else if (datatype(f) == VFF_TYP_4_BYTE)
  {
    int32_t *FL = SLONGDATA(f);
    for (x = 0; x < N; x++) if (FL[x] < seuil) FL[x] = NDG_MIN; else FL[x] = NDG_MAX;  
  }
  else if (datatype(f) == VFF_TYP_FLOAT)
  {
    float *FF = FLOATDATA(f);
    for (x = 0; x < N; x++) if (FF[x] < seuil) FF[x] = 0.0; else FF[x] = 1.0;  
  }
  else
  {   fprintf(stderr,"lseuil() : bad datatype : %d\n", datatype(f));
      return 0;
  }
  return 1;
}

/* ==================================== */
int32_t lseuil2(
        struct xvimage *f,
        uint8_t seuilmin,
        uint8_t seuilmax,
        uint8_t valmin,
        uint8_t valmax)
/* ==================================== */
/* tous les pixels < seuilmin sont mis a valmin */
/* tous les pixels >= seuilmax sont mis a valmax */
{
  index_t x;                       /* index muet de pixel */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t d = depth(f);            /* nb. plans */
  index_t N = rs * cs * d;         /* taille image */

  if (datatype(f) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(f);
    for (x = 0; x < N; x++)
      if ((int32_t)(F[x]) < (int32_t)seuilmin) F[x] = valmin;  
      else if ((int32_t)(F[x]) >= (int32_t)seuilmax) F[x] = valmax;
  }
  else if (datatype(f) == VFF_TYP_4_BYTE)
  {
    int32_t *F = SLONGDATA(f);
    for (x = 0; x < N; x++)
      if ((int32_t)(F[x]) < (int32_t)seuilmin) F[x] = (int32_t)valmin;  
      else if ((int32_t)(F[x]) >= (int32_t)seuilmax) F[x] = (int32_t)valmax;
  }
  else if (datatype(f) == VFF_TYP_FLOAT)
  {
    float *F = FLOATDATA(f);
    for (x = 0; x < N; x++)
      if ((int32_t)(F[x]) < (int32_t)seuilmin) F[x] = (float)valmin;  
      else if ((int32_t)(F[x]) >= (int32_t)seuilmax) F[x] = (float)valmax;
  }
  else
  {   fprintf(stderr,"lseuil() : bad datatype : %d\n", datatype(f));
      return 0;
  }

  return 1;
}


/* ==================================== */
int32_t lseuil3(
        struct xvimage *f,
        double seuil, double seuil2)
/* ==================================== */
/* tous les seuil <= pixels < seuil2 sont mis a 255, les autres a 0 */
{
  index_t x;                       /* index muet de pixel */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t ds = depth(f);           /* nb. plans */
  index_t N = rs * cs * ds;        /* taille image */

  if (datatype(f) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(f);
    for (x = 0; x < N; x++) 
      if ((F[x] < seuil2) && (F[x]>=seuil))
	F[x] = NDG_MAX; 
      else F[x] = NDG_MIN;  
  }
  else if (datatype(f) == VFF_TYP_4_BYTE)
  {
    int32_t *FL = SLONGDATA(f);
    for (x = 0; x < N; x++) 
      if ((FL[x] < seuil2) && (FL[x]>=seuil))
	FL[x] = NDG_MAX; 
      else FL[x] = NDG_MIN;  
  }
  else if (datatype(f) == VFF_TYP_FLOAT)
  {
    float *FF = FLOATDATA(f);
    for (x = 0; x < N; x++) 
      if ((FF[x] < seuil2) && (FF[x]>=seuil))
	FF[x] = 1.0; 
      else FF[x] = 0.0;  
  }
  else
  {   fprintf(stderr,"lseuil() : bad datatype : %d\n", datatype(f));
      return 0;
  }
  return 1;
}
