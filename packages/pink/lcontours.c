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
/* Extraction de contours */
/* Michel Couprie -  avril 1998 */
/*
   Methode : l'image d'entree F est un noyau homotopique par abaissement, obtenu typiquement a partir
   d'une image de gradient.

   Cette image est seuillee (seuilhaut), ce qui donne l'image binaire S. 

   Dans S on elimine les points isoles, 
     et on repere les points extremites, qui sont empiles dans une LIFO.

   Pour chaque point extremite x : 
     selectionne un point y dans ((vois. de x) - S) qui maximise F[y]
     rajoute y a S
     si y extremite, empile y dans LIFO
*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfah.h>
#include <mctopo.h>
#include <mcindic.h>
#include <llabelextrema.h>
#include <lcontours.h>

#define PARANO
/*
#define DEBUG
*/

/* ==================================== */
int32_t lcontours(struct xvimage *image, int32_t seuilhaut, int32_t seuilbas)
/* ==================================== */
{ 
  int32_t i;
  int32_t n;
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t z;                       /* index muet (generalement un voisin de y) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *S;
  Fah * FAH;
  int32_t t, tb;
  int32_t prem;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lcontours: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  FAH = CreeFahVide(N);

  S = (uint8_t *)calloc(1,N * sizeof(char));
  if (S == NULL)
  {
    fprintf(stderr, "lcontours: malloc failed\n");
    return 0;
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /* seuil de l'image originale */
  /* ========================================================= */

  for (x = 0; x < N; x++) 
    if (F[x] >= seuilhaut) S[x] = NDG_MAX; else S[x] = 0;

  /* ========================================================= */
  /* selection des points extremite et elimination des points isoles */
  /* ========================================================= */

  for (x = 0; x < N; x++) 
    if (S[x]) 
    {
      n = nbvois8(S, x, rs, N);
      if (n == 1) FahPush(FAH, x, NDG_MAX - F[x]); 
      else if (n == 0) S[x] = 0;
    }

  /* ========================================================= */
  /* traitement des points extremite */
  /* ========================================================= */

  while (!FahVide(FAH))
  {
    x = FahPop(FAH);
    prem = 1; z = x;
#ifdef DEBUG
printf("POP x = %d,%d\n", x%rs, x/rs);
#endif
    for (k = 1; k < 8; k += 1)
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (!S[y]) && (!bord(y, rs, N)))
      {
        top8(S, y, rs, N, &t, &tb); 
        n = nbvois8(S, y, rs, N); 
#ifdef DEBUG
printf("y = %d,%d ; t = %d ; n = %d ; F[y] = %d\n", y%rs, y/rs, t, n, F[y]);
#endif
        if ((n == 1) || ((n > 1) && (t > 1)))
        {
          if (prem) { z = y; prem = 0; }
          else if (F[y] > F[z]) z = y;
        }
      }
    } /* for k */
    
    if ((z != x) && (F[z] >= seuilbas))
    {
      S[z] = NDG_MAX;
      if (extremite8(S, z, rs, N)) FahPush(FAH, z, NDG_MAX - F[z]); 
    }
  } /* for x */

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  for (x = 0; x < N; x++) F[x] = S[x];

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  FahTermine(FAH);
  return(1);
}
/* -library_code_end */
