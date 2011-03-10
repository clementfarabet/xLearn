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
/* operateur: egalisation d'histogramme */
/*

*/
/* Michel Couprie - septembre 1996 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcfah.h>
#include <legalise.h>

/* ==================================== */
int32_t legalise(struct xvimage *image, int32_t n)
/* n : nombre de niveaux de gris desires */
/* ==================================== */
{
  int32_t i;
  int32_t k;
  int32_t x;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t d = depth(image);        /* nb plans */
  int32_t N = d * rs * cs;         /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */
  Fah * FAH;                   /* la file d'attente hierarchique */
  double p, np;

  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "legalise() : CreeFahVide failed\n");
      return(0);
  }

  FahPush(FAH, -1, 0);               /* force la creation du niveau 0 dans la Fah. */
  for (x = 0; x < N; x++) FahPush(FAH, x, SOURCE[x]);
  (void)FahPop(FAH);

  p = (double)N / (double)n;   /* nombre de points par niveau */
  np = p;
  i = 0;                       /* nombre de points traites */
  k = 0;                       /* niveau courant */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    SOURCE[x] = k;
    i++;
    if ((double)i >= np) { np += p; k++; }
  } /* while (! FahVide(FAH)) */

  FahTermine(FAH);
  return(1);
}
/* -library_code_end */
