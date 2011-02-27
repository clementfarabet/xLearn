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
/* introduit un gradient d'illumination */
/* valeurs normales en haut - multipliees par un facteur alpha <= 1 en bas */
/* Michel Couprie - fevrier 1997 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mccodimage.h>

/* ==================================== */
int32_t lgradill(
  struct xvimage * image1,
  double alpha)
/* ==================================== */
{
  int32_t i;
  uint8_t *pt1;
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;
  
  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  pt1 = UCHARDATA(image1);
  for (i = 0; i < N; i++)
  {
    pt1[i] = (uint8_t)(pt1[i] * (1.0 - (alpha * (i/rs) / (double)(N/rs))));
  }
  return 1;
}





