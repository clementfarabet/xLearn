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
/* operateur de detection de bruit de type "salt and peper"
   avec correction par moyennage local */
/* d'apres Kak et Rosenfeld */
/* Michel Couprie - janvier 1997 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcutil.h>

/* ==================================== */
int32_t lremspnoise(
        struct xvimage *image,
        int32_t g,
        int32_t k)
/* g : nombre de niveaux d'ecart */
/* k : nombre de voisins consideres */
/* ==================================== */
{
  int32_t i, v, n, j, m, nv;
  uint8_t *pt;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;
  uint8_t *imagetmp;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lhtkern: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  pt = UCHARDATA(image);

  imagetmp = (uint8_t *)calloc(1,N*sizeof(char));
  if (imagetmp == NULL)
  {   printf("lremspnoise() : malloc failed\n");
      return(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  for (i = 0; i < N; i++)
  {
    /* detection du bruit : compte le nombre de voisins qui 
       different de plus de g niveaux
    */
    n = 0;
    nv = 0;
    m = 0;
    for (v = 0; v < 8; v++)  /* 8-voisins */
      if ((j = voisin(i, v, rs, N)) != -1)
      {
        if (mcabs(((int32_t)pt[i] - (int32_t)pt[j]) >= g)) n++;
        nv++;        /* compte les voisins */
        m += pt[j];  /* pour le calcul de la moyenne */
      }
    
    if (n >= k)
    {
      /* moyennage : remplace la valeur du point par la moyenne de ses
         8 voisins 
      */
      imagetmp[i] = (uint8_t)(m / nv);
    }
    else
      imagetmp[i] = pt[i];
  }

  for (i = 0; i < N; i++)
    pt[i] = imagetmp[i];

  free(imagetmp);
  return 1;
}
