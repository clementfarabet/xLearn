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
/* operateur: compression d'histogramme */
/* Michel Couprie - septembre 1996 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <lhisto.h>
#include <lcomphisto.h>

/* ==================================== */
int32_t lcomphisto(struct xvimage *image, int32_t n)
/* n: nombre de niveaux de gris desires */
/* ==================================== */
#undef F_NAME
#define F_NAME "lcomphisto"
{
  int32_t i;
  int32_t j;
  int32_t k;
  index_t x;
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t d = depth(image);        /* nb plans */
  index_t N = rs * cs * d;         /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */
  index_t * histo;
  uint8_t * tabcor;
  int32_t nndg;
  double p, np;

  histo = (index_t *)calloc(1,(NDG_MAX - NDG_MIN + 1) * sizeof(index_t));
  if (histo == NULL)
  {
    fprintf(stderr, "%s: malloc failed for histo\n", F_NAME);
    return(0);
  }

  tabcor = (uint8_t *)calloc(NDG_MAX - NDG_MIN + 1,  sizeof(char));
  if (tabcor == NULL)
  {
    fprintf(stderr, "%s: calloc failed for tabcor\n", F_NAME);
    return(0);
  }

  if (! lhisto1(image, histo))
  {
    fprintf(stderr, "%s: function lhisto failed\n", F_NAME);
    return(0);
  }  

  nndg = 0;
  for (i = NDG_MIN; i <= NDG_MAX; i++) 
    if (histo[i])
      nndg++;

  if (n > nndg)
  {
    fprintf(stderr, "%s: required histogram larger than actual one\n", F_NAME);
    return(0);
  }  

  p = (double)nndg / (double)n;      
  np = 0;

  i = NDG_MIN;   /* niveau de gris */
  k = 0;         /* nombre de niveaux non nuls deja traites */
  while (i <= NDG_MAX)
  {
    while ((i <= NDG_MAX) && (histo[i] == 0)) i++; /* saute les niveaux nuls */
    if ((double)k >= np) { j = i; np += p; }
    tabcor[i] = j;
    i++;
    k++;
  }

  free(histo);

  for (x = 0; x < N; x++) SOURCE[x] = tabcor[SOURCE[x]];
  return(1);
}
/* -library_code_end */
