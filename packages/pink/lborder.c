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
/* frontiere d'une image binaire */
/*
   la frontiere de F(X) de X est definie par:
   F(X) = {x de X, Gamma_n(x) inter Xbar non vide}
   ou n est la connexite du complémentaire de l'objet
*/
/* Michel Couprie -  avril 1999 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mctopo.h>
#include <mctopo3d.h>
#include <lborder.h>

/* ==================================== */
int32_t mctopo3d_lborder(struct xvimage *image, int32_t connex)
/* ==================================== */
{
  int32_t x;                       /* index muet de pixel */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ds = depth(image);       /* nb plans */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t N = ps * ds;             /* taille image */
  uint8_t *I = UCHARDATA(image);     /* l'image de depart */
  int32_t *IL = SLONGDATA(image);    /* l'image de depart si 32 bits */
  struct xvimage *tmp;
  uint8_t *T;

  switch(datatype(image))
  {
    case VFF_TYP_1_BYTE:
      tmp = copyimage(image);
      if (tmp == NULL)
      {
        fprintf(stderr, "mctopo3d_lborder: copyimage failed\n");
        return 0;
      }
      T = UCHARDATA(tmp);
      razimage(image);
      switch (connex)
      {
        case 4: 
          for (x = 0; x < N; x++)
            if (T[x] && (nbvoisc8(T, x, rs, N)) != 0) I[x] = NDG_MAX;
          break;
        case 8: 
          for (x = 0; x < N; x++)
            if (T[x] && (nbvoisc4(T, x, rs, N)) != 0) I[x] = NDG_MAX;
          break;
        case 6: 
          for (x = 0; x < N; x++)
            if (T[x] && nonbord3d(x,rs,ps,N) && (mctopo3d_nbvoisc26(T,x,rs,ps,N)) != 0) I[x] = NDG_MAX;
          break;
        case 26: 
          for (x = 0; x < N; x++)
            if (T[x] && nonbord3d(x,rs,ps,N) && (mctopo3d_nbvoisc6(T,x,rs,ps,N)) != 0) I[x] = NDG_MAX;
          break;
        default: 
          fprintf(stderr, "mctopo3d_lborder: mauvaise connexite: %d\n", connex);
          return 0;
      } /* switch (connex) */  
      break;
    case VFF_TYP_4_BYTE:
      tmp = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
      if (tmp == NULL)
      {
        fprintf(stderr, "mctopo3d_lborder: allocimage failed\n");
        return 0;
      }
      T = UCHARDATA(tmp);
      razimage(tmp);
      switch (connex)
      {
        case 4: 
          for (x = 0; x < N; x++)
            if (nbvois8neq(IL,x,rs,N) != 0) T[x] = NDG_MAX;
          break;
        case 8: 
          for (x = 0; x < N; x++)
            if (nbvois4neq(IL,x,rs,N) != 0) T[x] = NDG_MAX;
          break;
        default: 
          fprintf(stderr, "mctopo3d_lborder: mauvaise connexite: %d\n", connex);
          return 0;
      } /* switch (connex) */
      for (x = 0; x < N; x++) IL[x] = (int32_t) T[x];
      break;
    default:
      fprintf(stderr,"mctopo3d_lborder() : bad data type %d\n", datatype(image));
      return 0;
  } /* switch(datatype(image)) */

  freeimage(tmp);
  return(1);
}
